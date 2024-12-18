#ifndef MY_SEGMENT_H
#define MY_SEGMENT_H

#include <vector>
#include <utility>
#include <list>
#include <memory>

#include "cycle.hpp"
#include "../basic/graph.hpp"
#include "../basic/utils.hpp"

class Segment : public SubGraph {
private:
    std::vector<const Node*> attachmentNodes_m{};
    std::vector<Color> attachmentColor_m{};
    const SubGraph& higherLevel_m;
    const BlackCycle& originalCycle_m;
    ArrayPointers<const Node> higherLevelNodesPointers_m{};
public:
    Segment(const SubGraph* higherLevel, const BlackCycle* cycle);
    bool isPath() const;
    const std::vector<const Node*>& getAttachments() const;
    void addAttachment(const Node& attachment, Color color);
    bool isNodeAnAttachment(const Node& node) const;
    bool isNodeRedAttachment(const Node& node) const;
    bool isNodeBlueAttachment(const Node& node) const;
    bool isNodeBlackAttachment(const Node& node) const;
    bool isNodeAttachmentOfColor(const Node& node, const Color color) const;
    Color getColorOfAttachment(const Node& attachment) const;
    void addNode(Color color=Color::BLACK) override;
    std::list<const Node*> computePathBetweenAttachments(const Node& start,
        const Node& end, Color color=Color::ANY) const;
    const BlackCycle& getOriginalCycle() const;
    const SubGraph& getHigherLevel() const;
    const Node& getHigherLevelNode(const Node& node) const;
    void setHigherLevelNode(const Node& node, const Node& higherLevelNode);
};

class SegmentsHandler {
private:
    std::vector<std::unique_ptr<Segment>> segments_m{};
    const BlackCycle& originalCycle_m;
    const SubGraph& higherLevel_m;
    void buildSegment(Segment& segment, std::vector<const Node*>& nodes,
        std::vector<std::pair<const Node*, const Edge*>>& edges);
    void buildChord(Segment& chord, const Node& attachment1, const Node& attachment2, Color color);
    void dfsFindSegments(const Node& node, bool isNodeVisited[],
        std::vector<const Node*>& nodesInSegment,
        std::vector<std::pair<const Node*, const Edge*>>& edgesInSegment);
    void findSegments();
    void findChords();
public:
    SegmentsHandler(const SubGraph* higherLevel, const BlackCycle* cycle);
    const Segment& getSegment(int index) const;
    int size() const;
    void print() const;
};

#endif
