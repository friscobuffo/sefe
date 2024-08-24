#ifndef MY_BICOLORED_GRAPH_H
#define MY_BICOLORED_GRAPH_H

#include <vector>
#include <cassert>

#include "../basic/utils.hpp"
#include "../basic/graph.hpp"

enum Color {
    RED,
    BLUE,
    BLACK,
    RED_AND_BLUE,
    NONE,
};

inline const char* color2string(const Color color) {
    switch (color) {
        case Color::RED: return "red";
        case Color::BLUE: return "blue";
        case Color::BLACK: return "black";
        default: assert(false);
    }
}

class NodeWithColors;

struct Edge {
    const NodeWithColors* node;
    const Color color;
};

class BicoloredGraph;

class NodeWithColors {
private:
    const int index_m;
    std::vector<Edge> edges_m;
    const BicoloredGraph* graph_m;
    int numberOfBlackEdges_m = 0;
public:
    NodeWithColors(const int index, const BicoloredGraph* graph);
    const int getIndex() const;
    const std::vector<Edge>& getEdges() const;
    std::vector<Edge>& getEdges();
    void addEdge(const NodeWithColors* neighbor, const Color color);
    const BicoloredGraph* getBicoloredGraph() const;
    const int getNumberOfBlackEdges() const;
};

class BicoloredGraph {
private:
    Graph intersection_m;
protected:
    std::vector<NodeWithColors> nodes_m;
    std::vector<const NodeWithColors*> nodesPointers_m;
public:
    void addEdge(NodeWithColors* from, NodeWithColors* to, Color color);
    void addEdge(const int fromIndex, const int toIndex, Color color);
    BicoloredGraph(const Graph* graph1, const Graph* graph2);
    BicoloredGraph(const int numberOfNodes);
    const NodeWithColors* getNode(const int index) const;
    NodeWithColors* getNode(const int index);
    const std::vector<const NodeWithColors*> getNodes() const;
    const int size() const;
    virtual void print() const;
    const Graph* getIntersection() const;
};

class BicoloredSubGraph : public BicoloredGraph {
private:
    const BicoloredGraph* originalGraph_m;
    ArrayPointers<const NodeWithColors> originalNodes_m;
public:
    BicoloredSubGraph(const int numberOfNodes, const BicoloredGraph* graph);
    BicoloredSubGraph(const BicoloredGraph* graph);
    const NodeWithColors* getOriginalNode(const NodeWithColors* node) const;
    void setOriginalNode(const NodeWithColors* node, const NodeWithColors* originalNode);
    void print() const override;
    const BicoloredGraph* getOriginalGraph() const;
};

#endif
