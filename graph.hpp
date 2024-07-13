#ifndef MY_GRAPH_H
#define MY_GRAPH_H

#include <vector>
#include <optional>
#include <memory>

class Graph;

class Node {
private:
    int index_m;
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
};

class NodePointers {
private:
    std::vector<const Node*> pointers_m;
public:
    NodePointers(const int numberOfNodes);
    const Node* getNodePointer(const int index) const;
    void setNodePointer(const int index, const Node* node);
};

class SubGraph : public Graph {
private:
    const Graph* originalGraph_m;
    NodePointers originalNodes_m;
public:
    SubGraph(const int numberOfNodes, const Graph* graph);
    const Node* getOriginalNode(const Node* node) const;
    void setOriginalNode(const Node* node, const Node* originalNode);
    void print() const override;
};

#endif