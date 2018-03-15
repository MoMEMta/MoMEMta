#pragma once

#include <momemta/config.h>
#include <momemta/Configuration.h>
#include <momemta/Module.h>

#include <ExecutionPath.h>

#include <map>
#include <string>
#include <vector>

#include <boost/config.hpp>
#include <boost/functional/hash.hpp>
#include <boost/graph/adjacency_list.hpp>

#ifdef DEBUG_TIMING
#include <chrono>
#endif

namespace momemta {

// Graph definitions

/// A vertex of the graph, symbolizing a module
struct Vertex {
    uint32_t id;
    std::string name; // Unique name of the module
    std::string type; // Module type
    momemta::ModuleList::value_type def;
    Configuration::ModuleDecl decl;
};

/// An edge of the graph, symbolizing the connection between two modules (an InputTag)
struct Edge {
    std::string description;
    bool virt; ///< If true, this edge is virtual and represent an additional constrain to the graph
    InputTag tag; ///< InputTag of the connection
};

typedef boost::adjacency_list<boost::listS, boost::listS, boost::bidirectionalS, Vertex, Edge> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<Graph>::edge_descriptor edge_t;

/**
 * Abstraction of the computation graph
 *
 * The computation graph is defined as the list of operation to perform to compute the final weight. Each operations are
 * implemented by modules, symbolized as Vertex in the graph. Connections between modules, symbolized as Edges, as used
 * as constrains to deduce in which orders the operations must be done.
 *
 * If Loopers are present in the configuration, subgraph are also created, one for each looper. These subgraphs are built
 * using the modules present in the execution path of the loopers, in the same way the main graph is built. This means
 * that any modules present in a looper executation path won't be present in the main graph, but will be present in
 * a subgraph. These subgraph are directly managed by the loopers themselves.
 */
class ComputationGraph {
public:
    /**
     * \brief Create and configure the modules making the computation graph. Modules will use \p pool to allocate their
     * memory
     *
     * \param pool The memory pool used by the modules to allocate their memory
     */
    void initialize(PoolPtr pool);

    // Interface to module methods
    /// Call Module::configure() for each module of the computation graph.
    void configure();
    /// Call Module::beginIntegration() for each module of the computation graph.
    void beginIntegration();
    /// Execute each module of the computation graph.
    Module::Status execute();
    /// Call Module::endIntegration() for each module of the computation graph.
    void endIntegration();
    /// Call Module::finish() for each module of the computation graph.
    void finish();

#ifdef DEBUG_TIMING
    /// \private
    void logTimings() const;
#endif

    /**
     * \brief Set the number of integration dimensions needed by the computation graph
     * \param n Number of dimensions
     */
    void setNDimensions(size_t n);

    /// \return The number of integration dimensions needed by the computation graph
    size_t getNDimensions() const;

    /// \private ; only public for unit tests
    void addDecl(const boost::uuids::uuid& path, const Configuration::ModuleDecl& decl);
    /// \private ; only public for unit tests
    const std::vector<boost::uuids::uuid>& getPaths() const;
    /// \private ; only public for unit tests
    const std::vector<Configuration::ModuleDecl>& getDecls(const boost::uuids::uuid& path) const;

private:
    std::vector<boost::uuids::uuid> sorted_execution_paths;
    std::unordered_map<
            boost::uuids::uuid,
            std::vector<Configuration::ModuleDecl>,
            boost::hash<boost::uuids::uuid>
    > module_decls;

    std::vector<ModulePtr> modules;

    size_t n_dimensions; ///< Number of integration dimensions needed, after modules pruning

#ifdef DEBUG_TIMING
    std::unordered_map<Module *, std::chrono::high_resolution_clock::duration> module_timings;
#endif
};

/**
 * Builder for the computation graph.
 *
 * Connections between modules are used as constrains for proper ordering of operations. If a module does not contribute
 * to the graph (it's output is not used and it's not a sticky module), it's ignored and removed from the final graph.
 *
 * In case it's not possible to build the computation graph (cyclic dependencies for example), an exception is thrown.
 *
 * \sa momemta::ComputationGraph
 */
class ComputationGraphBuilder {
public:

    /**
     * \brief Create a new builder
     *
     * \warning The configuration may be updated if, after pruning of unused modules, the number of integration
     * dimensions requested have changed.
     *
     * \param available_modules List of definitions of all the available modules.
     * \param configuration The configuration to use to build the computation graph. It may be updated if, after
     * pruning of unused modules, the number of integration dimensions requested changed.
     *
     * \sa momemta::ModuleRegistry::exportList()
     */
    ComputationGraphBuilder(
            const momemta::ModuleList& available_modules,
            const Configuration& configuration
    );

    /**
     * \brief Build the computation graph.
     *
     * An exception is thrown if a graph cannot be build. In such case, a graphviz representation of the graph
     * is automatically dumped into the current working directory into a file named `graph.debug`, which may be used
     * to diagnosticate the issue.
     *
     * \return A pointer to the computation graph.
     */
    std::shared_ptr<ComputationGraph> build();

    /**
     * \brief Export a GraphViz representation of the computation graph to a file.
     *
     * \note You must first construct the graph with build() before calling this function
     *
     * \param output The name of the output file
     */
    void exportGraph(const std::string& output) const;

private:
    void prune_graph();
    void sort_graph();
    void validate();

    const momemta::ModuleList& available_modules;
    const Configuration& configuration;

    Graph g;
    std::unordered_map<std::string, vertex_t> vertices;

    std::list<vertex_t> sorted_vertices; // filled only if sort_graph is called

    bool graph_exportable = false;
};

}
