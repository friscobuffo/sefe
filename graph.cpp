#include "graph.hpp"

#include <iostream>
#include <list>
#include <cassert>

#include "utils.hpp"

Node::Node(int index, const Graph* graph) : index_m(index), graph_m(graph) {}

int Node::getIndex() const {
    return index_m;
}

std::vector<Node*>& Node::getNeighbors() {
    return neighbors_m;
}

const std::vector<Node*>& Node::getNeighbors() const {
    return neighbors_m;
}

void Node::addNeighbor(Node* neighbor) {
    neighbors_m.push_back(neighbor);
}

const Graph* Node::getGraph() const {
    return graph_m;
}


Graph::Graph(int numberOfNodes) {
    assert(numberOfNodes > 0);
    nodes_m.resize(numberOfNodes);
    for (int i = 0; i < numberOfNodes; ++i)
        nodes_m[i] = std::make_unique<Node>(i, this);
}

// assumes edge is not already in graph
// adds edge from-to and edge to-from
void Graph::addEdge(int fromIndex, int toIndex) {
    assert(fromIndex >= 0 && fromIndex < size());
    assert(toIndex >= 0 && toIndex < size());
    addEdge(nodes_m[fromIndex].get(), nodes_m[toIndex].get());
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
        std::cout << "node [" << node->getIndex() << "]: neighbors: [";
        for (Node* neighbor : node->getNeighbors()) {
            std::cout << " " << neighbor->getIndex();
        }
        std::cout << " ]\n";
    }
};

// if the graph is bipartite: returns a vector
// with 0s and 1s for each node dividing the nodes into two partitions
const std::optional<std::vector<int>> Graph::computeBipartition() {
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

const Node* Graph::getNode(int index) const {
    return nodes_m[index].get();
}

Node* Graph::getNode(int index) {
    return nodes_m[index].get();
}


NodePointers::NodePointers(int numberOfNodes) {
    pointers_m.resize(numberOfNodes);
    for (int i = 0; i < numberOfNodes; ++i)
        pointers_m[i] = nullptr;
}

const Node* NodePointers::getNodePointer(int index) const {
    assert(pointers_m[index] != nullptr);
    return pointers_m[index];
}

void NodePointers::setNodePointer(int index, const Node* node) {
    assert(pointers_m[index] == nullptr);
    pointers_m[index] = node;
}



SubGraph::SubGraph(int numberOfNodes, const Graph* graph) 
: Graph(numberOfNodes), originalNodes_m(numberOfNodes), originalGraph_m(graph) {
    assert(numberOfNodes <= graph->size());
}

const Node* SubGraph::getOriginalNode(const Node* node) const {
    int index = node->getIndex();
    return originalNodes_m.getNodePointer(index);
}

void SubGraph::setOriginalNode(const Node* node, const Node* originalNode) {
    int index = node->getIndex();
    originalNodes_m.setNodePointer(index, originalNode);
}

void SubGraph::print() const {
    for (auto& node : nodes_m) {
        int originalIndex = getOriginalNode(node.get())->getIndex();
        std::vector<Node*>& neighbors = node->getNeighbors();
        std::cout << "node: " << originalIndex << " neighbors: " << neighbors.size() << " [ ";
        for (Node* neighbor : neighbors)
            std::cout << getOriginalNode(neighbor)->getIndex() << " ";
        std::cout << "]\n";
    }
}