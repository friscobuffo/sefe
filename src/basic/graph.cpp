#include "graph.hpp"

#include <iostream>
#include <list>
#include <cassert>

/**
 * @brief Constructs a Node with a given index and associated graph.
 * 
 * @param index The index of the node.
 * @param graph Pointer to the graph to which this node belongs.
 */
Node::Node(const int index, const Graph* graph) : index_m(index), graph_m(graph) {}

/**
 * @brief Gets the index of the node.
 * 
 * @return int The index of the node.
 */
int Node::getIndex() const {
    return index_m;
}

/**
 * @brief Gets the neighbors of the node.
 * 
 * @return std::vector<const Node*>& A vector of pointers to the neighboring nodes.
 */
std::vector<const Node*>& Node::getNeighbors() {
    return neighbors_m;
}

/**
 * @brief Gets the neighbors of the node (const version).
 * 
 * @return const std::vector<const Node*>& A vector of pointers to the neighboring nodes.
 */
const std::vector<const Node*>& Node::getNeighbors() const {
    return neighbors_m;
}

/**
 * @brief Adds a neighbor to the node.
 * 
 * @param neighbor Pointer to the neighboring node to be added.
 */
void Node::addNeighbor(const Node* neighbor) {
    neighbors_m.push_back(neighbor);
}


/**
 * @brief Gets the graph to which this node belongs.
 * 
 * @return const Graph* Pointer to the graph.
 */
const Graph* Node::getGraph() const {
    return graph_m;
}

/**
 * @brief Constructs a Graph with a given number of nodes.
 * 
 * @param numberOfNodes The number of nodes in the graph.
 */
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

/**
 * @brief Adds an edge between two nodes specified by their indices.
 * Assumes edge is not already in graph.
 * 
 * @param fromIndex The index of the starting node.
 * @param toIndex The index of the ending node.
 */
void Graph::addEdge(const int fromIndex, const int toIndex) {
    assert(fromIndex >= 0 && fromIndex < size());
    assert(toIndex >= 0 && toIndex < size());
    addEdge(getNode(fromIndex), getNode(toIndex));
}

/**
 * @brief Adds an edge between two nodes.
 * Assumes edge is not already in graph.
 * 
 * @param from Pointer to the starting node.
 * @param to Pointer to the ending node.
 */
void Graph::addEdge(Node* from, Node* to) {
    from->addNeighbor(to);
    to->addNeighbor(from);
}

/**
 * @brief Gets the number of nodes in the graph.
 * 
 * @return int The number of nodes.
 */
int Graph::size() const {
    return nodes_m.size();
}

/**
 * @brief Prints the graph to the standard output.
 */
void Graph::print() const {
    for (auto& node : nodes_m) {
        std::cout << "node [" << node.getIndex() << "]: neighbors: [";
        for (const Node* neighbor : node.getNeighbors()) {
            std::cout << " " << neighbor->getIndex();
        }
        std::cout << " ]\n";
    }
};

/**
 * @brief Computes the bipartition of the graph if it is bipartite.
 * 
 * @return std::optional<std::vector<int>> A vector with 0s and 1s for each node
 * dividing the nodes into two partitions, or std::nullopt if the graph is not bipartite.
 */
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

/**
 * @brief Performs a BFS to check if the graph can be bipartitioned starting from a given node.
 * 
 * @param nodeIndex The index of the starting node.
 * @param bipartition Reference to the bipartition vector.
 * @return bool True if the graph can be bipartitioned, false otherwise.
 */
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

/**
 * @brief Gets a node by its index (const version).
 * 
 * @param index The index of the node.
 * @return const Node* Pointer to the node.
 */
const Node* Graph::getNode(const int index) const {
    return &nodes_m[index];
}

/**
 * @brief Gets a node by its index.
 * 
 * @param index The index of the node.
 * @return Node* Pointer to the node.
 */
Node* Graph::getNode(const int index) {
    return &nodes_m[index];
}

/**
 * @brief Gets all nodes in the graph.
 * 
 * @return const std::vector<const Node*> A vector of pointers to all nodes.
 */
const std::vector<const Node*> Graph::getNodes() const {
    return nodesPointers_m;
}

/**
 * @brief Computes the intersection of this graph with another graph.
 * 
 * @param graph Pointer to the other graph.
 * @return Graph* Pointer to the intersection graph.
 */
Graph* Graph::computeIntersection(const Graph* graph) const {
    assert(size() == graph->size());
    Graph* intersection = new Graph(size());
    computeIntersection(graph, intersection);
    return intersection;
}

/**
 * @brief Computes the intersection of this graph with another graph and stores
 * it in the provided intersection graph.
 * 
 * @param graph Pointer to the other graph.
 * @param intersection Pointer to the graph where the intersection will be stored.
 */
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

/**
 * @brief Checks if there is an edge between two nodes specified by their indices.
 * 
 * @param fromIndex The index of the starting node.
 * @param toIndex The index of the ending node.
 * @return bool True if there is an edge, false otherwise.
 */
bool Graph::hasEdge(int fromIndex, int toIndex) const {
    if (getNode(fromIndex)->getNeighbors().size() > getNode(toIndex)->getNeighbors().size()) {
        int temp = fromIndex;
        fromIndex = toIndex;
        toIndex = temp;
    }
    const Node* node = getNode(fromIndex);
    for (const Node* neighbor : node->getNeighbors())
        if (neighbor->getIndex() == toIndex)
            return true;
    return false;
}

/**
 * @brief Checks if the graph is connected.
 * 
 * @return bool true if the graph is connected, false otherwise.
 */
bool Graph::isConnected() const {
    std::vector<bool> visited(size(), false);
    std::list<int> queue{};
    queue.push_back(0);
    visited[0] = true;
    while (queue.size() > 0) {
        int nodeIndex = queue.front();
        queue.pop_front();
        const Node* node = getNode(nodeIndex);
        for (const Node* neighbor : node->getNeighbors()) {
            int neighborIndex = neighbor->getIndex();
            if (!visited[neighborIndex]) {
                visited[neighborIndex] = true;
                queue.push_back(neighborIndex);
            }
        }
    }
    for (bool visit : visited)
        if (!visit) return false;
    return true;
}

/**
 * @brief Gets the number of edges in the graph.
 * 
 * @return int The number of edges.
 */
int Graph::numberOfEdges() const {
    int edges = 0;
    for (int i = 0; i < size(); ++i)
        edges += getNode(i)->getNeighbors().size();
    return edges/2;
}

/**
 * @brief Constructs a SubGraph with a given number of nodes and an original graph.
 * 
 * @param numberOfNodes The number of nodes in the subgraph.
 * @param graph Pointer to the original graph.
 */
SubGraph::SubGraph(const int numberOfNodes, const Graph* graph) 
: Graph(numberOfNodes), originalNodes_m(numberOfNodes), originalGraph_m(graph) {
    assert(numberOfNodes <= graph->size());
}

/**
 * @brief Gets the original node corresponding to a node in the subgraph.
 * 
 * @param node Pointer to the node in the subgraph.
 * @return const Node* Pointer to the original node.
 */
const Node* SubGraph::getOriginalNode(const Node* node) const {
    const int index = node->getIndex();
    return originalNodes_m.getPointer(index);
}

/**
 * @brief Sets the original node corresponding to a node in the subgraph.
 * 
 * @param node Pointer to the node in the subgraph.
 * @param originalNode Pointer to the original node.
 */
void SubGraph::setOriginalNode(const Node* node, const Node* originalNode) {
    const int index = node->getIndex();
    originalNodes_m.setPointer(index, originalNode);
}

/**
 * @brief Prints the subgraph to the standard output.
 */
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