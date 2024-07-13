#ifndef MY_SEGMENT_H
#define MY_SEGMENT_H

#include <vector>
#include <utility>
#include <list>
#include <memory>

#include "cycle.hpp"
#include "biconnectedComponent.hpp"

class Segment : public SubGraph {
private:
    std::vector<const Node*> attachmentNodes_m{};
    std::vector<bool> isNodeAnAttachment_m{};
    const SubGraph* originalComponent_m;
    const Cycle* originalCycle_m;
    NodePointers componentNodesPointers_m;
public:
    Segment(int numberOfNodes, const SubGraph* originalComponent, const Cycle* cycle);
    bool isPath() const;
    const std::vector<const Node*>& getAttachments() const;
    void addAttachment(const Node* attachment);
    bool isNodeAnAttachment(const Node* node) const;
    std::list<const Node*> computePathBetweenAttachments(const Node* start, const Node* end) const;
    const Cycle* getOriginalCycle() const;
    const SubGraph* getOriginalComponent() const;
    const Node* getComponentNode(const Node* node) const;
    void setComponentNode(const Node* node, const Node* componentNode);
};

class SegmentsHandler {
private:
    std::vector<std::unique_ptr<const Segment>> segments_m{};
    const Cycle* originalCycle_m;
    const SubGraph* originalComponent_m;
    const Segment* buildSegment(std::vector<const Node*>& nodes,
        std::vector<std::pair<const Node*, const Node*>>& edges);
    const Segment* buildChord(const Node* attachment1, const Node* attachment2);
    void dfsFindSegments(const Node* node, bool isNodeVisited[],
        std::vector<const Node*>& nodesInSegment,
        std::vector<std::pair<const Node*, const Node*>>& edgesInSegment);
    void findSegments();
    void findChords();
    void segmentCheck(const Segment* segment);
public:
    SegmentsHandler(const SubGraph* component, const Cycle* cycle);
    const Segment* getSegment(int index) const;
    int size() const;
    void print() const;
};

#endif