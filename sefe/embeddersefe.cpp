#include "embedder.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

#include "interlacement.hpp"
#include "../basic/utils.hpp"

// for each segment, it computes the minimum and the maximum of all of its attachments,
// using attachment notation based on the position in the cycle (0, ..., cycleSize-1)
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

// assuming the cycle is drawn CLOCKWISE, and assuming the segments incident to the 
// attachment "cycleNode" must be drawn OUTSIDE the cycle, computes the order of
// placement of these segments such that they don't intersect
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
        int min = maxSegments[i];
        for (int j = i+1; j < maxSegments.size(); ++j) {
            int candidate = maxSegments[j];
            if (segmentsMinAttachment[candidate] < segmentsMinAttachment[min])
                continue;
            if (segmentsMinAttachment[candidate] > segmentsMinAttachment[min]) {
                min = maxSegments[candidate];
                continue;
            }
            int numAttachmentsMin = segmentsHandler.getSegment(min)->getAttachments().size();
            int numAttachmentsCandidate = segmentsHandler.getSegment(candidate)->getAttachments().size();
            assert(numAttachmentsMin == 2 || numAttachmentsMin == 3);
            assert(numAttachmentsCandidate == 2 || numAttachmentsCandidate == 3);
            if (numAttachmentsMin == 2 && numAttachmentsCandidate == 2) {
                if (min > candidate)
                    continue;
                min = candidate;
                continue;
            }
            if (numAttachmentsCandidate == 3) {
                assert(numAttachmentsMin == 2);
                min = candidate;
            }
        }
        int temp = maxSegments[min];
        maxSegments[min] = maxSegments[i];
        maxSegments[i] = temp;
    }
    // ordering minSegments
    for (int i = 0; i < int(minSegments.size())-1; ++i) {
        int min = minSegments[i];
        for (int j = i+1; j < minSegments.size(); ++j) {
            int candidate = minSegments[j];
            if (segmentsMaxAttachment[candidate] < segmentsMaxAttachment[min])
                continue;
            if (segmentsMaxAttachment[candidate] > segmentsMaxAttachment[min]) {
                min = minSegments[candidate];
                continue;
            }
            int numAttachmentsMax = segmentsHandler.getSegment(min)->getAttachments().size();
            int numAttachmentsCandidate = segmentsHandler.getSegment(candidate)->getAttachments().size();
            assert(numAttachmentsMax == 2 || numAttachmentsMax == 3);
            assert(numAttachmentsCandidate == 2 || numAttachmentsCandidate == 3);
            if (numAttachmentsMax == 2 && numAttachmentsCandidate == 2) {
                if (min > candidate)
                    continue;
                min = candidate;
                continue;
            }
            if (numAttachmentsMax == 3) {
                assert(numAttachmentsCandidate == 2);
                min = candidate;
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

// the embedding is "compatible" with the cycle if, drawn the cycle clockwise,
// the embedding of the segment places the segment inside the cycle
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