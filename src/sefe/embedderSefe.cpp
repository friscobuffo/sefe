#include "embedderSefe.hpp"

#include <iostream>
#include <cassert>
#include <vector>

#include "../components/biconnectedComponent.hpp"
#include "interlacementSefe.hpp"

bool EmbedderSefe::testSefe(const Graph& graph1, const Graph& graph2) const {
    std::unique_ptr<Graph> unionGraph =  Graph::makeUnionGraph(graph1, graph2);
    std::unique_ptr<Graph> intersection = unionGraph->computeBlackProjection();
    BiconnectedComponentsHandler bicCompHandler(*intersection);
    if (bicCompHandler.size() > 1) {
        std::cout << "intersection must be biconnected\n";
        return false;
    }
    std::unique_ptr<SubGraph> subGraph = unionGraph->createCopy();
    BlackCycle cycle(*subGraph);
    return testSefe(*subGraph, cycle);
}

// assumes intersection is biconnected
bool EmbedderSefe::testSefe(const SubGraph& unionGraph, BlackCycle& cycle) const {
    const SegmentsHandler segmentsHandler(&unionGraph, &cycle);
    if (segmentsHandler.size() == 0) // entire biconnected component is a cycle
        return true;
    if (segmentsHandler.size() == 1) {
        const Segment& segment = segmentsHandler.getSegment(0);
        if (segment.isPath())
            return true;
        // chosen cycle is bad
        makeCycleGood(cycle, segment);
        return testSefe(unionGraph, cycle);
    }
    InterlacementGraphSefe interlacementGraph(cycle, segmentsHandler);
    std::optional<std::vector<int>> bipartition = interlacementGraph.computeBipartition();
    if (!bipartition) return false;
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const Segment& segment = segmentsHandler.getSegment(i);
        BlackCycle cycle(segment);
        if (!testSefe(segment, cycle)) return false;
    }
    return true;
}

// it may happen that a cycle induces only one segment, which is not a base case
// so the cycle must be recomputed such that it ensures at least two segments
void EmbedderSefe::makeCycleGood(BlackCycle& cycle, const Segment& segment) const {
    assert(!segment.isPath());
    bool isCycleNodeAttachment[cycle.size()];
    for (int i = 0; i < cycle.size(); ++i)
        isCycleNodeAttachment[i] = false;
    for (const Node* attachment : segment.getAttachments()) {
        const Node& attachmentHigherLevel = segment.getHigherLevelNode(*attachment);
        isCycleNodeAttachment[cycle.getPositionOfNode(attachmentHigherLevel).value()] = true;
    }
    int foundAttachments = 0;
    const Node* attachmentsToUse[2];
    const Node* attachmentToInclude = nullptr;
    for (int i = 0; i < cycle.size(); ++i) {
        if (!isCycleNodeAttachment[i]) continue;
        const Node& node = cycle.getNode(i);
        const Node& nodeSegment = segment.getNode(i);
        assert(&segment.getHigherLevelNode(nodeSegment) == &node);
        if (!segment.isNodeBlackAttachment(nodeSegment)) {
            attachmentToInclude = &nodeSegment;
            continue;
        }
        if (foundAttachments < 2)
            attachmentsToUse[foundAttachments++] = &nodeSegment;
        else
            attachmentToInclude = &nodeSegment;
        if (foundAttachments == 2 && attachmentToInclude != nullptr) break;
    }
    std::list<const Node*> path = segment.computePathBetweenAttachments(*attachmentsToUse[0], *attachmentsToUse[1], Color::BLACK);
    std::list<const Node*> pathHigherLevel;
    for (const Node* node : path)
        pathHigherLevel.push_back(&segment.getHigherLevelNode(*node));
    if (attachmentToInclude != nullptr) attachmentToInclude = &segment.getHigherLevelNode(*attachmentToInclude);
    cycle.changeWithPath(pathHigherLevel, attachmentToInclude);
}

// base case: biconnected component is a cycle
const SubGraph* EmbedderSefe::baseCaseCycle(const SubGraph& cycle) const {
    SubGraph* embedding = new SubGraph(&cycle);
    for (int i = 0; i < cycle.size(); ++i) {
        embedding->addNode(Color::BLACK);
        embedding->setOriginalNode(embedding->getNode(i), cycle.getOriginalNode(cycle.getNode(i)));
    }
    for (int i = 0; i < cycle.size()-1; ++i)
        for (const Edge& edge : cycle.getNode(i).getEdges())
            if (i < edge.to.getIndex())
                embedding->addEdge(i, edge.to.getIndex(), edge.weight, edge.color);
    return embedding;
}

std::optional<const SubGraph*> EmbedderSefe::embedGraph(const SubGraph& graph,
BlackCycle& cycle) const {
    const SegmentsHandler* segmentsHandler = new SegmentsHandler(&graph, &cycle);
    std::unique_ptr<const SegmentsHandler> segmentsHandlerPtrUnique(segmentsHandler);
    if (segmentsHandler->size() == 0) // entire biconnected component is a cycle
        return baseCaseCycle(graph);
    if (segmentsHandler->size() == 1) {
        const Segment& segment = segmentsHandler->getSegment(0);
        if (segment.isPath())
            return baseCasePath(graph, cycle);
        // chosen cycle is bad
        makeCycleGood(cycle, segment);
        segmentsHandlerPtrUnique.release();
        delete segmentsHandler;
        segmentsHandler = new SegmentsHandler(&graph, &cycle);
        segmentsHandlerPtrUnique.reset(segmentsHandler);
        if (segmentsHandler->size() == 1) return std::nullopt;
    }
    InterlacementGraphSefe interlacementGraph(cycle, *segmentsHandler);
    std::optional<std::vector<int>> bipartition = interlacementGraph.computeBipartition();
    if (!bipartition) return std::nullopt;
    std::vector<std::unique_ptr<const SubGraph>> embeddings{};
    for (int i = 0; i < segmentsHandler->size(); ++i) {
        const Segment& segment = segmentsHandler->getSegment(i);
        std::optional<const SubGraph*> embedding = embedGraph(segment);
        if (!embedding.has_value()) return std::nullopt;
        embeddings.push_back(std::unique_ptr<const SubGraph>(embedding.value()));
    }
    const SubGraph* embedding = mergeSegmentsEmbeddings(graph, cycle, embeddings, *segmentsHandler, bipartition.value());
    return embedding;
}

// base case: segment is a path
const SubGraph* EmbedderSefe::baseCasePath(const SubGraph& component, const BlackCycle& cycle) const {
    SubGraph* embedding = new SubGraph(&component);
    for (int i = 0; i < component.size(); ++i) {
        embedding->addNode(Color::BLACK);
        embedding->setOriginalNode(embedding->getNode(i), component.getOriginalNode(component.getNode(i)));
    }
    for (int nodeIndex = 0; nodeIndex < component.size(); ++nodeIndex) {
        const Node& node = component.getNode(nodeIndex);
        const auto& edges = node.getEdges();
        if (edges.size() == 2) { // attachment nodes will be handled later
            embedding->addSingleEdge(node.getIndex(), edges[0].to.getIndex(), edges[0].weight, edges[0].color);
            embedding->addSingleEdge(node.getIndex(), edges[1].to.getIndex(), edges[1].weight, edges[1].color);
            continue;
        }
        assert(edges.size() == 3);
        int neighborsOrder[3];
        Color neighborsOrderColor[3];
        double neighborsOrderWeight[3];
        for (int i = 0; i < 3; ++i) {
            neighborsOrder[i] = -1;
        }
        for (const auto& edge : edges) {
            const Node& neighbor = edge.to;
            if (&cycle.getNextOfNode(node) == &neighbor) {
                neighborsOrder[0] = neighbor.getIndex();
                neighborsOrderColor[0] = edge.color;
                neighborsOrderWeight[0] = edge.weight;
                continue;
            }
            if (&cycle.getPrevOfNode(node) == &neighbor) {
                neighborsOrder[2] = neighbor.getIndex();
                neighborsOrderColor[2] = edge.color;
                neighborsOrderWeight[2] = edge.weight;
                continue;
            }
            neighborsOrder[1] = neighbor.getIndex();
            neighborsOrderColor[1] = edge.color;
            neighborsOrderWeight[1] = edge.weight;
        }
        for (int i = 0; i < 3; ++i) {
            assert(neighborsOrder[i] != -1);
            embedding->addSingleEdge(node.getIndex(), neighborsOrder[i], neighborsOrderWeight[i], neighborsOrderColor[i]);
        }
    }
    return embedding;
}

std::optional<const SubGraph*> EmbedderSefe::embedGraph(const SubGraph& graph) const {
    BlackCycle cycle(graph);
    return embedGraph(graph, cycle);
}

// base case: graph has <4 nodes
const SubGraph* EmbedderSefe::baseCaseGraph(const Graph& graph) const {
    assert(graph.size() < 4);
    SubGraph* embedding = new SubGraph(&graph);
    for (int i = 0; i < graph.size(); ++i) {
        embedding->addNode(Color::BLACK);
        embedding->setOriginalNode(embedding->getNode(i), graph.getNode(i));
    }
    for (int i = 0; i < graph.size(); ++i) {
        const Node& node = graph.getNode(i);
        for (const Edge& edge : node.getEdges())
            if (node.getIndex() < edge.to.getIndex())
                embedding->addEdge(node.getIndex(), edge.to.getIndex(), edge.weight, edge.color);
    }
    return embedding;
}

std::optional<std::unique_ptr<const SubGraph>> EmbedderSefe::embedGraph(const Graph& graph) const {
    if (graph.size() < 4) return std::unique_ptr<const SubGraph>(baseCaseGraph(graph));
    std::unique_ptr<const SubGraph> intersection = graph.computeBlackProjection();
    BiconnectedComponentsHandler bch(*intersection);
    if (bch.size() > 1) {
        std::cout << "intersection must be biconnected\n";
        return std::nullopt;
    }
    std::optional<const SubGraph*> embedding = embedGraph(*graph.createCopy());
    if (!embedding.has_value()) return std::nullopt;
    return std::unique_ptr<const SubGraph>(embedding.value());
}

// for each segment, it computes the minimum and the maximum of all of its attachments,
// using attachment notation based on the position in the cycle (0, ..., cycleSize-1)
void EmbedderSefe::computeMinAndMaxSegmentsAttachments(const SegmentsHandler& segmentsHandler,
int segmentsMinMaxRedAttachment[][2], int segmentsMinMaxBlueAttachment[][2],
bool segmentsHaveBetweenRedAttachment[], bool segmentsHaveBetweenBlueAttachment[]) const {
    for (int i = 0; i < segmentsHandler.size(); i++) {
        int minRed = segmentsHandler.getSegment(i).size();
        int maxRed = -1;
        int minBlue = segmentsHandler.getSegment(i).size();
        int maxBlue = -1;
        const Segment& segment = segmentsHandler.getSegment(i);
        for (const Node* attachment : segment.getAttachments()) {
            const Color color = segment.getColorOfAttachment(*attachment);
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
        for (const Node* attachment : segment.getAttachments()) {
            int index = attachment->getIndex();
            if (segment.isNodeRedAttachment(*attachment)) {
                if (minRed < index && index < maxRed)
                    segmentsHaveBetweenRedAttachment[i] = true;
                else
                    assert(index == minRed || index == maxRed);
            }
            if (segment.isNodeBlueAttachment(*attachment)) {
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

const SubGraph* EmbedderSefe::mergeSegmentsEmbeddings(const SubGraph& graph, const BlackCycle& cycle,
const std::vector<std::unique_ptr<const SubGraph>>& embeddings, const SegmentsHandler& segmentsHandler,
const std::vector<int>& bipartition) const {
    SubGraph* output = new SubGraph(&graph);
    for (int i = 0; i < graph.size(); ++i) {
        output->addNode(Color::BLACK);
        output->setOriginalNode(output->getNode(i), graph.getOriginalNode(graph.getNode(i)));
    }
    int segmentsMinMaxRedAttachment[segmentsHandler.size()][2];
    int segmentsMinMaxBlueAttachment[segmentsHandler.size()][2];
    bool segmentsHaveBetweenRedAttachment[segmentsHandler.size()]; // true if segment has a red attachment between its min and max red attachments
    bool segmentsHaveBetweenBlueAttachment[segmentsHandler.size()]; // true if segment has a blue attachment between its min and max blue attachments
    computeMinAndMaxSegmentsAttachments(segmentsHandler, segmentsMinMaxRedAttachment, segmentsMinMaxBlueAttachment,
        segmentsHaveBetweenRedAttachment, segmentsHaveBetweenBlueAttachment);
    std::vector<bool> isSegmentCompatible = compatibilityEmbeddingsAndCycle(graph, cycle, embeddings, segmentsHandler);
    for (int i = 0; i < segmentsHandler.size(); ++i)
        if (bipartition[i] == 1) isSegmentCompatible[i] = !isSegmentCompatible[i];
    for (int cycleNodePosition = 0; cycleNodePosition < cycle.size(); ++cycleNodePosition) {
        std::vector<int> insideSegments{};
        std::vector<int> outsideSegments{};
        for (int i = 0; i < segmentsHandler.size(); ++i) {
            const Node& node = segmentsHandler.getSegment(i).getNode(cycleNodePosition);
            if (segmentsHandler.getSegment(i).isNodeAnAttachment(node)) {
                if (bipartition[i] == 0) insideSegments.push_back(i);
                else outsideSegments.push_back(i);
            }
        }
        const Node& cycleNode = cycle.getNode(cycleNodePosition);
        const Node& prevCycleNode = cycle.getPrevOfNode(cycleNode);
        const Node& nextCycleNode = cycle.getNextOfNode(cycleNode);
        // order of the segments inside the cycle
        std::vector<int> insideOrder = computeOrder(cycleNode, insideSegments, segmentsMinMaxRedAttachment, segmentsMinMaxBlueAttachment,
            segmentsHandler, cycleNodePosition, segmentsHaveBetweenRedAttachment, segmentsHaveBetweenBlueAttachment);
        reverseVector(insideOrder);
        // order of the segments outside the cycle
        std::vector<int> outsideOrder = computeOrder(cycleNode, outsideSegments, segmentsMinMaxRedAttachment, segmentsMinMaxBlueAttachment,
            segmentsHandler, cycleNodePosition, segmentsHaveBetweenRedAttachment, segmentsHaveBetweenBlueAttachment);
        output->addSingleEdge(cycleNode.getIndex(), nextCycleNode.getIndex(), 1.0, Color::BLACK);
        for (int i = 0; i < insideOrder.size(); ++i) {
            const Segment& segment = segmentsHandler.getSegment(insideOrder[i]);
            const SubGraph& embedding = *embeddings[insideOrder[i]];
            addMiddleEdges(segment, embedding, cycleNodePosition, graph, isSegmentCompatible[insideOrder[i]], *output);
        }
        output->addSingleEdge(cycleNode.getIndex(), prevCycleNode.getIndex(), 1.0, Color::BLACK);
        for (int i = 0; i < outsideOrder.size(); ++i) {
            const Segment& segment = segmentsHandler.getSegment(outsideOrder[i]);
            const SubGraph& embedding = *embeddings[outsideOrder[i]];
            addMiddleEdges(segment, embedding, cycleNodePosition, graph, isSegmentCompatible[outsideOrder[i]], *output);
        }
    }
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const Segment& segment = segmentsHandler.getSegment(i);
        const SubGraph& embedding = *embeddings[i];
        for (int nodeIndex = 0; nodeIndex < segment.size(); ++nodeIndex) {
            const Node& node = segment.getNode(nodeIndex);
            const Node& higherLevelNode = segment.getHigherLevelNode(node);
            if (cycle.hasNode(higherLevelNode)) continue;
            std::vector<int> neighborsToAdd;
            std::vector<Color> neighborsToAddColor;
            std::vector<double> neighborsToAddWeight;
            const Node& embeddingNode = embedding.getNode(nodeIndex);
            for (const Edge& edge : embeddingNode.getEdges()) {
                const Node& neighbor = edge.to;
                const Node& neighborSegment = segment.getNode(neighbor.getIndex());
                const Node& neighborHigherLevel = segment.getHigherLevelNode(neighborSegment);
                neighborsToAdd.push_back(neighborHigherLevel.getIndex());
                neighborsToAddColor.push_back(edge.color);
                neighborsToAddWeight.push_back(edge.weight);
            }
            if (isSegmentCompatible[i])
                for (int j = 0; j < neighborsToAdd.size(); ++j)
                    output->addSingleEdge(higherLevelNode.getIndex(), neighborsToAdd[j], neighborsToAddWeight[j], neighborsToAddColor[j]);
            else
                for (int j = neighborsToAdd.size()-1; j >= 0; --j)
                    output->addSingleEdge(higherLevelNode.getIndex(), neighborsToAdd[j], neighborsToAddWeight[j], neighborsToAddColor[j]);
        }
    }
    return output;
}

void EmbedderSefe::addMiddleEdges(const Segment& segment, const SubGraph& embedding, int cycleNodeIndex,
const SubGraph& higherLevel, bool compatible, SubGraph& output) const {
    const BlackCycle& cycle = segment.getOriginalCycle();
    const Node& cycleNode = cycle.getNode(cycleNodeIndex);
    const Node& prevCycleNode = cycle.getPrevOfNode(cycleNode);
    const Node& nextCycleNode = cycle.getNextOfNode(cycleNode);
    assert(&cycleNode.getOwner() == &higherLevel);
    assert(&prevCycleNode.getOwner() == &higherLevel);
    assert(&nextCycleNode.getOwner() == &higherLevel);
    assert(segment.isNodeAnAttachment(segment.getNode(cycleNodeIndex)));
    const auto& edges = embedding.getNode(cycleNodeIndex).getEdges();
    int positionOfLastAddedNode = -1;
    for (int i = 0; i < edges.size(); ++i) {
        const Node& neighbor = edges[i].to;
        const Node& neighborSegment = segment.getNode(neighbor.getIndex());
        const Node& neighborHigherLevel = segment.getHigherLevelNode(neighborSegment);
        if (neighborHigherLevel.getIndex() == prevCycleNode.getIndex()) {
            positionOfLastAddedNode = i;
            break;
        }
        if (neighborHigherLevel.getIndex() == nextCycleNode.getIndex()) {
            positionOfLastAddedNode = i;
            break;
        }
    }
    assert(positionOfLastAddedNode != -1);
    std::vector<int> neighborsToAdd;
    std::vector<Color> neighborsToAddColor;
    std::vector<double> neighborsToAddWeight;
    for (int i = 1; i < edges.size(); ++i) {
        const int index = (i+positionOfLastAddedNode)%edges.size();
        const Node& neighbor = edges[index].to;
        const Node& neighborSegment = segment.getNode(neighbor.getIndex());
        const Node& neighborHigherLevel = segment.getHigherLevelNode(neighborSegment);
        assert(&neighborHigherLevel.getOwner() == &higherLevel);
        assert(&segment.getOriginalNode(neighborSegment) == &higherLevel.getOriginalNode(neighborHigherLevel));
        assert(&segment.getOriginalNode(neighborSegment) == &embedding.getOriginalNode(neighbor));
        if (&nextCycleNode == &neighborHigherLevel) continue;
        if (&prevCycleNode == &neighborHigherLevel) continue;
        neighborsToAdd.push_back(neighborHigherLevel.getIndex());
        neighborsToAddColor.push_back(edges[index].color);
        neighborsToAddWeight.push_back(edges[index].weight);
    }
    if (compatible)
        for (int j = 0; j < neighborsToAdd.size(); ++j) {
            Node& from = output.getNode(cycleNode.getIndex());
            Node& to = output.getNode(neighborsToAdd[j]);
            output.addSingleEdge(from, to, neighborsToAddWeight[j], neighborsToAddColor[j]);
        }
    else
        for (int j = neighborsToAdd.size()-1; j >= 0; --j) {
            Node& from = output.getNode(cycleNode.getIndex());
            Node& to = output.getNode(neighborsToAdd[j]);
            output.addSingleEdge(from, to, neighborsToAddWeight[j], neighborsToAddColor[j]);
        }
}

// the embedding is "compatible" with the cycle if, drawn the cycle clockwise,
// the embedding of the segment places the segment inside the cycle
std::vector<bool> EmbedderSefe::compatibilityEmbeddingsAndCycle(const SubGraph& component, const BlackCycle& cycle,
const std::vector<std::unique_ptr<const SubGraph>>& embeddings, const SegmentsHandler& segmentsHandler) const {
    std::vector<bool> isCompatible(segmentsHandler.size());
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const Segment& segment = segmentsHandler.getSegment(i);
        const SubGraph& embedding = *embeddings[i];
        const Node& attachment = *segment.getAttachments()[0]; // any attachment is good
        const Node& higherLevelNode = segment.getHigherLevelNode(attachment);
        const Node& next = cycle.getNextOfNode(higherLevelNode);
        const Node& prev = cycle.getPrevOfNode(higherLevelNode);
        int position = -1;
        int attachmentIndex = attachment.getIndex();
        const auto& edges = embedding.getNode(attachmentIndex).getEdges();
        assert(edges.size() >= 3);
        for (int j = 0; j < edges.size(); ++j) {
            const Node& neighbor = edges[j].to;
            const Node& higherLevelNeighbor = segment.getHigherLevelNode(neighbor);
            if (&higherLevelNeighbor == &next) {
                position = j;
                break;
            }
        }
        assert(position != -1);
        int nextPosition = (position+1)%edges.size();
        isCompatible[i] = (&segment.getHigherLevelNode(edges[nextPosition].to) != &prev);
    }
    return isCompatible;
}

// returns -1 if the first segment goes before the second,
// returns 1 if the first segment goes after the second,
// returns 0 if they can go in any order
// (considering the segments to be placed outside the cycle)
int EmbedderSefe::compareSegments(int segmentsMinMaxAttachment[][2], int segment1index, int segment2index,
int cycleNodePosition, bool segmentsHasBetweenAttachment[]) const {
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

bool EmbedderSefe::handleDrawsOfSegments(int cycleNodePosition, const SegmentsHandler& segmentsHandler,
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
std::vector<int> EmbedderSefe::computeOrder(const Node& cycleNode, const std::vector<int>& segmentsIndexes,
int segmentsMinMaxRedAttachment[][2], int segmentsMinMaxBlueAttachment[][2], const SegmentsHandler& segmentsHandler,
int cycleNodePosition, bool segmentsHasBetweenRedAttachment[], bool segmentsHasBetweenBlueAttachment[]) const {
    std::vector<int> order(segmentsIndexes);
    for (int i = 0; i < int(order.size())-1; ++i) {
        int min = i;
        for (int j = i+1; j < order.size(); ++j) {
            int v = compareSegments(segmentsMinMaxRedAttachment, order[min], order[j],
                cycleNodePosition, segmentsHasBetweenRedAttachment);
            if (v == 0)
                v = compareSegments(segmentsMinMaxBlueAttachment, order[min], order[j],
                    cycleNodePosition, segmentsHasBetweenBlueAttachment);
            if (v < 0)
                continue;
            if (v > 0) {
                min = j;
                continue;
            }
            bool changeMinSegment = handleDrawsOfSegments(cycleNodePosition, segmentsHandler, segmentsMinMaxRedAttachment,
                segmentsMinMaxBlueAttachment, order[j], order[min]);
            if (changeMinSegment)
                min = j;
        }
        int temp = order[min];
        order[min] = order[i];
        order[i] = temp;
    }
    return order;
}