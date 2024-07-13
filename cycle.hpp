#ifndef MY_CYCLE_H
#define MY_CYCLE_H

#include <vector>
#include <list>
#include <optional>

#include "biconnectedComponent.hpp"

class Cycle {
private:
    std::vector<const Node*> nodes_m{};
    std::vector<int> posInCycle_m{};
    const SubGraph* originalComponent_m;
    void dfsBuildCycle(const Node* node, bool isNodeVisited[], const Node* prev);
    void cleanupCycle();
    void nextIndex(int& index);
public:
    Cycle(const SubGraph* component);
    void changeWithPath(std::list<const Node*>& path);
    bool hasNode(const Node* node) const;
    int size() const;
    const Node* getPrevOfNode(const Node* node) const;
    const Node* getNextOfNode(const Node* node) const;
    const Node* getNode(int position) const;
    void reverse();
    void print() const;
    int getOriginalComponentSize() const;
    std::optional<int> getPositionOfNode(const Node* node) const;
};

#endif