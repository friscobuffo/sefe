#ifndef MY_EMBEDDER_H
#define MY_EMBEDDER_H

#include <optional>
#include <vector>
#include <string>
#include <memory>

#include "../basic/graph.hpp"
#include "../components/biconnectedComponent.hpp"
#include "../components/cycle.hpp"
#include "../components/segment.hpp"

class Embedder {
private:
    void makeCycleGood(BlackCycle& cycle, const Segment& segment) const;
    std::unique_ptr<const SubGraph> baseCaseGraph(const Graph& graph) const;
    std::unique_ptr<const SubGraph> baseCaseComponent(const SubGraph& segment, const BlackCycle& cycle) const;
    std::unique_ptr<const SubGraph> baseCaseCycle(const SubGraph& cycle) const;
    std::optional<std::unique_ptr<const SubGraph>> embedComponent(const SubGraph& component) const;
    std::optional<std::unique_ptr<const SubGraph>> embedComponent(const SubGraph& component, BlackCycle& cycle) const;
    void computeMinAndMaxSegmentsAttachments(const SegmentsHandler& segmentsHandler,
        int segmentsMinAttachment[], int segmentsMaxAttachment[]) const;
    std::vector<int> computeOrder(const Node& cycleNode, const std::vector<int>& segmentsIndexes,
        int segmentsMinAttachment[], int segmentsMaxAttachment[], const SegmentsHandler& segmentsHandler, int cycleNodePosition) const;
    std::vector<bool> compatibilityEmbeddingsAndCycle(const SubGraph& component, const BlackCycle& cycle,
        const std::vector<std::unique_ptr<const SubGraph>>& embeddings, const SegmentsHandler& segmentsHandler) const;
    void addMiddleEdges(const Segment& segment, const SubGraph& embedding, int cycleNodeIndex,
        const SubGraph& component, bool compatible, SubGraph& output) const;
    void addEdgesIncidentToCycle(const SegmentsHandler& segmentsHandler, const BlackCycle& cycle, const SubGraph& component,
        const std::vector<std::unique_ptr<const SubGraph>>& embeddings, const std::vector<int>& bipartition, SubGraph& output,
        int segmentsMinAttachment[], int segmentsMaxAttachment[], std::vector<bool> isSegmentCompatible) const;
    void addEdgesNotIncidentToCycle(const SegmentsHandler& segmentsHandler, SubGraph& output, const BlackCycle& cycle,
        const std::vector<std::unique_ptr<const SubGraph>>& embeddings, std::vector<bool> isSegmentCompatible) const;
    std::unique_ptr<const SubGraph> mergeSegmentsEmbeddings(const SubGraph& component, const BlackCycle& cycle,
        const std::vector<std::unique_ptr<const SubGraph>>& embeddings, const SegmentsHandler& segmentsHandler,
        const std::vector<int>& bipartition) const;
    std::unique_ptr<const SubGraph> mergeBiconnectedComponents(const Graph& graph, const BiconnectedComponentsHandler& biconnectedComponents,
        const std::vector<std::unique_ptr<const SubGraph>>& embeddings) const;
    int computeNumberOfFacesInEmbedding(const SubGraph& embedding) const;
    bool isEmbeddingGood(const SubGraph& embedding) const;
public:
    std::optional<std::unique_ptr<const SubGraph>> embedGraph(const Graph& graph) const;
};

#endif