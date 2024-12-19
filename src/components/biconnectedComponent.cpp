#include "biconnectedComponent.hpp"

#include <iostream>
#include <cassert>

#include "../basic/utils.hpp"

void BiconnectedComponentsHandler::print() const {
    std::cout << "Biconnected components:\n";
    std::cout << "Cutvertices: ";
    printIterable(cutVertices_m);
    int index = 0;
    for (auto& component : components_m) {
        std::cout << "Biconnected component [" << index << "]:\n";
        component->print();
        ++index;
    }
}

int BiconnectedComponentsHandler::size() const {
    return components_m.size();
}

const SubGraph& BiconnectedComponentsHandler::getComponent(int index) const {
    return *components_m[index];
}

void BiconnectedComponentsHandler::buildComponent(SubGraph& component, std::list<const Node*>& nodes,
std::list<std::pair<const Node*, const Edge*>>& edges) {
    for (const Node* node : nodes)
        component.addNode(node->getColor());
    int oldToNewNodes[originalGraph_m.size()];
    int newIndex = 0;
    for (const Node* node : nodes) {
        int oldIndex = node->getIndex();
        oldToNewNodes[node->getIndex()] = newIndex;
        const Node& nodeComponent = component.getNode(newIndex);
        const Node& nodeOriginalGraph = originalGraph_m.getNode(oldIndex);
        component.setOriginalNode(nodeComponent, nodeOriginalGraph);
        ++newIndex;
    }
    for (std::pair<const Node*, const Edge*>& edge: edges) {
        int fromIndex = oldToNewNodes[edge.first->getIndex()];
        int toIndex = oldToNewNodes[edge.second->to.getIndex()];
        component.addEdge(fromIndex, toIndex, edge.second->weight, edge.second->color);
    }
}

BiconnectedComponentsHandler::BiconnectedComponentsHandler(const Graph& graph) : originalGraph_m(graph) {
    int graphSize = graph.size();
    for (int i = 0; i < graphSize; ++i)
        isCutVertex_m.push_back(false);
    int nodeId[graphSize];
    const Node* prevOfNode[graphSize];
    int lowPoint[graphSize];
    for (int i = 0; i < graphSize; ++i) {
        nodeId[i] = -1;
        prevOfNode[i] = nullptr;
        lowPoint[i] = -1;
    }
    int nextIdToAssign = 0;
    std::list<const Node*> stackOfNodes{};
    std::list<std::pair<const Node*, const Edge*>> stackOfEdges{};
    for (int i = 0; i < graphSize; i++)
        if (nodeId[i] == -1) // node not visited
            dfsBicCom(graph.getNode(i), nodeId, prevOfNode, nextIdToAssign, lowPoint, stackOfNodes, stackOfEdges);
    assert(stackOfNodes.size() == 0);
    assert(stackOfEdges.size() == 0);
    for (int node = 0; node < graph.size(); ++node)
        if (isCutVertex_m[node]) cutVertices_m.push_back(node);
}

void BiconnectedComponentsHandler::dfsBicCom(const Node& node, int nodeId[],
const Node* prevOfNode[], int& nextIdToAssign, int lowPoint[], std::list<const Node*>& stackOfNodes,
std::list<std::pair<const Node*, const Edge*>>& stackOfEdges) {
    int nodeIndex = node.getIndex();
    nodeId[nodeIndex] = nextIdToAssign;
    lowPoint[nodeIndex] = nextIdToAssign;
    ++nextIdToAssign;
    int childrenNumber = 0;
    for (const Edge& edge : node.getEdges()) {
        int neighborIndex = edge.to.getIndex();
        const Node* neighbor = &edge.to;
        if (prevOfNode[nodeIndex] == neighbor)
            continue;
        if (nodeId[neighborIndex] == -1) { // means node is not visited
            std::list<const Node*> newStackOfNodes{};
            std::list<std::pair<const Node*, const Edge*>> newStackOfEdges{};
            ++childrenNumber;
            prevOfNode[neighborIndex] = &node;
            newStackOfNodes.push_back(neighbor);
            newStackOfEdges.push_back(std::make_pair(&node, &edge));
            dfsBicCom(*neighbor, nodeId, prevOfNode, nextIdToAssign, lowPoint, newStackOfNodes, newStackOfEdges);
            if (lowPoint[neighborIndex] < lowPoint[nodeIndex])
                lowPoint[nodeIndex] = lowPoint[neighborIndex];
            if (lowPoint[neighborIndex] >= nodeId[nodeIndex]) {
                newStackOfNodes.push_back(&node);
                components_m.push_back(std::make_unique<SubGraph>(&originalGraph_m));
                SubGraph& component = *components_m.back();
                buildComponent(component, newStackOfNodes, newStackOfEdges);
                if (prevOfNode[nodeIndex] != nullptr) // the root needs to be handled differently
                    // (handled at end of function)
                    isCutVertex_m[nodeIndex] = true;
            }
            else {
                stackOfNodes.splice(stackOfNodes.end(), newStackOfNodes);
                stackOfEdges.splice(stackOfEdges.end(), newStackOfEdges);
            }
        }
        else { // node got already visited
            int neighborNodeId = nodeId[neighborIndex];
            if (neighborNodeId < nodeId[nodeIndex]) {
                stackOfEdges.push_back(std::make_pair(&node, &edge));
                if (neighborNodeId < lowPoint[nodeIndex])
                    lowPoint[nodeIndex] = neighborNodeId;
            }
        }
    }
    if (prevOfNode[nodeIndex] == nullptr) { // handling of node with no parents (the root)
        if (childrenNumber >= 2)
            isCutVertex_m[nodeIndex] = true;
        else if (childrenNumber == 0) { // node is isolated
            components_m.push_back(std::make_unique<SubGraph>(&originalGraph_m));
            SubGraph& component = *components_m.back();
            component.addNode(node.getColor());
            component.setOriginalNode(component.getNode(0), node);
        }
    }
}