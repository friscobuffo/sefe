#ifndef MY_GRAPH_H
#define MY_GRAPH_H

#include <vector>
#include <optional>
#include <memory>

class Graph;

class Node {
private:
    int index_m;
    std::vector<Node*> neighbors_m;
    const Graph* graph_m;
public:
    Node(int index, const Graph* graph);
    int getIndex() const;
    const std::vector<Node*>& getNeighbors() const;
    std::vector<Node*>& getNeighbors();
    void addNeighbor(Node* neighbor);
    const Graph* getGraph() const;
};

class Graph {
private:
    bool bfsBipartition(int node, std::vector<int>& bipartition) const;
protected:
    std::vector<std::unique_ptr<Node>> nodes_m;
public:
    Graph(int numberOfNodes);
    const Node* getNode(int index) const;
    Node* getNode(int index);
    void addEdge(Node* from, Node* to);
    void addEdge(int fromIndex, int toIndex);
    int size() const;
    virtual void print() const;
    const std::optional<std::vector<int>> computeBipartition();
};

class NodePointers {
private:
    std::vector<const Node*> pointers_m;
public:
    NodePointers(int numberOfNodes);
    const Node* getNodePointer(int index) const;
    void setNodePointer(int index, const Node* node);
};

class SubGraph : public Graph {
private:
    const Graph* originalGraph_m;
    NodePointers originalNodes_m;
public:
    SubGraph(int numberOfNodes, const Graph* graph);
    const Node* getOriginalNode(const Node* node) const;
    void setOriginalNode(const Node* node, const Node* originalNode);
    void print() const override;
};

#endif