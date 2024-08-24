#include "embedder.hpp"

#include <cassert>

#include "interlacement.hpp"
#include "../basic/utils.hpp

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
