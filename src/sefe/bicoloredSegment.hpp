#ifndef MY_BICOLORED_SEGMENT_H
#define MY_BICOLORED_SEGMENT_H

#include <vector>
#include <utility>
#include <list>
#include <memory>

#include "intersectionCycle.hpp"
#include "bicoloredGraph.hpp"

class IntersectionCycle;

class BicoloredSegment : public BicoloredSubGraph {
private:
    std::vector<const NodeWithColors*> attachmentNodes_m{};
    std::vector<Color> attachmentColor_m{};
    const BicoloredSubGraph* higherLevel_m;
    const IntersectionCycle* originalCycle_m;
    ArrayPointers<const NodeWithColors> higherLevelNodesPointers_m;
public:
    BicoloredSegment(const int numberOfNodes, const BicoloredSubGraph* higherLevel, const IntersectionCycle* cycle);
    bool isBlackPath() const;
    bool isPath() const;
    const int getNumberOfAttachments() const;
    const NodeWithColors* getAttachment(const int index) const;
    void addAttachment(const NodeWithColors* attachment, const Color color);
    bool isNodeAnAttachment(const NodeWithColors* node) const;
    bool isNodeRedAttachment(const NodeWithColors* node) const;
    bool isNodeBlueAttachment(const NodeWithColors* node) const;
    bool isNodeBlackAttachment(const NodeWithColors* node) const;
    bool isNodeAttachmentOfColor(const NodeWithColors* node, const Color color) const;
    std::list<const NodeWithColors*> computeBlackPathBetweenAttachments(const NodeWithColors* start,
        const NodeWithColors* end) const;
    const IntersectionCycle* getOriginalCycle() const;
    const BicoloredSubGraph* getHigherLevel() const;
    const NodeWithColors* getHigherLevelNode(const NodeWithColors* node) const;
    void setHigherLevelNode(const NodeWithColors* node, const NodeWithColors* higherLevelNode);
    void print() const;
};

class BicoloredSegmentsHandler {
private:
    std::vector<std::unique_ptr<const BicoloredSegment>> segments_m{};
    const IntersectionCycle* originalCycle_m;
    const BicoloredSubGraph* higherLevel_m;
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
    BicoloredSegmentsHandler(const BicoloredSubGraph* graph, const IntersectionCycle* cycle);
    const BicoloredSegment* getSegment(const int index) const;
    int size() const;
    void print() const;
};

#endif