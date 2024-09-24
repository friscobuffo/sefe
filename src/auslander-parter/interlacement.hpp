#ifndef MY_INTERLACEMENT_H
#define MY_INTERLACEMENT_H

#include "../basic/graph.hpp"
#include "segment.hpp"
#include "cycle.hpp"

/**
 * @class InterlacementGraph
 * @brief Represents a specialized graph used for analysis of conflicts between segments.
 * 
 * Each node of the graph represents a segment of the cycle and each edge represents
 * a conflict between two segments.
 * 
 * @see Graph
 * @see Cycle
 * @see SegmentsHandler
 */
class InterlacementGraph : public Graph {
private:
    const Cycle* cycle_m;
    const SegmentsHandler& segmentsHandler_m;

    void computeConflicts();
    void computeCycleLabels(const Segment* segment, int cycleLabels[]);
public:
    InterlacementGraph(const Cycle* cycle, const SegmentsHandler& segmentsHandler);
};

#endif
