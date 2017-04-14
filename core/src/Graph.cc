/*
 *  MoMEMta: a modular implementation of the Matrix Element Method
 *  Copyright (C) 2017  Universite catholique de Louvain (UCL), Belgium
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Graph.h>

#include <ModuleUtils.h>

#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/topological_sort.hpp>

#include <array>

namespace graph {

// Graph definitions

struct Vertex {
    uint32_t id;
    std::string name; // Unique name of the module
    std::string type; // Module type
    momemta::ModuleList::value_type def;
    Configuration::ModuleDecl decl;
};

struct Edge {
    std::string description;
    bool virt;
};

typedef boost::adjacency_list<boost::listS, boost::listS, boost::bidirectionalS, Vertex, Edge> Graph;

typedef boost::graph_traits<Graph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<Graph>::edge_descriptor edge_t;
typedef boost::graph_traits<Graph>::out_edge_iterator out_edge_iterator_t;
typedef boost::graph_traits<Graph>::in_edge_iterator in_edge_iterator_t;

class incomplete_looper_path: public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class graph_writer {
public:
    graph_writer(Graph g,
                 const std::vector<std::shared_ptr<ExecutionPath>>& paths):
            graph(g), paths(paths) {}


    // Vertex writer
    void writeVertex(std::ostream& out, const vertex_t& v) const {
        std::string shape = "ellipse";
        std::string color = "black";
        std::string style = "solid";
        std::string extra = "";

        if (graph[v].def.internal) {
            shape = "rectangle";
            color = "black";
            style = "dashed";
        }

        if (graph[v].type == "Looper") {
            style = "filled";
            extra = "fillcolor=\"" + path_colors.at(graph[v].decl.parameters->get<ExecutionPath>("path").id) + "\"";
        }

        out << "[shape=\"" << shape << "\",color=\"" << color << "\",style=\"" << style
            << "\",label=\"" << graph[v].name << "\"";

        if (!extra.empty()) {
            out << "," << extra;
        }

        out << "]";
    }

    // Edge writer
    void writeEdge(std::ostream& out, const edge_t& e) const {
        std::string color = "black";
        std::string style = "solid";
        std::string extra = "";

        if (graph[e].virt) {
            style = "invis";
            extra = "constraint=false";
        }

        out << "[color=\"" << color << "\",style=\"" << style
            << "\",label=\"" << graph[e].description << "\"";

        if (!extra.empty()) {
            out << "," << extra;
        }

        out << "]";
    }

    // Graph writer
    void writeGraph(std::ostream& out) const {

        size_t index = 0;
        size_t color_index = 0;
        for (const auto& path: paths) {

            out << "subgraph cluster_" << index << " {" << std::endl;

            out << R"(style=filled; fillcolor=")" << colors[color_index] << R"(";)" << std::endl;
            path_colors.emplace(path->id, colors[color_index]);

            auto looper_vertex = find_looper(path->id);

            if (looper_vertex != boost::graph_traits<Graph>::null_vertex())
                out << "    label=\"" << graph[looper_vertex].name << " execution path\";" << std::endl;

            out << "    ";
            for (const auto& e: path->elements) {
                auto vertex = find_vertex(e);
                out << graph[vertex].id << "; ";
            }

            out << std::endl;
            out << "}" << std::endl;

            index++;
            color_index++;
            if (color_index >= colors.size())
                color_index = 0;
        }
    }

private:
    vertex_t find_vertex(const std::string& name) const {
        typename boost::graph_traits<Graph>::vertex_iterator vtx_it, vtx_it_end;

        for(boost::tie(vtx_it, vtx_it_end) = boost::vertices(graph); vtx_it != vtx_it_end; ++vtx_it) {
            if (graph[*vtx_it].name == name)
                return *vtx_it;
        }

        return boost::graph_traits<Graph>::null_vertex();
    }

    vertex_t find_looper(const boost::uuids::uuid& path) const {
        typename boost::graph_traits<Graph>::vertex_iterator vtx_it, vtx_it_end;

        for(boost::tie(vtx_it, vtx_it_end) = boost::vertices(graph); vtx_it != vtx_it_end; ++vtx_it) {
            if (graph[*vtx_it].type == "Looper") {
                const auto& looper_path = graph[*vtx_it].decl.parameters->get<ExecutionPath>("path");
                if (looper_path.id == path)
                    return *vtx_it;
            }
        }

        return boost::graph_traits<Graph>::null_vertex();
    }

    Graph graph;
    const std::vector<std::shared_ptr<ExecutionPath>> paths;

    mutable std::unordered_map<boost::uuids::uuid, std::string,
                       boost::hash<boost::uuids::uuid>> path_colors;

    const std::array<std::string, 5> colors = {{
            "#BEEB9F",
            "#ACF0F2",
            "#F3FFE2",
            "#79BD8F88",
            "##EB7F0099"
    }};
};

class graph_writer_wrapper {
public:
    graph_writer_wrapper(std::shared_ptr<graph_writer> writer):
            writer(writer) {}

    void operator()(std::ostream& out, const vertex_t& v) const {
        writer->writeVertex(out, v);
    }

    void operator()(std::ostream& out, const edge_t& e) const {
        writer->writeEdge(out, e);
    }

    void operator()(std::ostream& out) const {
        writer->writeGraph(out);
    }

private:
    std::shared_ptr<graph_writer> writer;
};

void graphviz_export(const Graph& g,
                     const std::vector<std::shared_ptr<ExecutionPath>>& paths,
                     const std::string& filename) {

    std::ofstream f(filename.c_str());
    auto writer = std::make_shared<graph_writer>(g, paths);
    boost::write_graphviz(f, g, graph_writer_wrapper(writer), graph_writer_wrapper(writer),
                          graph_writer_wrapper(writer), boost::get(&Vertex::id, g));
}


bool isConnectedToByOut(Graph& g, vertex_t vertex, vertex_t to) {
    out_edge_iterator_t o, o_end;
    std::tie(o, o_end) = boost::out_edges(vertex, g);

    for (; o != o_end; ++o) {
        vertex_t target = boost::target(*o, g);
        if (target == to)
            return true;

        if (isConnectedToByOut(g, target, to))
            return true;
    }

    return false;
}

bool isConnectedToByIn(Graph& g, vertex_t vertex, vertex_t to) {
    in_edge_iterator_t i, i_end;
    std::tie(i, i_end) = boost::in_edges(vertex, g);

    for (; i != i_end; ++i) {
        vertex_t source = boost::source(*i, g);
        if (source == to)
            return true;

        if (isConnectedToByIn(g, source, to))
            return true;
    }

    return false;
}

bool isConnectedTo(Graph& g, vertex_t vertex, vertex_t to) {
    if (isConnectedToByOut(g, vertex, to))
        return true;

    return isConnectedToByIn(g, vertex, to);
}

/**
 * Test if two vertices are connected directly (an edge exists between them). Link is assumed to be from -> to.
 * \param g
 * \param from
 * \param to
 * \return
 */
bool isConnectedDirectlyTo(Graph& g, vertex_t from, vertex_t to) {
    out_edge_iterator_t o, o_end;
    std::tie(o, o_end) = boost::out_edges(from, g);

    for (; o != o_end; ++o) {
        vertex_t target = boost::target(*o, g);
        if (target == to)
            return true;
    }

    return false;
}

bool checkInPath(const Configuration::ModuleDecl& looper_decl, const std::string& module_name) {

    const auto& looper_path = looper_decl.parameters->get<ExecutionPath>("path");

    auto it = std::find_if(looper_path.elements.begin(), looper_path.elements.end(),
                           [&module_name](const std::string& m) {
                               return m == module_name;
                           });

    return it != looper_path.elements.end();
}

void sort_modules(const momemta::ModuleList& available_modules,
                  const std::vector<Configuration::ModuleDecl>& requested_modules,
                  const std::vector<std::shared_ptr<ExecutionPath>>& execution_paths,
                  SortedModuleList& modules,
                  const std::string& output /* = ""*/) {


    static auto get_module_def = [&available_modules](const std::string& module_type) {

        auto it = std::find_if(available_modules.begin(), available_modules.end(),
                               [&module_type](const momemta::ModuleList::value_type& m) {
                                   return m.name == module_type;
                               });

        assert(it != available_modules.end());

        return *it;
    };

    Graph g;
    uint32_t id = 0;

    // Keep a map of all the vertices for easy access
    std::unordered_map<std::string, vertex_t> vertices;

    // Create graph vertices. Each vertex is a module requested in the configuration file
    for (const auto& module: requested_modules) {
        vertex_t v = boost::add_vertex(g);

        auto& vertex = g[v];
        vertex.id = id++;
        vertex.name = module.name;
        vertex.type = module.type;

        // Attach module definition to the vertex
        vertex.def = get_module_def(module.type);

        // Attach module declaration to the vertex
        vertex.decl = module;

        vertices.emplace(module.name, v);
    }

    // Create edges, connecting modules together. An edge link module's outputs to module's inputs
    typename boost::graph_traits<Graph>::vertex_iterator vtx_it, vtx_it_end;
    for (std::tie(vtx_it, vtx_it_end) = boost::vertices(g); vtx_it != vtx_it_end; vtx_it++) {

        const auto& vertex = g[*vtx_it];

        // Connect each output of this module to any module needing it
        for (const auto& output: vertex.def.outputs) {

            // Find any module using this output
            for (const auto& module: requested_modules) {

                // Skip ourselves
                if (module.name == vertex.name)
                    continue;

                // Get definition of this new module
                const auto& input_module_def = get_module_def(module.type);
                for (const auto& input: input_module_def.inputs) {
                    // Grab the InputTag for each input, and see if it points to the vertex
                    momemta::gtl::optional<std::vector<InputTag>> inputTags =
                            momemta::getInputTagsForInput(input, *module.parameters);

                    // If the input is optional, we may not have anything
                    if (! inputTags)
                        continue;

                    for (const auto& inputTag: *inputTags) {
                        if (vertex.name == inputTag.module && output.name == inputTag.parameter) {
                            // We have a match, the InputTag points to the vertex output
                            // Create a new edge in the graph

                            edge_t e;
                            bool inserted;
                            std::tie(e, inserted) = boost::add_edge(*vtx_it, vertices.at(module.name), g);

                            auto& edge = g[e];
                            edge.virt = false;
                            edge.description = inputTag.parameter;
                            if (inputTag.isIndexed()) {
                                edge.description += "[" + std::to_string(inputTag.index) + "]";
                            }
                        }
                    }
                }
            }
        }
    }

    // Modules are connected together. Find all vertices not connected to anything and remove them
    for (auto it = vertices.begin(), ite = vertices.end(); it != ite;) {
        if (boost::out_degree(it->second, g) == 0) {

            auto& vertex = g[it->second];

            // Don't consider internal or sticky modules
            if (vertex.def.internal || vertex.def.sticky) {
                ++it;
                continue;
            }

            // Check in the definition if this module has any output
            // If not, we keep it.
            if (vertex.def.outputs.empty()) {
                ++it;
                continue;
            }

            // Otherwise, remove it
            LOG(info) << "Module '" << it->first << "' output is not used by any other module. Removing it from the configuration.";
            boost::clear_vertex(it->second, g);
            boost::remove_vertex(it->second, g);

            it = vertices.erase(it);
        } else
            ++it;
    }

    // We need to make sure that any dependencies of a module inside a looper
    // is ran before the looper itself.
    for (const auto& vertex: vertices) {
        if (g[vertex.second].type == "Looper") {

            const auto& decl = g[vertex.second].decl;

            // Retrieve the looper path
            const auto& looper_path = decl.parameters->get<ExecutionPath>("path");

            // Add virtual link between the looper and all module inside its execution path
            for (const auto& m: looper_path.elements) {

                auto module_it = vertices.find(m);
                if (module_it == vertices.end()) {
                    LOG(warning) << "Module '" << m << "' present in Looper '" << decl.name << "' execution path does "
                                "not exists";
                    continue;
                }

                auto module_vertex = module_it->second;
                if (!isConnectedDirectlyTo(g, vertex.second, module_vertex)) {
                    edge_t e;
                    bool inserted;
                    std::tie(e, inserted) = boost::add_edge(vertex.second, module_vertex, g);
                    g[e].description = "virtual link (module in path)";
                    g[e].virt = true;
                }
            }

            out_edge_iterator_t e, e_end;
            std::tie(e, e_end) = boost::out_edges(vertex.second, g);

            // Iterator over all edges this Looper vertex is connected to
            for (; e != e_end; ++e) {
                auto target = boost::target(*e, g);

                // Iterate over all edges connected to the module
                in_edge_iterator_t i, i_end;
                std::tie(i, i_end) = boost::in_edges(target, g);

                for (; i != i_end; ++i) {
                    auto source = boost::source(*i, g);

                    if (source == vertex.second)
                        continue;

                    // Check if the source vertex is connected to the looper in any way
                    if (!isConnectedTo(g, source, vertex.second)) {
                        edge_t e;
                        bool inserted;
                        std::tie(e, inserted) = boost::add_edge(source, vertex.second, g);
                        g[e].description = "virtual link";
                        g[e].virt = true;
                    }
                }
            }
        }
    }

    // Ensure ids are continuous
    id = 0;
    for (std::tie(vtx_it, vtx_it_end) = boost::vertices(g); vtx_it != vtx_it_end; vtx_it++) {
        g[*vtx_it].id = id++;
    }

    // Sort graph
    std::list<vertex_t> sorted_vertices;
    try {
        boost::topological_sort(g, std::front_inserter(sorted_vertices),
                                boost::vertex_index_map(boost::get(&Vertex::id, g)));
    } catch (...) {
        graphviz_export(g, execution_paths, "graph.debug");
        LOG(fatal) << "Exception while sorting the graph. Graphviz representation saved as graph.debug";
        throw;
    }

    if (!output.empty())
        graphviz_export(g, execution_paths, output);

    // Remove vertices corresponding to internal modules
    sorted_vertices.erase(std::remove_if(sorted_vertices.begin(), sorted_vertices.end(),
                                         [&g](const vertex_t& vertex) -> bool {
                                             return g[vertex].def.internal;
                                         }), sorted_vertices.end());

    // Ensure all the modules using a Looper output are present in the looper path
    std::map<vertex_t, std::vector<vertex_t>> modules_not_in_path;
    for (const auto& vertex: sorted_vertices) {
        if (g[vertex].type == "Looper") {

            const auto& decl = g[vertex].decl;

            out_edge_iterator_t e, e_end;
            std::tie(e, e_end) = boost::out_edges(vertex, g);

            // Iterator over all edges connected to this Looper vertex
            for (; e != e_end; ++e) {
                auto target = boost::target(*e, g);

                // Check if target is inside the looper path
                if (! checkInPath(decl, g[target].name)) {
                    auto& loopers = modules_not_in_path[target];
                    auto it = std::find(loopers.begin(), loopers.end(), vertex);
                    if (it == loopers.end())
                        loopers.push_back(vertex);
                } else {
                    modules_not_in_path.erase(target);
                }
            }
        }
    }

    if (modules_not_in_path.size() != 0) {
        // Only print a message for the first module not in path
        auto it = modules_not_in_path.begin();

        auto target = g[it->first];

        std::stringstream loopers;
        for (size_t i = 0; i < it->second.size(); i++) {
            loopers << "'" << g[it->second[i]].name << "'";
            if (i != it->second.size() - 1)
                loopers << ", ";
        }

        std::string plural = it->second.size() ? "s" : "";
        std::string one_of_the = it->second.size() ? "one of the" : "the";

        LOG(fatal) << "Module '" << target.name << "' is configured to use Looper " << loopers.str()
                   << " output" << plural << ", but is not actually part of the Looper" << plural << " execution path. This will lead to undefined "
                   << "behavior. You can fix the issue by adding the module '"
                   << target.name
                   << "' to " << one_of_the << " Looper" << plural << " execution path";

        throw incomplete_looper_path("A module is using the looper output but not actually part of its "
                                             "execution path");
    }

    for (auto vertex: sorted_vertices) {

        // Find in which execution path this module is. If it's not found inside any execution path
        // declared in the configuration, then the module is assigned to the default execution path.

        static auto find_module_in_path = [&vertex, &g](std::shared_ptr<ExecutionPath> p) -> bool {
            // Returns true if the module is inside the execution path `p`, False otherwise
            auto it = std::find_if(p->elements.begin(), p->elements.end(),
                                   [&vertex, &g](const std::string& element) -> bool {
                                       return g[vertex].name == element;
                                   }
            );

            return it != p->elements.end();
        };


        auto execution_path_it = std::find_if(execution_paths.begin(), execution_paths.end(), find_module_in_path);
        auto execution_path = execution_path_it == execution_paths.end() ? DEFAULT_EXECUTION_PATH :
                              (*execution_path_it)->id;

        // The first declared path must always be the default one, otherwise we are in trouble
        assert(!modules.getPaths().empty() || execution_path == DEFAULT_EXECUTION_PATH);

        modules.addModule(execution_path, g[vertex].decl);
    }
}

void SortedModuleList::addModule(const boost::uuids::uuid& path, const Configuration::ModuleDecl& module) {
    // Check if an entry already exists for the execution path.
    auto map_it = modules.find(path);
    if (map_it == modules.end()) {
        // Add the path into the list of know path, keeping track of the order
        sorted_execution_paths.emplace_back(path);

        // Add the module into its path
        modules[path].emplace_back(module);
    } else {
        map_it->second.emplace_back(module);
    }
}

const std::vector<boost::uuids::uuid>& SortedModuleList::getPaths() const {
    return sorted_execution_paths;
}

const std::vector<Configuration::ModuleDecl>& SortedModuleList::getModules(const boost::uuids::uuid& path) const {
    return modules.at(path);
}

}