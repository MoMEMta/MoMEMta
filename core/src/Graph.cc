#include <Graph.h>

#include <boost/graph/graphviz.hpp>
#include <boost/graph/topological_sort.hpp>

#include <momemta/Logging.h>
#include <momemta/Module.h>
#include <momemta/ParameterSet.h>
#include <momemta/Path.h>

namespace graph {

class unresolved_input: public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class incomplete_looper_path: public std::runtime_error {
    using std::runtime_error::runtime_error;
};

Graph build(const Pool::DescriptionMap& description, std::vector<ModulePtr>& modules, const std::vector<PathElementsPtr>& paths, std::function<void(const std::string&)> on_module_removed) {

    Graph g;

    uint32_t id = 0;
    std::unordered_map<std::string, vertex_t> vertices;

    // Create vertices. Each vertex is a module
    // If a module is part of a Path, then it's not added to the main graph, but
    // into a subgraph.
    for (const auto& d: description) {
        vertex_t v = boost::add_vertex(g);
        vertices.emplace(d.first, v);

        g[v].name = d.first;
        g[v].configuration_module = d.second.module;
        g[v].id = id;
        auto module = std::find_if(modules.begin(), modules.end(), [&d](const ModulePtr& m) { return m->name() == d.first; });
        if (module != modules.end())
            g[v].module = *module;

        PathElementsPtr path = nullptr;
        auto path_it = std::find_if(paths.begin(), paths.end(), [&d](const PathElementsPtr path) {
            auto it = std::find_if(path->elements.begin(), path->elements.end(), [&d](const std::string& name) { return name == d.first; });
            return it != path->elements.end();
        });

        if (path_it != paths.end())
            path = *path_it;

        if (path) {
            assert(!path->resolved);
        }

        // If null, it means the modules belong to the main path
        g[v].path = path;

        id++;
    }

    modules.clear();

    // Create edges. One edge connect one module output to one module input
    for (const auto& vertex: vertices) {
        const auto& outputs = description.at(vertex.first).outputs;

        // Find all modules having for input this output
        for (const auto& output: outputs) {

            for (const auto& module: description) {
                // Skip ourself
                if (module.first == vertex.first)
                    continue;

                for (const auto& input: module.second.inputs) {
                    if ((input.module == vertex.first) && (input.parameter == output)) {
                        edge_t e;
                        bool inserted;
                        std::tie(e, inserted) = boost::add_edge(vertex.second, vertices.at(module.first), g);
                        g[e].name = input.parameter;
                        g[e].description = input.parameter;
                        if (input.isIndexed()) {
                            g[e].description += "[" + std::to_string(input.index) + "]";
                        }
                    }
                }
            }
        }
    }

    // Find any module whose output is not used by any module. It's useless, so remove it
    // Only allowed module in this situation is virtual modules, or modules whose leafModule() method returns true.
    for (auto it = vertices.begin(), ite = vertices.end(); it != ite;) {
        if (boost::out_degree(it->second, g) == 0) {

            if (Module::is_virtual_module(it->first) || (g[it->second].module && g[it->second].module->leafModule())) {
                ++it;
                continue;
            }

            on_module_removed(it->first);
            LOG(info) << "Module '" << it->first << "' output is not used by any other module. Removing it from the configuration.";
            boost::clear_vertex(it->second, g);
            boost::remove_vertex(it->second, g);
            it = vertices.erase(it);
        } else
            ++it;
    }

    auto log_and_throw_unresolved_input = [](const std::string& module_name, const InputTag& input) {
        LOG(fatal) << "Module '" << module_name << "' requested a non-existing input (" << input.toString() << ")";
        throw unresolved_input("Module '" + module_name + "' requested a non-existing input (" + input.toString() + ")");
    };

    // Find any module whose input point to a non-existing module / parameter
    for (const auto& vertex: vertices) {
        const auto& inputs = description.at(vertex.first).inputs;

        for (const auto& input: inputs) {
            auto it = vertices.find(input.module);
            if (it == vertices.end()) {
                // Non-existing module
                log_and_throw_unresolved_input(vertex.first, input);
            }

            // Look for input in module's output
            const auto& target_module_output = description.at(it->first).outputs;
            auto it_input = std::find_if(target_module_output.begin(), target_module_output.end(), [&input](const std::string& output) {
                    return input.parameter == output;
                    });

            if (it_input == target_module_output.end()) {
                // Non-existing parameter
                log_and_throw_unresolved_input(vertex.first, input);
            }
        }
    }

    // Re-assign each vertex a continuous id
    id = 0;
    typename boost::graph_traits<Graph>::vertex_iterator it_i, it_end;
    for (std::tie(it_i, it_end) = boost::vertices(g); it_i != it_end; it_i++) {
        g[*it_i].id = id++;
    }

    // Sort graph
    std::list<vertex_t> sorted_vertices;
    boost::topological_sort(g, std::front_inserter(sorted_vertices), boost::vertex_index_map(boost::get(&graph::Vertex::id, g)));

    // Remove virtual vertices
    sorted_vertices.erase(std::remove_if(sorted_vertices.begin(), sorted_vertices.end(),
                [&g](const vertex_t& vertex) {
                    return Module::is_virtual_module(g[vertex].name);
                }), sorted_vertices.end());

    // Re-fill modules vector with new content (sorted & cleaned)
    for (const auto& vertex: sorted_vertices) {
        assert(g[vertex].module);

        PathElementsPtr path = g[vertex].path;
        if (path) {
            path->modules.push_back(g[vertex].module);
        } else {
            modules.push_back(g[vertex].module);
        }
    }

    for (auto& path: paths) {
        path->resolved = true;
        for (const auto& name: path->elements) {
            auto it = std::find_if(path->modules.begin(), path->modules.end(), [&name](const ModulePtr& module) { return module->name() == name; });
            if (it == path->modules.end()) {
                LOG(warning) << "Module '" << name << "' in path not found.";
            }
        }
    }


    // Check for if a module use a Looper output but is not actually declared in the looper path
    for (const auto& vertex: sorted_vertices) {
        if (g[vertex].configuration_module.type == "Looper") {
            // We know that the module is a Looper
            // grab its execution path from its configuration
            auto looper_path = g[vertex].configuration_module.parameters->get<Path>("path");
            const auto& path_modules = looper_path.modules();

            out_edge_iterator_t e, e_end;
            std::tie(e, e_end) = boost::out_edges(vertex, g);

            // Iterator over all edges connected to this Looper vertex
            for (; e != e_end; ++e) {
                auto target = g[boost::target(*e, g)];

                // Check if target is inside the looper path
                auto it = std::find_if(path_modules.begin(), path_modules.end(), [&target](const ModulePtr& m) {
                    return m->name() == target.name;
                });

                if (it == path_modules.end()) {
                    LOG(fatal) << "Module '" << target.name << "' is configured to use Looper '" << g[vertex].name
                               << "' output, but is not actually part of the Looper execution path. This will lead to undefined "
                               << "behavior. You can fix the issue by adding the module '"
                               << target.name
                               << "' to the Looper execution path";

                    throw incomplete_looper_path("A module is using the looper output but not actually path of its "
                                                 "executation path");
                }
            }
        }
    }

    return g;
}

class edge_writer {
    public:
        edge_writer(Graph g) : graph(g) {}
        template <class VertexOrEdge>
            void operator()(std::ostream& out, const VertexOrEdge& v) const {
                out << "[label=\"" << graph[v].name << "\"]";
            }
    private:
        Graph graph;
};

void graphviz_export(const Graph& g, const std::string& filename) {

    std::ofstream f(filename.c_str());

    auto vertices_name = boost::get(&Vertex::name, g);
    auto edges_name = boost::get(&Edge::description, g);

    boost::write_graphviz(f, g, make_label_writer(vertices_name), make_label_writer(edges_name), boost::default_writer(), boost::get(&Vertex::id, g));
}
}
