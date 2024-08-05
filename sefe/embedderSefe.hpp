#ifndef MY_EMBEDDER_SEFE_H
#define MY_EMBEDDER_SEFE_H

#include <optional>
#include <vector>
#include <string>

#include "graph.hpp"
#include "bicoloredGraph.hpp"
#include "intersectionCycle.hpp"

class EmbedderSefe {
private:
    bool testSefe(const BicoloredGraph* bicoloredGraph, IntersectionCycle* cycle) const;
public:
    bool testSefe(const Graph* graph1, const Graph* graph2) const;
};

#endif