#ifndef MY_BICONNECTED_COMPONENT_H
#define MY_BICONNECTED_COMPONENT_H

#include <vector>
#include <list>
#include <utility>
#include <memory>

#include "../basic/graph.hpp"

/**
 * @class BiconnectedComponentsHandler
 * @brief Handles the computation and storage of biconnected components in a graph.
 *
 * This class is responsible for identifying and managing biconnected components
 * within a given graph. It uses the Tarjan's algorithm to identify the components in O(n+m).
 */
class BiconnectedComponentsHandler {
private:
    const Graph* originalGraph_m;
    std::vector<bool> isCutVertex_m{};
    std::vector<int> cutVertices_m{};
    std::vector<std::unique_ptr<const SubGraph>> components_m{};
    void dfsBicCom(const Node* node, int nodeId[], const Node* prevOfNode[], int& nextIdToAssign, int lowPoint[],
        std::list<const Node*>& nodes, std::list<std::pair<const Node*, const Node*>>& edges);
    SubGraph* buildComponent(std::list<const Node*>& nodes, std::list<std::pair<const Node*, const Node*>>& edges);
public:
    BiconnectedComponentsHandler(const Graph* graph);
    void print() const;
    int size() const;
    const SubGraph* getComponent(const int index) const;
};

#endif