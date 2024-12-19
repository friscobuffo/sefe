#include "palm-tree.hpp"

PalmTree::PalmTree(const Graph& graph) : m_originalGaph(graph) {
    m_numberOfDescendants.resize(graph.size());
    m_lowPoint1.reserve(graph.size());
    m_lowPoint2.reserve(graph.size());
    m_fronds.resize(graph.size());
    m_newIndexToOldIndex.resize(graph.size());
    m_oldIndexToNewIndex.resize(graph.size());
    bool flag[graph.size()];
    for (int i = 0; i < graph.size(); i++) {
        flag[i] = true;
        m_numberOfDescendants[i] = 1;
        m_newIndexToOldIndex[i] = -1;
        m_oldIndexToNewIndex[i] = -1;
    }
    dfsBuildPalmTree(graph.getNode(0), flag, getRoot());
}

void PalmTree::dfsBuildPalmTree(const Node& node, bool flag[], Node& palmTreeNode) {
    m_lowPoint1[palmTreeNode.getIndex()] = palmTreeNode.getIndex();
    m_lowPoint2[palmTreeNode.getIndex()] = palmTreeNode.getIndex();
    setOldIndex(palmTreeNode, node.getIndex());
    for (const Edge& edge : node.getEdges()) {
        const Node& neighbor = edge.to;
        if (m_oldIndexToNewIndex[neighbor.getIndex()] == -1) {
            Node& child = addChild(palmTreeNode, edge.weight);
            dfsBuildPalmTree(neighbor, flag, child);
            if (getLowPoint1(child) < getLowPoint1(palmTreeNode)) {
                int min = std::min(getLowPoint1(palmTreeNode), getLowPoint2(child));
                setLowPoint2(palmTreeNode, min);
                setLowPoint1(palmTreeNode, getLowPoint1(child));
            }
            else if (getLowPoint1(child) == getLowPoint1(palmTreeNode)) {
                int min = std::min(getLowPoint2(palmTreeNode), getLowPoint2(child));
                setLowPoint2(palmTreeNode, min);
            }
            else {
                int min = std::min(getLowPoint2(palmTreeNode), getLowPoint1(child));
                setLowPoint2(palmTreeNode, min);
            }
            m_numberOfDescendants[palmTreeNode.getIndex()] += m_numberOfDescendants[child.getIndex()];
        }
        else {
            Node& palmNeighbor = getNode(m_oldIndexToNewIndex[neighbor.getIndex()]);
            if (palmNeighbor.getIndex() < palmTreeNode.getIndex()
            && ((&palmNeighbor != getParent(palmTreeNode)) || !flag[palmTreeNode.getIndex()])) {
                addFrond(palmTreeNode, palmNeighbor, edge.weight);
                if (palmNeighbor.getIndex() < getLowPoint1(palmTreeNode)) {
                    setLowPoint2(palmTreeNode, getLowPoint1(palmTreeNode));
                    setLowPoint1(palmTreeNode, palmNeighbor.getIndex());
                }
                else if (palmNeighbor.getIndex() > getLowPoint1(palmTreeNode)) {
                    int min = std::min(getLowPoint2(palmTreeNode), palmNeighbor.getIndex());
                    setLowPoint2(palmTreeNode, min);
                }
            }
        }
        Node& palmNeighbor = getNode(m_oldIndexToNewIndex[neighbor.getIndex()]);
        if (&palmNeighbor == getParent(palmTreeNode)) {
            flag[node.getIndex()] = false;
        }
    }
}

void PalmTree::setOldIndex(Node& node, int oldIndex) {
    assert(&node.getOwner() == this);
    assert(m_newIndexToOldIndex[node.getIndex()] == -1);
    assert(m_oldIndexToNewIndex[oldIndex] == -1);
    m_newIndexToOldIndex[node.getIndex()] = oldIndex;
    m_oldIndexToNewIndex[oldIndex] = node.getIndex();
}

void PalmTree::setLowPoint1(Node& node, int lowPoint1) {
    assert(&node.getOwner() == this);
    m_lowPoint1[node.getIndex()] = lowPoint1;
}

void PalmTree::setLowPoint2(Node& node, int lowPoint2) {
    assert(&node.getOwner() == this);
    m_lowPoint2[node.getIndex()] = lowPoint2;
}

std::string PalmTree::toString() const {
    std::string result = "PalmTree\n";
    for (int i = 0; i < size(); i++) {
        int oldIndex = m_newIndexToOldIndex[i];
        result += "Node " + std::to_string(oldIndex) + ":\n";
        int lowPoint1 = m_newIndexToOldIndex[m_lowPoint1[i]];
        int lowPoint2 = m_newIndexToOldIndex[m_lowPoint2[i]];
        result += "    LowPoint1: " + std::to_string(lowPoint1) + "\n";
        result += "    LowPoint2: " + std::to_string(lowPoint2) + "\n";
        result += "    NumberOfDescendants: " + std::to_string(m_numberOfDescendants[i]) + "\n";
        result += "    Children: [";
        for (const Edge& edge : getNode(i).getEdges()) {
            int childOldIndex = m_newIndexToOldIndex[edge.to.getIndex()];
            result += " " + std::to_string(childOldIndex);
        }
        result += " ]\n    Fronds: [";
        for (const Edge& edge : m_fronds[i]) {
            int frondOldIndex = m_newIndexToOldIndex[edge.to.getIndex()];
            result += " " + std::to_string(frondOldIndex);
        }
        result += " ]\n";
    }
    return result;
}

void PalmTree::addFrond(Node& from, Node& to, double weight) {
    assert(&from.getOwner() == this);
    assert(&to.getOwner() == this);
    Edge* edge = new Edge{from, to, weight, Color::BLACK};
    m_fronds[from.getIndex()].addElement(std::unique_ptr<Edge>(edge));
}

const Container<Edge>& PalmTree::getFronds(const Node& node) const {
    assert(&node.getOwner() == this);
    return m_fronds[node.getIndex()];
}

int PalmTree::getLowPoint1(const Node& node) const {
    assert(&node.getOwner() == this);
    return m_lowPoint1[node.getIndex()];
}

int PalmTree::getLowPoint2(const Node& node) const {
    assert(&node.getOwner() == this);
    return m_lowPoint2[node.getIndex()];
}