#include "segment.hpp"

#include <iostream>
#include <cassert>

#include "utils.hpp"

Segment::Segment(int numberOfNodes, const SubGraph* component, const Cycle* cycle)
: SubGraph(numberOfNodes, component), originalComponent_m(component),
originalCycle_m(cycle), componentNodesPointers_m(numberOfNodes) {
    isNodeAnAttachment_m.resize(numberOfNodes);
    for (int i = 0; i < numberOfNodes; ++i) {
        isNodeAnAttachment_m[i] = false;
    }
}

void Segment::addAttachment(const Node* attachment) {
    if (isNodeAnAttachment(attachment)) return;
    isNodeAnAttachment_m[attachment->getIndex()] = true;
    attachmentNodes_m.push_back(attachment);
}

bool Segment::isNodeAnAttachment(const Node* node) const {
    return isNodeAnAttachment_m[node->getIndex()];
}

bool Segment::isPath() const {
    for (int i = 0; i < size(); ++i) {
        const Node* node = getNode(i);
        if (isNodeAnAttachment(node)) continue;
        if (node->getNeighbors().size() > 2)
            return false;
    }
    return true;
}

const std::vector<const Node*>& Segment::getAttachments() const {
    return attachmentNodes_m;
}

std::list<const Node*> Segment::computePathBetweenAttachments(const Node* start, const Node* end) const {
    assert(isNodeAnAttachment(start));
    assert(isNodeAnAttachment(end));
    const Node* prevOfNode[size()];
    for (int i = 0; i < size(); ++i)
        prevOfNode[i] = nullptr;
    std::list<const Node*> queue{};
    queue.push_back(start);
    while (queue.size() != 0) {
        const Node* node = queue.front();
        queue.pop_front();
        const Node* componentNode = getComponentNode(node);
        for (const Node* neighbor : node->getNeighbors()) {
            const Node* componentNeighbor = getComponentNode(neighbor);
            if (originalCycle_m->hasNode(componentNode) && originalCycle_m->hasNode(componentNeighbor))
                continue;
            if (prevOfNode[neighbor->getIndex()] == nullptr) {
                prevOfNode[neighbor->getIndex()] = node;
                queue.push_back(neighbor);
                if (neighbor == end) break;
            }
        }
        if (prevOfNode[end->getIndex()] != nullptr) break;
    }
    std::list<const Node*> path{};
    const Node* crawl = end;
    while (crawl != start) {
        path.push_front(crawl);
        crawl = prevOfNode[crawl->getIndex()];
    }
    path.push_front(crawl);
    return path;
}

const Cycle* Segment::getOriginalCycle() const {
    return originalCycle_m;
}

const SubGraph* Segment::getOriginalComponent() const {
    return originalComponent_m;
}

const Node* Segment::getComponentNode(const Node* node) const {
    return componentNodesPointers_m.getNodePointer(node->getIndex());
}

void Segment::setComponentNode(const Node* node, const Node* componentNode) {
    componentNodesPointers_m.setNodePointer(node->getIndex(), componentNode);
}

void SegmentsHandler::segmentCheck(const Segment* segment) {
    for (int i = 0; i < segment->size(); ++i) {
        const Node* node = segment->getNode(i);
        const Node* nodeComponent = segment->getComponentNode(node);
        assert(nodeComponent->getGraph() == originalComponent_m);
        assert(segment->getOriginalNode(node) == originalComponent_m->getOriginalNode(nodeComponent));
    }
}

SegmentsHandler::SegmentsHandler(const SubGraph* component, const Cycle* cycle)
: originalComponent_m(component), originalCycle_m(cycle) {
    findSegments();
    findChords();
    for (int i = 0; i < size(); ++i)
        segmentCheck(getSegment(i));
}

void SegmentsHandler::dfsFindSegments(const Node* node, bool isNodeVisited[], std::vector<const Node*>& nodesInSegment,
std::vector<std::pair<const Node*, const Node*>>& edgesInSegment) {
    int nodeIndex = node->getIndex();
    nodesInSegment.push_back(node);
    isNodeVisited[nodeIndex] = true;
    for (const Node* neighbor : node->getNeighbors()) {
        int neighborIndex = neighbor->getIndex();
        if (originalCycle_m->hasNode(neighbor)) {
            edgesInSegment.push_back(std::make_pair(node, neighbor));
            continue;
        }
        if (nodeIndex < neighborIndex)
            edgesInSegment.push_back(std::make_pair(node, neighbor));
        if (!isNodeVisited[neighborIndex])
            dfsFindSegments(neighbor, isNodeVisited, nodesInSegment, edgesInSegment);
    }
}

void SegmentsHandler::findChords() {
    for (int i = 0; i < originalCycle_m->size(); ++i) {
        const Node* node = originalCycle_m->getNode(i);
        for (const Node* neighbor : node->getNeighbors()) {
            if (node->getIndex() < neighbor->getIndex()) continue;
            if (originalCycle_m->hasNode(neighbor))
                if (neighbor != originalCycle_m->getPrevOfNode(node) && neighbor != originalCycle_m->getNextOfNode(node)) {
                    const Segment* chord = buildChord(node, neighbor);
                    segments_m.push_back(std::unique_ptr<const Segment>(chord));
                }
        }
    }
}

void SegmentsHandler::findSegments() {
    int size = originalComponent_m->size();
    bool isNodeVisited[size];
    for (int i = 0; i < size; ++i)
        isNodeVisited[i] = false;
    for (int i = 0; i < size; ++i)
        if (originalCycle_m->hasNode(originalComponent_m->getNode(i)))
            isNodeVisited[i] = true;
    for (int i = 0; i < size; ++i) {
        const Node* node = originalComponent_m->getNode(i);
        if (!isNodeVisited[i]) {
            std::vector<const Node*> nodes{}; // does NOT contain cycle nodes
            std::vector<std::pair<const Node*, const Node*>> edges{}; // does NOT contain edges of the cycle
            dfsFindSegments(node, isNodeVisited, nodes, edges);
            const Segment* segment = buildSegment(nodes, edges);
            segments_m.push_back(std::unique_ptr<const Segment>(segment));
        }
    }
}

// nodes vector does NOT contain cycle nodes
// edges vector does NOT contain cycle edges
const Segment* SegmentsHandler::buildSegment(std::vector<const Node*>& nodes,
std::vector<std::pair<const Node*, const Node*>>& edges) {
    Segment* segment = new Segment(nodes.size()+originalCycle_m->size(), originalComponent_m, originalCycle_m);
    // assigning labels
    // first nodes MUST be the same of the cycle in the SAME ORDER
    int oldToNewLabel[originalComponent_m->size()];
    for (int i = 0; i < originalCycle_m->size(); ++i) {
        const Node* cycleNode = originalCycle_m->getNode(i);
        oldToNewLabel[cycleNode->getIndex()] = i;
        const Node* newNode = segment->getNode(i);
        segment->setComponentNode(newNode, cycleNode);
        const Node* originalNode = originalComponent_m->getOriginalNode(cycleNode);
        segment->setOriginalNode(newNode, originalNode);
    }
    for (int i = 0; i < nodes.size(); ++i) { // remember that nodes does not include cycle nodes
        const Node* oldNode = nodes[i];
        int index = i+originalCycle_m->size();
        oldToNewLabel[oldNode->getIndex()] = index;
        const Node* newNode = segment->getNode(index);
        segment->setComponentNode(newNode, oldNode);
        const Node* originalNode = originalComponent_m->getOriginalNode(oldNode);
        segment->setOriginalNode(newNode, originalNode);
    }
    // adding edges
    for (std::pair<const Node*, const Node*>& edge : edges) {
        int fromIndex = oldToNewLabel[edge.first->getIndex()];
        int toIndex = oldToNewLabel[edge.second->getIndex()];
        segment->addEdge(fromIndex, toIndex);
        // adding attachment
        if (originalCycle_m->hasNode(edge.first))
            segment->addAttachment(segment->getNode(fromIndex));
        if (originalCycle_m->hasNode(edge.second))
            segment->addAttachment(segment->getNode(toIndex));
    }
    // adding cycle edges
    for (int i = 0; i < originalCycle_m->size()-1; ++i)
        segment->addEdge(i, i+1);
    segment->addEdge(0, originalCycle_m->size()-1);
    return segment;
}

const Segment* SegmentsHandler::buildChord(const Node* attachment1, const Node* attachment2) {
    Segment* chord = new Segment(originalCycle_m->size(), originalComponent_m, originalCycle_m);
    // assigning labels
    // first nodes MUST be the same of the cycle in the SAME ORDER
    for (int i = 0; i < originalCycle_m->size(); ++i) {
        const Node* oldNode = originalCycle_m->getNode(i);
        const Node* newNode = chord->getNode(i);
        chord->setComponentNode(newNode, oldNode);
        const Node* originalNode = originalComponent_m->getOriginalNode(oldNode);
        chord->setOriginalNode(newNode, originalNode);
    }
    // adding cycle edges
    for (int i = 0; i < originalCycle_m->size()-1; ++i)
        chord->addEdge(i, i+1);
    chord->addEdge(0, originalCycle_m->size()-1);
    // adding chord edge
    std::optional<int> fromIndex = originalCycle_m->getPositionOfNode(attachment1);
    std::optional<int> toIndex = originalCycle_m->getPositionOfNode(attachment2);
    assert(fromIndex);
    assert(toIndex);
    chord->addEdge(fromIndex.value(), toIndex.value());
    chord->addAttachment(chord->getNode(fromIndex.value()));
    chord->addAttachment(chord->getNode(toIndex.value()));
    return chord;
}

const Segment* SegmentsHandler::getSegment(int index) const {
    return segments_m[index].get();
}

void SegmentsHandler::print() const {
    for (int i = 0; i < size(); ++i) {
        std::cout << "segment [" << i << "]\n";
        const Segment* segment = getSegment(i);
        segment->print();
    }
}

int SegmentsHandler::size() const {
    return segments_m.size();
}