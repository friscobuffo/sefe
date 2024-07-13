#include <iostream>
#include <optional>
#include <string>

#include "graph.hpp"
#include "graphLoader.hpp"
#include "embedder.hpp"

int main(int argc, char* argv[]) {
    GraphLoader loader{};
    Embedder embedder{};
    int index = 0;
    for (int i = 1; i < argc; ++i) {
        const Graph* graph = loader.loadFromFile(argv[i]);
        std::cout << "graph:\n";
        graph->print();
        std::optional<const Embedding*> embedding = embedder.embedGraph(graph);
        std::cout << std::boolalpha << "graph is planar: " << embedding.has_value() << ".\n";
        if (embedding.has_value()) {
            std::cout << "embedding:\n";
            embedding.value()->print();
            std::string path = "embeddings/embedding" + std::to_string(++index) + ".svg";
            embedder.embedToSvg(graph, path);
            delete embedding.value();
        }
        std::cout << "\n";
        delete graph;
    }
    return 0;
}