#ifndef MY_INTERSECTION_CYCLE_H
#define MY_INTERSECTION_CYCLE_H

#include <vector>
#include <list>
#include <optional>

#include "utils.hpp"
#include "bicoloredGraph.hpp"

class IntersectionCycle {
private:
    std::vector<const NodeWithColors*> nodes_m{};
    std::vector<int> posInCycle_m{};
    const BicoloredGraph* originalBicoloredGraph_m;
    void dfsBuildCycle(const NodeWithColors* node, bool isNodeVisited[], const NodeWithColors* prev);
    void cleanupCycle();
    void nextIndex(int& index);
public:
    IntersectionCycle(const BicoloredGraph* graph);
    void changeWithPath(std::list<const NodeWithColors*>& path);
    bool hasNode(const NodeWithColors* node) const;
    int size() const;
    const NodeWithColors* getPrevOfNode(const NodeWithColors* node) const;
    const NodeWithColors* getNextOfNode(const NodeWithColors* node) const;
    const NodeWithColors* getNode(const int position) const;
    void reverse();
    void print() const;
    int getOriginalBicoloredGraphSize() const;
    std::optional<int> getPositionOfNode(const NodeWithColors* node) const;
};

#endif