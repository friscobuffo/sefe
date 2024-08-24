#ifndef MY_INTERLACEMENT_SEFE_H
#define MY_INTERLACEMENT_SEFE_H

#include "../basic/graph.hpp"
#include "bicoloredSegment.hpp"
#include "intersectionCycle.hpp"

class InterlacementGraphSefe : public Graph {
private:
    const IntersectionCycle* cycle_m;
    const BicoloredSegmentsHandler* segmentsHandler_m;

    bool areInConflict(const BicoloredSegment* segment1, const BicoloredSegment* segment2,
        const int cycleLabels[], const Color color, const int numberOfAttachments);
    void computeConflicts();
    void computeCycleLabels(const BicoloredSegment* segment, int cycleLabels[], Color color, int& numberOfColoredAttachments);
public:
    InterlacementGraphSefe(const IntersectionCycle* cycle, const BicoloredSegmentsHandler* segmentsHandler);
};

#endif
