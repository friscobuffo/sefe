#ifndef MY_BICOLORED_GRAPH_H
#define MY_BICOLORED_GRAPH_H

#include <vector>
#include <cassert>

#include "../basic/utils.hpp"
#include "../basic/graph.hpp"

/**
 * @enum Color
 * @brief Represents the color of edges and attachments in a bicolored graph.
 * 
 * This enumeration defines the possible colors that an edge or an attachment
 * in a bicolored graph can have.
 */
enum Color {
    RED,
    BLUE,
    BLACK,
    RED_AND_BLUE,
    NONE,
};

/**
 * @brief Converts a Color enum value to its corresponding string representation.
 *
 * This function takes a Color enum value and returns a constant character pointer
 * to a string that represents the color. The possible color values and their
 * corresponding strings are:
 * - Color::RED: "red"
 * - Color::BLUE: "blue"
 * - Color::BLACK: "black"
 *
 * @param color The Color enum value to be converted to a string.
 * @return A constant character pointer to the string representation of the color.
 * @note If the color value does not match any of the predefined cases, the function
 *       will trigger an assertion failure.
 */
inline const char* color2string(const Color color) {
    switch (color) {
        case Color::RED: return "red";
        case Color::BLUE: return "blue";
        case Color::BLACK: return "black";
        default: assert(false);
    }
}

class NodeWithColors;

/**
 * @struct Edge
 * @brief Represents an edge in a bicolored graph.
 * 
 * This structure defines an edge in a bicolored graph, which consists of a pointer
 * to a node and a color. The color indicates the type of the edge.
 */
struct Edge {
    const NodeWithColors* const node;
    const Color color;
};

class BicoloredGraph;

/**
 * @class NodeWithColors
 * @brief Represents a node in a bicolored graph, maintaining its index, edges, and associated graph.
 *
 * This class encapsulates the properties and behaviors of a node within a bicolored graph. 
 * Each node keeps track of its index, the edges connected to it, and the graph it belongs to. 
 * Additionally, it maintains a count of the number of black edges connected to it.
 *
 * @note The index and graph pointers are constant and cannot be modified after initialization.
 */
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

/**
 * @class BicoloredGraph
 * @brief Represents a graph with nodes that can have two colors and supports operations on such graphs.
 *
 * This class provides functionalities to create and manipulate a bicolored graph. It supports adding edges
 * between nodes with specified colors, retrieving nodes, and computing subgraphs based on colors.
 *
 * @note The graph is constructed using two other graphs or a specified number of nodes.
 */
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
    const Graph* computeRed() const;
    const Graph* computeBlue() const;
    std::string toString() const;
};

/**
 * @class BicoloredSubGraph
 * @brief A class representing a subgraph of a bicolored graph.
 * 
 * This class extends the BicoloredGraph class and represents a subgraph
 * of an existing bicolored graph. It maintains a reference to the original
 * graph and provides methods to map nodes between the subgraph and the 
 * original graph.
 * 
 * @note
 * The class uses an array of pointers to store the mapping between nodes
 * in the subgraph and the original graph.
 * 
 * @see BicoloredGraph
 */
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
