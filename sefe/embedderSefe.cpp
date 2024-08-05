#include "embedderSefe.hpp"

#include <iostream>

#include "biconnectedComponent.hpp"
#include "bicoloredSegment.hpp"

bool EmbedderSefe::testSefe(const Graph* graph1, const Graph* graph2) const {
    BicoloredGraph* bicoloredGraph = new BicoloredGraph(graph1, graph2);
    const Graph* intersection = bicoloredGraph->getIntersection();
    BiconnectedComponentsHandler bicCompHandler(intersection);
    if (bicCompHandler.size() > 1) {
        std::cout << "intersection must be biconnected\n";
        return false;
    }
    IntersectionCycle* cycle = new IntersectionCycle(bicoloredGraph);
    return testSefe(bicoloredGraph, cycle);
}

// assumes intersection is biconnected
bool EmbedderSefe::testSefe(const BicoloredGraph* bicoloredGraph, IntersectionCycle* cycle) const {
    const BicoloredSegmentsHandler segmentsHandler = BicoloredSegmentsHandler(component, cycle);
    if (segmentsHandler.size() == 0) // entire biconnected component is a cycle
        return baseCaseCycle(component);
    if (segmentsHandler.size() == 1) {
        const BicoloredSegment* segment = segmentsHandler.getSegment(0);
        if (segment->isPath())
            return baseCaseComponent(component, cycle);
        // chosen cycle is bad
        makeCycleGood(cycle, segment);
        return embedComponent(component, cycle);
    }
    InterlacementGraph interlacementGraph(cycle, segmentsHandler);
    std::optional<std::vector<int>> bipartition = interlacementGraph.computeBipartition();
    if (!bipartition) return std::nullopt;
    std::vector<std::unique_ptr<const Embedding>> embeddings{};
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const Segment* segment = segmentsHandler.getSegment(i);
        std::optional<const Embedding*> embedding = embedComponent(segment);
        if (!embedding.has_value())
            return std::nullopt;
        segmentAndEmbeddingChecks(segment, embedding.value());
        embeddings.push_back(std::unique_ptr<const Embedding>(embedding.value()));
    }
    const Embedding* embedding = mergeSegmentsEmbeddings(component, cycle, embeddings, segmentsHandler, bipartition.value());
    return embedding;
}