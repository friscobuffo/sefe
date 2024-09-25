#include "embedderSefe.hpp"

#include <iostream>
#include <cassert>
#include <vector>

#include "../auslander-parter/biconnectedComponent.hpp"
#include "bicoloredGraph.hpp"
#include "bicoloredSegment.hpp"
#include "interlacementSefe.hpp"
#include "intersectionCycle.hpp"

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

const Embedding* EmbeddingSefe::computeRedEmbedding(const Graph* red) const {
    Embedding* embedding = new Embedding(red);
    for (int i = 0; i < size(); ++i) {
        const NodeWithColors* node = getNode(i);
        for (const Edge& edge : node->getEdges())
            if (edge.color == Color::BLACK || edge.color == Color::RED)
                embedding->addSingleEdge(node->getIndex(), edge.node->getIndex());
    }
    return embedding;
}

const Embedding* EmbeddingSefe::computeBlueEmbedding(const Graph* blue) const {
    Embedding* embedding = new Embedding(blue);
    for (int i = 0; i < size(); ++i) {
        const NodeWithColors* node = getNode(i);
        for (const Edge& edge : node->getEdges())
            if (edge.color == Color::BLACK || edge.color == Color::BLUE)
                embedding->addSingleEdge(node->getIndex(), edge.node->getIndex());
    }
    return embedding;
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
    bool isCycleNodeAttachment[cycle->size()];
    for (int i = 0; i < cycle->size(); ++i)
        isCycleNodeAttachment[i] = false;
    for (const NodeWithColors* attachment : segment->getAttachments()) {
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
        for (int i = 0; i < 3; ++i) {
            neighborsOrder[i] = -1;
            neighborsOrderColor[i] = Color::NONE;
        }
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
            assert(neighborsOrderColor[i] != Color::NONE);
            embedding->addSingleEdge(node->getIndex(), neighborsOrder[i], neighborsOrderColor[i]);
        }
    }
    return embedding;
}

std::optional<const EmbeddingSefe*> EmbedderSefe::embedGraph(const BicoloredSubGraph* graph) const {
    IntersectionCycle cycle(graph);
    return embedGraph(graph, &cycle);
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
    const Graph* intersection = graph->getIntersection();
    BiconnectedComponentsHandler bch(intersection);
    if (bch.size() > 1) {
        std::cout << "intersection must be biconnected\n";
        return std::nullopt;
    }
    const BicoloredSubGraph sGraph(graph);
    return embedGraph(&sGraph);
}

// for each segment, it computes the minimum and the maximum of all of its attachments,
// using attachment notation based on the position in the cycle (0, ..., cycleSize-1)
void EmbedderSefe::computeMinAndMaxSegmentsAttachments(const BicoloredSegmentsHandler& segmentsHandler,
int segmentsMinMaxRedAttachment[][2], int segmentsMinMaxBlueAttachment[][2],
bool segmentsHaveBetweenRedAttachment[], bool segmentsHaveBetweenBlueAttachment[]) const {
    for (int i = 0; i < segmentsHandler.size(); i++) {
        int minRed = segmentsHandler.getSegment(i)->size();
        int maxRed = -1;
        int minBlue = segmentsHandler.getSegment(i)->size();
        int maxBlue = -1;
        const BicoloredSegment* segment = segmentsHandler.getSegment(i);
        for (const NodeWithColors* attachment : segment->getAttachments()) {
            const Color color = segment->getColorOfAttachment(attachment);
            int index = attachment->getIndex();
            switch (color) {
                case Color::RED:
                    if (index < minRed) minRed = index;
                    if (index > maxRed) maxRed = index;
                    break;
                case Color::BLUE:
                    if (index < minBlue) minBlue = index;
                    if (index > maxBlue) maxBlue = index;
                    break;
                case Color::NONE:
                    std::cout << "error\n";
                    exit(1);
                    break;
                default:
                    if (index < minRed) minRed = index;
                    if (index > maxRed) maxRed = index;
                    if (index < minBlue) minBlue = index;
                    if (index > maxBlue) maxBlue = index;
                    break;
            }
        }
        segmentsMinMaxRedAttachment[i][0] = minRed;
        segmentsMinMaxRedAttachment[i][1] = maxRed;
        segmentsMinMaxBlueAttachment[i][0] = minBlue;
        segmentsMinMaxBlueAttachment[i][1] = maxBlue;
        segmentsHaveBetweenRedAttachment[i] = false;
        segmentsHaveBetweenBlueAttachment[i] = false;
        for (const NodeWithColors* attachment : segment->getAttachments()) {
            int index = attachment->getIndex();
            if (segment->isNodeRedAttachment(attachment)) {
                if (minRed < index && index < maxRed)
                    segmentsHaveBetweenRedAttachment[i] = true;
                else
                    assert(index == minRed || index == maxRed);
            }
            if (segment->isNodeBlueAttachment(attachment)) {
                if (minBlue < index && index < maxBlue)
                    segmentsHaveBetweenBlueAttachment[i] = true;
                else
                    assert(index == minBlue || index == maxBlue);
            }
            if (segmentsHaveBetweenRedAttachment[i] && segmentsHaveBetweenBlueAttachment[i])
                break;
        }
    }
}

const EmbeddingSefe* EmbedderSefe::mergeSegmentsEmbeddings(const BicoloredSubGraph* graph, const IntersectionCycle* cycle,
const std::vector<std::unique_ptr<const EmbeddingSefe>>& embeddings, const BicoloredSegmentsHandler& segmentsHandler,
const std::vector<int>& bipartition) const {
    EmbeddingSefe* output = new EmbeddingSefe(graph);
    int segmentsMinMaxRedAttachment[segmentsHandler.size()][2];
    int segmentsMinMaxBlueAttachment[segmentsHandler.size()][2];
    bool segmentsHaveBetweenRedAttachment[segmentsHandler.size()]; // true if segment has a red attachment between its min and max red attachments
    bool segmentsHaveBetweenBlueAttachment[segmentsHandler.size()]; // true if segment has a blue attachment between its min and max blue attachments
    computeMinAndMaxSegmentsAttachments(segmentsHandler, segmentsMinMaxRedAttachment, segmentsMinMaxBlueAttachment,
        segmentsHaveBetweenRedAttachment, segmentsHaveBetweenBlueAttachment);
    std::vector<bool> isSegmentCompatible = compatibilityEmbeddingsAndCycle(graph, cycle, embeddings, segmentsHandler);
    for (int cycleNodePosition = 0; cycleNodePosition < cycle->size(); ++cycleNodePosition) {
        std::vector<int> insideSegments{};
        std::vector<int> outsideSegments{};
        for (int i = 0; i < segmentsHandler.size(); ++i) {
            const NodeWithColors* node = segmentsHandler.getSegment(i)->getNode(cycleNodePosition);
            if (segmentsHandler.getSegment(i)->isNodeAnAttachment(node)) {
                if (bipartition[i] == 0) insideSegments.push_back(i);
                else outsideSegments.push_back(i);
            }
        }
        const NodeWithColors* cycleNode = cycle->getNode(cycleNodePosition);
        const NodeWithColors* prevCycleNode = cycle->getPrevOfNode(cycleNode);
        const NodeWithColors* nextCycleNode = cycle->getNextOfNode(cycleNode);
        // order of the segments inside the cycle
        std::vector<int> insideOrder = computeOrder(cycleNode, insideSegments, segmentsMinMaxRedAttachment, segmentsMinMaxBlueAttachment,
            segmentsHandler, cycleNodePosition, segmentsHaveBetweenRedAttachment, segmentsHaveBetweenBlueAttachment);
        reverseVector(insideOrder);
        // order of the segments outside the cycle
        std::vector<int> outsideOrder = computeOrder(cycleNode, outsideSegments, segmentsMinMaxRedAttachment, segmentsMinMaxBlueAttachment,
            segmentsHandler, cycleNodePosition, segmentsHaveBetweenRedAttachment, segmentsHaveBetweenBlueAttachment);
        for (int index : outsideOrder)
            isSegmentCompatible[index] = !isSegmentCompatible[index];
        output->addSingleEdge(cycleNode->getIndex(), nextCycleNode->getIndex(), Color::BLACK);
        for (int i = 0; i < insideOrder.size(); ++i) {
            const BicoloredSegment* segment = segmentsHandler.getSegment(insideOrder[i]);
            const EmbeddingSefe* embedding = embeddings[insideOrder[i]].get();
            addMiddleEdges(segment, embedding, cycleNodePosition, graph, isSegmentCompatible[insideOrder[i]], output);
        }
        output->addSingleEdge(cycleNode->getIndex(), prevCycleNode->getIndex(), Color::BLACK);
        for (int i = 0; i < outsideOrder.size(); ++i) {
            const BicoloredSegment* segment = segmentsHandler.getSegment(outsideOrder[i]);
            const EmbeddingSefe* embedding = embeddings[outsideOrder[i]].get();
            addMiddleEdges(segment, embedding, cycleNodePosition, graph, isSegmentCompatible[outsideOrder[i]], output);
        }
    }
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const BicoloredSegment* segment = segmentsHandler.getSegment(i);
        const EmbeddingSefe* embedding = embeddings[i].get();
        for (int nodeIndex = 0; nodeIndex < segment->size(); ++nodeIndex) {
            const NodeWithColors* node = segment->getNode(nodeIndex);
            const NodeWithColors* higherLevelNode = segment->getHigherLevelNode(node);
            // int label = segment.getLabelOfNode(nodeIndex);
            if (cycle->hasNode(higherLevelNode)) continue;
            std::vector<int> neighborsToAdd;
            std::vector<Color> neighborsToAddColor;
            const NodeWithColors* embeddingNode = embedding->getNode(nodeIndex);
            for (const Edge& edge : embeddingNode->getEdges()) {
                const NodeWithColors* neighbor = edge.node;
                const NodeWithColors* neighborSegment = segment->getNode(neighbor->getIndex());
                const NodeWithColors* neighborHigherLevel = segment->getHigherLevelNode(neighborSegment);
                neighborsToAdd.push_back(neighborHigherLevel->getIndex());
                neighborsToAddColor.push_back(edge.color);
            }
            if (isSegmentCompatible[i])
                for (int j = 0; j < neighborsToAdd.size(); ++j)
                    output->addSingleEdge(higherLevelNode->getIndex(), neighborsToAdd[j], neighborsToAddColor[j]);
            else
                for (int j = neighborsToAdd.size()-1; j >= 0; --j)
                    output->addSingleEdge(higherLevelNode->getIndex(), neighborsToAdd[j], neighborsToAddColor[j]);
        }
    }
    return output;
}

void EmbedderSefe::addMiddleEdges(const BicoloredSegment* segment, const EmbeddingSefe* embedding, int cycleNodeIndex,
const BicoloredSubGraph* higherLevel, bool compatible, EmbeddingSefe* output) const {
    const IntersectionCycle* cycle = segment->getOriginalCycle();
    const NodeWithColors* cycleNode = cycle->getNode(cycleNodeIndex);
    const NodeWithColors* prevCycleNode = cycle->getPrevOfNode(cycleNode);
    const NodeWithColors* nextCycleNode = cycle->getNextOfNode(cycleNode);
    assert(cycleNode->getBicoloredGraph() == higherLevel);
    assert(prevCycleNode->getBicoloredGraph() == higherLevel);
    assert(nextCycleNode->getBicoloredGraph() == higherLevel);
    assert(segment->isNodeAnAttachment(segment->getNode(cycleNodeIndex)));
    const std::vector<Edge>& edges = embedding->getNode(cycleNodeIndex)->getEdges();
    int positionOfLastAddedNode = -1;
    for (int i = 0; i < edges.size(); ++i) {
        const NodeWithColors* neighbor = edges[i].node;
        const NodeWithColors* neighborSegment = segment->getNode(neighbor->getIndex());
        const NodeWithColors* neighborHigherLevel = segment->getHigherLevelNode(neighborSegment);
        if (neighborHigherLevel->getIndex() == prevCycleNode->getIndex()) {
            positionOfLastAddedNode = i;
            break;
        }
        if (neighborHigherLevel->getIndex() == nextCycleNode->getIndex()) {
            positionOfLastAddedNode = i;
            break;
        }
    }
    assert(positionOfLastAddedNode != -1);
    std::vector<int> neighborsToAdd;
    std::vector<Color> neighborsToAddColor;
    for (int i = 1; i < edges.size(); ++i) {
        const int index = (i+positionOfLastAddedNode)%edges.size();
        const NodeWithColors* neighbor = edges[index].node;
        const NodeWithColors* neighborSegment = segment->getNode(neighbor->getIndex());
        const NodeWithColors* neighborHigherLevel = segment->getHigherLevelNode(neighborSegment);
        assert(neighborHigherLevel->getBicoloredGraph() == higherLevel);
        assert(segment->getOriginalNode(neighborSegment) == higherLevel->getOriginalNode(neighborHigherLevel));
        assert(segment->getOriginalNode(neighborSegment) == embedding->getOriginalNode(neighbor));
        if (nextCycleNode == neighborHigherLevel) continue;
        if (prevCycleNode == neighborHigherLevel) continue;
        neighborsToAdd.push_back(neighborHigherLevel->getIndex());
        neighborsToAddColor.push_back(edges[index].color);
    }
    if (compatible)
        for (int j = 0; j < neighborsToAdd.size(); ++j) {
            NodeWithColors* from = output->getNode(cycleNode->getIndex());
            NodeWithColors* to = output->getNode(neighborsToAdd[j]);
            output->addSingleEdge(from, to, neighborsToAddColor[j]);
        }
    else
        for (int j = neighborsToAdd.size()-1; j >= 0; --j) {
            NodeWithColors* from = output->getNode(cycleNode->getIndex());
            NodeWithColors* to = output->getNode(neighborsToAdd[j]);
            output->addSingleEdge(from, to, neighborsToAddColor[j]);
        }
}

// the embedding is "compatible" with the cycle if, drawn the cycle clockwise,
// the embedding of the segment places the segment inside the cycle
std::vector<bool> EmbedderSefe::compatibilityEmbeddingsAndCycle(const BicoloredSubGraph* component, const IntersectionCycle* cycle,
const std::vector<std::unique_ptr<const EmbeddingSefe>>& embeddings, const BicoloredSegmentsHandler& segmentsHandler) const {
    std::vector<bool> isCompatible(segmentsHandler.size());
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const BicoloredSegment* segment = segmentsHandler.getSegment(i);
        const EmbeddingSefe* embedding = embeddings[i].get();
        const NodeWithColors* attachment = segment->getAttachments()[0]; // any attachment is good
        const NodeWithColors* higherLevelNode = segment->getHigherLevelNode(attachment);
        const NodeWithColors* next = cycle->getNextOfNode(higherLevelNode);
        const NodeWithColors* prev = cycle->getPrevOfNode(higherLevelNode);
        int position = -1;
        int attachmentIndex = attachment->getIndex();
        const std::vector<Edge>& edges = embedding->getNode(attachmentIndex)->getEdges();
        assert(edges.size() >= 3);
        for (int j = 0; j < edges.size(); ++j) {
            const NodeWithColors* neighbor = edges[j].node;
            const NodeWithColors* higherLevelNeighbor = segment->getHigherLevelNode(neighbor);
            if (higherLevelNeighbor == next) {
                position = j;
                break;
            }
        }
        assert(position != -1);
        int nextPosition = (position+1)%edges.size();
        isCompatible[i] = (segment->getHigherLevelNode(edges[nextPosition].node) != prev);
    }
    return isCompatible;
}

// returns -1 if the first segment goes before the second,
// returns 1 if the first segment goes after the second,
// returns 0 if they can go in any order
// (considering the segments to be placed outside the cycle)
int compareSegments(const NodeWithColors* cycleNode, const BicoloredSegment* segment1, const BicoloredSegment* segment2,
int segmentsMinMaxAttachment[][2], int segment1index, int segment2index,
int cycleNodePosition, bool segmentsHasBetweenAttachment[]) {
    int seg1min = segmentsMinMaxAttachment[segment1index][0];
    int seg1max = segmentsMinMaxAttachment[segment1index][1];
    bool hasSeg1between = segmentsHasBetweenAttachment[segment1index];

    int seg2min = segmentsMinMaxAttachment[segment2index][0];
    int seg2max = segmentsMinMaxAttachment[segment2index][1];
    bool hasSeg2between = segmentsHasBetweenAttachment[segment2index];

    if (seg1max == -1 || seg2max == -1) return 0;

    if (cycleNodePosition == seg1min && cycleNodePosition == seg2min) {
        if (seg1max < seg2max) return 1;
        if (seg1max > seg2max) return -1;
        if (hasSeg1between) {
            assert(!hasSeg2between);
            return 1;
        }
        if (hasSeg2between) {
            assert(!hasSeg1between);
            return -1;
        }
        return 0;
    }
    if (cycleNodePosition == seg1min) {
        if (seg2min < seg1min) return 1;
        if (seg2min > seg1min) return -1;
        return 0;
    }
    if (cycleNodePosition == seg2min) {
        if (seg1min < seg2min) return -1;
        if (seg1min > seg2min) return 1;
        return 0;
    }
    if (cycleNodePosition == seg1max && cycleNodePosition == seg2max) {
        if (seg1min < seg2min) return 1;
        if (seg1min > seg2min) return -1;
        if (hasSeg1between) {
            assert(!hasSeg2between);
            return -1;
        }
        if (hasSeg2between) {
            assert(!hasSeg1between);
            return 1;
        }
        return 0;
    }
    if (cycleNodePosition == seg1max) {
        if (seg2max > seg1max) return -1;
        if (seg2max < seg1max) return 1;
        return 0;
    }
    if (cycleNodePosition == seg2max) {
        if (seg1max > seg2max) return 1;
        if (seg1max < seg2max) return -1;
        return 0;
    }
    return 0;
}

bool EmbedderSefe::handleDrawsOfSegments(int cycleNodePosition, const BicoloredSegmentsHandler& segmentsHandler,
int segmentsMinMaxRedAttachment[][2], int segmentsMinMaxBlueAttachment[][2], int segmentIndex1, int segmentIndex2) const {
    bool isCycleNodeMinAttachment = false;
    if (segmentsMinMaxRedAttachment[segmentIndex1][0] == cycleNodePosition ||
        segmentsMinMaxBlueAttachment[segmentIndex1][0] == cycleNodePosition ||
        segmentsMinMaxRedAttachment[segmentIndex2][0] == cycleNodePosition ||
        segmentsMinMaxBlueAttachment[segmentIndex2][0] == cycleNodePosition)
        isCycleNodeMinAttachment = true;
    if (!isCycleNodeMinAttachment) { // then it has to be max attachment
        assert(segmentsMinMaxRedAttachment[segmentIndex1][1] == cycleNodePosition ||
            segmentsMinMaxBlueAttachment[segmentIndex1][1] == cycleNodePosition ||
            segmentsMinMaxRedAttachment[segmentIndex2][1] == cycleNodePosition ||
            segmentsMinMaxBlueAttachment[segmentIndex2][1] == cycleNodePosition);
    }
    bool change = segmentIndex1 < segmentIndex2;
    if (isCycleNodeMinAttachment) change = !change;
    return change;
}

// assuming the cycle is drawn CLOCKWISE, and assuming the segments incident to the
// attachment "cycleNode" must be drawn OUTSIDE the cycle, computes the order of
// placement of these segments such that they don't intersect
std::vector<int> EmbedderSefe::computeOrder(const NodeWithColors* cycleNode, const std::vector<int>& segmentsIndexes,
int segmentsMinMaxRedAttachment[][2], int segmentsMinMaxBlueAttachment[][2], const BicoloredSegmentsHandler& segmentsHandler,
int cycleNodePosition, bool segmentsHasBetweenRedAttachment[], bool segmentsHasBetweenBlueAttachment[]) const {
    std::vector<int> order(segmentsIndexes);
    for (int i = 0; i < int(order.size())-1; ++i) {
        int min = i;
        const BicoloredSegment* minSegment = segmentsHandler.getSegment(order[min]);
        for (int j = i+1; j < order.size(); ++j) {
            const BicoloredSegment* segmentCandidate = segmentsHandler.getSegment(order[j]);
            int v = compareSegments(cycleNode, minSegment, segmentCandidate, segmentsMinMaxRedAttachment,
                order[min], order[j], cycleNodePosition, segmentsHasBetweenRedAttachment);
            if (v == 0)
                v = compareSegments(cycleNode, minSegment, segmentCandidate, segmentsMinMaxBlueAttachment,
                    order[min], order[j], cycleNodePosition, segmentsHasBetweenBlueAttachment);
            if (v < 0)
                continue;
            if (v > 0) {
                min = j;
                minSegment = segmentsHandler.getSegment(order[min]);
                continue;
            }
            assert(!segmentsHasBetweenBlueAttachment[order[min]]);
            assert(!segmentsHasBetweenBlueAttachment[order[j]]);
            assert(!segmentsHasBetweenRedAttachment[order[min]]);
            assert(!segmentsHasBetweenRedAttachment[order[j]]);
            bool changeMinSegment = handleDrawsOfSegments(cycleNodePosition, segmentsHandler, segmentsMinMaxRedAttachment,
                segmentsMinMaxBlueAttachment, order[j], order[min]);
            if (changeMinSegment) {
                min = j;
                minSegment = segmentsHandler.getSegment(order[min]);
            }
        }
        int temp = order[min];
        order[min] = order[i];
        order[i] = temp;
    }
    return order;
}