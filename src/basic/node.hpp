#ifndef MY_NODE_H
#define MY_NODE_H

#include <vector>
#include <string>

#include "color.hpp"
#include "utils.hpp"

class NodesStructure;

class Edge;

class Node {
private:
    int m_index;
    Container<Edge> m_edges{};
    Color m_color;
    const NodesStructure* m_owner;
public:
    Node(int index, Color color, NodesStructure& owner);
    int getIndex() const;
    const Container<Edge>& getEdges() const;
    Container<Edge>& getEdges();
    void addEdge(Node& neighbor, double weight, Color color);
    const NodesStructure& getOwner() const;
    int degree() const;
    Color getColor() const;
};

struct Edge {
    Node& from;
    Node& to;
    double weight;
    Color color;
};

class NodesStructure {
protected:
    Container<Node> m_nodes{};
public:
    const Node& getNode(int index) const;
    Node& getNode(int index);
    const Container<Node>& getNodes() const;
    Container<Node>& getNodes();
    int size() const;
    virtual std::string toString() const = 0;
    void print() const;
};

#endif