#include "interlacement.hpp"

#include <cassert>

/**
 * @brief Constructs an InterlacementGraph object.
 * 
 * This constructor initializes an InterlacementGraph with a given cycle and segments handler.
 * It computes conflicts between segments.
 * 
 * @param cycle Pointer to a Cycle object representing the cycle to be used.
 * @param segmentsHandler Reference to a SegmentsHandler object managing the segments.
 */
InterlacementGraph::InterlacementGraph(const Cycle* cycle, const SegmentsHandler& segmentsHandler)
    : Graph(segmentsHandler.size()), cycle_m(cycle) , segmentsHandler_m(segmentsHandler) {
    computeConflicts();
}

/**
 * @brief Computes the cycle labels for the nodes in the interlacement graph.
 *
 * This function assigns labels to the nodes in the interlacement graph based on
 * their attachment status and their position within the cycle. The labels are used
 * to quickly determine conflicts between segments.
 *
 * @param segment A pointer to the Segment object containing the attachments.
 * @param cycleLabels An array to store the computed cycle labels for each node.
 */
void InterlacementGraph::computeCycleLabels(const Segment* segment, int cycleLabels[]) {
    int originalComponentSize = cycle_m->getOriginalComponentSize();
    bool isCycleNodeAnAttachment[originalComponentSize];
    for (int i = 0; i < originalComponentSize; ++i)
        isCycleNodeAnAttachment[i] = false;
    for (const Node* attachment : segment->getAttachments())
        isCycleNodeAnAttachment[segment->getComponentNode(attachment)->getIndex()] = true;
    int foundAttachments = 0;
    int totalAttachments = segment->getAttachments().size();
    for (int i = 0; i < cycle_m->size(); ++i) {
        const Node* node = cycle_m->getNode(i);
        if (isCycleNodeAnAttachment[node->getIndex()])
            cycleLabels[node->getIndex()] = 2*(foundAttachments++);
        else
            if (foundAttachments == 0)
                cycleLabels[node->getIndex()] = 2*totalAttachments-1;
            else
                cycleLabels[node->getIndex()] = 2*foundAttachments-1;
    }
    assert(foundAttachments == totalAttachments);
}

/**
 * @brief Computes conflicts between segments.
 *
 * This function computes conflicts between segments based on their attachments and
 * their position within the cycle. Two segments are in conflict if any of their non-cycle
 * edges may intersect.
 */
void InterlacementGraph::computeConflicts() {
    int cycleLabels[cycle_m->getOriginalComponentSize()];
    for (int i = 0; i < segmentsHandler_m.size()-1; ++i) {
        const Segment* segment = segmentsHandler_m.getSegment(i);
        computeCycleLabels(segment, cycleLabels);
        int numberOfLabels = 2*segment->getAttachments().size();
        int labels[numberOfLabels];
        for (int j = i+1; j < segmentsHandler_m.size(); ++j) {
            const Segment* otherSegment = segmentsHandler_m.getSegment(j);
            for (int k = 0; k < numberOfLabels; ++k)
                labels[k] = 0;
            for (const Node* attachment : otherSegment->getAttachments()) {
                int attachmentComponent = otherSegment->getComponentNode(attachment)->getIndex();
                labels[cycleLabels[attachmentComponent]] = 1;
            }
            int sum = 0;
            for (int k = 0; k < numberOfLabels; ++k)
                sum += labels[k];
            int partSum = labels[0] + labels[1] + labels[2];
            bool areInConflict = true;
            for (int k = 0; k <= numberOfLabels-2; k += 2) {
                if (partSum == sum) {
                    areInConflict = false;
                    break;
                }
                partSum = partSum + labels[(3+k) % numberOfLabels] + labels[(4+k) % numberOfLabels];
                partSum = partSum - labels[k] - labels[(1+k) % numberOfLabels];
            }
            if (areInConflict) addEdge(i, j);
        }
    }
}
