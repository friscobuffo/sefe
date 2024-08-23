#include "bicoloredSegment.hpp"

#include <iostream>
#include <cassert>

#include "../basic/utils.hpp"

BicoloredSegment::BicoloredSegment(const BicoloredGraph* bicoloredGraph)
: BicoloredGraph(bicoloredGraph->size()), higherLevel_m(nullptr), originalCycle_m(nullptr),
higherLevelNodesPointers_m(bicoloredGraph->size()), originalNodesPointers_m(bicoloredGraph->size()) {
    for (int i = 0; i < bicoloredGraph->size(); ++i) {
        const NodeWithColors* node = bicoloredGraph->getNode(i);
        setHigherLevelNode(getNode(i), node);
        setOriginalNode(getNode(i), node);
        for (const Edge& edge : node->getEdges())
            if (i < edge.node->getIndex())
                addEdge(i, edge.node->getIndex(), edge.color);
    }
    attachmentColor_m.resize(bicoloredGraph->size());
    for (int i = 0; i < bicoloredGraph->size(); ++i)
        attachmentColor_m[i] = Color::NONE;
}

BicoloredSegment::BicoloredSegment(const int numberOfNodes, const BicoloredSegment* higherLevel, const IntersectionCycle* cycle)
: BicoloredGraph(numberOfNodes), higherLevel_m(higherLevel),
originalCycle_m(cycle), higherLevelNodesPointers_m(numberOfNodes), originalNodesPointers_m(numberOfNodes) {
    attachmentColor_m.resize(numberOfNodes);
    for (int i = 0; i < numberOfNodes; ++i)
        attachmentColor_m[i] = Color::NONE;
}

const BicoloredSegment* BicoloredSegmentsHandler::buildChord(const NodeWithColors* attachment1,
const NodeWithColors* attachment2, const Color color) {
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
        chord->addEdge(i, i+1, Color::BLACK);
    chord->addEdge(0, originalCycle_m->size()-1, Color::BLACK);
    // adding chord edge
    std::optional<int> fromIndex = originalCycle_m->getPositionOfNode(attachment1);
    std::optional<int> toIndex = originalCycle_m->getPositionOfNode(attachment2);
    assert(fromIndex);
    assert(toIndex);
    chord->addEdge(fromIndex.value(), toIndex.value(), color);
    chord->addAttachment(chord->getNode(fromIndex.value()), color);
    chord->addAttachment(chord->getNode(toIndex.value()), color);
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
            if (edge.color != Color::BLACK) continue;
            const NodeWithColors* neighbor = edge.node;
            const NodeWithColors* higherLevelNeighbor = getHigherLevelNode(neighbor);
            if (originalCycle_m->hasNode(higherLevelNode) && originalCycle_m->hasNode(higherLevelNeighbor))
                continue;
            if (prevOfNode[neighbor->getIndex()] == nullptr) {
                prevOfNode[neighbor->getIndex()] = node;
                queue.push_back(neighbor);
                if (neighbor->getIndex() == end->getIndex()) break;
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

void BicoloredSegment::addAttachment(const NodeWithColors* attachment, const Color color) {
    if (isNodeBlackAttachment(attachment)) return;
    if (!isNodeAnAttachment(attachment))
        attachmentNodes_m.push_back(attachment);
    const int index = attachment->getIndex();
    switch (color) {
        case Color::BLACK:
            attachmentColor_m[index] = Color::BLACK;
            break;
        case Color::BLUE:
            if (isNodeRedAttachment(attachment))
                attachmentColor_m[index] = Color::RED_AND_BLUE;
            else
                attachmentColor_m[index] = Color::BLUE;
            break;
        case Color::RED:
            if (isNodeBlueAttachment(attachment))
                attachmentColor_m[index] = Color::RED_AND_BLUE;
            else
                attachmentColor_m[index] = Color::RED;
            break;
        case Color::RED_AND_BLUE:
            attachmentColor_m[index] = Color::RED_AND_BLUE;
            break;
        default:
            std::cout << "invalid color\n";
            exit(1);
    }
}

bool BicoloredSegment::isNodeAnAttachment(const NodeWithColors* node) const {
    return attachmentColor_m[node->getIndex()] != Color::NONE;
}

bool BicoloredSegment::isNodeRedAttachment(const NodeWithColors* node) const {
    Color color = attachmentColor_m[node->getIndex()];
    switch (color) {
        case Color::BLACK: return true;
        case Color::RED_AND_BLUE: return true;
        case Color::RED: return true;
        default: return false;
    }
}

bool BicoloredSegment::isNodeBlueAttachment(const NodeWithColors* node) const {
    Color color = attachmentColor_m[node->getIndex()];
    switch (color) {
        case Color::BLACK: return true;
        case Color::RED_AND_BLUE: return true;
        case Color::BLUE: return true;
        default: return false;
    }
}

bool BicoloredSegment::isNodeBlackAttachment(const NodeWithColors* node) const {
    return attachmentColor_m[node->getIndex()] == Color::BLACK;
}

bool BicoloredSegment::isNodeAttachmentOfColor(const NodeWithColors* node, const Color color) const {
    switch (color) {
        case Color::BLACK:
            return isNodeBlackAttachment(node);
        case Color::RED:
            return isNodeRedAttachment(node);
        case Color::BLUE:
            return isNodeBlueAttachment(node);
        default: assert(false);
    }
}

const int BicoloredSegment::getNumberOfAttachments() const {
    return attachmentNodes_m.size();
}

const NodeWithColors* BicoloredSegment::getAttachment(const int index) const {
    assert(index < getNumberOfAttachments());
    return attachmentNodes_m[index];
}

// returns true if the segment is just a black path
// with some red/blue edges attached to the path, false otherwise
bool BicoloredSegment::isBlackPath() const {
    for (int i = 0; i < size(); ++i) {
        const NodeWithColors* node = getNode(i);
        if (isNodeAnAttachment(node)) {
            if (node->getNumberOfBlackEdges() > 3)
                return false;
            continue;
        }
        if (node->getNumberOfBlackEdges() > 2)
            return false;
    }
    return true;
}

// returns true if the segment is just a black path
// or a chord of any color
bool BicoloredSegment::isPath() const {
    for (int i = 0; i < size(); ++i) {
        const NodeWithColors* node = getNode(i);
        if (isNodeAnAttachment(node)) {
            if (node->getEdges().size() > 3)
                return false;
            continue;
        }
        if (node->getEdges().size() > 2)
            return false;
    }
    return true;
}

const NodeWithColors* BicoloredSegment::getOriginalNode(const NodeWithColors* node) const {
    return originalNodesPointers_m.getPointer(node->getIndex());
}

void BicoloredSegment::setOriginalNode(const NodeWithColors* node, const NodeWithColors* originalNode) {
    originalNodesPointers_m.setPointer(node->getIndex(), originalNode);
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

void BicoloredSegment::setHigherLevelNode(const NodeWithColors* node, const NodeWithColors* higherLevelNode) {
    higherLevelNodesPointers_m.setPointer(node->getIndex(), higherLevelNode);
}

void BicoloredSegment::print() const {
    for (int i = 0; i < size(); ++i) {
        const NodeWithColors* node = getNode(i);
        const NodeWithColors* original = getOriginalNode(node);
        const int index = original->getIndex();
        const std::vector<Edge>& edges = node->getEdges();
        std::cout << "node: " << index << " neighbors: " << edges.size() << " [ ";
        for (const Edge& edge : edges) {
            const NodeWithColors* originalNeighbor = getOriginalNode(edge.node);
            std::cout << "(" << originalNeighbor->getIndex() << " " << color2string(edge.color) << ") ";
        }
        std::cout << "]\n";
    }
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
                if (neighbor->getIndex() != originalCycle_m->getPrevOfNode(node)->getIndex()
                && neighbor->getIndex() != originalCycle_m->getNextOfNode(node)->getIndex()) {
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
            segment->addAttachment(segment->getNode(fromIndex), color);
        if (originalCycle_m->hasNode(edge.second.node))
            segment->addAttachment(segment->getNode(toIndex), color);
    }
    // adding cycle edges
    for (int i = 0; i < originalCycle_m->size()-1; ++i)
        segment->addEdge(i, i+1, Color::BLACK);
    segment->addEdge(0, originalCycle_m->size()-1, Color::BLACK);
    return segment;
}