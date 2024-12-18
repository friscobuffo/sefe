#ifndef MY_INTERLACEMENT_SEFE_H
#define MY_INTERLACEMENT_SEFE_H

#include "../basic/graph.hpp"
#include "../components/segment.hpp"
#include "../components/cycle.hpp"

class InterlacementGraphSefe : public Graph {
private:
    const BlackCycle& cycle_m;
    const SegmentsHandler& segmentsHandler_m;

    bool areInConflict(const Segment& segment1, const Segment& segment2,
        const int cycleLabels[], const Color color, const int numberOfAttachments);
    void computeConflicts();
    void computeCycleLabels(const Segment& segment, int cycleLabels[], Color color, int& numberOfColoredAttachments);
public:
    InterlacementGraphSefe(const BlackCycle& cycle, const SegmentsHandler& segmentsHandler);
};

#endif
