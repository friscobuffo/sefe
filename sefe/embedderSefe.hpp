#ifndef MY_EMBEDDER_SEFE_H
#define MY_EMBEDDER_SEFE_H

#include <optional>
#include <vector>
#include <string>

#include "../basic/graph.hpp"
#include "bicoloredGraph.hpp"
#include "bicoloredSegment.hpp"
#include "intersectionCycle.hpp"

class EmbedderSefe {
private:
    bool testSefe(const BicoloredSegment* bicoloredGraph, IntersectionCycle* cycle) const;
    void makeCycleGood(IntersectionCycle* cycle, const BicoloredSegment* segment) const;
public:
    bool testSefe(const Graph* graph1, const Graph* graph2) const;
};

#endif