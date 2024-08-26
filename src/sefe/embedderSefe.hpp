#ifndef MY_EMBEDDER_SEFE_H
#define MY_EMBEDDER_SEFE_H

#include <optional>
#include <vector>

#include "../basic/graph.hpp"
#include "bicoloredGraph.hpp"
#include "bicoloredSegment.hpp"
#include "intersectionCycle.hpp"
#include "../auslander-parter/embedder.hpp"

class EmbeddingSefe : public BicoloredSubGraph {
public:
    EmbeddingSefe(const BicoloredSubGraph* originalGraph);
    EmbeddingSefe(const BicoloredGraph* originalGraph);
    void addSingleEdge(int fromIndex, int toIndex, Color color);
    void addSingleEdge(NodeWithColors* from, const NodeWithColors* to, Color color);
    const Embedding* computeRedEmbedding(const Graph* red) const;
    const Embedding* computeBlueEmbedding(const Graph* blue) const;
};

class EmbedderSefe {
private:
    bool testSefe(const BicoloredSubGraph* bicoloredGraph, IntersectionCycle* cycle) const;
    void makeCycleGood(IntersectionCycle* cycle, const BicoloredSegment* segment) const;
    const EmbeddingSefe* baseCaseGraph(const BicoloredGraph* graph) const;
    std::optional<const EmbeddingSefe*> embedGraph(const BicoloredSubGraph* graph) const;
    std::optional<const EmbeddingSefe*> embedGraph(const BicoloredSubGraph* graph,
        IntersectionCycle* cycle) const;
    const EmbeddingSefe* baseCaseCycle(const BicoloredSubGraph* cycle) const;
    const EmbeddingSefe* baseCasePath(const BicoloredSubGraph* component, const IntersectionCycle* cycle) const;
    const EmbeddingSefe* mergeSegmentsEmbeddings(const BicoloredSubGraph* graph, const IntersectionCycle* cycle,
        const std::vector<std::unique_ptr<const EmbeddingSefe>>& embeddings, const BicoloredSegmentsHandler& segmentsHandler,
        const std::vector<int>& bipartition) const;
    void computeMinAndMaxSegmentsAttachments(const BicoloredSegmentsHandler& segmentsHandler,
        int segmentsMinMaxRedAttachment[][2], int segmentsMinMaxBlueAttachment[][2],
        bool segmentsHaveBetweenRedAttachment[], bool segmentsHaveBetweenBlueAttachment[]) const;
    std::vector<bool> compatibilityEmbeddingsAndCycle(const BicoloredSubGraph* component, const IntersectionCycle* cycle,
    const std::vector<std::unique_ptr<const EmbeddingSefe>>& embeddings, const BicoloredSegmentsHandler& segmentsHandler) const;
    std::vector<int> computeOrder(const NodeWithColors* cycleNode, const std::vector<int>& segmentsIndexes,
        int segmentsMinMaxRedAttachment[][2], int segmentsMinMaxBlueAttachment[][2],
        const BicoloredSegmentsHandler& segmentsHandler, int cycleNodePosition,
        bool segmentsHaveBetweenRedAttachment[], bool segmentsHaveBetweenBlueAttachment[]) const;
    void addMiddleEdges(const BicoloredSegment* segment, const EmbeddingSefe* embedding, int cycleNodeIndex,
        const BicoloredSubGraph* higherLevel, bool compatible, EmbeddingSefe* output) const;
public:
    bool testSefe(const Graph* graph1, const Graph* graph2) const;
    std::optional<const EmbeddingSefe*> embedGraph(const BicoloredGraph* graph) const;
    void embedToSvg(const BicoloredGraph* graph) const;
};

#endif
