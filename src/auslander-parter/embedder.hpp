#ifndef MY_EMBEDDER_H
#define MY_EMBEDDER_H

#include <optional>
#include <vector>
#include <string>

#include "../basic/graph.hpp"
#include "biconnectedComponent.hpp"
#include "cycle.hpp"
#include "segment.hpp"

class Embedding : public SubGraph {
public:
    Embedding(const SubGraph* originalGraph);
    Embedding(const Graph* originalGraph);
    void addSingleEdge(int fromIndex, int toIndex);
    void addSingleEdge(Node* from, const Node* to);
};

class Embedder {
private:
    void makeCycleGood(Cycle* cycle, const Segment* segment) const;
    const Embedding* baseCaseGraph(const Graph* graph) const;
    const Embedding* baseCaseComponent(const SubGraph* segment, const Cycle* cycle) const;
    const Embedding* baseCaseCycle(const SubGraph* cycle) const;
    std::optional<const Embedding*> embedComponent(const SubGraph* component) const;
    std::optional<const Embedding*> embedComponent(const SubGraph* component, Cycle* cycle) const;
    void computeMinAndMaxSegmentsAttachments(const SegmentsHandler& segmentsHandler,
        int segmentsMinAttachment[], int segmentsMaxAttachment[]) const;
    std::vector<int> computeOrder(const Node* cycleNode, const std::vector<int>& segmentsIndexes,
        int segmentsMinAttachment[], int segmentsMaxAttachment[], const SegmentsHandler& segmentsHandler, int cycleNodePosition) const;
    std::vector<bool> compatibilityEmbeddingsAndCycle(const SubGraph* component, const Cycle* cycle,
        const std::vector<std::unique_ptr<const Embedding>>& embeddings, const SegmentsHandler& segmentsHandler) const;
    void addMiddleEdges(const Segment* segment, const Embedding* embedding, int cycleNodeIndex,
        const SubGraph* component, bool compatible, Embedding* output) const;
    const Embedding* mergeSegmentsEmbeddings(const SubGraph* component, const Cycle* cycle,
        const std::vector<std::unique_ptr<const Embedding>>& embeddings, const SegmentsHandler& segmentsHandler,
        const std::vector<int>& bipartition) const;
    void graphAndEmbeddingChecks(const Graph* graph, const Embedding* embedding) const;
    void segmentAndEmbeddingChecks(const Segment* segment, const Embedding* embedding) const;
    void componentAndEmbeddingChecks(const SubGraph* component, const Embedding* embedding) const;
    const Embedding* mergeBiconnectedComponents(const Graph* graph, const BiconnectedComponentsHandler& biconnectedComponents,
        const std::vector<std::unique_ptr<const Embedding>>& embeddings) const;
public:
    std::optional<const Embedding*> embedGraph(const Graph* graph) const;
    void embedToSvg(const Graph* graph, std::string& outputPath) const;
};

#endif