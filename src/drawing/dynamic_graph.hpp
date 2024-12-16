#ifndef MY_DYNAMIC_GRAPH_H
#define MY_DYNAMIC_GRAPH_H

#include <vector>
#include <iostream>
#include <algorithm>
#include <limits>

template <typename T>
class GenericNode;

template <typename T>
struct Edge {
public:
    const GenericNode<T>* neighbor;
    const double weight;
};

template <typename T>
class GenericNode {
private:
    const int m_index;
    std::vector<Edge<T>> m_edges;
    T m_content;
public:
    GenericNode(const int index, T content) 
    : m_index(index), m_content(content) {}

    int getIndex() const {
        return m_index;
    }

    T& getContent() {
        return m_content;
    }

    const std::vector<Edge<T>>& getEdges() const {
        return m_edges;
    }

    std::vector<Edge<T>>& getEdges() {
        return m_edges;
    }

    void addEdge(const GenericNode& neighbor, const double weight) {
        m_edges.push_back(Edge<T>{&neighbor, weight});
    }
};

template <typename T>
class DynamicGraph {
private:
    std::vector<GenericNode<T>> m_nodes;
public:
    void addNode(T content) {
        int index = m_nodes.size();
        m_nodes.push_back(GenericNode<T>(index, content));
    }

    const GenericNode<T>& getNode(const int index) const {
        return m_nodes[index];
    }

    GenericNode<T>& getNode(const int index) {
        return m_nodes[index];
    }

    void addEdge(GenericNode<T>& from, GenericNode<T>& to, double weight) {
        from.addEdge(to, weight);
        to.addEdge(from, weight);
    }

    void addEdge(const int fromIndex, const int toIndex, double weight) {
        GenericNode<T>& from = getNode(fromIndex);
        GenericNode<T>& to = getNode(toIndex);
        addEdge(from, to, weight);
    }

    int size() const {
        return m_nodes.size();
    }

    void print() {
        for (GenericNode<T>& node : m_nodes) {
            std::cout << "Node " << node.getIndex() << " with content " << node.getContent() << " has edges: ";
            for (const auto& edge : node.getEdges())
                std::cout << edge.neighbor->getIndex() << " with weight " << edge.weight << ", ";
            std::cout << std::endl;
        }
    }

    std::vector<int> shortestPath(const int fromIndex, const int toIndex) {
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
            for (const auto& edge : m_nodes[u].getEdges()) {
                int v = edge.neighbor->getIndex();
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
};

#endif