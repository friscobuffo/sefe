#include "cycle.hpp"

#include <cassert>

void BlackCycle::dfsBuildCycle(const Node& node, bool isNodeVisited[], const Node* prev) {
    nodes_m.push_back(&node);
    isNodeVisited[node.getIndex()] = true;
    for (const auto& edge : node.getEdges()) {
        if (&edge.to == prev)
            continue;
        if (edge.color != Color::BLACK)
            continue;
        if (!isNodeVisited[edge.to.getIndex()]) {
            dfsBuildCycle(edge.to, isNodeVisited, &node);
            break;
        }
        nodes_m.push_back(&edge.to);
        return;
    }
}

void BlackCycle::cleanupCycle() {
    std::vector<const Node*> copyNodes(nodes_m);
    nodes_m.clear();
    bool foundEnd{false};
    for (int i = 0; i < copyNodes.size()-1; ++i) {
        if (foundEnd)
            nodes_m.push_back(copyNodes[i]);
        else if (copyNodes[i] == copyNodes.back()) {
            nodes_m.push_back(copyNodes[i]);
            foundEnd = true;
        }
    }
}

BlackCycle::BlackCycle(const SubGraph& component)
: originalComponent_m(component) {
    assert(component.size() > 2);
    int size = component.size();
    bool isNodeVisited[size];
    for (int node = 0; node < size; ++node)
        isNodeVisited[node] = false;
    dfsBuildCycle(component.getNode(0), isNodeVisited, nullptr);
    cleanupCycle();
    posInCycle_m.resize(size);
    for (int i = 0; i < size; ++i)
        posInCycle_m[i] = -1;
    int index = 0;
    for (const Node* node : nodes_m)
        posInCycle_m[node->getIndex()] = index++;
}

void BlackCycle::reverse() {
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
void BlackCycle::changeWithPath(const std::list<const Node*>& path, const Node* nodeToInclude) {
    std::list<const Node*> nodesCopy(path); // newCycleList
    const Node* firstOfPath = path.front();
    const Node* lastOfPath = path.back();
    int i = getPositionOfNode(*lastOfPath).value();
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

bool BlackCycle::hasNode(const Node& node) const {
    return getPositionOfNode(node).has_value();
}

int BlackCycle::size() const {
    return nodes_m.size();
}

const Node& BlackCycle::getPrevOfNode(const Node& node) const {
    int pos = posInCycle_m[node.getIndex()];
    assert(pos != -1);
    prevIndex(pos);
    return *nodes_m[pos];
}

const Node& BlackCycle::getNextOfNode(const Node& node) const {
    int pos = posInCycle_m[node.getIndex()];
    assert(pos != -1);
    nextIndex(pos);
    return *nodes_m[pos];
}

void BlackCycle::nextIndex(int& index) const {
    ++index;
    index %= nodes_m.size();
}

void BlackCycle::prevIndex(int& index) const {
    --index;
    if (index < 0)
        index = nodes_m.size()-1;
}

const Node& BlackCycle::getNode(int position) const {
    assert(position >= 0 && position < size());
    return *nodes_m[position];
}

void BlackCycle::print() const {
    std::cout << "cycle: [ ";
    for (int i = 0; i < size(); ++i) {
        const Node& node = getNode(i);
        const int label = originalComponent_m.getOriginalNode(node).getIndex();
        std::cout << label << " ";
    }
    for (const Node* node : nodes_m) {
    }
    std::cout << "]\n";
}

int BlackCycle::getOriginalComponentSize() const {
    return originalComponent_m.size();
}

std::optional<int> BlackCycle::getPositionOfNode(const Node& node) const {
    int pos = posInCycle_m[node.getIndex()];
    if (pos != -1)
        return pos;
    return std::nullopt;
}