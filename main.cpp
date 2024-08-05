#include <iostream>
#include <optional>
#include <string>
#include <fstream>
#include <sstream>

#include "graph.hpp"
#include "graphLoader.hpp"
#include "embedder.hpp"

const Graph* loadFromfile(const char* filename) {
    int nodesNumber{};
    std::ifstream infile(filename);
    if (infile.is_open()) {
        std::string line;
        std::getline(infile, line);
        nodesNumber = stoi(line);
        Graph* graph = new Graph(nodesNumber);
        int fromIndex, toIndex;
        while (std::getline(infile, line)) {
            if (line.find("//") == 0)
                continue;
            std::istringstream iss(line);
            if (iss >> fromIndex >> toIndex)
                graph->addEdge(fromIndex, toIndex);
        }
        infile.close();
        return graph;
    } else {
        std::cerr << "Unable to open file" << std::endl;
        return nullptr;
    }
}

extern "C" {
    void embedLoadedFile() {
        const Graph* graph = loadFromfile("input.txt");
        std::cout << "graph:\n";
        graph->print();
        Embedder embedder{};
        std::optional<const Embedding*> embedding = embedder.embedGraph(graph);
        std::cout << std::boolalpha << "graph is planar: " << embedding.has_value() << ".\n";
        if (embedding.has_value()) {
            std::cout << "embedding:\n";
            embedding.value()->print();
            std::string path = "embedding.svg";
            embedder.embedToSvg(graph, path);
            delete embedding.value();
        }
        std::cout << "\n";
        std::cout << "all good\n";
        delete graph;
    }
}