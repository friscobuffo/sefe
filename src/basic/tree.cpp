#include "tree.hpp"

#include <sstream>
#include <cassert>

#include "color.hpp"
#include <iostream>

Node& Tree::addChild(Node& parent, double weight) {    
    assert(&parent.getOwner() == this);
    int index = m_nodes.size();
    m_nodes.addElement(std::make_unique<Node>(index, Color::BLACK, *this));
    Node& child = getNode(index);
    m_parents.emplace_back(&parent);
    parent.addEdge(child, weight, Color::BLACK);
    return child;
}

const Node* Tree::getParent(const Node& node) const {
    assert(&node.getOwner() == this);
    return m_parents[node.getIndex()];
}

Tree::Tree() {
    m_nodes.addElement(std::make_unique<Node>(0, Color::BLACK, *this));
    m_parents.push_back(nullptr);
}

std::string Tree::toString() const {
    std::ostringstream result;
    for (const Node& node : getNodes()) {
        result << "Node " << node.getIndex() << " has children: ";
        for (const Edge& edge : node.getEdges())
            result << edge.to.getIndex() << " ";
        result << "\n";
    }
    return result.str();
}

Node& Tree::getRoot() {
    return getNode(0);
}

const Node& Tree::getRoot() const {
    return getNode(0);
}