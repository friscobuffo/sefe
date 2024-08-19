#include "embedderSefe.hpp"

#include <iostream>

#include "../auslander-parter/biconnectedComponent.hpp"
#include "interlacementSefe.hpp"

bool EmbedderSefe::testSefe(const Graph* graph1, const Graph* graph2) const {
    BicoloredGraph bicoloredGraph(graph1, graph2);
    const Graph* intersection = bicoloredGraph.getIntersection();
    BiconnectedComponentsHandler bicCompHandler(intersection);
    if (bicCompHandler.size() > 1) {
        std::cout << "intersection must be biconnected\n";
        return false;
    }
    BicoloredSegment bicoloredSegment(&bicoloredGraph);
    IntersectionCycle cycle(&bicoloredSegment);
    return testSefe(&bicoloredSegment, &cycle);
}

// assumes intersection is biconnected
bool EmbedderSefe::testSefe(const BicoloredSegment* bicoloredSegment, IntersectionCycle* cycle) const {
    const BicoloredSegmentsHandler segmentsHandler = BicoloredSegmentsHandler(bicoloredSegment, cycle);
    if (segmentsHandler.size() == 0) // entire biconnected component is a cycle
        return true;
    if (segmentsHandler.size() == 1) {
        const BicoloredSegment* segment = segmentsHandler.getSegment(0);
        if (segment->isBlackPath())
            return isBlackPathGood();
        // chosen cycle is bad
        makeCycleGood(cycle, segment);
        return testSefe(bicoloredSegment, cycle);
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
    std::vector<int> attachmentsComponent{};
    for (int i = 0; i < segment->getNumberOfAttachments(); ++i) {
        const NodeWithColors* attachment = segment->getAttachment(i);
        attachmentsComponent.push_back(segment->getHigherLevelNode(attachment)->getIndex());
    }
    int foundAttachments = 0;
    const NodeWithColors* attachmentsToUse[2];
    for (int i = 0; i < cycle->size(); ++i) {
        const NodeWithColors* node = cycle->getNode(i);
        int index = findIndex(attachmentsComponent, node->getIndex());
        if (index == -1) continue;
        const NodeWithColors* attachment = segment->getAttachment(index);
        if (!segment->isNodeBlackAttachment(attachment))
            continue;
        attachmentsToUse[foundAttachments++] = segment->getAttachment(index);
        if (foundAttachments == 2) break;
    }
    std::list<const NodeWithColors*> path = segment->computeBlackPathBetweenAttachments(attachmentsToUse[0], attachmentsToUse[1]);
    std::list<const NodeWithColors*> pathHigherLevel;
    for (const NodeWithColors* node : path)
        pathHigherLevel.push_back(segment->getHigherLevelNode(node));
    cycle->changeWithPath(pathHigherLevel);
}

bool EmbedderSefe::isBlackPathGood() const {
    return true;
}