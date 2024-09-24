#include "embedder.hpp"
#include <cassert>

#include <iostream>
#include <sstream>

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/planarlayout/PlanarDrawLayout.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/planarity/EmbedderModule.h>

#include "interlacement.hpp"
#include "../basic/utils.hpp"

/**
 * @brief Constructs an Embedding object from the given original graph.
 * 
 * This constructor initializes an Embedding object by copying the structure
 * of the provided original graph. It then maps each node in the
 * SubGraph to the corresponding node in the original graph.
 *
 * @param originalGraph A pointer to the original Graph object from which
 *                      the Embedding is created.
 */
Embedding::Embedding(const SubGraph* originalGraph)
: SubGraph(originalGraph->size(), originalGraph) {
    for (int i = 0; i < size(); ++i) {
        const Node* node = originalGraph->getNode(i);
        const Node* originalNode = originalGraph->getOriginalNode(node);
        setOriginalNode(getNode(i), originalNode);
    }
}

/**
 * @brief Constructs an Embedding object from the given original graph.
 *
 * This constructor initializes an Embedding object by copying the structure
 * of the provided original graph. It iterates through each node in the 
 * original graph, retrieves the corresponding original node, and sets it 
 * in the new Embedding object.
 *
 * @param originalGraph A pointer to the original SubGraph from which the 
 *                      Embedding is to be created.
 */
Embedding::Embedding(const Graph* originalGraph)
: SubGraph(originalGraph->size(), originalGraph) {
    for (int i = 0; i < size(); ++i)
        setOriginalNode(getNode(i), originalGraph->getNode(i));
}

/**
 * @brief Adds a single edge from one node to another in the embedding.
 *
 * This function adds a directed edge from the node pointed to by `from` to the node pointed to by `to`.
 * It modifies the `from` node to include the `to` node as a neighbor.
 *
 * @param from Pointer to the source node where the edge starts.
 * @param to Pointer to the destination node where the edge ends.
 */
void Embedding::addSingleEdge(Node* from, const Node* to) {
    from->addNeighbor(to);
}

/**
 * @brief Adds a single edge between two nodes identified by their indices.
 *
 * This function retrieves the nodes corresponding to the given indices and 
 * adds an edge between them.
 *
 * @param fromIndex The index of the starting node.
 * @param toIndex The index of the ending node.
 */
void Embedding::addSingleEdge(int fromIndex, int toIndex) {
    Node* from = getNode(fromIndex);
    Node* to = getNode(toIndex);
    addSingleEdge(from, to);
}

/**
 * @brief Merges embeddings of biconnected components of a graph into a single embedding.
 *
 * This function takes a graph, its biconnected components, and their corresponding embeddings,
 * and merges them into a single embedding.
 *
 * @param graph The original graph.
 * @param biconnectedComponents Handler for the biconnected components of the graph.
 * @param embeddings A vector of unique pointers to the embeddings of the biconnected components.
 * @return A pointer to the merged embedding.
 */
const Embedding* Embedder::mergeBiconnectedComponents(const Graph* graph, const BiconnectedComponentsHandler& biconnectedComponents,
const std::vector<std::unique_ptr<const Embedding>>& embeddings) const {
    Embedding* output = new Embedding(graph);
    assert(biconnectedComponents.size() == embeddings.size());
    for (int i = 0; i < biconnectedComponents.size(); ++i) {
        const SubGraph* component = biconnectedComponents.getComponent(i);
        const Embedding* embedding = embeddings[i].get();
        for (int i = 0; i < embedding->size(); ++i) {
            const Node* node = embedding->getNode(i);
            const Node* nodeOriginal = embedding->getOriginalNode(node);
            for (const Node* neighbor : node->getNeighbors()) {
                const Node* neighborOriginal = embedding->getOriginalNode(neighbor);
                output->addSingleEdge(nodeOriginal->getIndex(), neighborOriginal->getIndex());
            }
        }
    }
    return output;
}

/**
 * @brief Attempts to embed a given graph.
 *
 * This function tries to embed the provided graph. If the graph is not planar, the function
 * returns an empty optional. Otherwise, it returns the final embedding.
 *
 * @param graph A pointer to the graph to be embedded.
 * @return An optional containing a pointer to the embedding if successful, or an empty optional if embedding is not possible.
 */
std::optional<const Embedding*> Embedder::embedGraph(const Graph* graph) const {
    if (graph->size() < 4) return baseCaseGraph(graph);
    if (graph->numberOfEdges() > (3*graph->size()-6))
        return std::nullopt;
    const BiconnectedComponentsHandler bicComps(graph);
    std::vector<std::unique_ptr<const Embedding>> embeddings{};
    for (int i = 0; i < bicComps.size(); ++i) {
        const SubGraph* component = bicComps.getComponent(i);
        std::optional<const Embedding*> embedding = embedComponent(component);
        if (!embedding.has_value())
            return std::nullopt;
        embeddings.push_back(std::unique_ptr<const Embedding>(embedding.value()));
    }
    const Embedding* embedding = mergeBiconnectedComponents(graph, bicComps, embeddings);
    return embedding;
}

/**
 * @brief Computes the minimum and maximum attachment indices for each segment.
 *
 * This function iterates over all segments in the provided SegmentsHandler and determines
 * the minimum and maximum attachment indices for each segment. The results are stored in
 * the provided arrays `segmentsMinAttachment` and `segmentsMaxAttachment`. The min/max index
 * used is based on the position in the cycle (0, ..., cycleSize-1)
 *
 * @param segmentsHandler A reference to the SegmentsHandler containing the segments to process.
 * @param segmentsMinAttachment An array to store the minimum attachment index for each segment.
 * @param segmentsMaxAttachment An array to store the maximum attachment index for each segment.
 */
void Embedder::computeMinAndMaxSegmentsAttachments(const SegmentsHandler& segmentsHandler,
int segmentsMinAttachment[], int segmentsMaxAttachment[]) const {
    for (int i = 0; i < segmentsHandler.size(); i++) {
        int min = segmentsHandler.getSegment(i)->size();
        int max = 0;
        for (const Node* attachment : segmentsHandler.getSegment(i)->getAttachments()) {
            int index = attachment->getIndex();
            if (index < min) min = index;
            if (index > max) max = index;
        }
        segmentsMinAttachment[i] = min;
        segmentsMaxAttachment[i] = max;
    }
}

/**
 * @brief Computes the order of segments based on their attachments to a cycle node.
 *
 * This function takes a cycle node and a list of incident segment indexes, along with
 * their minimum and maximum attachment points, and computes an ordered list of these segment
 * indexes, such that the segments are placed clockwise outside the cycle node 
 * without intersecting.
 *
 * @param cycleNode Pointer to the cycle node.
 * @param segmentsIndexes Vector of segment indexes.
 * @param segmentsMinAttachment Array of minimum attachment points for each segment.
 * @param segmentsMaxAttachment Array of maximum attachment points for each segment.
 * @param segmentsHandler Reference to the SegmentsHandler object.
 * @param cycleNodePosition Position of the cycle node.
 * @return A vector of segment indexes ordered based on their attachment points and number of attachments.
 */
std::vector<int> Embedder::computeOrder(const Node* cycleNode, const std::vector<int>& segmentsIndexes,
int segmentsMinAttachment[], int segmentsMaxAttachment[], const SegmentsHandler& segmentsHandler, int cycleNodePosition) const {
    std::optional<int> middleSegment;
    std::vector<int> minSegments{};
    std::vector<int> maxSegments{};
    int cycleNodeIndex = cycleNode->getIndex();
    for (int i = 0; i < segmentsIndexes.size(); ++i) {
        int segIndex = segmentsIndexes[i];
        if (segmentsMinAttachment[segIndex] == cycleNodePosition) {
            minSegments.push_back(segIndex);
            continue;
        }
        if (segmentsMaxAttachment[segIndex] == cycleNodePosition) {
            maxSegments.push_back(segIndex);
            continue;
        }
        assert(!middleSegment.has_value());
        middleSegment = segIndex;
    }
    for (int i = 0; i < int(maxSegments.size())-1; ++i) {
        int min = i;
        int minSeg = maxSegments[i];
        for (int j = i+1; j < maxSegments.size(); ++j) {
            int candidate = maxSegments[j];
            if (segmentsMinAttachment[candidate] < segmentsMinAttachment[minSeg])
                continue;
            if (segmentsMinAttachment[candidate] > segmentsMinAttachment[minSeg]) {
                minSeg = maxSegments[candidate];
                min = j;
                continue;
            }
            int numAttachmentsMin = segmentsHandler.getSegment(minSeg)->getAttachments().size();
            int numAttachmentsCandidate = segmentsHandler.getSegment(candidate)->getAttachments().size();
            assert(numAttachmentsMin == 2 || numAttachmentsMin == 3);
            assert(numAttachmentsCandidate == 2 || numAttachmentsCandidate == 3);
            if (numAttachmentsMin == 2 && numAttachmentsCandidate == 2) {
                if (minSeg > candidate)
                    continue;
                minSeg = candidate;
                min = j;
                continue;
            }
            if (numAttachmentsCandidate == 3) {
                assert(numAttachmentsMin == 2);
                minSeg = candidate;
                min = j;
            }
        }
        int temp = maxSegments[min];
        maxSegments[min] = maxSegments[i];
        maxSegments[i] = temp;
    }
    // ordering minSegments
    for (int i = 0; i < int(minSegments.size())-1; ++i) {
        int min = i;
        int minSeg = minSegments[i];
        for (int j = i+1; j < minSegments.size(); ++j) {
            int candidate = minSegments[j];
            if (segmentsMaxAttachment[candidate] < segmentsMaxAttachment[minSeg])
                continue;
            if (segmentsMaxAttachment[candidate] > segmentsMaxAttachment[minSeg]) {
                min = j;
                minSeg = minSegments[j];
                continue;
            }
            int numAttachmentsMax = segmentsHandler.getSegment(minSeg)->getAttachments().size();
            int numAttachmentsCandidate = segmentsHandler.getSegment(candidate)->getAttachments().size();
            assert(numAttachmentsMax == 2 || numAttachmentsMax == 3);
            assert(numAttachmentsCandidate == 2 || numAttachmentsCandidate == 3);
            if (numAttachmentsMax == 2 && numAttachmentsCandidate == 2) {
                if (minSeg < candidate)
                    continue;
                minSeg = candidate;
                min = j;
                continue;
            }
            if (numAttachmentsMax == 3) {
                assert(numAttachmentsCandidate == 2);
                minSeg = candidate;
                min = j;
            }
        }
        int temp = minSegments[min];
        minSegments[min] = minSegments[i];
        minSegments[i] = temp;
    }
    std::vector<int> order{};
    for (int segmentIndex : maxSegments)
        order.push_back(segmentIndex);
    if (middleSegment) order.push_back(middleSegment.value());
    for (int segmentIndex : minSegments)
        order.push_back(segmentIndex);
    assert(order.size() == segmentsIndexes.size());
    return order;
}

/**
 * @brief Determines the compatibility of embeddings with a given cycle.
 *
 * The embedding is "compatible" with the cycle if, drawn the cycle clockwise,
 * the embedding of the segment places the segment inside the cycle.
 * 
 * @param component A pointer to the SubGraph component.
 * @param cycle A pointer to the Cycle object.
 * @param embeddings A vector of unique pointers to constant Embedding objects.
 * @param segmentsHandler A reference to the SegmentsHandler object.
 * @return A vector of boolean values where each value indicates the compatibility of the corresponding embedding with the cycle.
 */
std::vector<bool> Embedder::compatibilityEmbeddingsAndCycle(const SubGraph* component, const Cycle* cycle,
const std::vector<std::unique_ptr<const Embedding>>& embeddings, const SegmentsHandler& segmentsHandler) const {
    std::vector<bool> isCompatible(segmentsHandler.size());
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const Segment* segment = segmentsHandler.getSegment(i);
        const Embedding* embedding = embeddings[i].get();
        const Node* attachment = segment->getAttachments()[0]; // any attachment is good
        const Node* componentNode = segment->getComponentNode(attachment);
        const Node* next = cycle->getNextOfNode(componentNode);
        const Node* prev = cycle->getPrevOfNode(componentNode);
        int position = -1;
        int attachmentIndex = attachment->getIndex();
        const std::vector<const Node*>& neighbors = embedding->getNode(attachmentIndex)->getNeighbors();
        assert(neighbors.size() >= 3);
        for (int j = 0; j < neighbors.size(); ++j) {
            const Node* neighbor = neighbors[j];
            const Node* componentNeighbor = segment->getComponentNode(neighbor);
            if (componentNeighbor == next) {
                position = j;
                break;
            }
        }
        assert(position != -1);
        int nextPosition = (position+1)%neighbors.size();
        isCompatible[i] = (segment->getComponentNode(neighbors[nextPosition]) != prev);
    }
    return isCompatible;
}


/**
 * @brief Adds middle edges of a segment to the final embedding. Middle edges are edges,
 * of the segment, that are not incident to the cycle node.
 * 
 * This function processes the neighbors of a cycle node within a segment and adds edges
 * to the output embedding. The edges are added in a specific order based on the compatibility flag.
 *
 * @param segment Pointer to the Segment object containing the cycle and nodes.
 * @param embedding Pointer to the Embedding object representing the current embedding state.
 * @param cycleNodeIndex Index of the node in the cycle to process.
 * @param component Pointer to the SubGraph object representing the component of the graph.
 * @param compatible Boolean flag indicating if the segment is compatible with the cycle
 *                  (a segment embedding is compatible is drawn inside the cycle,
 *                  which is drawn clockwise):
 *                   - If true, edges are added in the order of neighbors.
 *                   - If false, edges are added in the reverse order of neighbors.
 * @param output Pointer to the Embedding object where the new edges will be added.
 */
void Embedder::addMiddleEdges(const Segment* segment, const Embedding* embedding, int cycleNodeIndex,
const SubGraph* component, bool compatible, Embedding* output) const {
    const Cycle* cycle = segment->getOriginalCycle();
    const Node* cycleNode = cycle->getNode(cycleNodeIndex);
    const Node* prevCycleNode = cycle->getPrevOfNode(cycleNode);
    const Node* nextCycleNode = cycle->getNextOfNode(cycleNode);
    assert(cycleNode->getGraph() == component);
    assert(prevCycleNode->getGraph() == component);
    assert(nextCycleNode->getGraph() == component);
    assert(segment->isNodeAnAttachment(segment->getNode(cycleNodeIndex)));
    std::vector<int> neighborsToAdd;
    const std::vector<const Node*>& neighbors = embedding->getNode(cycleNodeIndex)->getNeighbors();
    int positionOfLastAddedNode = -1;
    for (int i = 0; i < neighbors.size(); ++i) {
        const Node* neighbor = neighbors[i];
        const Node* neighborSegment = segment->getNode(neighbor->getIndex());
        const Node* neighborComponent = segment->getComponentNode(neighborSegment);
        if (neighborComponent->getIndex() == prevCycleNode->getIndex()) {
            positionOfLastAddedNode = i;
            break;
        }
        if (neighborComponent->getIndex() == nextCycleNode->getIndex()) {
            positionOfLastAddedNode = i;
            break;
        }
    }
    assert(positionOfLastAddedNode != -1);
    for (int i = 1; i < neighbors.size(); ++i) {
        const int index = (i+positionOfLastAddedNode)%neighbors.size();
        const Node* neighbor = neighbors[index];
        const Node* neighborSegment = segment->getNode(neighbor->getIndex());
        const Node* neighborComponent = segment->getComponentNode(neighborSegment);
        assert(neighborComponent->getGraph() == component);
        assert(segment->getOriginalNode(neighborSegment) == component->getOriginalNode(neighborComponent));
        assert(segment->getOriginalNode(neighborSegment) == embedding->getOriginalNode(neighbor));
        if (nextCycleNode == neighborComponent) continue;
        if (prevCycleNode == neighborComponent) continue;
        neighborsToAdd.push_back(neighborComponent->getIndex());
    }
    if (compatible)
        for (int j = 0; j < neighborsToAdd.size(); ++j) {
            Node* from = output->getNode(cycleNode->getIndex());
            Node* to = output->getNode(neighborsToAdd[j]);
            output->addSingleEdge(from, to);
        }
            
    else
        for (int j = neighborsToAdd.size()-1; j >= 0; --j) {
            Node* from = output->getNode(cycleNode->getIndex());
            Node* to = output->getNode(neighborsToAdd[j]);
            output->addSingleEdge(from, to);
        }
}

/**
 * @brief Merges segment embeddings into a single embedding for a given biconnected
 * component and cycle.
 *
 * @param component The subgraph biconnected component for which the embedding is being created.
 * @param cycle The cycle within the subgraph component.
 * @param embeddings A vector of unique pointers to the embeddings of the segments.
 * @param segmentsHandler Handler for managing segments within the subgraph.
 * @param bipartition A vector representing the bipartition of the segments inside
 *                      and outside the cycle.
 * @return A pointer to the newly created Embedding object.
 */
const Embedding* Embedder::mergeSegmentsEmbeddings(const SubGraph* component, const Cycle* cycle,
const std::vector<std::unique_ptr<const Embedding>>& embeddings, const SegmentsHandler& segmentsHandler,
const std::vector<int>& bipartition) const {
    Embedding* output = new Embedding(component);
    int segmentsMinAttachment[segmentsHandler.size()];
    int segmentsMaxAttachment[segmentsHandler.size()];
    computeMinAndMaxSegmentsAttachments(segmentsHandler, segmentsMinAttachment, segmentsMaxAttachment);
    std::vector<bool> isSegmentCompatible = compatibilityEmbeddingsAndCycle(component, cycle, embeddings, segmentsHandler);
    for (int cycleNodePosition = 0; cycleNodePosition < cycle->size(); ++cycleNodePosition) {
        std::vector<int> insideSegments{};
        std::vector<int> outsideSegments{};
        for (int i = 0; i < segmentsHandler.size(); ++i) {
            const Node* node = segmentsHandler.getSegment(i)->getNode(cycleNodePosition);
            if (segmentsHandler.getSegment(i)->isNodeAnAttachment(node)) {
                if (bipartition[i] == 0) insideSegments.push_back(i);
                else outsideSegments.push_back(i);
            }
        }
        const Node* cycleNode = cycle->getNode(cycleNodePosition);
        const Node* prevCycleNode = cycle->getPrevOfNode(cycleNode);
        const Node* nextCycleNode = cycle->getNextOfNode(cycleNode);
        // order of the segments inside the cycle
        std::vector<int> insideOrder = computeOrder(cycleNode, insideSegments,
            segmentsMinAttachment, segmentsMaxAttachment, segmentsHandler, cycleNodePosition);
        reverseVector(insideOrder);
        // order of the segments outside the cycle
        std::vector<int> outsideOrder = computeOrder(cycleNode, outsideSegments,
            segmentsMinAttachment, segmentsMaxAttachment, segmentsHandler, cycleNodePosition);
        for (int index : outsideOrder)
            isSegmentCompatible[index] = !isSegmentCompatible[index];
        output->addSingleEdge(cycleNode->getIndex(), nextCycleNode->getIndex());
        for (int i = 0; i < insideOrder.size(); ++i) {
            const Segment* segment = segmentsHandler.getSegment(insideOrder[i]);
            const Embedding* embedding = embeddings[insideOrder[i]].get();
            addMiddleEdges(segment, embedding, cycleNodePosition, component, isSegmentCompatible[insideOrder[i]], output);
        }
        output->addSingleEdge(cycleNode->getIndex(), prevCycleNode->getIndex());
        for (int i = 0; i < outsideOrder.size(); ++i) {
            const Segment* segment = segmentsHandler.getSegment(outsideOrder[i]);
            const Embedding* embedding = embeddings[outsideOrder[i]].get();
            addMiddleEdges(segment, embedding, cycleNodePosition, component, isSegmentCompatible[outsideOrder[i]], output);
        }
    }
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const Segment* segment = segmentsHandler.getSegment(i);
        const Embedding* embedding = embeddings[i].get();
        for (int nodeIndex = 0; nodeIndex < segment->size(); ++nodeIndex) {
            const Node* node = segment->getNode(nodeIndex);
            const Node* componentNode = segment->getComponentNode(node);
            // int label = segment.getLabelOfNode(nodeIndex);
            if (cycle->hasNode(componentNode)) continue;
            std::vector<int> neighborsToAdd;
            const Node* embeddingNode = embedding->getNode(nodeIndex);
            for (const Node* neighbor : embeddingNode->getNeighbors()) {
                const Node* neighborSegment = segment->getNode(neighbor->getIndex());
                const Node* neighborComponent = segment->getComponentNode(neighborSegment);
                neighborsToAdd.push_back(neighborComponent->getIndex());
            }
            if (isSegmentCompatible[i])
                for (int j = 0; j < neighborsToAdd.size(); ++j)
                    output->addSingleEdge(componentNode->getIndex(), neighborsToAdd[j]);
            else
                for (int j = neighborsToAdd.size()-1; j >= 0; --j)
                    output->addSingleEdge(componentNode->getIndex(), neighborsToAdd[j]);
        } 
    }
    return output;
}

/**
 * @brief Embeds a subgraph biconnected component given a cycle.
 *
 * The function uses a recursive approach to handle 
 * complex embeddings by breaking down the component into smaller segments 
 * and merging their embeddings.
 *
 * @param component A pointer to the subgraph component to be embedded.
 * @param cycle A pointer to the cycle into which the component is to be embedded.
 * @return An optional containing a pointer to the resulting embedding if successful, 
 *         or std::nullopt if the embedding fails.
 */
std::optional<const Embedding*> Embedder::embedComponent(const SubGraph* component, Cycle* cycle) const {
    const SegmentsHandler segmentsHandler = SegmentsHandler(component, cycle);
    if (segmentsHandler.size() == 0) // entire biconnected component is a cycle
        return baseCaseCycle(component);
    if (segmentsHandler.size() == 1) {
        const Segment* segment = segmentsHandler.getSegment(0);
        if (segment->isPath())
            return baseCaseComponent(component, cycle);
        // chosen cycle is bad
        makeCycleGood(cycle, segment);
        return embedComponent(component, cycle);
    }
    InterlacementGraph interlacementGraph(cycle, segmentsHandler);
    std::optional<std::vector<int>> bipartition = interlacementGraph.computeBipartition();
    if (!bipartition) return std::nullopt;
    std::vector<std::unique_ptr<const Embedding>> embeddings{};
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const Segment* segment = segmentsHandler.getSegment(i);
        std::optional<const Embedding*> embedding = embedComponent(segment);
        if (!embedding.has_value())
            return std::nullopt;
        embeddings.push_back(std::unique_ptr<const Embedding>(embedding.value()));
    }
    const Embedding* embedding = mergeSegmentsEmbeddings(component, cycle, embeddings, segmentsHandler, bipartition.value());
    return embedding;
}

/**
 * @brief Embeds a given subgraph biconnected component into an embedding.
 *
 * This function attempts to embed the provided subgraph component into an embedding.
 * If the embedding is successful, it returns the embedding; otherwise,
 * it returns an empty optional.
 *
 * @param component A pointer to the subgraph component to be embedded.
 * @return std::optional<const Embedding*> An optional containing the embedding if successful,
 *         or std::nullopt if the embedding could not be created.
 */
std::optional<const Embedding*> Embedder::embedComponent(const SubGraph* component) const {
    Cycle cycle(component);
    std::optional<const Embedding*> embedding = embedComponent(component, &cycle);
    if (!embedding.has_value()) return std::nullopt;
    return embedding;
}

// it may happen that a cycle induces only one segment, which is not a base case
// so the cycle must be recomputed such that it ensures at least two segments



/**
 * @brief Ensures that the given not "separating" cycle becomes "separating" by modifying
 * it based on the provided segment.
 * 
 * A cycle is "separating" if it has at least two segments.
 *
 * @param cycle A pointer to the Cycle object that needs to be modified.
 * @param segment A pointer to the Segment object that provides the nodes and attachments to be used.
 */
void Embedder::makeCycleGood(Cycle* cycle, const Segment* segment) const {
    assert(!segment->isPath());
    bool isCycleNodeAttachment[cycle->size()];
    for (int i = 0; i < cycle->size(); ++i)
        isCycleNodeAttachment[i] = false;
    for (const Node* attachment : segment->getAttachments()) {
        const Node* attachmentComponent = segment->getComponentNode(attachment);
        isCycleNodeAttachment[cycle->getPositionOfNode(attachmentComponent).value()] = true;
    }
    int foundAttachments = 0;
    const Node* attachmentsToUse[2];
    const Node* attachmentToInclude = nullptr;
    for (int i = 0; i < cycle->size(); ++i) {
        if (!isCycleNodeAttachment[i]) continue;
        const Node* node = cycle->getNode(i);
        const Node* nodeSegment = segment->getNode(i);
        assert(segment->getComponentNode(nodeSegment) == node);
        if (foundAttachments < 2)
            attachmentsToUse[foundAttachments++] = nodeSegment;
        else
            attachmentToInclude = nodeSegment;
        if (foundAttachments == 2 && attachmentToInclude != nullptr) break;
    }
    std::list<const Node*> path = segment->computePathBetweenAttachments(attachmentsToUse[0], attachmentsToUse[1]);
    std::list<const Node*> pathComponent;
    for (const Node* node : path)
        pathComponent.push_back(segment->getComponentNode(node));
    if (attachmentToInclude != nullptr)
        attachmentToInclude = segment->getComponentNode(attachmentToInclude);
    cycle->changeWithPath(pathComponent, attachmentToInclude);
}

// base case: graph has <4 nodes
const Embedding* Embedder::baseCaseGraph(const Graph* graph) const {
    assert(graph->size() < 4);
    Embedding* embedding = new Embedding(graph);
    for (int nodeIndex = 0; nodeIndex < graph->size(); ++nodeIndex) {
        for (const Node* neighbor : graph->getNode(nodeIndex)->getNeighbors())
            if (nodeIndex < neighbor->getIndex())
                embedding->addEdge(nodeIndex, neighbor->getIndex());
    }
    return embedding;
}

// base case: segment is a path
const Embedding* Embedder::baseCaseComponent(const SubGraph* component, const Cycle* cycle) const {
    Embedding* embedding = new Embedding(component);
    for (int nodeIndex = 0; nodeIndex < component->size(); ++nodeIndex) {
        const Node* node = component->getNode(nodeIndex);
        const std::vector<const Node*>& neighbors = node->getNeighbors();
        if (neighbors.size() == 2) { // attachment nodes will be handled later
            embedding->addSingleEdge(node->getIndex(), neighbors[0]->getIndex());
            embedding->addSingleEdge(node->getIndex(), neighbors[1]->getIndex());
            continue;
        }
        assert(neighbors.size() == 3);
        int neighborsOrder[3];
        for (int i = 0; i < 3; ++i)
            neighborsOrder[i] = -1;
        for (const Node* neighbor : neighbors) {
            if (cycle->getNextOfNode(node) == neighbor) {
                neighborsOrder[0] = neighbor->getIndex();
                continue;
            }
            if (cycle->getPrevOfNode(node) == neighbor) {
                neighborsOrder[2] = neighbor->getIndex();
                continue;
            }
            neighborsOrder[1] = neighbor->getIndex();
        }
        for (int i = 0; i < 3; ++i) {
            assert(neighborsOrder[i] != -1);
            embedding->addSingleEdge(node->getIndex(), neighborsOrder[i]);
        }
    }
    return embedding;
}

// base case: biconnected component is a cycle
const Embedding* Embedder::baseCaseCycle(const SubGraph* cycle) const {
    Embedding* embedding = new Embedding(cycle);
    for (int i = 0; i < cycle->size()-1; ++i)
        for (const Node* neighbor : cycle->getNode(i)->getNeighbors())
            if (i < neighbor->getIndex())
                embedding->addEdge(i, neighbor->getIndex());
    return embedding;
}

class AuslanderParterEmbedder : public ogdf::EmbedderModule {
public:
    void doCall(ogdf::Graph& graph, ogdf::adjEntry &adjExternal) {
        const Graph* myGraph = OgdfUtils::ogdfGraphToMyGraph(&graph);
        Embedder embedder;
        std::optional<const Embedding*> embeddingOpt = embedder.embedGraph(myGraph);
        if (!embeddingOpt) {
            std::cout << "error\n";
            exit(1);
        }
        const Embedding* embedding = embeddingOpt.value();
        std::vector<int> position(embedding->size());
        for (ogdf::node n : graph.nodes) {
            const int index = n->index();
            const Node* node = embedding->getNode(index);
            const std::vector<const Node*>& neighbors = node->getNeighbors();
            for (int i = 0; i < neighbors.size(); ++i)
                position[neighbors[i]->getIndex()] = i;
            std::vector<ogdf::adjEntry> order(neighbors.size());
            for (ogdf::adjEntry& adj : n->adjEntries) {
                const int neighbor = adj->twinNode()->index();
                order[position[neighbor]] = adj;
            }
            ogdf::List<ogdf::adjEntry> newOrder;
            for (ogdf::adjEntry& adj : order)
                newOrder.pushBack(adj);
            graph.sort(n, newOrder);
        }
        delete myGraph;
        delete embedding;
    }
};

int Embedder::embedToSvg(const Graph* graph) const {
    if (!graph->isConnected()) {
        std::cerr << "Graph is not connected." << std::endl;
        return -1;
    }
    std::unique_ptr<ogdf::Graph> ogdfGraph = std::unique_ptr<ogdf::Graph>(OgdfUtils::myGraphToOgdf(graph));
    ogdf::GraphAttributes GA(*ogdfGraph, ogdf::GraphAttributes::nodeGraphics | ogdf::GraphAttributes::edgeGraphics |
                        ogdf::GraphAttributes::nodeLabel | ogdf::GraphAttributes::edgeStyle |
                        ogdf::GraphAttributes::nodeStyle | ogdf::GraphAttributes::edgeArrow);
    for (ogdf::node v : ogdfGraph->nodes) {
        GA.label(v) = std::to_string(v->index());
        GA.shape(v) = ogdf::Shape::Ellipse;
    }
    for (ogdf::edge e : ogdfGraph->edges) {
        GA.strokeWidth(e) = 1.5;
        GA.arrowType(e) = ogdf::EdgeArrow::None;
    }
    ogdf::PlanarDrawLayout layout;
    layout.setEmbedder(new AuslanderParterEmbedder);
    layout.call(GA);

    std::ostringstream svgStream;
    ogdf::GraphIO::SVGSettings svgSettings;
    if (ogdf::GraphIO::drawSVG(GA, svgStream, svgSettings)) {
        std::string svgContent = svgStream.str();
        saveStringToFile("/embedding.svg", svgContent);
        return 1;
    }
    std::cerr << "Error generating SVG content." << std::endl;
    return -2;
}