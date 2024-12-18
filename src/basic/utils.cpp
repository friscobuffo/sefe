#include "utils.hpp"

#include "graph.hpp"

std::unique_ptr<ogdf::Graph> OgdfUtils::myGraphToOgdf(const Graph& myGraph) {
    ogdf::Graph* graph = new ogdf::Graph();
    std::vector<ogdf::node> nodes(myGraph.size());
    for (int i = 0; i < myGraph.size(); ++i)
        nodes[i] = graph->newNode();
    for (int i = 0; i < myGraph.size(); ++i) {
        const Node& node = myGraph.getNode(i);
        for (auto& edge : node.getEdges())
            if (i < edge.to.getIndex())
                graph->newEdge(nodes[i], nodes[edge.to.getIndex()]);
    }
    return std::unique_ptr<ogdf::Graph>(graph);
}

std::unique_ptr<const Graph> OgdfUtils::ogdfGraphToMyGraph(ogdf::Graph& graph) {
    auto myGraph = std::make_unique<Graph>();
    for (int i = 0; i < graph.numberOfNodes(); ++i)
        myGraph->addNode(Color::NONE);
    for (ogdf::node n : graph.nodes) {
        const int node = n->index();
        for (ogdf::adjEntry& adj : n->adjEntries) {
            const int neighbor = adj->twinNode()->index();
            if (node > neighbor) continue;
            myGraph->addEdge(node, neighbor, 1.0, Color::BLACK);
        }
    }
    return myGraph;
}

void OgdfUtils::printOgdfGraph(ogdf::Graph& graph) {
    for (ogdf::node node : graph.nodes) {
        std::cout << node->index() << ": [ ";
        for (ogdf::adjEntry& adj : node->adjEntries) {
            const int neighbor = adj->twinNode()->index();
            std::cout << neighbor << " ";
        }
        std::cout << "]\n";
    }
}