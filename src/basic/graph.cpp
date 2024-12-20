#include "graph.hpp"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <limits>
#include <list>
#include <fstream>
#include <cassert>

void Graph::addNode(Color color) {
    int index = m_nodes.size();
    m_nodes.addElement(std::make_unique<Node>(index, color, *this));
}

void Graph::addEdge(Node& from, Node& to, double weight, Color color) {
    assert(&from.getOwner() == this);
    assert(&to.getOwner() == this);
    assert(from.getIndex() != to.getIndex());
    assert(from.getIndex() < size());
    assert(to.getIndex() < size());
    from.addEdge(to, weight, color);
    to.addEdge(from, weight, color);
}

void Graph::addEdge(int fromIndex, int toIndex, double weight, Color color) {
    Node& from = getNode(fromIndex);
    Node& to = getNode(toIndex);
    addEdge(from, to, weight, color);
}

void Graph::addSingleEdge(Node& from, Node& to, double weight, Color color) {
    assert(&from.getOwner() == this);
    assert(&to.getOwner() == this);
    assert(&from != &to);
    from.addEdge(to, weight, color);
}

void Graph::addSingleEdge(int fromIndex, int toIndex, double weight, Color color) {
    assert(fromIndex != toIndex);
    assert(fromIndex < size());
    assert(toIndex < size());
    assert(fromIndex >= 0);
    assert(toIndex >= 0);
    Node& from = getNode(fromIndex);
    Node& to = getNode(toIndex);
    addSingleEdge(from, to, weight, color);
}

std::vector<int> Graph::shortestPath(int fromIndex, int toIndex) const {
    std::vector<int> path;
    std::vector<double> distances(m_nodes.size(), std::numeric_limits<double>::infinity());
    std::vector<int> previous(m_nodes.size(), -1);
    std::vector<bool> visited(m_nodes.size(), false);
    distances[fromIndex] = 0;
    for (int i = 0; i < m_nodes.size(); ++i) {
        int u = -1;
        for (int j = 0; j < m_nodes.size(); ++j)
            if (!visited[j] && (u == -1 || distances[j] < distances[u]))
                u = j;
        if (distances[u] == std::numeric_limits<double>::infinity())
            break;
        visited[u] = true;
        for (const Edge& edge : m_nodes[u].getEdges()) {
            int v = edge.to.getIndex();
            double weight = edge.weight;
            if (distances[u] + weight < distances[v]) {
                distances[v] = distances[u] + weight;
                previous[v] = u;
            }
        }
    }
    for (int at = toIndex; at != -1; at = previous[at])
        path.push_back(at);
    std::reverse(path.begin(), path.end());
    if (path.size() == 1 && path[0] != fromIndex)
        path.clear();
    return path;
}

std::optional<std::vector<int>> Graph::computeBipartition() const {
    std::vector<int> bipartition{};
    bipartition.resize(size());
    for (int i = 0; i < size(); ++i)
        bipartition[i] = -1;
    for (int i = 0; i < size(); ++i) {
        if (bipartition[i] == -1) {
            bool check = bfsBipartition(i, bipartition);
            if (!check) return std::nullopt;
        }
    }
    return bipartition;
}

bool Graph::bfsBipartition(int nodeIndex, std::vector<int>& bipartition) const {
    bipartition[nodeIndex] = 0;
    std::list<int> queue{};
    queue.push_back(nodeIndex);
    while (queue.size() > 0) {
        nodeIndex = queue.front();
        queue.pop_front();
        const Node& node = getNode(nodeIndex);
        for (const Edge& edge : node.getEdges()) {
            int neighborIndex = edge.to.getIndex();
            if (bipartition[neighborIndex] == -1) {
                bipartition[neighborIndex] = 1-bipartition[nodeIndex];
                queue.push_back(neighborIndex);
            }
            else
                if (bipartition[neighborIndex] == bipartition[nodeIndex]) return false;
        }
    }
    return true;
}

std::unique_ptr<Graph> Graph::computeIntersection(const Graph& graph) const {
    assert(size() == graph.size());
    std::unique_ptr<Graph> intersection = std::make_unique<Graph>();
    for (int i = 0; i < size(); ++i)
        intersection->addNode(Color::BLACK);
    bool isEdgeInGraph1[size()];
    bool isEdgeInGraph2[size()];
    for (int i = 0; i < size(); ++i) {
        const Node& node1 = getNode(i);
        const Node& node2 = graph.getNode(i);
        for (int j = 0; j < size(); ++j) {
            isEdgeInGraph1[j] = false;
            isEdgeInGraph2[j] = false;
        }
        for (const Edge& edge : node1.getEdges())
            isEdgeInGraph1[edge.to.getIndex()] = true;
        for (const Edge& edge : node2.getEdges())
            isEdgeInGraph2[edge.to.getIndex()] = true;
        for (int j = 0; j < size(); ++j)
            if (isEdgeInGraph1[j] && isEdgeInGraph2[j] && i < j)
                intersection->addEdge(i, j, 1.0, Color::BLACK);
    }
    return intersection;
}

bool Graph::hasEdge(int fromIndex, int toIndex) const {
    if (getNode(fromIndex).getEdges().size() > getNode(toIndex).getEdges().size()) {
        int temp = fromIndex;
        fromIndex = toIndex;
        toIndex = temp;
    }
    const Node& node = getNode(fromIndex);
    for (const Edge& edge : node.getEdges())
        if (edge.to.getIndex() == toIndex)
            return true;
    return false;
}

bool Graph::isConnected() const {
    std::vector<bool> visited(size(), false);
    std::list<int> queue{};
    queue.push_back(0);
    visited[0] = true;
    while (queue.size() > 0) {
        int nodeIndex = queue.front();
        queue.pop_front();
        const Node& node = getNode(nodeIndex);
        for (const Edge& edge : node.getEdges()) {
            int neighborIndex = edge.to.getIndex();
            if (!visited[neighborIndex]) {
                visited[neighborIndex] = true;
                queue.push_back(neighborIndex);
            }
        }
    }
    for (bool visit : visited)
        if (!visit) return false;
    return true;
}

int Graph::totalNumberOfEdges() const {
    int edges = 0;
    for (int i = 0; i < size(); ++i)
        edges += getNode(i).getEdges().size();
    return edges;
}

std::unique_ptr<SubGraph> Graph::computeRedProjection() const {
    std::unique_ptr<SubGraph> red = std::make_unique<SubGraph>(this);
    for (int i = 0; i < size(); ++i) {
        red->addNode(Color::BLACK);
        red->setOriginalNode(red->getNode(i), getNode(i));
    }
    for (int i = 0; i < size(); ++i) {
        const Node& node = getNode(i);
        for (const Edge& edge : node.getEdges())
            if (edge.color == Color::BLACK || edge.color == Color::RED)
                red->addSingleEdge(node.getIndex(), edge.to.getIndex(), edge.weight, edge.color);
    }
    return red;
}

std::unique_ptr<SubGraph> Graph::computeBlueProjection() const {
    std::unique_ptr<SubGraph> blue = std::make_unique<SubGraph>(this);
    for (int i = 0; i < size(); ++i) {
        blue->addNode(Color::BLACK);
        blue->setOriginalNode(blue->getNode(i), getNode(i));
    }
    for (int i = 0; i < size(); ++i) {
        const Node& node = getNode(i);
        for (const auto& edge : node.getEdges())
            if (edge.color == Color::BLACK || edge.color == Color::BLUE)
                blue->addSingleEdge(node.getIndex(), edge.to.getIndex(), edge.weight, edge.color);
    }
    return blue;
}

std::unique_ptr<SubGraph> Graph::computeBlackProjection() const {
    std::unique_ptr<SubGraph> black = std::make_unique<SubGraph>(this);
    for (int i = 0; i < size(); ++i) {
        black->addNode(Color::BLACK);
        black->setOriginalNode(black->getNode(i), getNode(i));
    }
    for (int i = 0; i < size(); ++i) {
        const Node& node = getNode(i);
        for (const auto& edge : node.getEdges())
            if (edge.color == Color::BLACK)
                black->addSingleEdge(node.getIndex(), edge.to.getIndex(), edge.weight, Color::BLACK);
    }
    return black;
}

bool Graph::isEmpty() const {
    return size() == 0;
}

std::string Graph::toString() const {
    std::ostringstream result;
    for (const Node& node : m_nodes) {
        const int index = node.getIndex();
        auto& edges = node.getEdges();
        result << "node: " << index << " neighbors: " << edges.size() << " [ ";
        for (const Edge& edge : edges)
            result << "(" << edge.to.getIndex() << " " << color2string(edge.color) << ") ";
        result << "]\n";
    }
    return result.str();
}

// checks that each edge is present in both directions
bool Graph::isGraphUndirected() const {
    bool foundEdge[size()][size()];
    for (int i = 0; i < size(); ++i)
        for (int j = 0; j < size(); ++j)
            foundEdge[i][j] = false;
    for (int i = 0; i < size(); ++i) {
        const Node& node = getNode(i);
        for (const Edge& edge : node.getEdges()) {
            const Node& neighbor = edge.to;
            if (foundEdge[i][neighbor.getIndex()])
                return false;
            foundEdge[i][neighbor.getIndex()] = true;
        }
    }
    return true;
}

std::unique_ptr<Graph> Graph::loadFromFile(std::string filename) {
    int nodesNumber{};
    std::ifstream infile(filename);
    if (infile.is_open()) {
        std::string line;
        std::getline(infile, line);
        nodesNumber = stoi(line);
        std::unique_ptr<Graph> graph = std::make_unique<Graph>();
        for (int i = 0; i < nodesNumber; ++i)
            graph->addNode(Color::BLACK);
        int fromIndex, toIndex;
        while (std::getline(infile, line)) {
            if (line.find("//") == 0)
                continue;
            std::istringstream iss(line);
            if (iss >> fromIndex >> toIndex)
                graph->addEdge(fromIndex, toIndex, 1.0, Color::BLACK);
        }
        infile.close();
        return graph;
    }
    std::cout << "Unable to open file\n";
    return std::make_unique<Graph>();
}

std::unique_ptr<Graph> Graph::makeUnionGraph(const Graph& graph1, const Graph& graph2) {
    std::unique_ptr<Graph> unionGraph = std::make_unique<Graph>();
    assert(graph1.size() == graph2.size());
    for (int i = 0; i < graph1.size(); ++i)
        unionGraph->addNode(Color::BLACK);
    bool isEdgeInGraph1[graph1.size()];
    bool isEdgeInGraph2[graph1.size()];
    for (int i = 0; i < graph1.size(); ++i) {
        const Node& node1 = graph1.getNode(i);
        const Node& node2 = graph2.getNode(i);
        for (int j = 0; j < graph1.size(); ++j) {
            isEdgeInGraph1[j] = false;
            isEdgeInGraph2[j] = false;
        }
        for (const auto& edge : node1.getEdges())
            isEdgeInGraph1[edge.to.getIndex()] = true;
        for (const auto& edge : node2.getEdges())
            isEdgeInGraph2[edge.to.getIndex()] = true;
        for (int j = 0; j < graph1.size(); ++j) {
            if (i > j) continue;
            if (isEdgeInGraph1[j] && isEdgeInGraph2[j]) {
                unionGraph->addEdge(i, j, 1.0, Color::BLACK);
                continue;
            }
            if (isEdgeInGraph1[j]) {
                unionGraph->addEdge(i, j, 1.0, Color::RED);
                continue;
            }
            if (isEdgeInGraph2[j]) {
                unionGraph->addEdge(i, j, 1.0, Color::BLUE);
                continue;
            }
        }
    }
    return unionGraph;
}

const std::vector<int> Graph::findBoundingFace(int p1index, int p2index, Color color) const {
    Color color2ignore;
    switch (color) {
        case Color::RED:
            color2ignore = Color::BLUE;
            break;
        case Color::BLUE:
            color2ignore = Color::RED;
            break;
        default:
            exit(1);
    }
    std::vector<int> face{};
    const Node& p1 = getNode(p1index);
    const Node& p2 = getNode(p2index);
    const Node* current = &p1;
    const Node* prev = &p2;
    do {
        face.push_back(current->getIndex());
        for (int i = 0; i < current->getEdges().size(); ++i) {
            const auto& edge = current->getEdges()[i];
            int degree = current->getEdges().size();
            if (&edge.to == prev) {
                int nextEdgeIndex = (i+1)%degree;
                while ((current->getEdges()[nextEdgeIndex].color == color2ignore)
                || (current == &p2 && &current->getEdges()[nextEdgeIndex].to == &p1)) {
                    nextEdgeIndex = (nextEdgeIndex+1)%degree;
                }
                const Node* temp = current;
                prev = current;
                current = &temp->getEdges()[nextEdgeIndex].to;
                break;
            }
        }
    } while (current != &p1);
    return face;
}

std::unique_ptr<SubGraph> Graph::createCopy() const {
    std::unique_ptr<SubGraph> subGraph = std::make_unique<SubGraph>(this);
    for (int i = 0; i < size(); ++i) {
        subGraph->addNode(Color::BLACK);
        const Node& originalNode = getNode(i);
        subGraph->setOriginalNode(subGraph->getNode(i), originalNode);
    }
    for (int i = 0; i < size(); ++i)
        for (const auto& edge : getNode(i).getEdges())
            if (i < edge.to.getIndex())
                subGraph->addEdge(i, edge.to.getIndex(), edge.weight, edge.color);
    return subGraph;
}

SubGraph::SubGraph(const Graph* graph) 
: originalGraph_m(*graph), originalNodes_m{} {
    assert(size() == 0);
}

SubGraph::SubGraph(const SubGraph* graph) 
: originalGraph_m(graph->getOriginalGraph()), originalNodes_m{} {
    assert(size() == 0);
}

const Node& SubGraph::getOriginalNode(const Node& node) const {
    const int index = node.getIndex();
    assert(&node.getOwner() == this);
    const Node& originalNode = *originalNodes_m.getPointer(index);
    assert(&originalNode.getOwner() == &originalGraph_m);
    return originalNode;
}

void SubGraph::setOriginalNode(const Node& node, const Node& originalNode) {
    const int index = node.getIndex();
    assert(&node.getOwner() == this);
    assert(&originalNode.getOwner() == &originalGraph_m);
    originalNodes_m.setPointer(index, &originalNode);
}

void SubGraph::resetOriginalNode(const Node& node) {
    const int index = node.getIndex();
    assert(&node.getOwner() == this);
    assert(&getOriginalNode(node).getOwner() == &originalGraph_m);
    originalNodes_m.resetPointer(index);
}

std::string SubGraph::toString() const {
    std::ostringstream result;
    for (const Node& node : m_nodes) {
        const int originalIndex = getOriginalNode(node).getIndex();
        result << "Node " << originalIndex << " has " << node.degree() << " edges:\n";
        for (const Edge& edge : node.getEdges()) {
            result << "    ";
            int originalIndexNeighbor = getOriginalNode(edge.to).getIndex();
            result << originalIndexNeighbor << " (weight " << edge.weight << " - ";
            result << "color " << color2string(edge.color) << ")\n";
        }
    }
    return result.str();
}

const Graph& SubGraph::getOriginalGraph() const {
    return originalGraph_m;
}

std::unique_ptr<SubGraph> SubGraph::createCopy() const {
    std::unique_ptr<SubGraph> subGraph = std::make_unique<SubGraph>(&this->getOriginalGraph());
    for (int i = 0; i < size(); ++i) {
        subGraph->addNode(Color::BLACK);
        const Node& node = getNode(i);
        subGraph->setOriginalNode(subGraph->getNode(i), getOriginalNode(node));
    }
    for (int i = 0; i < size(); ++i)
        for (const auto& edge : getNode(i).getEdges())
            if (i < edge.to.getIndex())
                subGraph->addEdge(i, edge.to.getIndex(), edge.weight, edge.color);
    return subGraph;
}