#include "segment.hpp"

#include <iostream>
#include <cassert>

/**
 * @brief Constructs a Segment object.
 * 
 * This constructor initializes a Segment object with the given number of nodes,
 * a pointer to a SubGraph component, and a pointer to a Cycle. It also initializes
 * the component nodes pointers and sets all nodes as non-attachment nodes.
 * 
 * @param numberOfNodes The number of nodes in the segment.
 * @param component A pointer to the SubGraph component associated with this segment.
 * @param cycle A pointer to the Cycle associated with this segment.
 */
Segment::Segment(const int numberOfNodes, const SubGraph* component, const Cycle* cycle)
: SubGraph(numberOfNodes, component), originalComponent_m(component),
originalCycle_m(cycle), componentNodesPointers_m(numberOfNodes) {
    isNodeAnAttachment_m.resize(numberOfNodes);
    for (int i = 0; i < numberOfNodes; ++i)
        isNodeAnAttachment_m[i] = false;
}

/**
 * @brief Adds a node attachment to the segment's list of attachments if it is
 * not already present.
 *
 * @param attachment A pointer to the Node object to be added as an attachment.
 */
void Segment::addAttachment(const Node* attachment) {
    if (isNodeAnAttachment(attachment)) return;
    isNodeAnAttachment_m[attachment->getIndex()] = true;
    attachmentNodes_m.push_back(attachment);
}

bool Segment::isNodeAnAttachment(const Node* node) const {
    return isNodeAnAttachment_m[node->getIndex()];
}

/**
 * @brief Checks if the segment is just a path inside (or outside) the cycle.
 *
 * @return true if the segment is a path, false otherwise.
 */
bool Segment::isPath() const {
    for (const Node* node : getNodes()) {
        if (isNodeAnAttachment(node)) continue;
        if (node->getNeighbors().size() > 2)
            return false;
    }
    return true;
}

/**
 * @brief Retrieves the attachment nodes associated with this segment.
 * 
 * This function returns a constant reference to a vector containing pointers
 * to the nodes that are attached to this segment.
 * 
 * @return A constant reference to a vector of pointers to the attachment nodes.
 */
const std::vector<const Node*>& Segment::getAttachments() const {
    return attachmentNodes_m;
}

/**
 * @brief Computes the path between two attachment nodes in the segment, assuring it
 * does not go trought the cycle.
 *
 * @param start The starting attachment node.
 * @param end The ending attachment node.
 * @return A list of nodes representing the path from the start node to the end node.
 *
 * @note The function assumes that both start and end nodes are attachment nodes
 *      and belong to the segment.
 */
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

/**
 * @brief Retrieves the original cycle associated with this segment.
 * 
 * @return const Cycle* Pointer to the original cycle.
 */
const Cycle* Segment::getOriginalCycle() const {
    return originalCycle_m;
}

/**
 * @brief Retrieves the original component associated with this segment.
 * 
 * @return const SubGraph* Pointer to the original subgraph component.
 */
const SubGraph* Segment::getOriginalComponent() const {
    return originalComponent_m;
}

/**
 * @brief Retrieves a pointer to a component node based on the given node.
 *
 * This function takes a pointer to a Node object and returns a pointer to the 
 * corresponding component node by using the node's index.
 *
 * @param node A pointer to the Node object for which the component node is to be retrieved.
 * @return A pointer to the component node corresponding to the given node.
 */
const Node* Segment::getComponentNode(const Node* node) const {
    return componentNodesPointers_m.getPointer(node->getIndex());
}

/**
 * @brief Sets the component node for a given node.
 * 
 * This function assigns a component node to a specified node by updating
 * the internal pointer structure. The component node is identified by its
 * index within the node structure.
 * 
 * @param node A pointer to the node for which the component node is being set.
 * @param componentNode A pointer to the component node to be assigned to the node.
 */
void Segment::setComponentNode(const Node* node, const Node* componentNode) {
    componentNodesPointers_m.setPointer(node->getIndex(), componentNode);
}

/**
 * @brief Constructs a SegmentsHandler object.
 * 
 * This constructor initializes the SegmentsHandler with a given subgraph biconnected
 * component and a cycle. It also triggers the process of finding segments and chords
 * within the provided component and cycle.
 * 
 * @param component Pointer to the SubGraph object representing the component.
 * @param cycle Pointer to the Cycle object representing the cycle.
 */
SegmentsHandler::SegmentsHandler(const SubGraph* component, const Cycle* cycle)
: originalComponent_m(component), originalCycle_m(cycle) {
    findSegments();
    findChords();
}

/**
 * @brief Performs a depth-first search (DFS) to find segments in a graph.
 *
 * This function traverses the graph starting from the given node, marking nodes as visited,
 * and collecting nodes and edges that form segments. It uses a depth-first search approach
 * to explore all reachable nodes and edges from the starting node.
 *
 * @param node The starting node for the DFS traversal.
 * @param isNodeVisited An array indicating whether each node has been visited.
 * @param nodesInSegment A vector to store the nodes that are part of the current segment.
 * @param edgesInSegment A vector to store the edges that are part of the current segment.
 */
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

/**
 * @brief Finds and constructs chords in the original cycle.
 * 
 * This function iterates through each node in the original cycle and checks its neighbors.
 * If a neighbor is part of the original cycle and is not the previous or next node in the cycle,
 * a chord is constructed between the node and the neighbor. 
 * 
 * @note A chord is defined as a segment connecting two non-adjacent nodes in the cycle.
 */
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

/**
 * @brief Finds segments in the original biconnected component. It uses a
 * depth-first search (DFS) to find these segments and then builds and stores them.
 */
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

/**
 * @brief Constructs a new Segment object from the given nodes and edges.
 *
 * This function creates a new Segment object that includes nodes from both the original cycle 
 * and the provided nodes vector. It sets up the necessary pointers and relationships between 
 * the new segment nodes and the original component nodes. Additionally, it adds edges between 
 * the nodes and handles attachments for nodes that are part of the original cycle.
 * The first nodes in the segment must be the same as the cycle nodes in the same order.
 * 
 * @note The nodes vector must not include the cycle nodes.
 * @note The edges vector must not contain cycle edges.
 *
 * @param nodes A vector of pointers to Node objects representing the nodes to be included in the segment.
 *              Note that this vector does not include the cycle nodes.
 * @param edges A vector of pairs of pointers to Node objects representing the edges to be included in the segment.
 *              Each pair represents an edge between two nodes.
 * @return A pointer to the newly created Segment object.
 */
const Segment* SegmentsHandler::buildSegment(std::vector<const Node*>& nodes,
std::vector<std::pair<const Node*, const Node*>>& edges) {
    Segment* segment = new Segment(nodes.size()+originalCycle_m->size(), originalComponent_m, originalCycle_m);
    // first nodes MUST be the same of the cycle in the SAME ORDER
    int oldToNewLabel[originalComponent_m->size()];
    // adding cycle nodes (which means setting up the pointers)
    for (int i = 0; i < originalCycle_m->size(); ++i) {
        const Node* cycleNode = originalCycle_m->getNode(i);
        oldToNewLabel[cycleNode->getIndex()] = i;
        const Node* newNode = segment->getNode(i);
        segment->setComponentNode(newNode, cycleNode);
        const Node* originalNode = originalComponent_m->getOriginalNode(cycleNode);
        segment->setOriginalNode(newNode, originalNode);
    }
    // adding the other nodes of the segment (setting up the pointers)
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

/**
 * @brief Constructs a new chord segment based on the provided attachment nodes.
 *
 * The chord is constructed by copying the nodes from the original cycle and maintaining the same order.
 *
 * @param attachment1 The first attachment node in the original cycle.
 * @param attachment2 The second attachment node in the original cycle.
 * @return A pointer to the newly created Segment object representing the chord.
 */
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

/**
 * @brief Retrieves a constant pointer to a Segment object at the specified index.
 * 
 * @param index The index of the segment to retrieve.
 * @return const Segment* A constant pointer to the Segment object at the specified index.
 */
const Segment* SegmentsHandler::getSegment(const int index) const {
    return segments_m[index].get();
}

/**
 * @brief Prints all segments managed by the SegmentsHandler.
 * 
 * This function iterates through all the segments managed by the SegmentsHandler
 * and prints each segment's index followed by the segment's details.
 */
void SegmentsHandler::print() const {
    for (int i = 0; i < size(); ++i) {
        std::cout << "segment [" << i << "]\n";
        const Segment* segment = getSegment(i);
        segment->print();
    }
}

/**
 * @brief Returns the number of segments managed by the SegmentsHandler.
 * 
 * @return int The number of segments.
 */
int SegmentsHandler::size() const {
    return segments_m.size();
}