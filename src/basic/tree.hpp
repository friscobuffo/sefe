#ifndef MY_TREE_H
#define MY_TREE_H

#include <string>
#include <vector>

#include "node.hpp"

class Tree : public NodesStructure {
private:
    std::vector<const Node*> m_parents{};
public:
    Tree();
    Node& addChild(Node& parent, double weight);
    const Node* getParent(const Node& node) const;
    std::string toString() const override;
    Node& getRoot();
    const Node& getRoot() const;
};

#endif