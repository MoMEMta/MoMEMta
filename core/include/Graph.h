#pragma once

#include <momemta/Configuration.h>
#include <momemta/Module.h>

#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <string>
#include <vector>

struct PathElements;

/// Generic graph representation of the module hierarchy
namespace graph {

struct Vertex {
    std::string name;
    Configuration::Module configuration_module;
    ModulePtr module;
    uint32_t id;
    PathElements* path;
};

struct Edge {
    std::string name;
    std::string description;
};

typedef boost::adjacency_list<boost::listS, boost::listS, boost::bidirectionalS, Vertex, Edge> Graph;

typedef boost::graph_traits<Graph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<Graph>::edge_descriptor edge_t;
typedef boost::graph_traits<Graph>::out_edge_iterator out_edge_iterator_t;

/**
 * \brief Build a graph representation of the modules.
 *
 * The graph allows us to correctly order the module based on inputs and outputs, detects cycle,
 * and much more.
 *
 * \param description Description of the relationship between the modules.
 * \param[in, out] modules Vector of modules contributing to the graph. This vector will be sorted and cleaned of un-used modules
 * \param on_module_removed A call-back called each time a module is removed from the graph. Call back signature `void (const std::string&);`
 *
 * \sa Pool::description()
 */
Graph build(const Pool::DescriptionMap& description, std::vector<ModulePtr>& modules, const std::vector<PathElements*>& paths, std::function<void(const std::string&)> on_module_removed);

/**
 * \brief Export a given graph in `dot` format
 *
 * \note To produce a PDF, run the following command
 * ```
 * dot -Tpdf filename.dot -o filename.pdf
 * ```
 *
 * \param g The graph to export
 * \param filename The output filename
 */
void graphviz_export(const Graph& g, const std::string& filename);

}
