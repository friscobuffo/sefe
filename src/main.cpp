#include <iostream>
#include <optional>
#include <string>
#include <memory>

#include "basic/graph.hpp"
#include "auslander-parter/embedder.hpp"
#include "sefe/bicoloredGraph.hpp"
#include "sefe/embedderSefe.hpp"
#include "ogdf-drawers.hpp"

extern "C" {
    int embedLoadedFile() {
        const Graph* graph = Graph::loadFromFile("input.txt");
        std::unique_ptr<const Graph> graphPtr(graph);
        if (!graph->isConnected()) {
            std::cerr << "Graph is not connected." << std::endl;
            return -1;
        }
        Embedder embedder{};
        std::optional<const Embedding*> embedding = embedder.embedGraph(graph);
        if (embedding.has_value()) {
            std::unique_ptr<const Embedding> embeddingPtr(embedding.value());
            drawEmbeddingToFile(graph, embedding.value());
            return 1;
        }
        return 0;
    }
}

extern "C" {
    int sefeLoadedFiles() {
        const Graph* red = Graph::loadFromFile("red.txt");
        const Graph* blue = Graph::loadFromFile("blue.txt");
        std::unique_ptr<const Graph> redPtr(red);
        std::unique_ptr<const Graph> bluePtr(blue);
        if (red->size() != blue->size()) {
            std::cerr << "Error: graphs have different number of nodes." << std::endl;
            return -2;
        }
        const BicoloredGraph graph(red, blue);
        const Graph* intersection = graph.getIntersection();
        BiconnectedComponentsHandler biconnectedComponentsHandler{intersection};
        if (biconnectedComponentsHandler.size() > 1) {
            std::cerr << "Error: intersection graph is not biconnected." << std::endl;
            return -1;
        }
        EmbedderSefe embedder{};
        std::optional<const EmbeddingSefe*> embedding = embedder.embedGraph(&graph);
        if (!embedding.has_value())
            return 0;
        std::unique_ptr<const EmbeddingSefe> embeddingPtr(embedding.value());
        // red embedding
        const Graph* redGraph = graph.computeRed();
        std::unique_ptr<const Graph> redGraphPtr(redGraph);
        const Embedding* redEmbedding = embedding.value()->computeRedEmbedding(redGraph);
        std::unique_ptr<const Graph> redEmbeddingPtr(redEmbedding);
        drawSefeEmbeddingToFile(redEmbedding, intersection, "red", "/embedding-red.svg");

        // blue embedding
        const Graph* blueGraph = graph.computeBlue();
        std::unique_ptr<const Graph> blueGraphPtr(blueGraph);
        const Embedding* blueEmbedding = embedding.value()->computeBlueEmbedding(blueGraph);
        std::unique_ptr<const Graph> blueEmbeddingPtr(blueEmbedding);
        drawSefeEmbeddingToFile(blueEmbedding, intersection, "blue", "/embedding-blue.svg");

        std::string embeddingString = embedding.value()->toString();
        saveStringToFile("/embedding-sefe.txt", embeddingString);
        return 1;
    }
}