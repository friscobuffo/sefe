#ifndef MY_EMBEDDER_SEFE_H
#define MY_EMBEDDER_SEFE_H

#include <optional>
#include <vector>
#include <memory>

#include "../basic/graph.hpp"
#include "../components/segment.hpp"
#include "../components/cycle.hpp"

class EmbedderSefe {
private:
    bool testSefe(const SubGraph& unionGraph, BlackCycle& cycle) const;
    void makeCycleGood(BlackCycle& cycle, const Segment& segment) const;
    const SubGraph* baseCaseGraph(const Graph& unionGraph) const;
    std::optional<const SubGraph*> embedGraph(const SubGraph& unionGraph) const;
    std::optional<const SubGraph*> embedGraph(const SubGraph& unionGraph, BlackCycle& cycle) const;
    const SubGraph* baseCaseCycle(const SubGraph& cycle) const;
    const SubGraph* baseCasePath(const SubGraph& unionGraph, const BlackCycle& cycle) const;
    const SubGraph* mergeSegmentsEmbeddings(const SubGraph& unionGraph, const BlackCycle& cycle,
        const std::vector<std::unique_ptr<const SubGraph>>& embeddings,
        const SegmentsHandler& segmentsHandler, const std::vector<int>& bipartition) const;
    void computeMinAndMaxSegmentsAttachments(const SegmentsHandler& segmentsHandler,
        int segmentsMinMaxRedAttachment[][2], int segmentsMinMaxBlueAttachment[][2],
        bool segmentsHaveBetweenRedAttachment[], bool segmentsHaveBetweenBlueAttachment[]) const;
    std::vector<bool> compatibilityEmbeddingsAndCycle(const SubGraph& component, const BlackCycle& cycle,
    const std::vector<std::unique_ptr<const SubGraph>>& embeddings, const SegmentsHandler& segmentsHandler) const;
    int compareSegments(int segmentsMinMaxAttachment[][2], int segment1index, int segment2index,
        int cycleNodePosition, bool segmentsHasBetweenAttachment[]) const;
    std::vector<int> computeOrder(const Node& cycleNode, const std::vector<int>& segmentsIndexes,
        int segmentsMinMaxRedAttachment[][2], int segmentsMinMaxBlueAttachment[][2],
        const SegmentsHandler& segmentsHandler, int cycleNodePosition,
        bool segmentsHaveBetweenRedAttachment[], bool segmentsHaveBetweenBlueAttachment[]) const;
    bool handleDrawsOfSegments(int cycleNodePosition, const SegmentsHandler& segmentsHandler,
        int segmentsMinMaxRedAttachment[][2], int segmentsMinMaxBlueAttachment[][2], int segmentIndex1, int segmentIndex2) const;
    void addMiddleEdges(const Segment& segment, const SubGraph& embedding, int cycleNodeIndex,
        const SubGraph& higherLevel, bool compatible, SubGraph& output) const;
public:
    bool testSefe(const Graph& graph1, const Graph& graph2) const;
    std::optional<std::unique_ptr<const SubGraph>> embedGraph(const Graph& unionGraph) const;
};

#endif