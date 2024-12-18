#ifndef MY_CYCLE_H
#define MY_CYCLE_H

#include <vector>
#include <list>
#include <optional>

#include "../basic/graph.hpp"

class BlackCycle {
private:
    std::vector<const Node*> nodes_m{};
    std::vector<int> posInCycle_m{};
    const SubGraph& originalComponent_m;
    void dfsBuildCycle(const Node& node, bool isNodeVisited[], const Node* prev);
    void cleanupCycle();
    void nextIndex(int& index) const;
    void prevIndex(int& index) const;
public:
    BlackCycle(const SubGraph& component);
    void changeWithPath(const std::list<const Node*>& path, const Node* nodeToInclude);
    bool hasNode(const Node& node) const;
    int size() const;
    const Node& getPrevOfNode(const Node& node) const;
    const Node& getNextOfNode(const Node& node) const;
    const Node& getNode(int position) const;
    void print() const;
    void reverse();
    int getOriginalComponentSize() const;
    std::optional<int> getPositionOfNode(const Node& node) const;
};

#endif