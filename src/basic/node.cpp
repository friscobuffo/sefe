#include "node.hpp"

#include <iostream>

Node::Node(int index, Color color, NodesStructure& owner)
: m_index(index), m_color(color), m_owner(&owner) {}

int Node::getIndex() const {
    return m_index;
}

Container<Edge>& Node::getEdges() {
    return m_edges;
}

const Container<Edge>& Node::getEdges() const {
    return m_edges;
}

void Node::addEdge(Node& neighbor, double weight, Color color) {
    assert(&neighbor.getOwner() == m_owner);
    Edge* edge = new Edge{*this, neighbor, weight, color};
    m_edges.addElement(std::unique_ptr<Edge>(edge));
}

const NodesStructure& Node::getOwner() const {
    return *m_owner;
}

int Node::degree() const {
    return m_edges.size();
}

Color Node::getColor() const {
    return m_color;
}

const Container<Node>& NodesStructure::getNodes() const {
    return m_nodes;
}

Container<Node>& NodesStructure::getNodes() {
    return m_nodes;
}

const Node& NodesStructure::getNode(int index) const {
    assert(index >= 0);
    assert(index < size());
    return m_nodes[index];
}

Node& NodesStructure::getNode(int index) {
    assert(index >= 0);
    assert(index < size());
    return m_nodes[index];
}

int NodesStructure::size() const {
    return m_nodes.size();
}

void NodesStructure::print() const {
    std::cout << toString();
}