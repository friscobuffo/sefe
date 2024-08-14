#include "interlacementSefe.hpp"

#include <cassert>

#include "../basic/utils.hpp"

InterlacementGraphSefe::InterlacementGraphSefe(const IntersectionCycle& cycle, const BicoloredSegmentsHandler& segmentsHandler) 
    : Graph(segmentsHandler.size()), cycle_m(cycle) , segmentsHandler_m(segmentsHandler) {
    computeConflicts();
}

void InterlacementGraphSefe::computeCycleLabels(const BicoloredSegment& segment,
int cycleLabels[], Color color, int& numberOfColoredAttachments) {
    assert(color != Color::BOTH);
    int originalSegmentSize = cycle_m.getOriginalBicoloredSegmentSize();
    bool isCycleNodeAnAttachment[originalSegmentSize];
    for (int i = 0; i < originalSegmentSize; ++i)
        isCycleNodeAnAttachment[i] = false;
    int foundAttachments = 0;
    int totalAttachments = 0;
    for (int i = 0; i < segment.getNumberOfAttachments(); ++i) {
        const NodeWithColors& attachment = segment.getAttachment(i);
        if (segment.isNodeAttachmentOfColor(attachment, color)) {
            isCycleNodeAnAttachment[segment.getHigherLevelNode(attachment).getIndex()] = true;
            ++totalAttachments;
        }
    }
    for (int i = 0; i < cycle_m.size(); ++i) {
        const NodeWithColors& node = cycle_m.getNode(i);
        if (isCycleNodeAnAttachment[node.getIndex()])
            cycleLabels[node.getIndex()] = 2*(foundAttachments++);
        else
            if (foundAttachments == 0)
                cycleLabels[node.getIndex()] = 2*totalAttachments-1;
            else
                cycleLabels[node.getIndex()] = 2*foundAttachments-1;
    }
    assert(foundAttachments == totalAttachments);
    numberOfColoredAttachments = totalAttachments;
}

bool InterlacementGraphSefe::areInConflict(const BicoloredSegment& segment1,
const BicoloredSegment& segment2, const int cycleLabels[], const Color color, const int numberOfAttachments) {
    int numberOfLabels = 2*numberOfAttachments;
    int labels[numberOfLabels];
    for (int k = 0; k < numberOfLabels; ++k)
        labels[k] = 0;
    for (int k = 0; k < segment2.getNumberOfAttachments(); ++k) {
        const NodeWithColors& attachment = segment2.getAttachment(k);
        if (!segment2.isNodeAttachmentOfColor(attachment, color))
            continue;
        int attachmentHigherLevel = segment2.getHigherLevelNode(attachment).getIndex();
        labels[cycleLabels[attachmentHigherLevel]] = 1;
    }
    int sum = 0;
    for (int k = 0; k < numberOfLabels; ++k)
        sum += labels[k];
    int partSum = labels[0] + labels[1] + labels[2];
    for (int k = 0; k <= numberOfLabels-2; k += 2) {
        if (partSum == sum)
            return false;
        partSum = partSum + labels[(3+k) % numberOfLabels] + labels[(4+k) % numberOfLabels];
        partSum = partSum - labels[k] - labels[(1+k) % numberOfLabels];
    }
    return true;
}

void InterlacementGraphSefe::computeConflicts() {
    int blueCycleLabels[cycle_m.getOriginalBicoloredSegmentSize()];
    int redCycleLabels[cycle_m.getOriginalBicoloredSegmentSize()];
    for (int i = 0; i < segmentsHandler_m.size()-1; ++i) {
        const BicoloredSegment& segment = segmentsHandler_m.getSegment(i);
        int numberOfRedAttachments = 0;
        int numberOfBlueAttachments = 0;
        computeCycleLabels(segment, blueCycleLabels, Color::BLUE, numberOfBlueAttachments);
        computeCycleLabels(segment, blueCycleLabels, Color::RED, numberOfRedAttachments);
        for (int j = i+1; j < segmentsHandler_m.size(); ++j) {
            const BicoloredSegment& otherSegment = segmentsHandler_m.getSegment(j);
            bool conflictRed = areInConflict(segment, otherSegment, redCycleLabels, Color::RED, numberOfRedAttachments);
            bool conflictBlue = areInConflict(segment, otherSegment, blueCycleLabels, Color::BLUE, numberOfBlueAttachments);
            if (conflictRed || conflictBlue) addEdge(i, j);
        }
    }   
}