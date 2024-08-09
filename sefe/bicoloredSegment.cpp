#include "bicoloredSegment.hpp"

#include <iostream>
#include <cassert>

BicoloredSegment::BicoloredSegment(const int numberOfNodes, const BicoloredSegment* higherLevel, const IntersectionCycle* cycle)
: BicoloredGraph(numberOfNodes), higherLevel_m(higherLevel),
originalCycle_m(cycle), higherLevelNodesPointers_m(numberOfNodes), originalNodesPointers_m(numberOfNodes) {
    isNodeAnAttachment_m.resize(numberOfNodes);
    for (int i = 0; i < numberOfNodes; ++i) {
        isNodeAnAttachment_m[i] = false;
    }
}

const BicoloredSegment* BicoloredSegmentsHandler::buildChord(const NodeWithColors* attachment1, const NodeWithColors* attachment2, const Color color) {
    BicoloredSegment* chord = new BicoloredSegment(originalCycle_m->size(), higherLevel_m, originalCycle_m);
    // assigning labels
    // first nodes MUST be the same of the cycle in the SAME ORDER
    for (int i = 0; i < originalCycle_m->size(); ++i) {
        const NodeWithColors* oldNode = originalCycle_m->getNode(i);
        const NodeWithColors* newNode = chord->getNode(i);
        chord->setHigherLevelNode(newNode, oldNode);
        const NodeWithColors* originalNode = higherLevel_m->getOriginalNode(oldNode);
        chord->setOriginalNode(newNode, originalNode);
    }
    // adding cycle edges
    for (int i = 0; i < originalCycle_m->size()-1; ++i)
        chord->addEdge(i, i+1, Color::BOTH);
    chord->addEdge(0, originalCycle_m->size()-1, Color::BOTH);
    // adding chord edge
    std::optional<int> fromIndex = originalCycle_m->getPositionOfNode(attachment1);
    std::optional<int> toIndex = originalCycle_m->getPositionOfNode(attachment2);
    assert(fromIndex);
    assert(toIndex);
    chord->addEdge(fromIndex.value(), toIndex.value(), color);
    chord->addAttachment(chord->getNode(fromIndex.value()));
    chord->addAttachment(chord->getNode(toIndex.value()));
    return chord;
}

const BicoloredSegment* BicoloredSegmentsHandler::getSegment(const int index) const {
    return segments_m[index].get();
}

// computes a path between two attachments, assuring it does not go trought the cycle
std::list<const NodeWithColors*> BicoloredSegment::computeBlackPathBetweenAttachments(const NodeWithColors* start, const NodeWithColors* end) const {
    assert(isNodeAnAttachment(start));
    assert(isNodeAnAttachment(end));
    const NodeWithColors* prevOfNode[size()];
    for (int i = 0; i < size(); ++i)
        prevOfNode[i] = nullptr;
    std::list<const NodeWithColors*> queue{};
    queue.push_back(start);
    while (queue.size() != 0) {
        const NodeWithColors* node = queue.front();
        queue.pop_front();
        const NodeWithColors* higherLevelNode = getHigherLevelNode(node);
        for (const Edge& edge : node->getEdges()) {
            if (edge.color != Color::BOTH) continue;
            const NodeWithColors* neighbor = edge.node;
            const NodeWithColors* higherLevelNeighbor = getHigherLevelNode(neighbor);
            if (originalCycle_m->hasNode(higherLevelNode) && originalCycle_m->hasNode(higherLevelNeighbor))
                continue;
            if (prevOfNode[neighbor->getIndex()] == nullptr) {
                prevOfNode[neighbor->getIndex()] = node;
                queue.push_back(neighbor);
                if (neighbor == end) break;
            }
        }
        if (prevOfNode[end->getIndex()] != nullptr) break;
    }
    std::list<const NodeWithColors*> path{};
    const NodeWithColors* crawl = end;
    while (crawl != start) {
        path.push_front(crawl);
        crawl = prevOfNode[crawl->getIndex()];
    }
    path.push_front(crawl);
    return path;
}

void BicoloredSegmentsHandler::print() const {
    for (int i = 0; i < size(); ++i) {
        std::cout << "segment [" << i << "]\n";
        const BicoloredSegment* segment = getSegment(i);
        segment->print();
    }
}

int BicoloredSegmentsHandler::size() const {
    return segments_m.size();
}

void BicoloredSegment::addAttachment(const NodeWithColors* attachment) {
    if (isNodeAnAttachment(attachment)) return;
    isNodeAnAttachment_m[attachment->getIndex()] = true;
    attachmentNodes_m.push_back(attachment);
}

bool BicoloredSegment::isNodeAnAttachment(const NodeWithColors* node) const {
    return isNodeAnAttachment_m[node->getIndex()];
}

const std::vector<const NodeWithColors*>& BicoloredSegment::getAttachments() const {
    return attachmentNodes_m;
}

// returns true if the segment is just a path inside (or outside) a cycle,
// false otherwise
bool BicoloredSegment::isPath() const {
    for (int i = 0; i < size(); ++i) {
        const NodeWithColors* node = getNode(i);
        if (isNodeAnAttachment(node)) continue;
        if (node->getEdges().size() > 2)
            return false;
    }
    return true;
}

const IntersectionCycle* BicoloredSegment::getOriginalCycle() const {
    return originalCycle_m;
}

const BicoloredSegment* BicoloredSegment::getHigherLevel() const {
    return higherLevel_m;
}

const NodeWithColors* BicoloredSegment::getHigherLevelNode(const NodeWithColors* node) const {
    return higherLevelNodesPointers_m.getPointer(node->getIndex());
}

void BicoloredSegment::setHigherLevelNode(const NodeWithColors* node, const NodeWithColors* componentNode) {
    higherLevelNodesPointers_m.setPointer(node->getIndex(), componentNode);
}

BicoloredSegmentsHandler::BicoloredSegmentsHandler(const BicoloredSegment* bicoloredSegment, const IntersectionCycle* cycle)
: higherLevel_m(bicoloredSegment), originalCycle_m(cycle) {
    findSegments();
    findChords();
}

void BicoloredSegmentsHandler::findChords() {
    for (int i = 0; i < originalCycle_m->size(); ++i) {
        const NodeWithColors* node = originalCycle_m->getNode(i);
        for (const Edge& edge : node->getEdges()) {
            const NodeWithColors* neighbor = edge.node;
            if (node->getIndex() < neighbor->getIndex()) continue;
            if (originalCycle_m->hasNode(neighbor))
                if (neighbor != originalCycle_m->getPrevOfNode(node) && neighbor != originalCycle_m->getNextOfNode(node)) {
                    const BicoloredSegment* chord = buildChord(node, neighbor, edge.color);
                    segments_m.push_back(std::unique_ptr<const BicoloredSegment>(chord));
                }
        }
    }
}

void BicoloredSegmentsHandler::findSegments() {
    int size = higherLevel_m->size();
    bool isNodeVisited[size];
    for (int i = 0; i < size; ++i)
        isNodeVisited[i] = false;
    for (int i = 0; i < size; ++i)
        if (originalCycle_m->hasNode(higherLevel_m->getNode(i)))
            isNodeVisited[i] = true;
    for (int i = 0; i < size; ++i) {
        const NodeWithColors* node = higherLevel_m->getNode(i);
        if (!isNodeVisited[i]) {
            std::vector<const NodeWithColors*> nodes{}; // does NOT contain cycle nodes
            std::vector<std::pair<const NodeWithColors*, const Edge>> edges{}; // does NOT contain edges of the cycle
            dfsFindSegments(node, isNodeVisited, nodes, edges);
            const BicoloredSegment* segment = buildSegment(nodes, edges);
            segments_m.push_back(std::unique_ptr<const BicoloredSegment>(segment));
        }
    }
}

void BicoloredSegmentsHandler::dfsFindSegments(const NodeWithColors* node, bool isNodeVisited[],
std::vector<const NodeWithColors*>& nodesInSegment,
std::vector<std::pair<const NodeWithColors*, const Edge>>& edgesInSegment) {
    int nodeIndex = node->getIndex();
    nodesInSegment.push_back(node);
    isNodeVisited[nodeIndex] = true;
    for (const Edge& edge : node->getEdges()) {
        const NodeWithColors* neighbor = edge.node;
        int neighborIndex = neighbor->getIndex();
        if (originalCycle_m->hasNode(neighbor)) {
            edgesInSegment.push_back(std::make_pair(node, Edge{neighbor, edge.color}));
            continue;
        }
        if (nodeIndex < neighborIndex)
            edgesInSegment.push_back(std::make_pair(node, Edge{neighbor, edge.color}));
        if (!isNodeVisited[neighborIndex])
            dfsFindSegments(neighbor, isNodeVisited, nodesInSegment, edgesInSegment);
    }
}

// nodes vector does NOT contain cycle nodes
// edges vector does NOT contain cycle edges
const BicoloredSegment* BicoloredSegmentsHandler::buildSegment(std::vector<const NodeWithColors*>& nodes,
std::vector<std::pair<const NodeWithColors*, const Edge>>& edges) {
    BicoloredSegment* segment = new BicoloredSegment(nodes.size()+originalCycle_m->size(), higherLevel_m, originalCycle_m);
    // first nodes MUST be the same of the cycle in the SAME ORDER
    int oldToNewLabel[higherLevel_m->size()];
    // adding cycle nodes (which means setting up the pointers)
    for (int i = 0; i < originalCycle_m->size(); ++i) {
        const NodeWithColors* cycleNode = originalCycle_m->getNode(i);
        oldToNewLabel[cycleNode->getIndex()] = i;
        const NodeWithColors* newNode = segment->getNode(i);
        segment->setHigherLevelNode(newNode, cycleNode);
        const NodeWithColors* originalNode = higherLevel_m->getOriginalNode(cycleNode);
        segment->setOriginalNode(newNode, originalNode);
    }
    // adding the other nodes of the segment (setting up the pointers)
    for (int i = 0; i < nodes.size(); ++i) { // remember that nodes does not include cycle nodes
        const NodeWithColors* oldNode = nodes[i];
        int index = i+originalCycle_m->size();
        oldToNewLabel[oldNode->getIndex()] = index;
        const NodeWithColors* newNode = segment->getNode(index);
        segment->setHigherLevelNode(newNode, oldNode);
        const NodeWithColors* originalNode = higherLevel_m->getOriginalNode(oldNode);
        segment->setOriginalNode(newNode, originalNode);
    }
    // adding edges
    for (std::pair<const NodeWithColors*, const Edge>& edge : edges) {
        int fromIndex = oldToNewLabel[edge.first->getIndex()];
        int toIndex = oldToNewLabel[edge.second.node->getIndex()];
        Color color = edge.second.color;
        segment->addEdge(fromIndex, toIndex, color);
        // adding attachment
        if (originalCycle_m->hasNode(edge.first))
            segment->addAttachment(segment->getNode(fromIndex));
        if (originalCycle_m->hasNode(edge.second.node))
            segment->addAttachment(segment->getNode(toIndex));
    }
    // adding cycle edges
    for (int i = 0; i < originalCycle_m->size()-1; ++i)
        segment->addEdge(i, i+1, Color::BOTH);
    segment->addEdge(0, originalCycle_m->size()-1, Color::BOTH);
    return segment;
}