#ifndef PALM_TREE_H
#define PALM_TREE_H

#include <vector>
#include <memory>

#include "../basic/graph.hpp"
#include "../basic/tree.hpp"
#include "../basic/utils.hpp"

class PalmTree : public Tree {
private:
    const Graph& m_originalGaph;
    std::vector<int> m_numberOfDescendants;
    std::vector<int> m_lowPoint1;
    std::vector<int> m_lowPoint2;
    std::vector<Container<Edge>> m_fronds;
    std::vector<int> m_newIndexToOldIndex;
    std::vector<int> m_oldIndexToNewIndex;
    
    void setOldIndex(Node& node, int oldIndex);
    void dfsBuildPalmTree(const Node& node, bool flag[], Node& palmTreeNode);
    void setLowPoint1(Node& node, int lowPoint1);
    void setLowPoint2(Node& node, int lowPoint2);
public:
    PalmTree(const Graph& graph);
    std::string toString() const override;
    void addFrond(Node& from, Node& to, double weight);
    const Container<Edge>& getFronds(const Node& node) const;
    int getLowPoint1(const Node& node) const;
    int getLowPoint2(const Node& node) const;
};

#endif