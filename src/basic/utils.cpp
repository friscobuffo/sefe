#include "utils.hpp"

#include "graph.hpp"

/**
 * @brief Converts a custom graph representation to an OGDF graph.
 *
 * This function takes a pointer to a custom graph (`myGraph`) and converts it
 * into an OGDF graph. It creates a new OGDF graph, adds nodes corresponding to
 * the nodes in the custom graph, and then adds edges between the nodes based
 * on the edges in the custom graph.
 *
 * @param myGraph A pointer to the custom graph to be converted.
 * @return A pointer to the newly created OGDF graph.
 */
ogdf::Graph* OgdfUtils::myGraphToOgdf(const Graph* myGraph) {
    ogdf::Graph* graph = new ogdf::Graph();
    std::vector<ogdf::node> nodes(myGraph->size());
    for (int i = 0; i < myGraph->size(); ++i)
        nodes[i] = graph->newNode();
    for (int i = 0; i < myGraph->size(); ++i) {
        const Node* node = myGraph->getNode(i);
        for (const Node* neighbor : node->getNeighbors())
            if (i < neighbor->getIndex())
                graph->newEdge(nodes[i], nodes[neighbor->getIndex()]);
    }
    return graph;
}

/**
 * @brief Converts an OGDF graph to a custom Graph representation.
 *
 * This function takes a pointer to an OGDF graph and converts it into a custom
 * Graph object. It iterates through all the nodes and their adjacent entries
 * in the OGDF graph, adding edges to the custom Graph object.
 *
 * @param graph A pointer to the OGDF graph to be converted.
 * @return A pointer to the newly created custom Graph object.
 */
const Graph* OgdfUtils::ogdfGraphToMyGraph(ogdf::Graph* graph) {
    Graph* myGraph = new Graph(graph->numberOfNodes());
    for (ogdf::node n : graph->nodes) {
        const int node = n->index();
        for (ogdf::adjEntry& adj : n->adjEntries) {
            const int neighbor = adj->twinNode()->index();
            if (node > neighbor) continue;
            myGraph->addEdge(node, neighbor);
        }
    }
    return myGraph;
}

/**
 * @brief Prints the adjacency list of an OGDF graph.
 *
 * This function iterates over all nodes in the given OGDF graph and prints
 * each node's index followed by a list of its adjacent nodes' indices.
 *
 * @param graph A pointer to the OGDF graph to be printed.
 */
void OgdfUtils::printOgdfGraph(ogdf::Graph* graph) {
    for (ogdf::node node : graph->nodes) {
        std::cout << node->index() << ": [ ";
        for (ogdf::adjEntry& adj : node->adjEntries) {
            const int neighbor = adj->twinNode()->index();
            std::cout << neighbor << " ";
        }
        std::cout << "]\n";
    }
}