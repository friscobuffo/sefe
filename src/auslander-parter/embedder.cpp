#include "embedder.hpp"
#include <cassert>

#include <iostream>

#include "interlacement.hpp"
#include "../basic/utils.hpp"

std::unique_ptr<const SubGraph> Embedder::mergeBiconnectedComponents(const Graph& graph, const BiconnectedComponentsHandler& biconnectedComponents,
const std::vector<std::unique_ptr<const SubGraph>>& embeddings) const {
    std::unique_ptr<SubGraph> output = std::make_unique<SubGraph>(&graph);
    for (int i = 0; i < graph.size(); ++i) {
        output->addNode(graph.getNode(0).getColor());
        output->setOriginalNode(output->getNode(i), graph.getNode(i));
    }
    assert(biconnectedComponents.size() == embeddings.size());
    for (int i = 0; i < biconnectedComponents.size(); ++i) {
        const SubGraph& component = biconnectedComponents.getComponent(i);
        const SubGraph& embedding = *embeddings[i];
        for (int i = 0; i < embedding.size(); ++i) {
            const Node& node = embedding.getNode(i);
            const Node& nodeOriginal = embedding.getOriginalNode(node);
            for (const auto& edge : node.getEdges()) {
                const Node& neighborOriginal = embedding.getOriginalNode(edge.to);
                output->addSingleEdge(nodeOriginal.getIndex(), neighborOriginal.getIndex(), edge.weight, edge.color);
            }
        }
    }
    return output;
}

std::optional<std::unique_ptr<const SubGraph>> Embedder::embedGraph(const Graph& graph) const {
    if (graph.size() < 4) return baseCaseGraph(graph);
    if (graph.totalNumberOfEdges() > (3*graph.size()-6))
        return std::nullopt;
    const BiconnectedComponentsHandler bicComps(graph);
    std::vector<std::unique_ptr<const SubGraph>> embeddings{};
    for (int i = 0; i < bicComps.size(); ++i) {
        const SubGraph& component = bicComps.getComponent(i);
        std::optional<std::unique_ptr<const SubGraph>> embedding = embedComponent(component);
        if (!embedding.has_value())
            return std::nullopt;
        embeddings.push_back(std::move(embedding.value()));
    }
    std::unique_ptr<const SubGraph> embedding = mergeBiconnectedComponents(graph, bicComps, embeddings);
    return embedding;
}

void Embedder::computeMinAndMaxSegmentsAttachments(const SegmentsHandler& segmentsHandler,
int segmentsMinAttachment[], int segmentsMaxAttachment[]) const {
    for (int i = 0; i < segmentsHandler.size(); i++) {
        int min = segmentsHandler.getSegment(i).size();
        int max = 0;
        for (const Node* attachment : segmentsHandler.getSegment(i).getAttachments()) {
            int index = attachment->getIndex();
            if (index < min) min = index;
            if (index > max) max = index;
        }
        segmentsMinAttachment[i] = min;
        segmentsMaxAttachment[i] = max;
    }
}

std::vector<int> Embedder::computeOrder(const Node& cycleNode, const std::vector<int>& segmentsIndexes,
int segmentsMinAttachment[], int segmentsMaxAttachment[], const SegmentsHandler& segmentsHandler, int cycleNodePosition) const {
    std::optional<int> middleSegment;
    std::vector<int> minSegments{};
    std::vector<int> maxSegments{};
    int cycleNodeIndex = cycleNode.getIndex();
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
        int min = i;
        int minSeg = maxSegments[i];
        for (int j = i+1; j < maxSegments.size(); ++j) {
            int candidate = maxSegments[j];
            if (segmentsMinAttachment[candidate] < segmentsMinAttachment[minSeg])
                continue;
            if (segmentsMinAttachment[candidate] > segmentsMinAttachment[minSeg]) {
                minSeg = maxSegments[candidate];
                min = j;
                continue;
            }
            int numAttachmentsMin = segmentsHandler.getSegment(minSeg).getAttachments().size();
            int numAttachmentsCandidate = segmentsHandler.getSegment(candidate).getAttachments().size();
            assert(numAttachmentsMin == 2 || numAttachmentsMin == 3);
            assert(numAttachmentsCandidate == 2 || numAttachmentsCandidate == 3);
            if (numAttachmentsMin == 2 && numAttachmentsCandidate == 2) {
                if (minSeg > candidate)
                    continue;
                minSeg = candidate;
                min = j;
                continue;
            }
            if (numAttachmentsCandidate == 3) {
                assert(numAttachmentsMin == 2);
                minSeg = candidate;
                min = j;
            }
        }
        int temp = maxSegments[min];
        maxSegments[min] = maxSegments[i];
        maxSegments[i] = temp;
    }
    // ordering minSegments
    for (int i = 0; i < int(minSegments.size())-1; ++i) {
        int min = i;
        int minSeg = minSegments[i];
        for (int j = i+1; j < minSegments.size(); ++j) {
            int candidate = minSegments[j];
            if (segmentsMaxAttachment[candidate] < segmentsMaxAttachment[minSeg])
                continue;
            if (segmentsMaxAttachment[candidate] > segmentsMaxAttachment[minSeg]) {
                min = j;
                minSeg = minSegments[j];
                continue;
            }
            int numAttachmentsMax = segmentsHandler.getSegment(minSeg).getAttachments().size();
            int numAttachmentsCandidate = segmentsHandler.getSegment(candidate).getAttachments().size();
            assert(numAttachmentsMax == 2 || numAttachmentsMax == 3);
            assert(numAttachmentsCandidate == 2 || numAttachmentsCandidate == 3);
            if (numAttachmentsMax == 2 && numAttachmentsCandidate == 2) {
                if (minSeg < candidate)
                    continue;
                minSeg = candidate;
                min = j;
                continue;
            }
            if (numAttachmentsMax == 3) {
                assert(numAttachmentsCandidate == 2);
                minSeg = candidate;
                min = j;
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

std::vector<bool> Embedder::compatibilityEmbeddingsAndCycle(const SubGraph& component, const BlackCycle& cycle,
const std::vector<std::unique_ptr<const SubGraph>>& embeddings, const SegmentsHandler& segmentsHandler) const {
    std::vector<bool> isCompatible(segmentsHandler.size());
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const Segment& segment = segmentsHandler.getSegment(i);
        const SubGraph& embedding = *embeddings[i];
        const Node& attachment = *segment.getAttachments()[0]; // any attachment is good
        const Node& componentNode = segment.getHigherLevelNode(attachment);
        const Node& next = cycle.getNextOfNode(componentNode);
        const Node& prev = cycle.getPrevOfNode(componentNode);
        int position = -1;
        int attachmentIndex = attachment.getIndex();
        const auto& edges = embedding.getNode(attachmentIndex).getEdges();
        assert(edges.size() >= 3);
        for (int j = 0; j < edges.size(); ++j) {
            const Node& neighbor = edges[j].to;
            const Node& componentNeighbor = segment.getHigherLevelNode(neighbor);
            if (&componentNeighbor == &next) {
                position = j;
                break;
            }
        }
        assert(position != -1);
        int nextPosition = (position+1)%edges.size();
        isCompatible[i] = (&segment.getHigherLevelNode(edges[nextPosition].to) != &prev);
    }
    return isCompatible;
}

void Embedder::addMiddleEdges(const Segment& segment, const SubGraph& embedding, int cycleNodeIndex,
const SubGraph& component, bool compatible, SubGraph& output) const {
    const BlackCycle& cycle = segment.getOriginalCycle();
    const Node& cycleNode = cycle.getNode(cycleNodeIndex);
    const Node& prevCycleNode = cycle.getPrevOfNode(cycleNode);
    const Node& nextCycleNode = cycle.getNextOfNode(cycleNode);
    assert(&cycleNode.getOwner() == &component);
    assert(&prevCycleNode.getOwner() == &component);
    assert(&nextCycleNode.getOwner() == &component);
    assert(segment.isNodeAnAttachment(segment.getNode(cycleNodeIndex)));
    std::vector<int> neighborsToAdd;
    std::vector<Color> neighborsToAddColor;
    std::vector<double> neighborsToAddWeight;
    const auto& edges = embedding.getNode(cycleNodeIndex).getEdges();
    int positionOfLastAddedNode = -1;
    for (int i = 0; i < edges.size(); ++i) {
        const Node& neighbor = edges[i].to;
        const Node& neighborSegment = segment.getNode(neighbor.getIndex());
        const Node& neighborComponent = segment.getHigherLevelNode(neighborSegment);
        if (neighborComponent.getIndex() == prevCycleNode.getIndex()) {
            positionOfLastAddedNode = i;
            break;
        }
        if (neighborComponent.getIndex() == nextCycleNode.getIndex()) {
            positionOfLastAddedNode = i;
            break;
        }
    }
    assert(positionOfLastAddedNode != -1);
    for (int i = 1; i < edges.size(); ++i) {
        const int index = (i+positionOfLastAddedNode)%edges.size();
        const Node& neighbor = edges[index].to;
        const Node& neighborSegment = segment.getNode(neighbor.getIndex());
        const Node& neighborComponent = segment.getHigherLevelNode(neighborSegment);
        assert(&neighborComponent.getOwner() == &component);
        assert(&segment.getOriginalNode(neighborSegment) == &component.getOriginalNode(neighborComponent));
        assert(&segment.getOriginalNode(neighborSegment) == &embedding.getOriginalNode(neighbor));
        if (&nextCycleNode == &neighborComponent) continue;
        if (&prevCycleNode == &neighborComponent) continue;
        neighborsToAdd.push_back(neighborComponent.getIndex());
        neighborsToAddColor.push_back(edges[index].color);
        neighborsToAddWeight.push_back(edges[index].weight);
    }
    if (compatible)
        for (int j = 0; j < neighborsToAdd.size(); ++j) {
            Node& from = output.getNode(cycleNode.getIndex());
            Node& to = output.getNode(neighborsToAdd[j]);
            output.addSingleEdge(from, to, neighborsToAddWeight[j], neighborsToAddColor[j]);
        }
    else
        for (int j = neighborsToAdd.size()-1; j >= 0; --j) {
            Node& from = output.getNode(cycleNode.getIndex());
            Node& to = output.getNode(neighborsToAdd[j]);
            output.addSingleEdge(from, to, neighborsToAddWeight[j], neighborsToAddColor[j]);
        }
}

void Embedder::addEdgesNotIncidentToCycle(const SegmentsHandler& segmentsHandler, SubGraph& output, const BlackCycle& cycle,
const std::vector<std::unique_ptr<const SubGraph>>& embeddings, std::vector<bool> isSegmentCompatible) const {
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const Segment& segment = segmentsHandler.getSegment(i);
        const SubGraph& embedding = *embeddings[i];
        for (int nodeIndex = 0; nodeIndex < segment.size(); ++nodeIndex) {
            const Node& node = segment.getNode(nodeIndex);
            const Node& componentNode = segment.getHigherLevelNode(node);
            if (cycle.hasNode(componentNode)) continue;
            std::vector<int> neighborsToAdd;
            std::vector<Color> neighborsToAddColor;
            const Node& embeddingNode = embedding.getNode(nodeIndex);
            for (const auto& edge : embeddingNode.getEdges()) {
                const Node& neighborSegment = segment.getNode(edge.to.getIndex());
                const Node& neighborComponent = segment.getHigherLevelNode(neighborSegment);
                neighborsToAdd.push_back(neighborComponent.getIndex());
                neighborsToAddColor.push_back(edge.color);
            }
            if (isSegmentCompatible[i])
                for (int j = 0; j < neighborsToAdd.size(); ++j)
                    output.addSingleEdge(componentNode.getIndex(), neighborsToAdd[j], 1.0, neighborsToAddColor[j]);
            else
                for (int j = neighborsToAdd.size()-1; j >= 0; --j)
                    output.addSingleEdge(componentNode.getIndex(), neighborsToAdd[j], 1.0, neighborsToAddColor[j]);
        } 
    }
}

void Embedder::addEdgesIncidentToCycle(const SegmentsHandler& segmentsHandler, const BlackCycle& cycle, const SubGraph& component,
const std::vector<std::unique_ptr<const SubGraph>>& embeddings, const std::vector<int>& bipartition, SubGraph& output,
int segmentsMinAttachment[], int segmentsMaxAttachment[], std::vector<bool> isSegmentCompatible) const {
    for (int cycleNodePosition = 0; cycleNodePosition < cycle.size(); ++cycleNodePosition) {
        std::vector<int> insideSegments{};
        std::vector<int> outsideSegments{};
        for (int i = 0; i < segmentsHandler.size(); ++i) {
            const Node& node = segmentsHandler.getSegment(i).getNode(cycleNodePosition);
            if (segmentsHandler.getSegment(i).isNodeAnAttachment(node)) {
                if (bipartition[i] == 0) insideSegments.push_back(i);
                else outsideSegments.push_back(i);
            }
        }
        const Node& cycleNode = cycle.getNode(cycleNodePosition);
        const Node& prevCycleNode = cycle.getPrevOfNode(cycleNode);
        const Node& nextCycleNode = cycle.getNextOfNode(cycleNode);
        // order of the segments inside the cycle
        std::vector<int> insideOrder = computeOrder(cycleNode, insideSegments,
            segmentsMinAttachment, segmentsMaxAttachment, segmentsHandler, cycleNodePosition);
        reverseVector(insideOrder);
        // order of the segments outside the cycle
        std::vector<int> outsideOrder = computeOrder(cycleNode, outsideSegments,
            segmentsMinAttachment, segmentsMaxAttachment, segmentsHandler, cycleNodePosition);
        output.addSingleEdge(cycleNode.getIndex(), nextCycleNode.getIndex(), 1.0, Color::BLACK);
        for (int i = 0; i < insideOrder.size(); ++i) {
            const Segment& segment = segmentsHandler.getSegment(insideOrder[i]);
            const SubGraph& embedding = *embeddings[insideOrder[i]];
            addMiddleEdges(segment, embedding, cycleNodePosition, component, isSegmentCompatible[insideOrder[i]], output);
        }
        output.addSingleEdge(cycleNode.getIndex(), prevCycleNode.getIndex(), 1.0, Color::BLACK);
        for (int i = 0; i < outsideOrder.size(); ++i) {
            const Segment& segment = segmentsHandler.getSegment(outsideOrder[i]);
            const SubGraph& embedding = *embeddings[outsideOrder[i]];
            addMiddleEdges(segment, embedding, cycleNodePosition, component, isSegmentCompatible[outsideOrder[i]], output);
        }
    }
}

std::unique_ptr<const SubGraph> Embedder::mergeSegmentsEmbeddings(const SubGraph& component, const BlackCycle& cycle,
const std::vector<std::unique_ptr<const SubGraph>>& embeddings, const SegmentsHandler& segmentsHandler,
const std::vector<int>& bipartition) const {
    std::unique_ptr<SubGraph> output = std::make_unique<SubGraph>(&component);
    for (int i = 0; i < component.size(); ++i) {
        output->addNode(component.getNode(i).getColor());
        output->setOriginalNode(output->getNode(i), component.getOriginalNode(component.getNode(i)));
    }
    int segmentsMinAttachment[segmentsHandler.size()];
    int segmentsMaxAttachment[segmentsHandler.size()];
    computeMinAndMaxSegmentsAttachments(segmentsHandler, segmentsMinAttachment, segmentsMaxAttachment);
    std::vector<bool> isSegmentCompatible = compatibilityEmbeddingsAndCycle(component, cycle, embeddings, segmentsHandler);
    for (int i = 0; i < segmentsHandler.size(); ++i)
        if (bipartition[i] == 1) isSegmentCompatible[i] = !isSegmentCompatible[i];
    addEdgesIncidentToCycle(segmentsHandler, cycle, component, embeddings, bipartition, *output, segmentsMinAttachment,
        segmentsMaxAttachment, isSegmentCompatible);
    addEdgesNotIncidentToCycle(segmentsHandler, *output, cycle, embeddings, isSegmentCompatible);
    return output;
}

std::optional<std::unique_ptr<const SubGraph>> Embedder::embedComponent(const SubGraph& component, BlackCycle& cycle) const {
    const SegmentsHandler segmentsHandler(&component, &cycle);
    if (segmentsHandler.size() == 0) // entire biconnected component is a cycle
        return baseCaseCycle(component);
    if (segmentsHandler.size() == 1) {
        const Segment& segment = segmentsHandler.getSegment(0);
        if (segment.isPath())
            return baseCaseComponent(component, cycle);
        // chosen cycle is bad
        makeCycleGood(cycle, segment);
        return embedComponent(component, cycle);
    }
    InterlacementGraph interlacementGraph(cycle, segmentsHandler);
    std::optional<std::vector<int>> bipartition = interlacementGraph.computeBipartition();
    if (!bipartition) return std::nullopt;
    std::vector<std::unique_ptr<const SubGraph>> embeddings{};
    for (int i = 0; i < segmentsHandler.size(); ++i) {
        const Segment& segment = segmentsHandler.getSegment(i);
        auto embedding = embedComponent(segment);
        if (!embedding.has_value())
            return std::nullopt;
        embeddings.push_back(std::move(embedding.value()));
    }
    std::unique_ptr<const SubGraph> embedding = mergeSegmentsEmbeddings(component, cycle, embeddings, segmentsHandler, bipartition.value());
    return embedding;
}

std::optional<std::unique_ptr<const SubGraph>> Embedder::embedComponent(const SubGraph& component) const {
    BlackCycle cycle(component);
    return embedComponent(component, cycle);
}

void Embedder::makeCycleGood(BlackCycle& cycle, const Segment& segment) const {
    assert(!segment.isPath());
    bool isCycleNodeAttachment[cycle.size()];
    for (int i = 0; i < cycle.size(); ++i)
        isCycleNodeAttachment[i] = false;
    for (const Node* attachment : segment.getAttachments()) {
        const Node& attachmentComponent = segment.getHigherLevelNode(*attachment);
        isCycleNodeAttachment[cycle.getPositionOfNode(attachmentComponent).value()] = true;
    }
    int foundAttachments = 0;
    const Node* attachmentsToUse[2];
    const Node* attachmentToInclude = nullptr;
    for (int i = 0; i < cycle.size(); ++i) {
        if (!isCycleNodeAttachment[i]) continue;
        const Node& node = cycle.getNode(i);
        const Node& nodeSegment = segment.getNode(i);
        assert(&segment.getHigherLevelNode(nodeSegment) == &node);
        if (foundAttachments < 2)
            attachmentsToUse[foundAttachments++] = &nodeSegment;
        else
            attachmentToInclude = &nodeSegment;
        if (foundAttachments == 2 && attachmentToInclude != nullptr) break;
    }
    std::list<const Node*> path = segment.computePathBetweenAttachments(*attachmentsToUse[0], *attachmentsToUse[1]);
    std::list<const Node*> pathComponent;
    for (const Node* node : path)
        pathComponent.push_back(&segment.getHigherLevelNode(*node));
    if (attachmentToInclude != nullptr)
        attachmentToInclude = &segment.getHigherLevelNode(*attachmentToInclude);
    cycle.changeWithPath(pathComponent, attachmentToInclude);
}

std::unique_ptr<const SubGraph> Embedder::baseCaseGraph(const Graph& graph) const {
    assert(graph.size() < 4);
    std::unique_ptr<SubGraph> embedding = std::make_unique<SubGraph>(&graph);
    for (int i = 0; i < graph.size(); ++i) {
        embedding->addNode(Color::BLACK);
        embedding->setOriginalNode(embedding->getNode(i), graph.getNode(i));
    }
    for (int nodeIndex = 0; nodeIndex < graph.size(); ++nodeIndex) {
        for (const auto& edge : graph.getNode(nodeIndex).getEdges())
            if (nodeIndex < edge.to.getIndex())
                embedding->addEdge(nodeIndex, edge.to.getIndex(), edge.weight, edge.color);
    }
    return embedding;
}

std::unique_ptr<const SubGraph> Embedder::baseCaseComponent(const SubGraph& component, const BlackCycle& cycle) const {
    std::unique_ptr<SubGraph> embedding = std::make_unique<SubGraph>(&component);
    for (int i = 0; i < component.size(); ++i) {
        embedding->addNode(Color::BLACK);
        embedding->setOriginalNode(embedding->getNode(i), component.getOriginalNode(component.getNode(i)));
    }
    for (int nodeIndex = 0; nodeIndex < component.size(); ++nodeIndex) {
        const Node& node = component.getNode(nodeIndex);
        const auto& edges = node.getEdges();
        if (edges.size() == 2) {
            embedding->addSingleEdge(nodeIndex, edges[0].to.getIndex(), 1.0, edges[0].color);
            embedding->addSingleEdge(nodeIndex, edges[1].to.getIndex(), 1.0, edges[1].color);
            continue;
        }
        assert(edges.size() == 3);
        int neighborsOrder[3] = {-1, -1, -1};
        Color neighborsOrderColor[3];
        for (const auto& edge : edges) {
            const Node& neighbor = edge.to;
            if (&cycle.getNextOfNode(node) == &neighbor) {
                neighborsOrder[0] = neighbor.getIndex();
                neighborsOrderColor[0] = edge.color;
                continue;
            }
            if (&cycle.getPrevOfNode(node) == &neighbor) {
                neighborsOrder[2] = neighbor.getIndex();
                neighborsOrderColor[2] = edge.color;
                continue;
            }
            neighborsOrder[1] = neighbor.getIndex();
            neighborsOrderColor[1] = edge.color;
        }
        for (int i = 0; i < 3; ++i) {
            assert(neighborsOrder[i] != -1);
            embedding->addSingleEdge(nodeIndex, neighborsOrder[i], 1.0, neighborsOrderColor[i]);
        }
    }
    return embedding;
}

std::unique_ptr<const SubGraph> Embedder::baseCaseCycle(const SubGraph& cycle) const {
    std::unique_ptr<SubGraph> embedding = std::make_unique<SubGraph>(&cycle);
    for (int i = 0; i < cycle.size(); ++i) {
        embedding->addNode(Color::BLACK);
        embedding->setOriginalNode(embedding->getNode(i), cycle.getOriginalNode(cycle.getNode(i)));
    }
    for (int i = 0; i < cycle.size()-1; ++i)
        for (const auto& edge : cycle.getNode(i).getEdges())
            if (i < edge.to.getIndex())
                embedding->addEdge(i, edge.to.getIndex(), edge.weight, edge.color);
    return embedding;
}


int Embedder::computeNumberOfFacesInEmbedding(const SubGraph& embedding) const {
    int numberOfFaces = 0;
    bool isEdgeVisited[embedding.size()][embedding.size()];
    for (int i = 0; i < embedding.size(); ++i)
        for (int j = 0; j < embedding.size(); ++j)
            isEdgeVisited[i][j] = false;
    for (int nodeIndex = 0; nodeIndex < embedding.size(); ++nodeIndex) {
        const Node& node = embedding.getNode(nodeIndex);
        for (const auto& edge : node.getEdges()) {
            const Node& neighbor = edge.to;
            if (isEdgeVisited[nodeIndex][neighbor.getIndex()]) continue;
            ++numberOfFaces;
            isEdgeVisited[nodeIndex][neighbor.getIndex()] = true;
            const Node* prev = &node;
            const Node* current = &neighbor;
            while (true) {
                for (int i = 0; i < current->getEdges().size(); ++i) {
                    const auto& candidateEdge = current->getEdges()[i];
                    const auto& candidateEdgeNext = current->getEdges()[(i+1)%current->getEdges().size()];
                    if (&candidateEdgeNext.to == prev) {
                        prev = current;
                        current = &candidateEdge.to;
                        assert(!isEdgeVisited[prev->getIndex()][current->getIndex()]);
                        isEdgeVisited[prev->getIndex()][current->getIndex()] = true;
                        break;
                    }
                }
                if (current == &node) break;
            }
        }
    }
    return numberOfFaces;
}

bool Embedder::isEmbeddingGood(const SubGraph& embedding) const {
    int numberOfFaces = computeNumberOfFacesInEmbedding(embedding);
    // std::cout << "Number of faces: " << numberOfFaces << "\n";
    return (embedding.size() - embedding.totalNumberOfEdges() + numberOfFaces) == 2;
}