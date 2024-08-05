#include "intersectionCycle.hpp"

#include <cassert>

#include "utils.hpp"

// assumes intersection of graph is biconnected
IntersectionCycle::IntersectionCycle(const BicoloredGraph* graph) 
: originalBicoloredGraph_m(graph) {
    int size = graph->size();
    bool isNodeVisited[size];
    for (int node = 0; node < size; ++node)
        isNodeVisited[node] = false;
    dfsBuildCycle(graph->getNode(0), isNodeVisited, nullptr);
    cleanupCycle();
    posInCycle_m.resize(size);
    for (int i = 0; i < size; ++i)
        posInCycle_m[i] = -1;
    int index = 0;
    for (const NodeWithColors* node : nodes_m) {
        posInCycle_m[node->getIndex()] = index;
        ++index;
    }
}

void IntersectionCycle::dfsBuildCycle(const NodeWithColors* node, bool isNodeVisited[], const NodeWithColors* prev) {
    nodes_m.push_back(node);
    isNodeVisited[node->getIndex()] = true;
    for (const Edge& neighbor : node->getEdges()) {
        if (neighbor.node == prev)
            continue;
        if (!isNodeVisited[neighbor.node->getIndex()]) {
            dfsBuildCycle(neighbor.node, isNodeVisited, node);
            break;
        }
        nodes_m.push_back(neighbor.node);
        return;
    }
}

// removes leading nodes of the list that do not partecipate in the cycle
// assumes starting node of the cycle is contained twice
// (at the start and at the end of the actual cycle)
void IntersectionCycle::cleanupCycle() {
    std::vector<const NodeWithColors*> copy(nodes_m);
    nodes_m.clear();
    bool foundEnd{false};
    for (int i = 0; i < copy.size(); ++i) {
        if (foundEnd) nodes_m.push_back(copy[i]);
        else if (copy[i] == copy.back()) foundEnd = true;
    }
}

// the list of nodes must contain, at the end, a node contained in the cycle
// the new cycle becomes: nodes in the path + nodes in the cycle after the last path's node
void IntersectionCycle::changeWithPath(std::list<const NodeWithColors*>& path) {
    std::list<const NodeWithColors*> nodesCopy(path); // newCycleList
    const NodeWithColors* firstOfPath = path.front();
    const NodeWithColors* lastOfPath = path.back();
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

bool IntersectionCycle::hasNode(const NodeWithColors* node) const {
    return posInCycle_m[node->getIndex()] != -1;
}

int IntersectionCycle::size() const {
    return nodes_m.size();
}

const NodeWithColors* IntersectionCycle::getPrevOfNode(const NodeWithColors* node) const {
    int pos = posInCycle_m[node->getIndex()];
    assert(pos != -1);
    if (pos == 0) return nodes_m[size()-1];
    return nodes_m[pos-1];
}

const NodeWithColors* IntersectionCycle::getNextOfNode(const NodeWithColors* node) const {
    int pos = posInCycle_m[node->getIndex()];
    assert(pos != -1);
    if (pos == size()-1) return nodes_m[0];
    return nodes_m[pos+1];
}

void IntersectionCycle::nextIndex(int& index) {
    ++index;
    index %= nodes_m.size();
}

const NodeWithColors* IntersectionCycle::getNode(const int position) const {
    return nodes_m[position];
}

void IntersectionCycle::reverse() {
    int start = 0;
    int end = nodes_m.size()-1;
    while (start < end) {
        const NodeWithColors* temp = nodes_m[start];
        nodes_m[start] = nodes_m[end];
        nodes_m[end] = temp;
        posInCycle_m[nodes_m[start]->getIndex()] = start;
        posInCycle_m[nodes_m[end]->getIndex()] = end;
        ++start;
        --end;
    }
}

int IntersectionCycle::getOriginalBicoloredGraphSize() const {
    return originalBicoloredGraph_m->size();
}

std::optional<int> IntersectionCycle::getPositionOfNode(const NodeWithColors* node) const {
    int pos = posInCycle_m[node->getIndex()];
    if (pos != -1) return pos;
    return std::nullopt;
}