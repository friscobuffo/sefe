#ifndef MY_BICONNECTED_COMPONENT_H
#define MY_BICONNECTED_COMPONENT_H

#include <vector>
#include <list>
#include <utility>

#include "graph.hpp"

class BiconnectedComponentsHandler {
private:
    const Graph* originalGraph_m;
    std::vector<bool> isCutVertex_m{};
    std::vector<int> cutVertices_m{};
    std::vector<std::unique_ptr<const SubGraph>> components_m{};
    void dfsBicCom(const Node* node, int nodeId[], const Node* prevOfNode[], int& nextIdToAssign, int lowPoint[],
        std::list<const Node*>& nodes, std::list<std::pair<const Node*, const Node*>>& edges);
    SubGraph* buildComponent(std::list<const Node*>& nodes, std::list<std::pair<const Node*, const Node*>>& edges);
    void componentCheck(const SubGraph* component);
public:
    BiconnectedComponentsHandler(const Graph* graph);
    void print() const;
    int size() const;
    const SubGraph* getComponent(int index) const;
};

#endif