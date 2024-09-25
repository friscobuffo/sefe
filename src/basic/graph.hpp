#ifndef MY_GRAPH_H
#define MY_GRAPH_H

#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <string>

#include "utils.hpp"

class Graph;

/**
 * @class Node
 * @brief Represents a node in a graph.
 * 
 * The Node class encapsulates the properties and behaviors of a node within a graph.
 * Each node has a unique index, a list of neighboring nodes, and a reference
 * to the graph it belongs to.
 */
class Node {
private:
    const int index_m;
    std::vector<const Node*> neighbors_m;
    const Graph* graph_m;
public:
    Node(const int index, const Graph* graph);
    int getIndex() const;
    const std::vector<const Node*>& getNeighbors() const;
    std::vector<const Node*>& getNeighbors();
    void addNeighbor(const Node* neighbor);
    const Graph* getGraph() const;
};

/**
 * @class Graph
 * @brief A class representing an undirected graph structure with nodes and edges.
 * 
 * The Graph class provides functionalities to manage a graph, including adding edges,
 * retrieving nodes, computing bipartitions, and intersections with other graphs.
 */
class Graph {
private:
    bool bfsBipartition(int node, std::vector<int>& bipartition) const;
protected:
    std::vector<Node> nodes_m;
    std::vector<const Node*> nodesPointers_m;
public:
    Graph(const int numberOfNodes);
    const std::vector<const Node*> getNodes() const;
    const Node* getNode(const int index) const;
    Node* getNode(const int index);
    void addEdge(Node* from, Node* to);
    void addEdge(const int fromIndex, const int toIndex);
    int size() const;
    virtual void print() const;
    const std::optional<std::vector<int>> computeBipartition() const;
    Graph* computeIntersection(const Graph* graph) const;
    void computeIntersection(const Graph* graph, Graph* intersection) const;
    bool hasEdge(int fromIndex, int toIndex) const;
    bool isConnected() const;
    int numberOfEdges() const;
    static const Graph* loadFromFile(std::string filename) {
        int nodesNumber{};
        std::ifstream infile(filename);
        if (infile.is_open()) {
            std::string line;
            std::getline(infile, line);
            nodesNumber = stoi(line);
            Graph* graph = new Graph(nodesNumber);
            int fromIndex, toIndex;
            while (std::getline(infile, line)) {
                if (line.find("//") == 0)
                    continue;
                std::istringstream iss(line);
                if (iss >> fromIndex >> toIndex)
                    graph->addEdge(fromIndex, toIndex);
            }
            infile.close();
            return graph;
        } else {
            std::cerr << "Unable to open file" << std::endl;
            return nullptr;
        }
    }
};

/**
 * @class SubGraph
 * @brief A class representing a subgraph derived from an original graph.
 * 
 * The SubGraph class inherits from the Graph class and represents a subgraph
 * that is part of a larger, original graph. It maintains a reference to the 
 * original graph and a mapping of nodes to their counterparts in the original graph.
 */
class SubGraph : public Graph {
private:
    const Graph* originalGraph_m;
    ArrayPointers<const Node> originalNodes_m;
public:
    SubGraph(const int numberOfNodes, const Graph* graph);
    const Node* getOriginalNode(const Node* node) const;
    void setOriginalNode(const Node* node, const Node* originalNode);
    void print() const override;
};

#endif