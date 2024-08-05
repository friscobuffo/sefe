#include "bicoloredSegment.hpp"

#include <iostream>
#include <cassert>

const BicoloredSegment* BicoloredSegmentsHandler::buildChord(const NodeWithColors* attachment1, const NodeWithColors* attachment2) {
    BicoloredSegment* chord = new BicoloredSegment(originalCycle_m->size(), higherLevel_m, originalCycle_m);
    // assigning labels
    // first nodes MUST be the same of the cycle in the SAME ORDER
    for (int i = 0; i < originalCycle_m->size(); ++i) {
        const Node* oldNode = originalCycle_m->getNode(i);
        const Node* newNode = chord->getNode(i);
        chord->setComponentNode(newNode, oldNode);
        const Node* originalNode = originalComponent_m->getOriginalNode(oldNode);
        chord->setOriginalNode(newNode, originalNode);
    }
    // adding cycle edges
    for (int i = 0; i < originalCycle_m->size()-1; ++i)
        chord->addEdge(i, i+1);
    chord->addEdge(0, originalCycle_m->size()-1);
    // adding chord edge
    std::optional<int> fromIndex = originalCycle_m->getPositionOfNode(attachment1);
    std::optional<int> toIndex = originalCycle_m->getPositionOfNode(attachment2);
    assert(fromIndex);
    assert(toIndex);
    chord->addEdge(fromIndex.value(), toIndex.value());
    chord->addAttachment(chord->getNode(fromIndex.value()));
    chord->addAttachment(chord->getNode(toIndex.value()));
    return chord;
}

const BicoloredSegment* BicoloredSegmentsHandler::getSegment(const int index) const {
    return segments_m[index].get();
}

void BicoloredSegmentsHandler::print() const {
    for (int i = 0; i < size(); ++i) {
        std::cout << "segment [" << i << "]\n";
        const BicoloredSegment* segment = getSegment(i);
        segment->print();
    }
}

int BicoloredSegmentsHandler::size() const {
    return segments_m.size();
}