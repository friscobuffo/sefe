#include <iostream>
#include <optional>
#include <string>
#include <memory>

#include "basic/graph.hpp"
#include "auslander-parter/embedder.hpp"
#include "sefe/embedderSefe.hpp"
#include "ogdf-drawers.hpp"

extern "C" {
    int embedLoadedFile() {
        std::unique_ptr<const Graph> graph = Graph::loadFromFile("input.txt");
        if (!graph->isConnected()) {
            std::cerr << "Graph is not connected." << std::endl;
            return -1;
        }
        Embedder embedder{};
        std::optional<std::unique_ptr<const SubGraph>> embedding = embedder.embedGraph(*graph);
        if (embedding.has_value()) {
            drawEmbeddingToFile(*graph, *embedding.value());
            return 1;
        }
        return 0;
    }
}

extern "C" {
    int sefeLoadedFiles() {
        std::unique_ptr<const Graph> red = Graph::loadFromFile("red.txt");
        std::unique_ptr<const Graph> blue = Graph::loadFromFile("blue.txt");
        if (red->size() != blue->size()) {
            std::cerr << "Error: graphs have different number of nodes." << std::endl;
            return -2;
        }
        auto unionGraph = Graph::makeUnionGraph(*red, *blue);
        auto intersection = unionGraph->computeBlackProjection();
        BiconnectedComponentsHandler biconnectedComponentsHandler{*intersection};
        if (biconnectedComponentsHandler.size() > 1) {
            std::cerr << "Error: intersection graph is not biconnected." << std::endl;
            return -1;
        }
        EmbedderSefe embedder{};
        auto embedding = embedder.embedGraph(*unionGraph);
        if (!embedding.has_value())
            return 0;

        // red embedding
        auto redEmbedding = embedding.value()->computeRedProjection();
        drawSefeProjectionEmbeddingToFile(*redEmbedding, *intersection, "red", "/embedding-red.svg");

        // blue embedding
        auto blueEmbedding = embedding.value()->computeBlueProjection();
        drawSefeProjectionEmbeddingToFile(*blueEmbedding, *intersection, "blue", "/embedding-blue.svg");

        std::string embeddingString = embedding.value()->toString();
        saveStringToFile("/embedding-sefe.txt", embeddingString);

        drawSefeEmbeddingToFile(*embedding.value(), *intersection, "/embedding-red.svg");
        
        return 1;
    }
}