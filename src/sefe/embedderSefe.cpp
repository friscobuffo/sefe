#include "embedderSefe.hpp"

#include <iostream>

#include "../auslander-parter/biconnectedComponent.hpp"
#include "interlacementSefe.hpp"

EmbeddingSefe::EmbeddingSefe(const BicoloredSubGraph* originalGraph)
: BicoloredSubGraph(originalGraph->size(), originalGraph) {
    for (int i = 0; i < size(); ++i) {
        const NodeWithColors* node = originalGraph->getNode(i);
        const NodeWithColors* originalNode = originalGraph->getOriginalNode(node);
        setOriginalNode(getNode(i), originalNode);
    }
}

EmbeddingSefe::EmbeddingSefe(const BicoloredGraph* originalGraph)
: BicoloredSubGraph(originalGraph->size(), originalGraph) {
    for (int i = 0; i < size(); ++i)
        setOriginalNode(getNode(i), originalGraph->getNode(i));
}

void EmbeddingSefe::addSingleEdge(NodeWithColors* from, const NodeWithColors* to, Color color) {
    from->addEdge(to, color);
}

void EmbeddingSefe::addSingleEdge(int fromIndex, int toIndex, Color color) {
    NodeWithColors* from = getNode(fromIndex);
    NodeWithColors* to = getNode(toIndex);
    addSingleEdge(from, to, color);
}

bool EmbedderSefe::testSefe(const Graph* graph1, const Graph* graph2) const {
    BicoloredGraph bicoloredGraph(graph1, graph2);
    const Graph* intersection = bicoloredGraph.getIntersection();
    BiconnectedComponentsHandler bicCompHandler(intersection);
    if (bicCompHandler.size() > 1) {
        std::cout << "intersection must be biconnected\n";
        return false;
    }
    BicoloredSubGraph bicoloredSubGraph(&bicoloredGraph);
    IntersectionCycle cycle(&bicoloredSubGraph);
    return testSefe(&bicoloredSubGraph, &cycle);
}

// assumes intersection is biconnected
bool EmbedderSefe::testSefe(const BicoloredSubGraph* bicoloredGraph, IntersectionCycle* cycle) const {
    const BicoloredSegmentsHandler segmentsHandler = BicoloredSegmentsHandler(bicoloredGraph, cycle);
    if (segmentsHandler.size() == 0) // entire biconnected component is a cycle
        return true;
    if (segmentsHandler.size() == 1) {
        const BicoloredSegment* segment = segmentsHandler.getSegment(0);
        if (segment->isPath())
            return true;
        // chosen cycle is bad
        makeCycleGood(cycle, segment);
        return testSefe(bicoloredGraph, cycle);
    }
    InterlacementGraphSefe interlacementGraph(cycle, &segmentsHandler);
    std::optional<std::vector<int>> bipartition = interlacementGraph.computeBipartition();
    if (!bipartition) return false;
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const BicoloredSegment* segment = segmentsHandler.getSegment(i);
        IntersectionCycle cycle(segment);
        if (!testSefe(segment, &cycle)) return false;
    }
    return true;
}

// it may happen that a cycle induces only one segment, which is not a base case
// so the cycle must be recomputed such that it ensures at least two segments
void EmbedderSefe::makeCycleGood(IntersectionCycle* cycle, const BicoloredSegment* segment) const {
    assert(!segment->isPath());
    if (segment->isBlackPath()) {
        std::cout << "black path with some chords\n";
    }
    bool isCycleNodeAttachment[cycle->size()];
    for (int i = 0; i < cycle->size(); ++i)
        isCycleNodeAttachment[i] = false;
    for (int i = 0; i < segment->getNumberOfAttachments(); ++i) {
        const NodeWithColors* attachment = segment->getAttachment(i);
        const NodeWithColors* attachmentHigherLevel = segment->getHigherLevelNode(attachment);
        isCycleNodeAttachment[cycle->getPositionOfNode(attachmentHigherLevel).value()] = true;
    }
    int foundAttachments = 0;
    const NodeWithColors* attachmentsToUse[2];
    const NodeWithColors* attachmentToInclude = nullptr;
    for (int i = 0; i < cycle->size(); ++i) {
        if (!isCycleNodeAttachment[i]) continue;
        const NodeWithColors* node = cycle->getNode(i);
        const NodeWithColors* nodeSegment = segment->getNode(i);
        assert(segment->getHigherLevelNode(nodeSegment) == node);
        if (!segment->isNodeBlackAttachment(nodeSegment)) {
            attachmentToInclude = nodeSegment;
            continue;
        }
        if (foundAttachments < 2)
            attachmentsToUse[foundAttachments++] = nodeSegment;
        else
            attachmentToInclude = nodeSegment;
        if (foundAttachments == 2 && attachmentToInclude != nullptr) break;
    }
    std::list<const NodeWithColors*> path = segment->computeBlackPathBetweenAttachments(attachmentsToUse[0], attachmentsToUse[1]);
    std::list<const NodeWithColors*> pathHigherLevel;
    for (const NodeWithColors* node : path)
        pathHigherLevel.push_back(segment->getHigherLevelNode(node));
    if (attachmentToInclude != nullptr) attachmentToInclude = segment->getHigherLevelNode(attachmentToInclude);
    cycle->changeWithPath(pathHigherLevel, attachmentToInclude);
}

//
//
//
//

// base case: biconnected component is a cycle
const EmbeddingSefe* EmbedderSefe::baseCaseCycle(const BicoloredSubGraph* cycle) const {
    EmbeddingSefe* embedding = new EmbeddingSefe(cycle);
    for (int i = 0; i < cycle->size()-1; ++i)
        for (const Edge& edge : cycle->getNode(i)->getEdges())
            if (i < edge.node->getIndex())
                embedding->addEdge(i, edge.node->getIndex(), edge.color);
    return embedding;
}

std::optional<const EmbeddingSefe*> EmbedderSefe::embedGraph(const BicoloredSubGraph* graph,
IntersectionCycle* cycle) const {
    const BicoloredSegmentsHandler segmentsHandler(graph, cycle);
    if (segmentsHandler.size() == 0) // entire biconnected component is a cycle
        return baseCaseCycle(graph);
    if (segmentsHandler.size() == 1) {
        const BicoloredSegment* segment = segmentsHandler.getSegment(0);
        if (segment->isPath())
            return baseCasePath(graph, cycle);
        // chosen cycle is bad
        makeCycleGood(cycle, segment);
        return embedGraph(graph, cycle);
    }
    InterlacementGraphSefe interlacementGraph(cycle, &segmentsHandler);
    std::optional<std::vector<int>> bipartition = interlacementGraph.computeBipartition();
    if (!bipartition) return std::nullopt;
    std::vector<std::unique_ptr<const EmbeddingSefe>> embeddings{};
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const BicoloredSegment* segment = segmentsHandler.getSegment(i);
        std::optional<const EmbeddingSefe*> embedding = embedGraph(segment);
        if (!embedding.has_value())
            return std::nullopt;
        embeddings.push_back(std::unique_ptr<const EmbeddingSefe>(embedding.value()));
    }
    const EmbeddingSefe* embedding = mergeSegmentsEmbeddings(graph, cycle, embeddings, segmentsHandler, bipartition.value());
    return embedding;
}

// base case: segment is a path
const EmbeddingSefe* EmbedderSefe::baseCasePath(const BicoloredSubGraph* component, const IntersectionCycle* cycle) const {
    EmbeddingSefe* embedding = new EmbeddingSefe(component);
    for (int nodeIndex = 0; nodeIndex < component->size(); ++nodeIndex) {
        const NodeWithColors* node = component->getNode(nodeIndex);
        const std::vector<Edge>& edges = node->getEdges();
        if (edges.size() == 2) { // attachment nodes will be handled later
            embedding->addSingleEdge(node->getIndex(), edges[0].node->getIndex(), edges[0].color);
            embedding->addSingleEdge(node->getIndex(), edges[1].node->getIndex(), edges[1].color);
            continue;
        }
        assert(edges.size() == 3);
        int neighborsOrder[3];
        Color neighborsOrderColor[3];
        for (int i = 0; i < 3; ++i)
            neighborsOrder[i] = -1;
        for (const Edge& edge : edges) {
            const NodeWithColors* neighbor = edge.node;
            Color color = edge.color;
            if (cycle->getNextOfNode(node) == neighbor) {
                neighborsOrder[0] = neighbor->getIndex();
                neighborsOrderColor[0] = color;
                continue;
            }
            if (cycle->getPrevOfNode(node) == neighbor) {
                neighborsOrder[2] = neighbor->getIndex();
                neighborsOrderColor[2] = color;
                continue;
            }
            neighborsOrder[1] = neighbor->getIndex();
            neighborsOrderColor[1] = color;
        }
        for (int i = 0; i < 3; ++i) {
            assert(neighborsOrder[i] != -1);
            embedding->addSingleEdge(node->getIndex(), neighborsOrder[i], neighborsOrderColor[i]);
        }
    }
    return embedding;
}

std::optional<const EmbeddingSefe*> EmbedderSefe::embedGraph(const BicoloredSubGraph* graph) const {
    IntersectionCycle cycle(graph);
    std::optional<const EmbeddingSefe*> embedding = embedGraph(graph, &cycle);
    if (!embedding.has_value()) return std::nullopt;
    return embedding;
}

// base case: graph has <4 nodes
const EmbeddingSefe* EmbedderSefe::baseCaseGraph(const BicoloredGraph* graph) const {
    assert(graph->size() < 4);
    EmbeddingSefe* embedding = new EmbeddingSefe(graph);
    for (const NodeWithColors* node : graph->getNodes())
        for (const Edge& edge : node->getEdges())
            if (node->getIndex() < edge.node->getIndex())
                embedding->addEdge(node->getIndex(), edge.node->getIndex(), edge.color);
    return embedding;
}

std::optional<const EmbeddingSefe*> EmbedderSefe::embedGraph(const BicoloredGraph* graph) const {
    if (graph->size() < 4) return baseCaseGraph(graph);
    const BicoloredSubGraph sGraph(graph);
    return embedGraph(&sGraph);
}

/*
const EmbeddingSefe* EmbedderSefe::mergeSegmentsEmbeddings(const BicoloredSubGraph* graph, const IntersectionCycle* cycle,
const std::vector<std::unique_ptr<const EmbeddingSefe>>& embeddings, const BicoloredSegmentsHandler& segmentsHandler,
const std::vector<int>& bipartition) const {
    EmbeddingSefe* output = new EmbeddingSefe(graph);
    int segmentsMinAttachment[segmentsHandler.size()];
    int segmentsMaxAttachment[segmentsHandler.size()];
    computeMinAndMaxSegmentsAttachments(segmentsHandler, segmentsMinAttachment, segmentsMaxAttachment);
    std::vector<bool> isSegmentCompatible = compatibilityEmbeddingsAndCycle(graph, cycle, embeddings, segmentsHandler);
    for (int cycleNodePosition = 0; cycleNodePosition < cycle->size(); ++cycleNodePosition) {
        std::vector<int> insideSegments{};
        std::vector<int> outsideSegments{};
        for (int i = 0; i < segmentsHandler.size(); ++i) {
            const Node* node = segmentsHandler.getSegment(i)->getNode(cycleNodePosition);
            if (segmentsHandler.getSegment(i)->isNodeAnAttachment(node)) {
                if (bipartition[i] == 0) insideSegments.push_back(i);
                else outsideSegments.push_back(i);
            }
        }
        const Node* cycleNode = cycle->getNode(cycleNodePosition);
        const Node* prevCycleNode = cycle->getPrevOfNode(cycleNode);
        const Node* nextCycleNode = cycle->getNextOfNode(cycleNode);
        // order of the segments inside the cycle
        std::vector<int> insideOrder = computeOrder(cycleNode, insideSegments,
            segmentsMinAttachment, segmentsMaxAttachment, segmentsHandler, cycleNodePosition);
        reverseVector(insideOrder);
        // order of the segments outside the cycle
        std::vector<int> outsideOrder = computeOrder(cycleNode, outsideSegments,
            segmentsMinAttachment, segmentsMaxAttachment, segmentsHandler, cycleNodePosition);
        for (int index : outsideOrder)
            isSegmentCompatible[index] = !isSegmentCompatible[index];
        output->addSingleEdge(cycleNode->getIndex(), nextCycleNode->getIndex());
        for (int i = 0; i < insideOrder.size(); ++i) {
            const Segment* segment = segmentsHandler.getSegment(insideOrder[i]);
            const Embedding* embedding = embeddings[insideOrder[i]].get();
            addMiddleEdges(segment, embedding, cycleNodePosition, graph, isSegmentCompatible[insideOrder[i]], output);
        }
        output->addSingleEdge(cycleNode->getIndex(), prevCycleNode->getIndex());
        for (int i = 0; i < outsideOrder.size(); ++i) {
            const Segment* segment = segmentsHandler.getSegment(outsideOrder[i]);
            const Embedding* embedding = embeddings[outsideOrder[i]].get();
            addMiddleEdges(segment, embedding, cycleNodePosition, graph, isSegmentCompatible[outsideOrder[i]], output);
        }
    }
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const Segment* segment = segmentsHandler.getSegment(i);
        const Embedding* embedding = embeddings[i].get();
        for (int nodeIndex = 0; nodeIndex < segment->size(); ++nodeIndex) {
            const Node* node = segment->getNode(nodeIndex);
            const Node* componentNode = segment->getComponentNode(node);
            // int label = segment.getLabelOfNode(nodeIndex);
            if (cycle->hasNode(componentNode)) continue;
            std::vector<int> neighborsToAdd;
            const Node* embeddingNode = embedding->getNode(nodeIndex);
            for (const Node* neighbor : embeddingNode->getNeighbors()) {
                const Node* neighborSegment = segment->getNode(neighbor->getIndex());
                const Node* neighborComponent = segment->getComponentNode(neighborSegment);
                neighborsToAdd.push_back(neighborComponent->getIndex());
            }
            if (isSegmentCompatible[i])
                for (int j = 0; j < neighborsToAdd.size(); ++j)
                    output->addSingleEdge(componentNode->getIndex(), neighborsToAdd[j]);
            else
                for (int j = neighborsToAdd.size()-1; j >= 0; --j)
                    output->addSingleEdge(componentNode->getIndex(), neighborsToAdd[j]);
        } 
    }
    return output;
}
*/