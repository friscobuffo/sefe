#include "bicoloredGraph.hpp"

#include <iostream>

/**
 * @brief Constructs a NodeWithColors object.
 * 
 * @param index The index of the node.
 * @param graph Pointer to the BicoloredGraph to which this node belongs.
 */
NodeWithColors::NodeWithColors(const int index, const BicoloredGraph* graph)
: index_m(index), graph_m(graph) {}

/**
 * @brief Retrieves the index of the node.
 * 
 * This function returns the index associated with the node. 
 * The index is a unique identifier for the node within the graph.
 * 
 * @return The index of the node.
 */
const int NodeWithColors::getIndex() const {
    return index_m;
}

/**
 * @brief Retrieves the edges associated with the node (const version).
 * 
 * This function returns a constant reference to the vector of edges
 * that are connected to the node. Each edge in the vector represents
 * a connection to another node in the graph.
 * 
 * @return const std::vector<Edge>& A constant reference to the vector of edges.
 */
const std::vector<Edge>& NodeWithColors::getEdges() const {
    return edges_m;
}

/**
 * @brief Retrieves the edges associated with the node.
 * 
 * This function returns a reference to the vector of edges
 * that are connected to the node. Each edge in the vector represents
 * a connection to another node in the graph.
 * 
 * @return std::vector<Edge>& A reference to the vector of edges.
 */
std::vector<Edge>& NodeWithColors::getEdges() {
    return edges_m;
}

/**
 * @brief Adds an edge to the current node with the specified neighbor and color.
 *
 * This function creates an edge between the current node and the given neighbor node,
 * assigning the specified color to the edge. It assumes that the edge does not already
 * exist in the graph.
 *
 * @param neighbor A pointer to the neighboring node to which the edge is added.
 * @param color The color of the edge being added.
 */
void NodeWithColors::addEdge(const NodeWithColors* neighbor, const Color color) {
    edges_m.push_back(Edge{neighbor, color});
    if (color == Color::BLACK)
        ++numberOfBlackEdges_m;
}

/**
 * @brief Retrieves the BicoloredGraph associated with the node.
 * 
 * This function returns a constant pointer to the BicoloredGraph to which
 * the node belongs.
 * 
 * @return const BicoloredGraph* A constant pointer to the BicoloredGraph.
 */
const BicoloredGraph* NodeWithColors::getBicoloredGraph() const {
    return graph_m;
}

/**
 * @brief Retrieves the number of black edges associated with the node.
 * 
 * @return The number of black edges.
 */
const int NodeWithColors::getNumberOfBlackEdges() const {
    return numberOfBlackEdges_m;
}


/**
 * @brief Constructs a BicoloredGraph from two input graphs.
 *
 * This constructor initializes a BicoloredGraph by taking two input graphs
 * and computing their intersection. It then iterates through the nodes and
 * edges of the input graphs to determine the color of the edges in the 
 * resulting BicoloredGraph. The edges are colored as follows:
 * - BLACK if the edge exists in both input graphs.
 * - RED if the edge exists only in the first input graph.
 * - BLUE if the edge exists only in the second input graph.
 *
 * @param graph1 Pointer to the first input graph.
 * @param graph2 Pointer to the second input graph.
 * 
 * @pre The sizes of graph1 and graph2 must be equal.
 */
BicoloredGraph::BicoloredGraph(const Graph* graph1, const Graph* graph2)
: BicoloredGraph(graph1->size()) {
    assert(graph1->size() == graph2->size());
    graph1->computeIntersection(graph2, &intersection_m);
    bool isEdgeInGraph1[size()];
    bool isEdgeInGraph2[size()];
    for (int i = 0; i < size(); ++i) {
        const Node* node1 = graph1->getNode(i);
        const Node* node2 = graph2->getNode(i);
        for (int j = 0; j < size(); ++j) {
            isEdgeInGraph1[j] = false;
            isEdgeInGraph2[j] = false;
        }
        for (const Node* neighbor : node1->getNeighbors())
            isEdgeInGraph1[neighbor->getIndex()] = true;
        for (const Node* neighbor : node2->getNeighbors())
            isEdgeInGraph2[neighbor->getIndex()] = true;
        for (int j = 0; j < size(); ++j) {
            if (i > j) continue;
            if (isEdgeInGraph1[j] && isEdgeInGraph2[j]) {
                addEdge(i, j, Color::BLACK);
                continue;
            }
            if (isEdgeInGraph1[j]) {
                addEdge(i, j, Color::RED);
                continue;
            }
            if (isEdgeInGraph2[j]) {
                addEdge(i, j, Color::BLUE);
                continue;
            }
        }
    }
}

/**
 * @brief Constructs a BicoloredGraph with a specified number of nodes.
 *
 * @param numberOfNodes The number of nodes to be created in the graph. Must be greater than zero.
 */
BicoloredGraph::BicoloredGraph(const int numberOfNodes)
: intersection_m(numberOfNodes) {
    assert(numberOfNodes > 0);
    for (int i = 0; i < numberOfNodes; ++i)
        nodes_m.push_back(NodeWithColors(i, this));
    for (int i = 0; i < numberOfNodes; ++i)
        nodesPointers_m.push_back(&nodes_m[i]);
    assert(nodes_m.size() == nodesPointers_m.size());
    for (int i = 0; i < size(); ++i)
        assert(getNode(i) == getNodes()[i]);
}

/**
 * @brief Computes a subgraph containing only the red and black edges.
 *
 * This function creates a new graph that includes only the edges from the 
 * original graph that are either red or black.
 *
 * @return A pointer to the newly created subgraph containing only the red 
 *         and black edges.
 */
const Graph* BicoloredGraph::computeRed() const {
    Graph* red = new Graph(size());
    for (int i = 0; i < size(); ++i) {
        const NodeWithColors* node = getNode(i);
        for (const Edge& edge : node->getEdges()) {
            if (edge.color == Color::BLACK || edge.color == Color::RED)
                if (node->getIndex() < edge.node->getIndex())
                    red->addEdge(node->getIndex(), edge.node->getIndex());
        }
    }
    return red;
}

/**
 * @brief Computes a subgraph containing only the blue and black edges.
 *
 * This function creates a new graph that includes only the edges from the 
 * original graph that are either blue or black.
 *
 * @return A pointer to the newly created subgraph containing only the blue 
 *         and black edges.
 */
const Graph* BicoloredGraph::computeBlue() const {
    Graph* blue = new Graph(size());
    for (int i = 0; i < size(); ++i) {
        const NodeWithColors* node = getNode(i);
        for (const Edge& edge : node->getEdges()) {
            if (edge.color == Color::BLACK || edge.color == Color::BLUE)
                if (node->getIndex() < edge.node->getIndex())
                    blue->addEdge(node->getIndex(), edge.node->getIndex());
        }
    }
    return blue;
}

/**
 * @brief Retrieves a node from the bicolored graph by its index.
 *
 * This function returns a constant pointer to a NodeWithColors object
 * located at the specified index within the nodes_m container.
 *
 * @param index The index of the node to retrieve.
 * @return const NodeWithColors* A constant pointer to the node at the specified index.
 */
const NodeWithColors* BicoloredGraph::getNode(const int index) const {
    return &nodes_m[index];
}

/**
 * @brief Retrieves a node with colors from the bicolored graph.
 *
 * This function returns a pointer to the NodeWithColors object at the specified index
 * within the nodes_m vector.
 *
 * @param index The index of the node to retrieve.
 * @return NodeWithColors* Pointer to the node at the specified index.
 */
NodeWithColors* BicoloredGraph::getNode(const int index) {
    return &nodes_m[index];
}

/**
 * @brief Retrieves the nodes of the BicoloredGraph.
 * 
 * This function returns a vector containing pointers to the nodes
 * with their associated colors in the BicoloredGraph.
 * 
 * @return std::vector<const NodeWithColors*> A vector of constant pointers to NodeWithColors.
 */
const std::vector<const NodeWithColors*> BicoloredGraph::getNodes() const {
    return nodesPointers_m;
}

/**
 * @brief Adds an edge between two nodes in the bicolored graph.
 *
 * This function creates a bidirectional edge between the specified nodes
 * with the given color. If the color of the edge is BLACK, it also adds
 * the edge to the intersection graph.
 *
 * @param from Pointer to the starting node of the edge.
 * @param to Pointer to the ending node of the edge.
 * @param color The color of the edge to be added.
 */
void BicoloredGraph::addEdge(NodeWithColors* from, NodeWithColors* to, Color color) {
    from->addEdge(to, color);
    to->addEdge(from, color);
    if (color == Color::BLACK)
        intersection_m.addEdge(from->getIndex(), to->getIndex());
}

/**
 * @brief Adds an edge between two nodes in the bicolored graph, given their indexes.
 * 
 * This function creates an edge between the nodes specified by their indices
 * and assigns the given color to the edge.
 *
 * @param fromIndex The index of the starting node.
 * @param toIndex The index of the ending node.
 * @param color The color to be assigned to the edge.
 */
void BicoloredGraph::addEdge(const int fromIndex, const int toIndex, Color color) {
    addEdge(getNode(fromIndex), getNode(toIndex), color);
}

/**
 * @brief Returns the number of nodes in the bicolored graph.
 * 
 * This function provides the size of the bicolored graph by returning
 * the number of nodes currently present in the graph.
 * 
 * @return The number of nodes in the bicolored graph.
 */
const int BicoloredGraph::size() const {
    return nodes_m.size();
}

/**
 * @brief Retrieves the intersection graph.
 *
 * This function returns a constant pointer to the intersection graph
 * stored within the BicoloredGraph object.
 *
 * @return A constant pointer to the intersection graph.
 */
const Graph* BicoloredGraph::getIntersection() const {
    return &intersection_m;
}

/**
 * @brief Computes a string with the details of the bicolored graph.
 *
 * This function iterates through all the nodes in the graph and saves each node's index,
 * the number of its neighboring edges, and the details of each edge. The details of each
 * edge include the index of the connected node and the color of the edge.
 *
 * The string format for each node is:
 * node: <index> neighbors: <number_of_edges> [ (<connected_node_index> <edge_color>) ... ]
 */
std::string BicoloredGraph::toString() const {
    std::ostringstream result;
    for (auto& node : nodes_m) {
        const int index = node.getIndex();
        const std::vector<Edge>& edges = node.getEdges();
        result << "node: " << index << " neighbors: " << edges.size() << " [ ";
        for (const Edge& edge : edges)
            result << "(" << edge.node->getIndex() << " " << color2string(edge.color) << ") ";
        result << "]\n";
    }
    return result.str();
}

/**
 * @brief Prints the details of the bicolored graph.
 */
void BicoloredGraph::print() const {
    std::cout << toString();
}

/**
 * @brief Constructs a BicoloredSubGraph from a given BicoloredGraph.
 *
 * This constructor initializes a BicoloredSubGraph by copying the structure
 * and edges of the provided BicoloredGraph. It sets up the original nodes
 * and edges, ensuring that each edge is added only once based on the node indices.
 *
 * @param graph A pointer to the BicoloredGraph from which the subgraph is constructed.
 */
BicoloredSubGraph::BicoloredSubGraph(const BicoloredGraph* graph)
: BicoloredGraph(graph->size()), originalNodes_m(graph->size()), originalGraph_m(graph) {
    for (const NodeWithColors* node : graph->getNodes()) {
        setOriginalNode(getNode(node->getIndex()), node);
        const std::vector<Edge>& edges = node->getEdges();
        for (const Edge& edge : edges) {
            const NodeWithColors* neighbor = edge.node;
            if (node->getIndex() < neighbor->getIndex()) continue;
            addEdge(node->getIndex(), neighbor->getIndex(), edge.color);
        }
    }
}

/**
 * @brief Constructs a BicoloredSubGraph object.
 * 
 * This constructor initializes a BicoloredSubGraph with a specified number of nodes
 * and a reference to an existing BicoloredGraph.
 * 
 * @param numberOfNodes The number of nodes in the subgraph.
 * @param graph A pointer to the original BicoloredGraph from which this subgraph is derived.
 */
BicoloredSubGraph::BicoloredSubGraph(const int numberOfNodes, const BicoloredGraph* graph)
: BicoloredGraph(numberOfNodes), originalNodes_m(numberOfNodes), originalGraph_m(graph) {}

/**
 * @brief Retrieves the original node corresponding to the given node.
 *
 * This function takes a pointer to a `NodeWithColors` object and returns a pointer
 * to the original node from which it was derived.
 *
 * @param node A pointer to the `NodeWithColors` object for which the original node is to be retrieved.
 * @return A pointer to the original `NodeWithColors` object.
 */
const NodeWithColors* BicoloredSubGraph::getOriginalNode(const NodeWithColors* node) const {
    const int index = node->getIndex();
    return originalNodes_m.getPointer(index);
}

/**
 * @brief Sets the original node for a given node in the bicolored subgraph.
 *
 * This function assigns an original node to a node in the bicolored subgraph.
 * It uses the index of the provided node to set the corresponding original node
 * in the internal storage.
 *
 * @param node Pointer to the node in the bicolored subgraph.
 * @param originalNode Pointer to the original node to be associated with the given node.
 */
void BicoloredSubGraph::setOriginalNode(const NodeWithColors* node, const NodeWithColors* originalNode) {
    const int index = node->getIndex();
    originalNodes_m.setPointer(index, originalNode);
}

/**
 * @brief Prints the details of the bicolored subgraph.
 * 
 * This function iterates through all the nodes in the bicolored subgraph and prints each node's 
 * original index, the number of its neighbors, and a list of its edges. For each edge, it prints 
 * the index of the connected node and the color of the edge.
 * 
 * @note The function assumes that the methods getNodes(), getOriginalNode(), and color2string() 
 * are defined and accessible within the scope of this class.
 */
void BicoloredSubGraph::print() const {
    for (const NodeWithColors* node : getNodes()) {
        const int originalIndex = getOriginalNode(node)->getIndex();
        const std::vector<Edge>& edges = node->getEdges();
        std::cout << "node: " << originalIndex << " neighbors: " << edges.size() << " [ ";
        for (const Edge& edge : edges)
            std::cout << "(" << getOriginalNode(edge.node)->getIndex() << " " << color2string(edge.color) << ") ";
        std::cout << "]\n";
    }
}

/**
 * @brief Retrieves the original bicolored graph associated with this subgraph.
 * 
 * This function returns a pointer to the original BicoloredGraph from which 
 * this BicoloredSubGraph is derived. The returned pointer is constant, 
 * ensuring that the original graph cannot be modified through this function.
 * 
 * @return const BicoloredGraph* A constant pointer to the original BicoloredGraph.
 */
const BicoloredGraph* BicoloredSubGraph::getOriginalGraph() const {
    return originalGraph_m;
}
