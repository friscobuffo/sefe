#include "embedder.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/planarlayout/PlanarDrawLayout.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/planarity/EmbedderModule.h>

#include "interlacement.hpp"
#include "../basic/utils.hpp"

Embedding::Embedding(const SubGraph* originalGraph)
: SubGraph(originalGraph->size(), originalGraph) {
    for (int i = 0; i < size(); ++i) {
        const Node* node = originalGraph->getNode(i);
        const Node* originalNode = originalGraph->getOriginalNode(node);
        setOriginalNode(getNode(i), originalNode);
    }
}

Embedding::Embedding(const Graph* originalGraph)
: SubGraph(originalGraph->size(), originalGraph) {
    for (int i = 0; i < size(); ++i)
        setOriginalNode(getNode(i), originalGraph->getNode(i));
}

void Embedding::addSingleEdge(Node* from, const Node* to) {
    from->addNeighbor(to);
}

void Embedding::addSingleEdge(int fromIndex, int toIndex) {
    Node* from = getNode(fromIndex);
    Node* to = getNode(toIndex);
    addSingleEdge(from, to);
}

void Embedder::graphAndEmbeddingChecks(const Graph* graph, const Embedding* embedding) const {
    assert(graph->size() == embedding->size());
    for (int i = 0; i < graph->size(); ++i) {
        const Node* node = graph->getNode(i);
        const Node* nodeEmbedding = embedding->getNode(i);
        assert(embedding->getOriginalNode(nodeEmbedding) == node);
        assert(nodeEmbedding->getNeighbors().size() == node->getNeighbors().size());
    }
}

const Embedding* Embedder::mergeBiconnectedComponents(const Graph* graph, const BiconnectedComponentsHandler& biconnectedComponents,
const std::vector<std::unique_ptr<const Embedding>>& embeddings) const {
    Embedding* output = new Embedding(graph);
    assert(biconnectedComponents.size() == embeddings.size());
    for (int i = 0; i < biconnectedComponents.size(); ++i) {
        const SubGraph* component = biconnectedComponents.getComponent(i);
        const Embedding* embedding = embeddings[i].get();
        for (int i = 0; i < embedding->size(); ++i) {
            const Node* node = embedding->getNode(i);
            const Node* nodeOriginal = embedding->getOriginalNode(node);
            for (const Node* neighbor : node->getNeighbors()) {
                const Node* neighborOriginal = embedding->getOriginalNode(neighbor);
                output->addSingleEdge(nodeOriginal->getIndex(), neighborOriginal->getIndex());
            }
        }
    }
    return output;
}

std::optional<const Embedding*> Embedder::embedGraph(const Graph* graph) const {
    if (graph->size() < 4) return baseCaseGraph(graph);
    const BiconnectedComponentsHandler bicComps(graph);
    std::vector<std::unique_ptr<const Embedding>> embeddings{};
    for (int i = 0; i < bicComps.size(); ++i) {
        const SubGraph* component = bicComps.getComponent(i);
        std::optional<const Embedding*> embedding = embedComponent(component);
        if (!embedding.has_value())
            return std::nullopt;
        embeddings.push_back(std::unique_ptr<const Embedding>(embedding.value()));
    }
    const Embedding* embedding = mergeBiconnectedComponents(graph, bicComps, embeddings);
    graphAndEmbeddingChecks(graph, embedding);
    return embedding;
}

// for each segment, it computes the minimum and the maximum of all of its attachments,
// using attachment notation based on the position in the cycle (0, ..., cycleSize-1)
void Embedder::computeMinAndMaxSegmentsAttachments(const SegmentsHandler& segmentsHandler,
int segmentsMinAttachment[], int segmentsMaxAttachment[]) const {
    for (int i = 0; i < segmentsHandler.size(); i++) {
        int min = segmentsHandler.getSegment(i)->size();
        int max = 0;
        for (const Node* attachment : segmentsHandler.getSegment(i)->getAttachments()) {
            int index = attachment->getIndex();
            if (index < min) min = index;
            if (index > max) max = index;
        }
        segmentsMinAttachment[i] = min;
        segmentsMaxAttachment[i] = max;
    }
}

// assuming the cycle is drawn CLOCKWISE, and assuming the segments incident to the 
// attachment "cycleNode" must be drawn OUTSIDE the cycle, computes the order of
// placement of these segments such that they don't intersect
std::vector<int> Embedder::computeOrder(const Node* cycleNode, const std::vector<int>& segmentsIndexes,
int segmentsMinAttachment[], int segmentsMaxAttachment[], const SegmentsHandler& segmentsHandler, int cycleNodePosition) const {
    std::optional<int> middleSegment;
    std::vector<int> minSegments{};
    std::vector<int> maxSegments{};
    int cycleNodeIndex = cycleNode->getIndex();
    for (int i = 0; i < segmentsIndexes.size(); ++i) {
        int segIndex = segmentsIndexes[i];
        if (segmentsMinAttachment[segIndex] == cycleNodePosition) {
            minSegments.push_back(segIndex);
            continue;
        }
        if (segmentsMaxAttachment[segIndex] == cycleNodePosition) {
            maxSegments.push_back(segIndex);
            continue;
        }
        assert(!middleSegment.has_value());
        middleSegment = segIndex;
    }
    for (int i = 0; i < int(maxSegments.size())-1; ++i) {
        int min = maxSegments[i];
        for (int j = i+1; j < maxSegments.size(); ++j) {
            int candidate = maxSegments[j];
            if (segmentsMinAttachment[candidate] < segmentsMinAttachment[min])
                continue;
            if (segmentsMinAttachment[candidate] > segmentsMinAttachment[min]) {
                min = maxSegments[candidate];
                continue;
            }
            int numAttachmentsMin = segmentsHandler.getSegment(min)->getAttachments().size();
            int numAttachmentsCandidate = segmentsHandler.getSegment(candidate)->getAttachments().size();
            assert(numAttachmentsMin == 2 || numAttachmentsMin == 3);
            assert(numAttachmentsCandidate == 2 || numAttachmentsCandidate == 3);
            if (numAttachmentsMin == 2 && numAttachmentsCandidate == 2) {
                if (min > candidate)
                    continue;
                min = candidate;
                continue;
            }
            if (numAttachmentsCandidate == 3) {
                assert(numAttachmentsMin == 2);
                min = candidate;
            }
        }
        int temp = maxSegments[min];
        maxSegments[min] = maxSegments[i];
        maxSegments[i] = temp;
    }
    // ordering minSegments
    for (int i = 0; i < int(minSegments.size())-1; ++i) {
        int min = minSegments[i];
        for (int j = i+1; j < minSegments.size(); ++j) {
            int candidate = minSegments[j];
            if (segmentsMaxAttachment[candidate] < segmentsMaxAttachment[min])
                continue;
            if (segmentsMaxAttachment[candidate] > segmentsMaxAttachment[min]) {
                min = minSegments[candidate];
                continue;
            }
            int numAttachmentsMax = segmentsHandler.getSegment(min)->getAttachments().size();
            int numAttachmentsCandidate = segmentsHandler.getSegment(candidate)->getAttachments().size();
            assert(numAttachmentsMax == 2 || numAttachmentsMax == 3);
            assert(numAttachmentsCandidate == 2 || numAttachmentsCandidate == 3);
            if (numAttachmentsMax == 2 && numAttachmentsCandidate == 2) {
                if (min > candidate)
                    continue;
                min = candidate;
                continue;
            }
            if (numAttachmentsMax == 3) {
                assert(numAttachmentsCandidate == 2);
                min = candidate;
            }
        }
        int temp = minSegments[min];
        minSegments[min] = minSegments[i];
        minSegments[i] = temp;
    }
    std::vector<int> order{};
    for (int segmentIndex : maxSegments)
        order.push_back(segmentIndex);
    if (middleSegment) order.push_back(middleSegment.value());
    for (int segmentIndex : minSegments)
        order.push_back(segmentIndex);
    assert(order.size() == segmentsIndexes.size());
    return order;
}

// the embedding is "compatible" with the cycle if, drawn the cycle clockwise,
// the embedding of the segment places the segment inside the cycle
std::vector<bool> Embedder::compatibilityEmbeddingsAndCycle(const SubGraph* component, const Cycle* cycle,
const std::vector<std::unique_ptr<const Embedding>>& embeddings, const SegmentsHandler& segmentsHandler) const {
    std::vector<bool> isCompatible(segmentsHandler.size());
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const Segment* segment = segmentsHandler.getSegment(i);
        const Embedding* embedding = embeddings[i].get();
        const Node* attachment = segment->getAttachments()[0]; // any attachment is good
        const Node* componentNode = segment->getComponentNode(attachment);
        const Node* next = cycle->getNextOfNode(componentNode);
        const Node* prev = cycle->getPrevOfNode(componentNode);
        int position = -1;
        int attachmentIndex = attachment->getIndex();
        const std::vector<const Node*>& neighbors = embedding->getNode(attachmentIndex)->getNeighbors();
        assert(neighbors.size() >= 3);
        for (int j = 0; j < neighbors.size(); ++j) {
            const Node* neighbor = neighbors[j];
            const Node* componentNeighbor = segment->getComponentNode(neighbor);
            if (componentNeighbor == next) {
                position = j;
                break;
            }
        }
        assert(position != -1);
        int nextPosition = (position+1)%neighbors.size();
        isCompatible[i] = (segment->getComponentNode(neighbors[nextPosition]) != prev);
    }
    return isCompatible;
}

void Embedder::addMiddleEdges(const Segment* segment, const Embedding* embedding, int cycleNodeIndex,
const SubGraph* component, bool compatible, Embedding* output) const {
    const Cycle* cycle = segment->getOriginalCycle();
    const Node* cycleNode = cycle->getNode(cycleNodeIndex);
    const Node* prevCycleNode = cycle->getPrevOfNode(cycleNode);
    const Node* nextCycleNode = cycle->getNextOfNode(cycleNode);
    assert(cycleNode->getGraph() == component);
    assert(prevCycleNode->getGraph() == component);
    assert(nextCycleNode->getGraph() == component);
    assert(segment->isNodeAnAttachment(segment->getNode(cycleNodeIndex)));
    std::vector<int> neighborsToAdd;
    const std::vector<const Node*>& neighbors = embedding->getNode(cycleNodeIndex)->getNeighbors();
    int positionOfLastAddedNode = -1;
    for (int i = 0; i < neighbors.size(); ++i) {
        const Node* neighbor = neighbors[i];
        const Node* neighborSegment = segment->getNode(neighbor->getIndex());
        const Node* neighborComponent = segment->getComponentNode(neighborSegment);
        if (neighborComponent->getIndex() == prevCycleNode->getIndex()) {
            positionOfLastAddedNode = i;
            break;
        }
        if (neighborComponent->getIndex() == nextCycleNode->getIndex()) {
            positionOfLastAddedNode = i;
            break;
        }
    }
    assert(positionOfLastAddedNode != -1);
    for (int i = 1; i < neighbors.size(); ++i) {
        const int index = (i+positionOfLastAddedNode)%neighbors.size();
        const Node* neighbor = neighbors[index];
        const Node* neighborSegment = segment->getNode(neighbor->getIndex());
        const Node* neighborComponent = segment->getComponentNode(neighborSegment);
        assert(neighborComponent->getGraph() == component);
        assert(segment->getOriginalNode(neighborSegment) == component->getOriginalNode(neighborComponent));
        assert(segment->getOriginalNode(neighborSegment) == embedding->getOriginalNode(neighbor));
        if (nextCycleNode == neighborComponent) continue;
        if (prevCycleNode == neighborComponent) continue;
        neighborsToAdd.push_back(neighborComponent->getIndex());
    }
    if (compatible)
        for (int j = 0; j < neighborsToAdd.size(); ++j) {
            Node* from = output->getNode(cycleNode->getIndex());
            Node* to = output->getNode(neighborsToAdd[j]);
            output->addSingleEdge(from, to);
        }
            
    else
        for (int j = neighborsToAdd.size()-1; j >= 0; --j) {
            Node* from = output->getNode(cycleNode->getIndex());
            Node* to = output->getNode(neighborsToAdd[j]);
            output->addSingleEdge(from, to);
        }
}

const Embedding* Embedder::mergeSegmentsEmbeddings(const SubGraph* component, const Cycle* cycle,
const std::vector<std::unique_ptr<const Embedding>>& embeddings, const SegmentsHandler& segmentsHandler,
const std::vector<int>& bipartition) const {
    Embedding* output = new Embedding(component);
    int segmentsMinAttachment[segmentsHandler.size()];
    int segmentsMaxAttachment[segmentsHandler.size()];
    computeMinAndMaxSegmentsAttachments(segmentsHandler, segmentsMinAttachment, segmentsMaxAttachment);
    std::vector<bool> isSegmentCompatible = compatibilityEmbeddingsAndCycle(component, cycle, embeddings, segmentsHandler);
    for (int cycleNodePosition = 0; cycleNodePosition < cycle->size(); ++cycleNodePosition) {
        std::vector<int> insideSegments{};
        std::vector<int> outsideSegments{};
        for (int i = 0; i < segmentsHandler.size(); ++i) {
            const Node* node = segmentsHandler.getSegment(i)->getNode(cycleNodePosition);
            if (segmentsHandler.getSegment(i)->isNodeAnAttachment(node)) {
                if (bipartition[i] == 0) insideSegments.push_back(i);
                else outsideSegments.push_back(i);
            }
        }
        const Node* cycleNode = cycle->getNode(cycleNodePosition);
        const Node* prevCycleNode = cycle->getPrevOfNode(cycleNode);
        const Node* nextCycleNode = cycle->getNextOfNode(cycleNode);
        // order of the segments inside the cycle
        std::vector<int> insideOrder = computeOrder(cycleNode, insideSegments,
            segmentsMinAttachment, segmentsMaxAttachment, segmentsHandler, cycleNodePosition);
        reverseVector(insideOrder);
        // order of the segments outside the cycle
        std::vector<int> outsideOrder = computeOrder(cycleNode, outsideSegments,
            segmentsMinAttachment, segmentsMaxAttachment, segmentsHandler, cycleNodePosition);
        for (int index : outsideOrder)
            isSegmentCompatible[index] = !isSegmentCompatible[index];
        output->addSingleEdge(cycleNode->getIndex(), nextCycleNode->getIndex());
        for (int i = 0; i < insideOrder.size(); ++i) {
            const Segment* segment = segmentsHandler.getSegment(insideOrder[i]);
            const Embedding* embedding = embeddings[insideOrder[i]].get();
            addMiddleEdges(segment, embedding, cycleNodePosition, component, isSegmentCompatible[insideOrder[i]], output);
        }
        output->addSingleEdge(cycleNode->getIndex(), prevCycleNode->getIndex());
        for (int i = 0; i < outsideOrder.size(); ++i) {
            const Segment* segment = segmentsHandler.getSegment(outsideOrder[i]);
            const Embedding* embedding = embeddings[outsideOrder[i]].get();
            addMiddleEdges(segment, embedding, cycleNodePosition, component, isSegmentCompatible[outsideOrder[i]], output);
        }
    }
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const Segment* segment = segmentsHandler.getSegment(i);
        const Embedding* embedding = embeddings[i].get();
        for (int nodeIndex = 0; nodeIndex < segment->size(); ++nodeIndex) {
            const Node* node = segment->getNode(nodeIndex);
            const Node* componentNode = segment->getComponentNode(node);
            // int label = segment.getLabelOfNode(nodeIndex);
            if (cycle->hasNode(componentNode)) continue;
            std::vector<int> neighborsToAdd;
            const Node* embeddingNode = embedding->getNode(nodeIndex);
            for (const Node* neighbor : embeddingNode->getNeighbors()) {
                const Node* neighborSegment = segment->getNode(neighbor->getIndex());
                const Node* neighborComponent = segment->getComponentNode(neighborSegment);
                neighborsToAdd.push_back(neighborComponent->getIndex());
            }
            if (isSegmentCompatible[i])
                for (int j = 0; j < neighborsToAdd.size(); ++j)
                    output->addSingleEdge(componentNode->getIndex(), neighborsToAdd[j]);
            else
                for (int j = neighborsToAdd.size()-1; j >= 0; --j)
                    output->addSingleEdge(componentNode->getIndex(), neighborsToAdd[j]);
        } 
    }
    return output;
}

void Embedder::componentAndEmbeddingChecks(const SubGraph* component, const Embedding* embedding) const {
    assert(component->size() == embedding->size());
    for (int i = 0; i < component->size(); ++i) {
        const Node* node = component->getNode(i);
        const Node* nodeEmbedding = embedding->getNode(i);
        assert(embedding->getOriginalNode(nodeEmbedding) == component->getOriginalNode(node));
        assert(node->getNeighbors().size() == nodeEmbedding->getNeighbors().size());
    }
}

void Embedder::segmentAndEmbeddingChecks(const Segment* segment, const Embedding* embedding) const {
    const Cycle* cycle = segment->getOriginalCycle();
    for (int i = 0; i < cycle->size(); ++i)
        assert(segment->getComponentNode(segment->getNode(i)) == cycle->getNode(i));
    assert(segment->size() == embedding->size());
    const SubGraph* component = segment->getOriginalComponent();
    for (int i = 0; i < segment->size(); ++i) {
        const Node* node = segment->getNode(i);
        const Node* nodeEmbedding = embedding->getNode(i);
        assert(segment->getOriginalNode(node) == embedding->getOriginalNode(nodeEmbedding));
        const Node* nodeComponent = segment->getComponentNode(node);
        assert(component->getOriginalNode(nodeComponent) == segment->getOriginalNode(node));
        assert(component->getOriginalNode(nodeComponent) == embedding->getOriginalNode(nodeEmbedding));
    }
}

std::optional<const Embedding*> Embedder::embedComponent(const SubGraph* component, Cycle* cycle) const {
    const SegmentsHandler segmentsHandler = SegmentsHandler(component, cycle);
    if (segmentsHandler.size() == 0) // entire biconnected component is a cycle
        return baseCaseCycle(component);
    if (segmentsHandler.size() == 1) {
        const Segment* segment = segmentsHandler.getSegment(0);
        if (segment->isPath())
            return baseCaseComponent(component, cycle);
        // chosen cycle is bad
        makeCycleGood(cycle, segment);
        return embedComponent(component, cycle);
    }
    InterlacementGraph interlacementGraph(cycle, segmentsHandler);
    std::optional<std::vector<int>> bipartition = interlacementGraph.computeBipartition();
    if (!bipartition) return std::nullopt;
    std::vector<std::unique_ptr<const Embedding>> embeddings{};
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const Segment* segment = segmentsHandler.getSegment(i);
        std::optional<const Embedding*> embedding = embedComponent(segment);
        if (!embedding.has_value())
            return std::nullopt;
        segmentAndEmbeddingChecks(segment, embedding.value());
        embeddings.push_back(std::unique_ptr<const Embedding>(embedding.value()));
    }
    const Embedding* embedding = mergeSegmentsEmbeddings(component, cycle, embeddings, segmentsHandler, bipartition.value());
    return embedding;
}

std::optional<const Embedding*> Embedder::embedComponent(const SubGraph* component) const {
    Cycle cycle(component);
    std::optional<const Embedding*> embedding = embedComponent(component, &cycle);
    if (!embedding.has_value()) return std::nullopt;
    componentAndEmbeddingChecks(component, embedding.value());
    return embedding;
}

// it may happen that a cycle induces only one segment, which is not a base case
// so the cycle must be recomputed such that it ensures at least two segments
void Embedder::makeCycleGood(Cycle* cycle, const Segment* segment) const {
    const std::vector<const Node*>& attachments = segment->getAttachments();
    std::vector<int> attachmentsComponent{};
    for (const Node* attachment : attachments)
        attachmentsComponent.push_back(segment->getComponentNode(attachment)->getIndex());
    int foundAttachments = 0;
    const Node* attachmentsToUse[2];
    for (int i = 0; i < cycle->size(); ++i) {
        const Node* node = cycle->getNode(i);
        int index = findIndex(attachmentsComponent, node->getIndex());
        if (index == -1) continue;
        attachmentsToUse[foundAttachments++] = attachments[index];
        if (foundAttachments == 2) break;
    }
    std::list<const Node*> path = segment->computePathBetweenAttachments(attachmentsToUse[0], attachmentsToUse[1]);
    std::list<const Node*> pathComponent;
    for (const Node* node : path)
        pathComponent.push_back(segment->getComponentNode(node));
    cycle->changeWithPath(pathComponent);
}

// base case: graph has <4 nodes
const Embedding* Embedder::baseCaseGraph(const Graph* graph) const {
    assert(graph->size() < 4);
    Embedding* embedding = new Embedding(graph);
    for (int nodeIndex = 0; nodeIndex < graph->size(); ++nodeIndex) {
        for (const Node* neighbor : graph->getNode(nodeIndex)->getNeighbors())
            if (nodeIndex < neighbor->getIndex())
                embedding->addEdge(nodeIndex, neighbor->getIndex());
    }
    return embedding;
}

// base case: segment is a path
const Embedding* Embedder::baseCaseComponent(const SubGraph* component, const Cycle* cycle) const {
    Embedding* embedding = new Embedding(component);
    for (int nodeIndex = 0; nodeIndex < component->size(); ++nodeIndex) {
        const Node* node = component->getNode(nodeIndex);
        const std::vector<const Node*>& neighbors = node->getNeighbors();
        if (neighbors.size() == 2) { // attachment nodes will be handled later
            embedding->addSingleEdge(node->getIndex(), neighbors[0]->getIndex());
            embedding->addSingleEdge(node->getIndex(), neighbors[1]->getIndex());
            continue;
        }
        assert(neighbors.size() == 3);
        int neighborsOrder[3];
        for (int i = 0; i < 3; ++i)
            neighborsOrder[i] = -1;
        for (const Node* neighbor : neighbors) {
            if (cycle->getNextOfNode(node) == neighbor) {
                neighborsOrder[0] = neighbor->getIndex();
                continue;
            }
            if (cycle->getPrevOfNode(node) == neighbor) {
                neighborsOrder[2] = neighbor->getIndex();
                continue;
            }
            neighborsOrder[1] = neighbor->getIndex();
        }
        for (int i = 0; i < 3; ++i) {
            assert(neighborsOrder[i] != -1);
            embedding->addSingleEdge(node->getIndex(), neighborsOrder[i]);
        }
    }
    componentAndEmbeddingChecks(component, embedding);
    return embedding;
}

// base case: biconnected component is a cycle
const Embedding* Embedder::baseCaseCycle(const SubGraph* cycle) const {
    Embedding* embedding = new Embedding(cycle);
    for (int i = 0; i < cycle->size()-1; ++i)
        for (const Node* neighbor : cycle->getNode(i)->getNeighbors())
            if (i < neighbor->getIndex())
                embedding->addEdge(i, neighbor->getIndex());
    return embedding;
}

ogdf::Graph myGraphToOgdf(const Graph* myGraph) {
    ogdf::Graph graph;
    std::vector<ogdf::node> nodes(myGraph->size());
    for (int i = 0; i < myGraph->size(); ++i)
        nodes[i] = graph.newNode();
    for (int i = 0; i < myGraph->size(); ++i) {
        const Node* node = myGraph->getNode(i);
        for (const Node* neighbor : node->getNeighbors())
            if (i < neighbor->getIndex())
                graph.newEdge(nodes[i], nodes[neighbor->getIndex()]);
    }
    return graph;
}

Graph ogdfGraphToMyGraph(const ogdf::Graph& graph) {
    Graph myGraph(graph.numberOfNodes());
    for (ogdf::node n : graph.nodes) {
        const int node = n->index();
        for (ogdf::adjEntry& adj : n->adjEntries) {
            const int neighbor = adj->twinNode()->index();
            if (node > neighbor) continue;
            myGraph.addEdge(node, neighbor);
        }
    }
    return myGraph;
}

void printOgdfGraph(ogdf::Graph& graph) {
    for (ogdf::node node : graph.nodes) {
        std::cout << node->index() << ": [ ";
        for (ogdf::adjEntry& adj : node->adjEntries) {
            const int neighbor = adj->twinNode()->index();
            std::cout << neighbor << " ";
        }
        std::cout << "]\n";
    }
}

class AuslanderParterEmbedder : public ogdf::EmbedderModule {
public:
    void doCall(ogdf::Graph& graph, ogdf::adjEntry &adjExternal) {
        Graph myGraph = ogdfGraphToMyGraph(graph);
        Embedder embedder;
        std::optional<const Embedding*> embeddingOpt = embedder.embedGraph(&myGraph);
        if (!embeddingOpt) {
            std::cout << "error\n";
            exit(1);
        }
        const Embedding* embedding = embeddingOpt.value();
        std::vector<int> position(embedding->size());
        for (ogdf::node n : graph.nodes) {
            const int index = n->index();
            const Node* node = embedding->getNode(index);
            const std::vector<const Node*>& neighbors = node->getNeighbors();
            for (int i = 0; i < neighbors.size(); ++i)
                position[neighbors[i]->getIndex()] = i;
            std::vector<ogdf::adjEntry> order(neighbors.size());
            for (ogdf::adjEntry& adj : n->adjEntries) {
                const int neighbor = adj->twinNode()->index();
                order[position[neighbor]] = adj;
            }
            ogdf::List<ogdf::adjEntry> newOrder;
            for (ogdf::adjEntry& adj : order)
                newOrder.pushBack(adj);
            graph.sort(n, newOrder);
        }
        delete embedding;
    }
};

void saveStringToFile(const char* filename, std::string content) {
    std::ofstream outfile(filename);
    if (outfile.is_open()) {
        outfile << content;
        outfile.close();
        std::cout << "File saved successfully." << std::endl;
    } else {
        std::cerr << "Unable to open file" << std::endl;
    }
}

void Embedder::embedToSvg(const Graph* graph, std::string& outputPath) const {
    ogdf::Graph ogdfGraph = myGraphToOgdf(graph);
    ogdf::GraphAttributes GA(ogdfGraph, ogdf::GraphAttributes::nodeGraphics | ogdf::GraphAttributes::edgeGraphics |
                        ogdf::GraphAttributes::nodeLabel | ogdf::GraphAttributes::edgeStyle |
                        ogdf::GraphAttributes::nodeStyle | ogdf::GraphAttributes::edgeArrow);
    for (ogdf::node v : ogdfGraph.nodes) {
        GA.label(v) = std::to_string(v->index());
        GA.shape(v) = ogdf::Shape::Ellipse;
    }
    for (ogdf::edge e : ogdfGraph.edges) {
        GA.strokeWidth(e) = 1.5;
        GA.arrowType(e) = ogdf::EdgeArrow::None;
    }
    ogdf::PlanarDrawLayout layout;
    layout.setEmbedder(new AuslanderParterEmbedder);
    layout.call(GA);

    std::ostringstream svgStream;
    ogdf::GraphIO::SVGSettings svgSettings;
    if (ogdf::GraphIO::drawSVG(GA, svgStream, svgSettings)) {
        std::string svgContent = svgStream.str();
        saveStringToFile("/embedding.svg", svgContent);
    } else {
        std::cerr << "Error generating SVG content." << std::endl;
    }
}