#include "cycle.hpp"

#include <cassert>

#include "../basic/utils.hpp"

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

// removes leading nodes of the list that do not partecipate in the cycle
// assumes starting node of the cycle is contained twice
// (at the start and at the end of the actual cycle)
void Cycle::cleanupCycle() {
    std::vector<const Node*> copy(nodes_m);
    nodes_m.clear();
    bool foundEnd{false};
    for (int i = 0; i < copy.size(); ++i) {
        if (foundEnd) nodes_m.push_back(copy[i]);
        else if (copy[i] == copy.back()) foundEnd = true;
    }
}

// assumes subgraph is a biconnected component
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

// the list of nodes must contain, at the end, a node contained in the cycle
// the new cycle becomes: nodes in the path + nodes in the cycle after the last path's node
void Cycle::changeWithPath(std::list<const Node*>& path) {
    std::list<const Node*> nodesCopy(path); // newCycleList
    const Node* firstOfPath = path.front();
    const Node* lastOfPath = path.back();
    int i = 0;
    while (nodes_m[i] != lastOfPath)
        nextIndex(i);
    nextIndex(i);
    while (nodes_m[i] != firstOfPath) {
        nodesCopy.push_back(nodes_m[i]);
        nextIndex(i);
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

bool Cycle::hasNode(const Node* node) const {
    return posInCycle_m[node->getIndex()] != -1;
}

int Cycle::size() const {
    return nodes_m.size();
}

const Node* Cycle::getPrevOfNode(const Node* node) const {
    int pos = posInCycle_m[node->getIndex()];
    assert(pos != -1);
    if (pos == 0) return nodes_m[size()-1];
    return nodes_m[pos-1];
}

const Node* Cycle::getNextOfNode(const Node* node) const {
    int pos = posInCycle_m[node->getIndex()];
    assert(pos != -1);
    if (pos == size()-1) return nodes_m[0];
    return nodes_m[pos+1];
}

void Cycle::nextIndex(int& index) {
    ++index;
    index %= nodes_m.size();
}

const Node* Cycle::getNode(const int position) const {
    return nodes_m[position];
}

void Cycle::print() const {
    std::cout << "cycle: [ ";
    for (const Node* node : nodes_m) {
        const int label = originalComponent_m->getOriginalNode(node)->getIndex();
        std::cout << label << " ";
    }
    std::cout << "]\n";
}

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

int Cycle::getOriginalComponentSize() const {
    return originalComponent_m->size();
}

std::optional<int> Cycle::getPositionOfNode(const Node* node) const {
    int pos = posInCycle_m[node->getIndex()];
    if (pos != -1) return pos;
    return std::nullopt;
}