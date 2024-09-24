#ifndef MY_CYCLE_H
#define MY_CYCLE_H

#include <vector>
#include <list>
#include <optional>

#include "../basic/graph.hpp"

/**
 * @class Cycle
 * @brief Represents a cycle within a subgraph.
 *
 * The Cycle class encapsulates the concept of a cycle in a graph, providing
 * methods to manipulate and query the cycle.
 *
 * @details This class maintains a list of nodes that form the cycle and their
 * positions within the cycle. It provides methods to build the cycle using
 * depth-first search, modify the cycle by including a path, and query various
 * properties of the cycle.
 */
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
    void changeWithPath(std::list<const Node*>& path, const Node* nodeToInclude);
    bool hasNode(const Node* node) const;
    int size() const;
    const Node* getPrevOfNode(const Node* node) const;
    const Node* getNextOfNode(const Node* node) const;
    const Node* getNode(const int position) const;
    void reverse();
    void print() const;
    int getOriginalComponentSize() const;
    std::optional<int> getPositionOfNode(const Node* node) const;
};

#endif
