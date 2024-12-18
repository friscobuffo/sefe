#ifndef MY_BICONNECTED_COMPONENT_H
#define MY_BICONNECTED_COMPONENT_H

#include <vector>
#include <list>
#include <utility>
#include <memory>

#include "../basic/graph.hpp"

class BiconnectedComponentsHandler {
private:
    const Graph& originalGraph_m;
    std::vector<bool> isCutVertex_m{};
    std::vector<int> cutVertices_m{};
    std::vector<std::unique_ptr<SubGraph>> components_m{};
    void dfsBicCom(const Node& node, int nodeId[], const Node* prevOfNode[], int& nextIdToAssign, int lowPoint[],
        std::list<const Node*>& nodes, std::list<std::pair<const Node*, const Edge*>>& edges);
    void buildComponent(SubGraph& compnent, std::list<const Node*>& nodes, std::list<std::pair<const Node*, const Edge*>>& edges);
public:
    BiconnectedComponentsHandler(const Graph& graph);
    void print() const;
    int size() const;
    const SubGraph& getComponent(int index) const;
};

#endif