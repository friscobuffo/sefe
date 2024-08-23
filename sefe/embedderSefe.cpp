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
        if (segment->isPath())
            return true;
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