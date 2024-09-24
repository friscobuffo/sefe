#include "cycle.hpp"

#include <cassert>

/**
 * @brief Performs a depth-first search (DFS) to build a cycle starting from the given node.
 *
 * This function recursively traverses the graph to build a cycle. It starts from the given node,
 * marks it as visited, and explores its neighbors. If a neighbor has not been visited, the function
 * continues the DFS from that neighbor. If a neighbor has been visited and is not the previous node,
 * it indicates the presence of a cycle, and the neighbor is added to the cycle.
 *
 * @param node The current node being visited.
 * @param isNodeVisited An array indicating whether each node has been visited.
 * @param prev The previous node in the DFS traversal.
 */
void Cycle::dfsBuildCycle(const Node* node, bool isNodeVisited[], const Node* prev) {
    nodes_m.push_back(node);
    isNodeVisited[node->getIndex()] = true;
    for (const Node* neighbor : node->getNeighbors()) {
        if (neighbor == prev)
            continue;
        if (!isNodeVisited[neighbor->getIndex()]) {
            dfsBuildCycle(neighbor, isNodeVisited, node);
            break;
        }
        nodes_m.push_back(neighbor);
        return;
    }
}

/**
 * @brief Cleans up the cycle by removing leading nodes of the list that do not
 * partecipate in the cycle. Assumes starting node of the cycle is contained twice.
 * 
 * @example If the original list is [1, 2, 3, 4, 5, 3], the cleaned-up list will be [3, 4, 5, 3].
 * 
 * @note This function assumes that the last node in the list is the end of the cycle.
 */
void Cycle::cleanupCycle() {
    std::vector<const Node*> copy(nodes_m);
    nodes_m.clear();
    bool foundEnd{false};
    for (int i = 0; i < copy.size(); ++i) {
        if (foundEnd) nodes_m.push_back(copy[i]);
        else if (copy[i] == copy.back()) foundEnd = true;
    }
}


/**
 * @brief Constructs a Cycle object from a given SubGraph component. Assumes subgraph
 * is a biconnected component.
 *
 * @param component A pointer to the SubGraph component from which the cycle is constructed.
 */
Cycle::Cycle(const SubGraph* component) : originalComponent_m(component) {
    int size = component->size();
    bool isNodeVisited[size];
    for (int node = 0; node < size; ++node)
        isNodeVisited[node] = false;
    dfsBuildCycle(component->getNode(0), isNodeVisited, nullptr);
    cleanupCycle();
    posInCycle_m.resize(size);
    for (int i = 0; i < size; ++i)
        posInCycle_m[i] = -1;
    int index = 0;
    for (const Node* node : nodes_m) {
        posInCycle_m[node->getIndex()] = index;
        ++index;
    }
}

/**
 * @brief Modifies the cycle by incorporating a given path. The starting and ending node
 * of the path must be contained in the cycle. It substitutes one of the two paths in
 * the cycle linking those two nodes. If given a node to include, the path removed from
 * the cycle is the one that does not contain the node to include.
 *
 * @param path A list of nodes representing the path to be incorporated into the cycle.
 * @param nodeToInclude A pointer to the node that must be included in the cycle. If nullptr, 
 *                      the function proceeds without checking for this node.
 */
void Cycle::changeWithPath(std::list<const Node*>& path, const Node* nodeToInclude) {
    std::list<const Node*> nodesCopy(path); // newCycleList
    const Node* firstOfPath = path.front();
    const Node* lastOfPath = path.back();
    int i = 0;
    while (nodes_m[i] != lastOfPath)
        nextIndex(i);
    nextIndex(i);
    bool foundNodeToInclude = false;
    if (nodeToInclude == nullptr) foundNodeToInclude = true;
    while (nodes_m[i] != firstOfPath) {
        nodesCopy.push_back(nodes_m[i]);
        if (nodes_m[i] == nodeToInclude) foundNodeToInclude = true;
        nextIndex(i);
    }
    if (!foundNodeToInclude) {
        reverse();
        changeWithPath(path, nodeToInclude);
        return;
    }
    nodes_m.clear();
    for (int i = 0; i < posInCycle_m.size(); ++i)
        posInCycle_m[i] = -1;
    i = 0;
    while (nodesCopy.size() > 0) {
        nodes_m.push_back(nodesCopy.back());
        posInCycle_m[nodesCopy.back()->getIndex()] = i;
        nodesCopy.pop_back();
        ++i;
    }
}

/**
 * @brief Checks if a given node is part of the cycle.
 *
 * @param node A pointer to the Node object to be checked.
 * @return true if the node is part of the cycle, false otherwise.
 */
bool Cycle::hasNode(const Node* node) const {
    return posInCycle_m[node->getIndex()] != -1;
}

/**
 * @brief Returns the number of nodes in the cycle.
 * 
 * @return int The number of nodes in the cycle.
 */
int Cycle::size() const {
    return nodes_m.size();
}

/**
 * @brief Retrieves the node that precedes the given node in the cycle.
 * If the specified node is the first node in the cycle, the function returns
 * the last node in the cycle.
 *
 * @param node A pointer to the node for which the previous node is to be found.
 * @return A pointer to the previous node in the cycle.
 * @throws std::assert if the given node is not in the cycle.
 */
const Node* Cycle::getPrevOfNode(const Node* node) const {
    int pos = posInCycle_m[node->getIndex()];
    assert(pos != -1);
    if (pos == 0) return nodes_m[size()-1];
    return nodes_m[pos-1];
}

/**
 * @brief Retrieves the node that follows the given node in the cycle.
 * If the specified node is the last node in the cycle, the function returns
 * the first node in the cycle.
 *
 * @param node A pointer to the node for which the next node is to be found.
 * @return A pointer to the next node in the cycle.
 * @throws std::assert if the given node is not in the cycle.
 */
const Node* Cycle::getNextOfNode(const Node* node) const {
    int pos = posInCycle_m[node->getIndex()];
    assert(pos != -1);
    if (pos == size()-1) return nodes_m[0];
    return nodes_m[pos+1];
}

/**
 * @brief Advances the given index to the next position in a cyclic manner.
 *
 * This function increments the provided index and wraps it around to the 
 * beginning if it exceeds the size of the nodes_m container, ensuring that 
 * the index always remains within valid bounds.
 *
 * @param index Reference to the index to be advanced.
 */
void Cycle::nextIndex(int& index) {
    ++index;
    index %= nodes_m.size();
}

/**
 * @brief Retrieves a node from the cycle at the specified position.
 * 
 * @param position The index of the node to retrieve.
 * @return const Node* A pointer to the node at the specified position.
 */
const Node* Cycle::getNode(const int position) const {
    return nodes_m[position];
}

/**
 * @brief Prints the cycle information to the standard output.
 * 
 * This function outputs the cycle in the format "cycle: [ <node1> <node2> ... <nodeN> ]",
 * where each node is represented by its index. The indices are retrieved from the 
 * original component associated with the cycle.
 */
void Cycle::print() const {
    std::cout << "cycle: [ ";
    for (const Node* node : nodes_m) {
        const int label = originalComponent_m->getOriginalNode(node)->getIndex();
        std::cout << label << " ";
    }
    std::cout << "]\n";
}

/**
 * @brief Reverses the order of nodes in the cycle.
 */
void Cycle::reverse() {
    int start = 0;
    int end = nodes_m.size()-1;
    while (start < end) {
        const Node* temp = nodes_m[start];
        nodes_m[start] = nodes_m[end];
        nodes_m[end] = temp;
        posInCycle_m[nodes_m[start]->getIndex()] = start;
        posInCycle_m[nodes_m[end]->getIndex()] = end;
        ++start;
        --end;
    }
}

/**
 * @brief Retrieves the size of the original component associated with the Cycle object.
 * 
 * @return int The size of the original component.
 */
int Cycle::getOriginalComponentSize() const {
    return originalComponent_m->size();
}

/**
 * @brief Retrieves the position of a given node in the cycle.
 *
 * This function returns the position of the specified node within the cycle.
 * If the node is not found in the cycle, it returns std::nullopt.
 *
 * @param node A pointer to the Node whose position is to be retrieved.
 * @return std::optional<int> The position of the node in the cycle, 
 *          or std::nullopt if the node is not in the cycle.
 */
std::optional<int> Cycle::getPositionOfNode(const Node* node) const {
    int pos = posInCycle_m[node->getIndex()];
    if (pos != -1) return pos;
    return std::nullopt;
}
