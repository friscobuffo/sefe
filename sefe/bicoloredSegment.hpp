#ifndef MY_BICOLORED_SEGMENT_H
#define MY_BICOLORED_SEGMENT_H

#include <vector>
#include <utility>
#include <list>
#include <memory>

#include "cycle.hpp"
#include "bicoloredGraph.hpp"
#include "utils.hpp"

class BicoloredSegment : public BicoloredGraph {
private:
    std::vector<const NodeWithColors*> attachmentNodes_m{};
    std::vector<bool> isNodeAnAttachment_m{};
    const BicoloredGraph* higherLevel_m;
    const Cycle* originalCycle_m;
    ArrayPointers<const NodeWithColors> higherLevelNodesPointers_m;
    ArrayPointers<const NodeWithColors> originalNodesPointers_m;
public:
    BicoloredSegment(const int numberOfNodes, const BicoloredGraph* higherLevel, const Cycle* cycle);
    bool isPath() const;
    const std::vector<const NodeWithColors*>& getAttachments() const;
    void addAttachment(const NodeWithColors* attachment);
    bool isNodeAnAttachment(const NodeWithColors* node) const;
    std::list<const Node*> computePathBetweenAttachments(const NodeWithColors* start, const NodeWithColors* end) const;
    const Cycle* getOriginalCycle() const;
    const BicoloredGraph* getHigherLevel() const;
    const NodeWithColors* getHigherLevelNode(const Node* node) const;
    void setHigherLevelNode(const NodeWithColors* node, const NodeWithColors* componentNode);
    const NodeWithColors* getOriginalNode(const NodeWithColors* node) const;
    void setOriginalNode(const NodeWithColors* node, const NodeWithColors* componentNode);
};

class BicoloredSegmentsHandler {
private:
    std::vector<std::unique_ptr<const BicoloredSegment>> segments_m{};
    const Cycle* originalCycle_m;
    const BicoloredGraph* higherLevel_m;
    const BicoloredSegment* buildSegment(std::vector<const Node*>& nodes,
        std::vector<std::pair<const Node*, const Node*>>& edges);
    const BicoloredSegment* buildChord(const NodeWithColors* attachment1, const NodeWithColors* attachment2);
    void dfsFindSegments(const Node* node, bool isNodeVisited[],
        std::vector<const Node*>& nodesInSegment,
        std::vector<std::pair<const Node*, const Node*>>& edgesInSegment);
    void findSegments();
    void findChords();
    void segmentCheck(const BicoloredSegment* segment);
public:
    BicoloredSegmentsHandler(const SubGraph* component, const Cycle* cycle);
    const BicoloredSegment* getSegment(const int index) const;
    int size() const;
    void print() const;
};

#endif