#include "utils.hpp"

#include "graph.hpp"

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