#include "biconnectedComponent.hpp"

#include <iostream>
#include <cassert>

#include "../basic/utils.hpp"

/**
 * @brief Prints the biconnected components and cut vertices to the standard output.
 */
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

/**
 * @brief Returns the number of biconnected components.
 * 
 * @return int The number of biconnected components.
 */
int BiconnectedComponentsHandler::size() const {
    return components_m.size();
}

/**
 * @brief Retrieves a biconnected component by its index.
 *
 * This function returns a pointer to a SubGraph object representing
 * the biconnected component at the specified index.
 *
 * @param index The index of the biconnected component to retrieve.
 * @return A pointer to the SubGraph object corresponding to the specified index.
 */
const SubGraph* BiconnectedComponentsHandler::getComponent(const int index) const {
    return components_m[index].get();
}

/**
 * @brief Constructs a subgraph component from the given nodes and edges.
 *
 * This function creates a new SubGraph object using the provided list of nodes and edges.
 * It maps the nodes from the original graph to the new subgraph and adds the corresponding edges.
 * 
 * @note Assumes each edge node is in nodes list
 *
 * @param nodes A list of pointers to the nodes that will be included in the subgraph.
 * @param edges A list of pairs of pointers to nodes representing the edges that will be included in the subgraph.
 * @return A pointer to the newly created SubGraph component.
 */
SubGraph* BiconnectedComponentsHandler::buildComponent(std::list<const Node*>& nodes,
std::list<std::pair<const Node*, const Node*>>& edges) {
    SubGraph* component = new SubGraph(nodes.size(), originalGraph_m);
    int oldToNewNodes[originalGraph_m->size()];
    int newIndex = 0;
    for (const Node* node : nodes) {
        int oldIndex = node->getIndex();
        oldToNewNodes[node->getIndex()] = newIndex;
        const Node* nodeComponent = component->getNode(newIndex);
        const Node* nodeOriginalGraph = originalGraph_m->getNode(oldIndex);
        component->setOriginalNode(nodeComponent, nodeOriginalGraph);
        ++newIndex;
    }
    for (std::pair<const Node*, const Node*>& edge: edges) {
        int fromIndex = oldToNewNodes[edge.first->getIndex()];
        int toIndex = oldToNewNodes[edge.second->getIndex()];
        component->addEdge(fromIndex, toIndex);
    }
    return component;
}

/**
 * @brief Constructs a BiconnectedComponentsHandler object for the given graph.
 *
 * This constructor initializes the handler by performing a depth-first search (DFS)
 * to identify biconnected components and cut vertices in the provided graph.
 *
 * @param graph A pointer to the Graph object for which biconnected components are to be identified.
 */
BiconnectedComponentsHandler::BiconnectedComponentsHandler(const Graph* graph) : originalGraph_m(graph) {
    int graphSize = graph->size();
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
    std::list<std::pair<const Node*, const Node*>> stackOfEdges{};
    for (int i = 0; i < graphSize; i++)
        if (nodeId[i] == -1) // node not visited
            dfsBicCom(graph->getNode(i), nodeId, prevOfNode, nextIdToAssign, lowPoint, stackOfNodes, stackOfEdges);
    assert(stackOfNodes.size() == 0);
    assert(stackOfEdges.size() == 0);
    for (int node = 0; node < graph->size(); ++node)
        if (isCutVertex_m[node]) cutVertices_m.push_back(node);
}

/**
 * @brief Performs a Depth-First Search (DFS) to find biconnected components in a graph.
 *
 * This function is a part of the BiconnectedComponentsHandler class and is used to identify
 * biconnected components within a graph. It uses DFS traversal to explore the graph and 
 * determine the low points and articulation points (cut vertices).
 *
 * @param node The current node being visited.
 * @param nodeId Array storing the discovery times of nodes.
 * @param prevOfNode Array storing the parent nodes in the DFS tree.
 * @param nextIdToAssign Reference to the next discovery time to assign.
 * @param lowPoint Array storing the lowest discovery time reachable from each node.
 * @param stackOfNodes Stack of nodes used to build biconnected components.
 * @param stackOfEdges Stack of edges used to build biconnected components.
 */
void BiconnectedComponentsHandler::dfsBicCom(const Node* node, int nodeId[],
const Node* prevOfNode[], int& nextIdToAssign, int lowPoint[], std::list<const Node*>& stackOfNodes,
std::list<std::pair<const Node*, const Node*>>& stackOfEdges) {
    int nodeIndex = node->getIndex();
    nodeId[nodeIndex] = nextIdToAssign;
    lowPoint[nodeIndex] = nextIdToAssign;
    ++nextIdToAssign;
    int childrenNumber = 0;
    for (const Node* neighbor : node->getNeighbors()) {
        int neighborIndex = neighbor->getIndex();
        if (prevOfNode[nodeIndex] == neighbor)
            continue;
        if (nodeId[neighborIndex] == -1) { // means node is not visited
            std::list<const Node*> newStackOfNodes{};
            std::list<std::pair<const Node*, const Node*>> newStackOfEdges{};
            ++childrenNumber;
            prevOfNode[neighborIndex] = node;
            newStackOfNodes.push_back(neighbor);
            newStackOfEdges.push_back(std::make_pair(node, neighbor));
            dfsBicCom(neighbor, nodeId, prevOfNode, nextIdToAssign, lowPoint, newStackOfNodes, newStackOfEdges);
            if (lowPoint[neighborIndex] < lowPoint[nodeIndex])
                lowPoint[nodeIndex] = lowPoint[neighborIndex];
            if (lowPoint[neighborIndex] >= nodeId[nodeIndex]) {
                newStackOfNodes.push_back(node);
                const SubGraph* component = buildComponent(newStackOfNodes, newStackOfEdges);
                components_m.push_back(std::unique_ptr<const SubGraph>(component));
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
                stackOfEdges.push_back(std::make_pair(node, neighbor));
                if (neighborNodeId < lowPoint[nodeIndex])
                    lowPoint[nodeIndex] = neighborNodeId;
            }
        }
    }
    if (prevOfNode[nodeIndex] == nullptr) { // handling of node with no parents (the root)
        if (childrenNumber >= 2)
            isCutVertex_m[nodeIndex] = true;
        else if (childrenNumber == 0) { // node is isolated
            SubGraph* component = new SubGraph(1, originalGraph_m);
            component->setOriginalNode(0, node);
            components_m.push_back(std::unique_ptr<const SubGraph>(component));
        }
    }
}
