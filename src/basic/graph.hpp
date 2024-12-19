#ifndef MY_GRAPH_H
#define MY_GRAPH_H

#include <optional>
#include <string>
#include <memory>
#include <vector>

#include "utils.hpp"
#include "color.hpp"
#include "node.hpp"

class SubGraph;

class Graph : public NodesStructure {
private:
    bool bfsBipartition(int nodeIndex, std::vector<int>& bipartition) const;
public:
    virtual void addNode(Color color);
    void addEdge(Node& from, Node& to, double weight, Color color);
    void addEdge(int fromIndex, int toIndex, double weight, Color color);
    void addSingleEdge(int fromIndex, int toIndex, double weight, Color color);
    void addSingleEdge(Node& from, Node& to, double weight, Color color);
    std::vector<int> shortestPath(int fromIndex, int toIndex) const;
    std::optional<std::vector<int>> computeBipartition() const;
    std::unique_ptr<Graph> computeIntersection(const Graph& graph) const;
    bool hasEdge(int fromIndex, int toIndex) const;
    bool isConnected() const;
    int totalNumberOfEdges() const;
    std::unique_ptr<SubGraph> computeBlackProjection() const;
    std::unique_ptr<SubGraph> computeRedProjection() const;
    std::unique_ptr<SubGraph> computeBlueProjection() const;
    bool isEmpty() const;
    static std::unique_ptr<Graph> loadFromFile(std::string filename);
    static std::unique_ptr<Graph> makeUnionGraph(const Graph& graph1, const Graph& graph2);
    const std::vector<int> findBoundingFace(int p1index, int p2index, Color color) const;
    virtual std::unique_ptr<SubGraph> createCopy() const;
    virtual std::string toString() const override;
    bool isGraphUndirected() const;
};

class SubGraph : public Graph {
private:
    const Graph& originalGraph_m;
    ArrayPointers<const Node> originalNodes_m;
public:
    SubGraph(const SubGraph* graph);
    SubGraph(const Graph* graph);
    const Node& getOriginalNode(const Node& node) const;
    void setOriginalNode(const Node& node, const Node& originalNode);
    void resetOriginalNode(const Node& node);
    const Graph& getOriginalGraph() const;
    std::unique_ptr<SubGraph> createCopy() const override;
    std::string toString() const override;
};

#endif