#ifndef MY_INTERLACEMENT_H
#define MY_INTERLACEMENT_H

#include "../basic/graph.hpp"
#include "../components/segment.hpp"
#include "../components/cycle.hpp"

class InterlacementGraph : public Graph {
private:
    const BlackCycle& cycle_m;
    const SegmentsHandler& segmentsHandler_m;

    void computeConflicts();
    void computeCycleLabels(const Segment& segment, int cycleLabels[]);
public:
    InterlacementGraph(const BlackCycle& cycle, const SegmentsHandler& segmentsHandler);
};

#endif
