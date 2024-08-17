#ifndef MY_INTERLACEMENT_H
#define MY_INTERLACEMENT_H

#include <vector>

#include "../basic/graph.hpp"
#include "segment.hpp"
#include "cycle.hpp"

class InterlacementGraph : public Graph {
private:
    const Cycle& cycle_m;
    const SegmentsHandler& segmentsHandler_m;

    void computeConflicts();
    void computeCycleLabels(const Segment& segment, int cycleLabels[]);
public:
    InterlacementGraph(const Cycle& cycle, const SegmentsHandler& segmentsHandler);
};

#endif
