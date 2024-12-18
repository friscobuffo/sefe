#include "segment.hpp"

#include <iostream>
#include <cassert>

Segment::Segment(const SubGraph* higherLevel, const BlackCycle* cycle)
: SubGraph(&higherLevel->getOriginalGraph()), higherLevel_m(*higherLevel), originalCycle_m(*cycle) {
    assert(size() == 0);
}

bool Segment::isNodeAnAttachment(const Node& node) const {
    return attachmentColor_m[node.getIndex()] != Color::NONE;
}

bool Segment::isNodeRedAttachment(const Node& node) const {
    Color color = attachmentColor_m[node.getIndex()];
    switch (color) {
        case Color::BLACK: return true;
        case Color::RED_AND_BLUE: return true;
        case Color::RED: return true;
        default: return false;
    }
}

bool Segment::isNodeBlueAttachment(const Node& node) const {
    Color color = attachmentColor_m[node.getIndex()];
    switch (color) {
        case Color::BLACK: return true;
        case Color::RED_AND_BLUE: return true;
        case Color::BLUE: return true;
        default: return false;
    }
}

bool Segment::isNodeBlackAttachment(const Node& node) const {
    return attachmentColor_m[node.getIndex()] == Color::BLACK;
}

bool Segment::isNodeAttachmentOfColor(const Node& node, Color color) const {
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

Color Segment::getColorOfAttachment(const Node& attachment) const {
    assert(isNodeAnAttachment(attachment));
    return attachmentColor_m[attachment.getIndex()];
}


void Segment::addNode(Color color) {
    SubGraph::addNode(color);
    attachmentColor_m.push_back(Color::NONE);
}

bool Segment::isPath() const {
    for (const Node& node : getNodes()) {
        if (isNodeAnAttachment(node)) continue;
        if (node.getEdges().size() > 2)
            return false;
    }
    return true;
}

const std::vector<const Node*>& Segment::getAttachments() const {
    return attachmentNodes_m;
}

void Segment::addAttachment(const Node& attachment, Color color) {
    if (isNodeBlackAttachment(attachment)) return;
    if (!isNodeAnAttachment(attachment))
        attachmentNodes_m.push_back(&attachment);
    int index = attachment.getIndex();
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

std::list<const Node*> Segment::computePathBetweenAttachments(const Node& start, const Node& end, Color color) const {
    assert(isNodeAnAttachment(start));
    assert(isNodeAnAttachment(end));
    const Node* prevOfNode[size()];
    for (int i = 0; i < size(); ++i)
        prevOfNode[i] = nullptr;
    std::list<const Node*> queue{};
    queue.push_back(&start);
    while (queue.size() != 0) {
        const Node* node = queue.front();
        queue.pop_front();
        const Node& higherLevelNode = getHigherLevelNode(*node);
        for (const Edge& edge : node->getEdges()) {
            if (color != Color::ANY && edge.color != color) continue;
            const Node& neighbor = edge.to;
            const Node& higherLevelNeighbor = getHigherLevelNode(neighbor);
            if (&neighbor == &end) {
                if (node == &start) continue;
                prevOfNode[neighbor.getIndex()] = node;
                break;
            }
            if (originalCycle_m.hasNode(higherLevelNeighbor))
                continue;
            if (prevOfNode[neighbor.getIndex()] == nullptr) {
                prevOfNode[neighbor.getIndex()] = node;
                queue.push_back(&neighbor);
            }
        }
        if (prevOfNode[end.getIndex()] != nullptr) break;
    }
    std::list<const Node*> path{};
    const Node* crawl = &end;
    while (crawl != &start) {
        path.push_front(crawl);
        crawl = prevOfNode[crawl->getIndex()];
    }
    path.push_front(crawl);
    return path;
}

const BlackCycle& Segment::getOriginalCycle() const {
    return originalCycle_m;
}

const SubGraph& Segment::getHigherLevel() const {
    return higherLevel_m;
}

const Node& Segment::getHigherLevelNode(const Node& node) const {
    return *higherLevelNodesPointers_m.getPointer(node.getIndex());
}

void Segment::setHigherLevelNode(const Node& node, const Node& higherLevelNode) {
    higherLevelNodesPointers_m.setPointer(node.getIndex(), &higherLevelNode);
}

SegmentsHandler::SegmentsHandler(const SubGraph* higherLevel, const BlackCycle* cycle)
: higherLevel_m(*higherLevel), originalCycle_m(*cycle) {
    findSegments();
    findChords();
}

void SegmentsHandler::dfsFindSegments(const Node& node, bool isNodeVisited[], std::vector<const Node*>& nodesInSegment,
std::vector<std::pair<const Node*, const Edge*>>& edgesInSegment) {
    int nodeIndex = node.getIndex();
    nodesInSegment.push_back(&node);
    isNodeVisited[nodeIndex] = true;
    for (const Edge& edge : node.getEdges()) {
        const Node& neighbor = edge.to;
        int neighborIndex = neighbor.getIndex();
        if (originalCycle_m.hasNode(neighbor)) {
            edgesInSegment.push_back(std::make_pair(&node, &edge));
            continue;
        }
        if (nodeIndex < neighborIndex)
            edgesInSegment.push_back(std::make_pair(&node, &edge));
        if (!isNodeVisited[neighborIndex])
            dfsFindSegments(neighbor, isNodeVisited, nodesInSegment, edgesInSegment);
    }
}

void SegmentsHandler::findChords() {
    for (int i = 0; i < originalCycle_m.size(); ++i) {
        const Node& node = originalCycle_m.getNode(i);
        for (const Edge& edge : node.getEdges()) {
            const Node& neighbor = edge.to;
            if (node.getIndex() < neighbor.getIndex()) continue;
            if (originalCycle_m.hasNode(neighbor))
                if (&neighbor != &originalCycle_m.getPrevOfNode(node) && &neighbor != &originalCycle_m.getNextOfNode(node)) {
                    segments_m.push_back(std::make_unique<Segment>(&higherLevel_m, &originalCycle_m));
                    Segment& chord = *segments_m.back();
                    buildChord(chord, node, neighbor, edge.color);
                }
        }
    }
}

void SegmentsHandler::findSegments() {
    int size = higherLevel_m.size();
    bool isNodeVisited[size];
    for (int i = 0; i < size; ++i)
        isNodeVisited[i] = false;
    for (int i = 0; i < size; ++i)
        if (originalCycle_m.hasNode(higherLevel_m.getNode(i)))
            isNodeVisited[i] = true;
    for (int i = 0; i < size; ++i) {
        const Node& node = higherLevel_m.getNode(i);
        if (!isNodeVisited[i]) {
            std::vector<const Node*> nodes{}; // does NOT contain cycle nodes
            std::vector<std::pair<const Node*, const Edge*>> edges{}; // does NOT contain edges of the cycle
            dfsFindSegments(node, isNodeVisited, nodes, edges);
            segments_m.push_back(std::make_unique<Segment>(&higherLevel_m, &originalCycle_m));
            Segment& segment = *segments_m.back();
            buildSegment(segment, nodes, edges);
        }
    }
}

void SegmentsHandler::buildSegment(Segment& segment, std::vector<const Node*>& nodes,
std::vector<std::pair<const Node*, const Edge*>>& edges) {
    assert(segment.size() == 0);
    int numberOfNodes = nodes.size()+originalCycle_m.size();
    for (int i = 0; i < numberOfNodes; ++i)
        segment.addNode();
    // first nodes MUST be the same of the cycle in the SAME ORDER
    int oldToNewLabel[higherLevel_m.size()];
    // adding cycle nodes (which means setting up the pointers)
    for (int i = 0; i < originalCycle_m.size(); ++i) {
        const Node& cycleNode = originalCycle_m.getNode(i);
        oldToNewLabel[cycleNode.getIndex()] = i;
        const Node& newNode = segment.getNode(i);
        segment.setHigherLevelNode(newNode, cycleNode);
        const Node& originalNode = higherLevel_m.getOriginalNode(cycleNode);
        segment.setOriginalNode(newNode, originalNode);
    }
    // adding the other nodes of the segment (setting up the pointers)
    for (int i = 0; i < nodes.size(); ++i) { // remember that nodes does not include cycle nodes
        const Node* oldNode = nodes[i];
        int index = i+originalCycle_m.size();
        oldToNewLabel[oldNode->getIndex()] = index;
        const Node& newNode = segment.getNode(index);
        segment.setHigherLevelNode(newNode, *oldNode);
        const Node& originalNode = higherLevel_m.getOriginalNode(*oldNode);
        segment.setOriginalNode(newNode, originalNode);
    }
    // adding edges
    for (std::pair<const Node*, const Edge*>& edge : edges) {
        int fromIndex = oldToNewLabel[edge.first->getIndex()];
        int toIndex = oldToNewLabel[edge.second->to.getIndex()];
        segment.addEdge(fromIndex, toIndex, edge.second->weight, edge.second->color);
        // adding attachment
        if (originalCycle_m.hasNode(*edge.first))
            segment.addAttachment(segment.getNode(fromIndex), edge.second->color);
        if (originalCycle_m.hasNode(edge.second->to))
            segment.addAttachment(segment.getNode(toIndex), edge.second->color);
    }
    // adding cycle edges
    for (int i = 0; i < originalCycle_m.size()-1; ++i)
        segment.addEdge(i, i+1, 1.0, Color::BLACK);
    segment.addEdge(0, originalCycle_m.size()-1, 1.0, Color::BLACK);
}

void SegmentsHandler::buildChord(Segment& chord, const Node& attachment1, const Node& attachment2, Color color) {
    for (int i = 0; i < originalCycle_m.size(); ++i)
        chord.addNode();
    // assigning labels
    // first nodes MUST be the same of the cycle in the SAME ORDER
    for (int i = 0; i < originalCycle_m.size(); ++i) {
        const Node& oldNode = originalCycle_m.getNode(i);
        const Node& newNode = chord.getNode(i);
        chord.setHigherLevelNode(newNode, oldNode);
        const Node& originalNode = higherLevel_m.getOriginalNode(oldNode);
        chord.setOriginalNode(newNode, originalNode);
    }
    // adding cycle edges
    for (int i = 0; i < originalCycle_m.size()-1; ++i)
        chord.addEdge(i, i+1, 1.0, Color::BLACK);
    chord.addEdge(0, originalCycle_m.size()-1, 1.0, Color::BLACK);
    // adding chord edge
    std::optional<int> fromIndex = originalCycle_m.getPositionOfNode(attachment1);
    std::optional<int> toIndex = originalCycle_m.getPositionOfNode(attachment2);
    assert(fromIndex);
    assert(toIndex);
    chord.addEdge(fromIndex.value(), toIndex.value(), 1.0, color);
    chord.addAttachment(chord.getNode(fromIndex.value()), color);
    chord.addAttachment(chord.getNode(toIndex.value()), color);
}

const Segment& SegmentsHandler::getSegment(const int index) const {
    return *segments_m[index];
}

void SegmentsHandler::print() const {
    for (int i = 0; i < size(); ++i) {
        std::cout << "segment [" << i << "]\n";
        const Segment& segment = getSegment(i);
        segment.print();
    }
}

int SegmentsHandler::size() const {
    return segments_m.size();
}