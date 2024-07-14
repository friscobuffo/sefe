#ifndef MY_BICOLORED_GRAPH_H
#define MY_BICOLORED_GRAPH_H

#include <vector>
#include <optional>
#include <memory>

#include "graph.hpp"

enum Color {
    RED,
    BLUE,
    BOTH
};

inline const char* color2string(const Color color) {
    switch (color) {
        case RED: return "red";
        case BLUE: return "blue";
        case BOTH: return "both";
        default: return "error";
    }
}

struct Edge {
    const NodeWithColors* node;
    const Color color;
};

class NodeWithColors {
private:
    const int index_m;
    std::vector<Edge> edges_m;
    const BicoloredGraph* graph_m;
public:
    NodeWithColors(const int index, const BicoloredGraph* graph);
    int getIndex() const;
    const std::vector<Edge>& getEdges() const;
    std::vector<Edge>& getEdges();
    void addEdge(const NodeWithColors* neighbor, const Color color);
    const BicoloredGraph* getBicoloredGraph() const;
};

class BicoloredGraph {
private:
    const Graph* graphRed_m;
    const Graph* graphBlue_m;
    std::unique_ptr<const Graph> intersection_m;
protected:
    std::vector<std::unique_ptr<NodeWithColors>> nodes_m;
    void addEdge(NodeWithColors* from, NodeWithColors* to, Color color);
    void addEdge(const int fromIndex, const int toIndex, Color color);
public:
    BicoloredGraph(const Graph* graph1, const Graph* graph2);
    const NodeWithColors* getNode(const int index) const;
    NodeWithColors* getNode(const int index);
    int size() const;
    virtual void print() const;
};

#endif