#include "embedderSefe.hpp"

#include <iostream>

#include "biconnectedComponent.hpp"

bool EmbedderSefe::testSefe(const Graph* graph1, const Graph* graph2) const {
    BicoloredGraph* bicoloredGraph = new BicoloredGraph(graph1, graph2);
    const Graph* intersection = bicoloredGraph->getIntersection();
    BiconnectedComponentsHandler bicCompHandler(intersection);
    if (bicCompHandler.size() > 1) {
        std::cout << "intersection must be biconnected\n";
        return false;
    }
    return testSefe(bicoloredGraph);
}

// assumes intersection is biconnected
bool EmbedderSefe::testSefe(const BicoloredGraph* bicoloredGraph) const {
    
}