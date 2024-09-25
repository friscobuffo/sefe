#ifndef MY_OGDF_DRAWERS_H
#define MY_OGDF_DRAWERS_H

#include <string>

#include "basic/graph.hpp"
#include "auslander-parter/embedder.hpp"

void drawEmbeddingToFile(const Graph* graph, const Embedding* embedding);

void drawSefeEmbeddingToFile(const Embedding* embedding, const Graph* intersection,
    const std::string& color, const std::string& outputFilename);

#endif