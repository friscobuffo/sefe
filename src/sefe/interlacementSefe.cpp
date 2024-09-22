#include "interlacementSefe.hpp"

#include <cassert>

InterlacementGraphSefe::InterlacementGraphSefe(const IntersectionCycle* cycle, const BicoloredSegmentsHandler* segmentsHandler)
    : Graph(segmentsHandler->size()), cycle_m(cycle) , segmentsHandler_m(segmentsHandler) {
    computeConflicts();
}

void InterlacementGraphSefe::computeCycleLabels(const BicoloredSegment* segment, int cycleLabels[], Color color, int& numberOfAttachments) {
    assert(color != Color::BLACK);
    int originalComponentSize = cycle_m->getOriginalGraphSize();
    int totalAttachments = 0;
    bool isCycleNodeAnAttachment[originalComponentSize];
    for (int i = 0; i < originalComponentSize; ++i)
        isCycleNodeAnAttachment[i] = false;
    for (const NodeWithColors* attachment : segment->getAttachments())
        if (segment->isNodeAttachmentOfColor(attachment, color)) {
            isCycleNodeAnAttachment[segment->getHigherLevelNode(attachment)->getIndex()] = true;
            ++totalAttachments;
        }
    int foundAttachments = 0;
    for (int i = 0; i < cycle_m->size(); ++i) {
        const NodeWithColors* node = cycle_m->getNode(i);
        if (isCycleNodeAnAttachment[node->getIndex()])
            cycleLabels[node->getIndex()] = 2*(foundAttachments++);
        else
            if (foundAttachments == 0)
                cycleLabels[node->getIndex()] = 2*totalAttachments-1;
            else
                cycleLabels[node->getIndex()] = 2*foundAttachments-1;
    }
    assert(foundAttachments == totalAttachments);
    numberOfAttachments = totalAttachments;
}

// two segments are in conflict if any of their non cycle edges may intersect
void InterlacementGraphSefe::computeConflicts() {
    int redCycleLabels[cycle_m->getOriginalGraphSize()];
    int blueCycleLabels[cycle_m->getOriginalGraphSize()];
    for (int i = 0; i < segmentsHandler_m->size()-1; ++i) {
        const BicoloredSegment* segment = segmentsHandler_m->getSegment(i);
        int numberOfRedAttachments;
        int numberOfBlueAttachments;
        computeCycleLabels(segment, redCycleLabels, Color::RED, numberOfRedAttachments);
        computeCycleLabels(segment, blueCycleLabels, Color::BLUE, numberOfBlueAttachments);
        for (int j = i+1; j < segmentsHandler_m->size(); ++j) {
            const BicoloredSegment* otherSegment = segmentsHandler_m->getSegment(j);
            if (areInConflict(segment, otherSegment, redCycleLabels, Color::RED, numberOfRedAttachments) ||
                areInConflict(segment, otherSegment, blueCycleLabels, Color::BLUE, numberOfBlueAttachments)) {
                addEdge(i, j);
            }
        }
    }
}

bool InterlacementGraphSefe::areInConflict(const BicoloredSegment* segment1, const BicoloredSegment* segment2,
const int cycleLabels[], const Color color, const int numberOfAttachments) {
    if (numberOfAttachments == 0) return false;
    int numberOfLabels = 2*numberOfAttachments;
    int labels[numberOfLabels];
    for (int k = 0; k < numberOfLabels; ++k)
        labels[k] = 0;
    int numberOfAttachments2 = 0;
    for (const NodeWithColors* attachment : segment2->getAttachments()) {
        if (!segment2->isNodeAttachmentOfColor(attachment, color)) continue;
        int attachmentComponent = segment2->getHigherLevelNode(attachment)->getIndex();
        labels[cycleLabels[attachmentComponent]] = 1;
        numberOfAttachments2++;
    }
    if (numberOfAttachments2 == 0) return false;
    int sum = 0;
    for (int k = 0; k < numberOfLabels; ++k)
        sum += labels[k];
    int partSum = labels[0] + labels[1] + labels[2];
    bool areInConflict = true;
    for (int k = 0; k <= numberOfLabels-2; k += 2) {
        if (partSum == sum) {
            return false;
        }
        partSum = partSum + labels[(3+k) % numberOfLabels] + labels[(4+k) % numberOfLabels];
        partSum = partSum - labels[k] - labels[(1+k) % numberOfLabels];
    }
    return true;
}