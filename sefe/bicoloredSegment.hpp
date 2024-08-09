#ifndef MY_BICOLORED_SEGMENT_H
#define MY_BICOLORED_SEGMENT_H

#include <vector>
#include <utility>
#include <list>
#include <memory>

#include "intersectionCycle.hpp"
#include "bicoloredGraph.hpp"

class IntersectionCycle;

class BicoloredSegment : public BicoloredGraph {
private:
    std::vector<const NodeWithColors*> attachmentNodes_m{};
    std::vector<bool> isNodeAnAttachment_m{};
    const BicoloredSegment* higherLevel_m;
    const IntersectionCycle* originalCycle_m;
    ArrayPointers<const NodeWithColors> higherLevelNodesPointers_m;
    ArrayPointers<const NodeWithColors> originalNodesPointers_m;
public:
    BicoloredSegment(const int numberOfNodes, const BicoloredSegment* higherLevel, const IntersectionCycle* cycle);
    BicoloredSegment(const BicoloredGraph* bicoloredGraph);
    bool isPath() const;
    const std::vector<const NodeWithColors*>& getAttachments() const;
    void addAttachment(const NodeWithColors* attachment);
    bool isNodeAnAttachment(const NodeWithColors* node) const;
    std::list<const NodeWithColors*> computeBlackPathBetweenAttachments(const NodeWithColors* start, const NodeWithColors* end) const;
    const IntersectionCycle* getOriginalCycle() const;
    const BicoloredSegment* getHigherLevel() const;
    const NodeWithColors* getHigherLevelNode(const NodeWithColors* node) const;
    void setHigherLevelNode(const NodeWithColors* node, const NodeWithColors* higherLevelNode);
    const NodeWithColors* getOriginalNode(const NodeWithColors* node) const;
    void setOriginalNode(const NodeWithColors* node, const NodeWithColors* originalNode);
};

class BicoloredSegmentsHandler {
private:
    std::vector<std::unique_ptr<const BicoloredSegment>> segments_m{};
    const IntersectionCycle* originalCycle_m;
    const BicoloredSegment* higherLevel_m;
    const BicoloredSegment* buildSegment(std::vector<const NodeWithColors*>& nodes,
        std::vector<std::pair<const NodeWithColors*, const Edge>>& edges);
    const BicoloredSegment* buildChord(const NodeWithColors* attachment1,
        const NodeWithColors* attachment2, const Color color);
    void dfsFindSegments(const NodeWithColors* node, bool isNodeVisited[],
        std::vector<const NodeWithColors*>& nodesInSegment,
        std::vector<std::pair<const NodeWithColors*, const Edge>>& edgesInSegment);
    void findSegments();
    void findChords();
public:
    BicoloredSegmentsHandler(const BicoloredSegment* bicoloredSegment, const IntersectionCycle* cycle);
    const BicoloredSegment* getSegment(const int index) const;
    int size() const;
    void print() const;
};

#endif