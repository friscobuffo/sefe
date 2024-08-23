#ifndef MY_EMBEDDER_SEFE_H
#define MY_EMBEDDER_SEFE_H

#include <optional>
#include <vector>
#include <string>

#include "../basic/graph.hpp"
#include "bicoloredGraph.hpp"
#include "bicoloredSegment.hpp"
#include "intersectionCycle.hpp"


class EmbeddingSefe : public BicoloredSubGraph {
public:
    EmbeddingSefe(const BicoloredSubGraph* originalGraph);
    EmbeddingSefe(const BicoloredGraph* originalGraph);
    void addSingleEdge(int fromIndex, int toIndex, Color color);
    void addSingleEdge(NodeWithColors* from, const NodeWithColors* to, Color color);
};

class EmbedderSefe {
private:
    bool testSefe(const BicoloredSubGraph* bicoloredGraph, IntersectionCycle* cycle) const;
    void makeCycleGood(IntersectionCycle* cycle, const BicoloredSegment* segment) const;
    bool testSefe(const BicoloredSubGraph* bicoloredGraph);
    //
    const EmbeddingSefe* baseCaseGraph(const BicoloredGraph* graph) const;
    std::optional<const EmbeddingSefe*> embedGraph(const BicoloredSubGraph* graph) const;
    std::optional<const EmbeddingSefe*> embedGraph(const BicoloredSubGraph* component,
        IntersectionCycle* cycle) const;
    const EmbeddingSefe* baseCaseCycle(const BicoloredSubGraph* cycle) const;
    const EmbeddingSefe* baseCasePath(const BicoloredSubGraph* component, const IntersectionCycle* cycle) const;
    const EmbeddingSefe* mergeSegmentsEmbeddings(const BicoloredSubGraph* graph, const IntersectionCycle* cycle,
        const std::vector<std::unique_ptr<const EmbeddingSefe>>& embeddings, const BicoloredSegmentsHandler& segmentsHandler,
        const std::vector<int>& bipartition) const;
public:
    bool testSefe(const Graph* graph1, const Graph* graph2) const;
    bool testSefe(const BicoloredGraph* bicoloredGraph);
    //
    std::optional<const EmbeddingSefe*> embedGraph(const BicoloredGraph* graph) const;
};

#endif