#include "graph.hpp"

#include <iostream>
#include <list>
#include <cassert>

Node::Node(const int index, const Graph* graph) : index_m(index), graph_m(graph) {}

int Node::getIndex() const {
    return index_m;
}

std::vector<const Node*>& Node::getNeighbors() {
    return neighbors_m;
}

const std::vector<const Node*>& Node::getNeighbors() const {
    return neighbors_m;
}

void Node::addNeighbor(const Node* neighbor) {
    neighbors_m.push_back(neighbor);
}

const Graph* Node::getGraph() const {
    return graph_m;
}

Graph::Graph(const int numberOfNodes) {
    assert(numberOfNodes > 0);
    for (int i = 0; i < numberOfNodes; ++i)
        nodes_m.push_back(Node(i, this));
    for (int i = 0; i < numberOfNodes; ++i)
        nodesPointers_m.push_back(&nodes_m[i]);
    assert(nodes_m.size() == nodesPointers_m.size());
    for (int i = 0; i < size(); ++i)
        assert(getNode(i) == getNodes()[i]);
}

// assumes edge is not already in graph
// adds edge from-to and edge to-from
void Graph::addEdge(const int fromIndex, const int toIndex) {
    assert(fromIndex >= 0 && fromIndex < size());
    assert(toIndex >= 0 && toIndex < size());
    addEdge(getNode(fromIndex), getNode(toIndex));
}

// assumes edge is not already in graph
// adds edge from-to and edge to-from
void Graph::addEdge(Node* from, Node* to) {
    from->addNeighbor(to);
    to->addNeighbor(from);
}

int Graph::size() const {
    return nodes_m.size();
}

void Graph::print() const {
    for (auto& node : nodes_m) {
        std::cout << "node [" << node.getIndex() << "]: neighbors: [";
        for (const Node* neighbor : node.getNeighbors()) {
            std::cout << " " << neighbor->getIndex();
        }
        std::cout << " ]\n";
    }
};

// if the graph is bipartite: returns a vector
// with 0s and 1s for each node dividing the nodes into two partitions
const std::optional<std::vector<int>> Graph::computeBipartition() const {
    std::vector<int> bipartition{};
    bipartition.resize(size());
    for (int i = 0; i < size(); ++i)
        bipartition[i] = -1;
    for (int i = 0; i < size(); ++i) {
        if (bipartition[i] == -1) {
            bool check = bfsBipartition(i, bipartition);
            if (!check) return std::nullopt;
        }
    }
    return bipartition;
}

bool Graph::bfsBipartition(int nodeIndex, std::vector<int>& bipartition) const {
    bipartition[nodeIndex] = 0;
    std::list<int> queue{};
    queue.push_back(nodeIndex);
    while (queue.size() > 0) {
        nodeIndex = queue.front();
        queue.pop_front();
        const Node* node = getNode(nodeIndex);
        for (const Node* neighbor : node->getNeighbors()) {
            int neighborIndex = neighbor->getIndex();
            if (bipartition[neighborIndex] == -1) {
                bipartition[neighborIndex] = 1-bipartition[nodeIndex];
                queue.push_back(neighborIndex);
            }
            else
                if (bipartition[neighborIndex] == bipartition[nodeIndex]) return false;
        }
    }
    return true;
}

const Node* Graph::getNode(const int index) const {
    return &nodes_m[index];
}

Node* Graph::getNode(const int index) {
    return &nodes_m[index];
}

const std::vector<const Node*> Graph::getNodes() const {
    return nodesPointers_m;
}

Graph* Graph::computeIntersection(const Graph* graph) const {
    assert(size() == graph->size());
    Graph* intersection = new Graph(size());
    computeIntersection(graph, intersection);
    return intersection;
}

void Graph::computeIntersection(const Graph* graph, Graph* intersection) const {
    assert(size() == graph->size());
    assert(size() == intersection->size());
    bool isEdgeInGraph1[size()];
    bool isEdgeInGraph2[size()];
    for (int i = 0; i < size(); ++i) {
        const Node* node1 = getNode(i);
        const Node* node2 = graph->getNode(i);
        for (int j = 0; j < size(); ++j) {
            isEdgeInGraph1[j] = false;
            isEdgeInGraph2[j] = false;
        }
        for (const Node* neighbor : node1->getNeighbors())
            isEdgeInGraph1[neighbor->getIndex()] = true;
        for (const Node* neighbor : node2->getNeighbors())
            isEdgeInGraph2[neighbor->getIndex()] = true;
        for (int j = 0; j < size(); ++j)
            if (isEdgeInGraph1[j] && isEdgeInGraph2[j] && i < j)
                intersection->addEdge(i, j);
    }
}

SubGraph::SubGraph(const int numberOfNodes, const Graph* graph) 
: Graph(numberOfNodes), originalNodes_m(numberOfNodes), originalGraph_m(graph) {
    assert(numberOfNodes <= graph->size());
}

const Node* SubGraph::getOriginalNode(const Node* node) const {
    const int index = node->getIndex();
    return originalNodes_m.getPointer(index);
}

void SubGraph::setOriginalNode(const Node* node, const Node* originalNode) {
    const int index = node->getIndex();
    originalNodes_m.setPointer(index, originalNode);
}

void SubGraph::print() const {
    for (auto& node : nodes_m) {
        const int originalIndex = getOriginalNode(&node)->getIndex();
        const std::vector<const Node*>& neighbors = node.getNeighbors();
        std::cout << "node: " << originalIndex << " neighbors: " << neighbors.size() << " [ ";
        for (const Node* neighbor : neighbors)
            std::cout << getOriginalNode(neighbor)->getIndex() << " ";
        std::cout << "]\n";
    }
}