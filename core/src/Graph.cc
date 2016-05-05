#include <Graph.h>
#include <logging.h>

#include <momemta/Module.h>

#include <boost/graph/graphviz.hpp>
#include <boost/graph/topological_sort.hpp>

namespace graph {

class unresolved_input: public std::runtime_error {
    using std::runtime_error::runtime_error;
};

Graph build(const Pool::DescriptionMap& description, std::vector<ModulePtr>& modules, std::function<void(const std::string&)> on_module_removed) {

    Graph g;

    uint32_t id = 0;
    std::unordered_map<std::string, vertex_t> vertices;

    // Create vertices. Each vertex is a module
    for (const auto& d: description) {
        vertex_t v = boost::add_vertex(g);
        vertices.emplace(d.first, v);

        g[v].name = d.first;
        g[v].id = id;
        auto module = std::find_if(modules.begin(), modules.end(), [&d](const ModulePtr& m) { return m->name() == d.first; });
        if (module != modules.end())
            g[v].module = *module;
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
                        if (input.isIndexed()) {
                            g[e].name += "[" + std::to_string(input.index) + "]";
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

            if (Module::is_virtual_module(it->first) || g[it->second].module->leafModule()) {
                ++it;
                continue;
            }

            on_module_removed(it->first);
            LOGGER->info("Module '{}' output is not used by any other module. Removing it from the configuration.", it->first);
            boost::clear_vertex(it->second, g);
            boost::remove_vertex(it->second, g);
            it = vertices.erase(it);
        } else
            ++it;
    }

    auto log_and_throw_unresolved_input = [](const std::string& module_name, const InputTag& input) {
        LOGGER->critical("Module '{}' requested a non-existing input ({})", module_name, input.toString());
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
        modules.push_back(g[vertex].module);
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
    auto edges_name = boost::get(&Edge::name, g);

    boost::write_graphviz(f, g, make_label_writer(vertices_name), make_label_writer(edges_name), boost::default_writer(), boost::get(&Vertex::id, g));
}
}
