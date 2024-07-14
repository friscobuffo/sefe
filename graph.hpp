#ifndef MY_GRAPH_H
#define MY_GRAPH_H

#include <vector>
#include <optional>
#include <memory>

#include "utils.hpp"

class Graph;

class Node {
private:
    const int index_m;
    std::vector<const Node*> neighbors_m;
    const Graph* graph_m;
public:
    Node(const int index, const Graph* graph);
    int getIndex() const;
    const std::vector<const Node*>& getNeighbors() const;
    std::vector<const Node*>& getNeighbors();
    void addNeighbor(const Node* neighbor);
    const Graph* getGraph() const;
};

class Graph {
private:
    bool bfsBipartition(int node, std::vector<int>& bipartition) const;
protected:
    std::vector<std::unique_ptr<Node>> nodes_m;
public:
    Graph(const int numberOfNodes);
    const Node* getNode(const int index) const;
    Node* getNode(const int index);
    void addEdge(Node* from, Node* to);
    void addEdge(const int fromIndex, const int toIndex);
    int size() const;
    virtual void print() const;
    const std::optional<std::vector<int>> computeBipartition() const;
    const Graph* computeIntersection(const Graph* graph) const;
};

class SubGraph : public Graph {
private:
    const Graph* originalGraph_m;
    ArrayPointers<const Node> originalNodes_m;
public:
    SubGraph(const int numberOfNodes, const Graph* graph);
    const Node* getOriginalNode(const Node* node) const;
    void setOriginalNode(const Node* node, const Node* originalNode);
    void print() const override;
};

#endif