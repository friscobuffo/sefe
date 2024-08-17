#include "bicoloredGraph.hpp"

#include <iostream>

#include "../auslander-parter/biconnectedComponent.hpp"

NodeWithColors::NodeWithColors(const int index, const BicoloredGraph& graph)
: index_m(index), graph_m(graph) {}

const int NodeWithColors::getIndex() const {
    return index_m;
}

const std::vector<Edge>& NodeWithColors::getEdges() const {
    return edges_m;
}

std::vector<Edge>& NodeWithColors::getEdges() {
    return edges_m;
}

// assumes edge is not already in graph
void NodeWithColors::addEdge(const NodeWithColors& neighbor, const Color color) {
    edges_m.push_back(Edge{neighbor, color});
    if (color == Color::BOTH)
        ++numberOfBlackEdges_m;
}

const BicoloredGraph& NodeWithColors::getBicoloredGraph() const {
    return graph_m;
}

const int NodeWithColors::getNumberOfBlackEdges() const {
    return numberOfBlackEdges_m;
}

BicoloredGraph::BicoloredGraph(const Graph& graph1, const Graph& graph2)
: intersection_m(graph1.size()) {
    assert(graph1.size() == graph2.size());
    for (int i = 0; i < graph1.size(); ++i)
        nodes_m.push_back(NodeWithColors(i, *this));
    graph1.computeIntersection(graph2, intersection_m);
    bool isEdgeInGraph1[size()];
    bool isEdgeInGraph2[size()];
    for (int i = 0; i < size(); ++i) {
        const Node& node1 = graph1.getNode(i);
        const Node& node2 = graph2.getNode(i);
        for (int j = 0; j < size(); ++j) {
            isEdgeInGraph1[j] = false;
            isEdgeInGraph2[j] = false;
        }
        for (int j = 0; j < node1.getNumberOfNeighbors(); ++j) {
            const Node& neighbor = node1.getNeighbor(j);
            isEdgeInGraph1[neighbor.getIndex()] = true;
        }
        for (int j = 0; j < node2.getNumberOfNeighbors(); ++j) {
            const Node& neighbor = node2.getNeighbor(j);
            isEdgeInGraph2[neighbor.getIndex()] = true;
        }
        for (int j = 0; j < size(); ++j) {
            if (i > j) continue;
            if (isEdgeInGraph1[j] && isEdgeInGraph2[j]) {
                addEdge(i, j, Color::BOTH);
                continue;
            }
            if (isEdgeInGraph1[j]) {
                addEdge(i, j, Color::RED);
                continue;
            }
            if (isEdgeInGraph2[j]) {
                addEdge(i, j, Color::BLUE);
                continue;
            }
        }
    }
}

BicoloredGraph::BicoloredGraph(const int numberOfNodes) 
: intersection_m(numberOfNodes) {
    for (int i = 0; i < numberOfNodes; ++i)
        nodes_m.push_back(NodeWithColors(i, *this));
}

const NodeWithColors& BicoloredGraph::getNode(const int index) const {
    return nodes_m[index];
}

NodeWithColors& BicoloredGraph::getNode(const int index) {
    return nodes_m[index];
}

void BicoloredGraph::addEdge(NodeWithColors& from, NodeWithColors& to, Color color) {
    from.addEdge(to, color);
    to.addEdge(from, color);
    if (color == Color::BOTH)
        intersection_m.addEdge(from.getIndex(), to.getIndex());
}

void BicoloredGraph::addEdge(const int fromIndex, const int toIndex, Color color) {
    addEdge(getNode(fromIndex), getNode(toIndex), color);
}

const int BicoloredGraph::size() const {
    return nodes_m.size();
}

const Graph& BicoloredGraph::getIntersection() const {
    return intersection_m;
}

void BicoloredGraph::print() const {
    for (auto& node : nodes_m) {
        const int index = node.getIndex();
        const std::vector<Edge>& edges = node.getEdges();
        std::cout << "node: " << index << " neighbors: " << edges.size() << " [ ";
        for (const Edge& edge : edges)
            std::cout << "(" << edge.node.getIndex() << " " << color2string(edge.color) << ") ";
        std::cout << "]\n";
    }
}
