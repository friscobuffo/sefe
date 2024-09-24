#include <iostream>
#include <optional>
#include <string>
#include <fstream>
#include <sstream>

#include "basic/graph.hpp"
#include "auslander-parter/embedder.hpp"

#include "sefe/bicoloredGraph.hpp"
#include "sefe/embedderSefe.hpp"

const Graph* loadFromfile(std::string filename) {
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
            embedder.embedToSvg(graph);
            delete embedding.value();
        }
        std::cout << "\n";
        std::cout << "all good\n";
        delete graph;
    }
}

extern "C" {
    int sefeLoadedFiles() {
        const Graph* red = loadFromfile("red.txt");
        const Graph* blue = loadFromfile("blue.txt");
        const BicoloredGraph graph(red, blue);
        std::cout << "graph:\n";
        graph.print();
        EmbedderSefe embedder{};
        std::optional<const EmbeddingSefe*> embedding = embedder.embedGraph(&graph);
        bool admitsSefe = embedder.testSefe(red, blue);
        std::cout << std::boolalpha << "graph is planar: " << admitsSefe << ".\n";
        if (embedding.has_value()) {
            std::cout << "embedding:\n";
            embedding.value()->print();
            embedder.embedToSvg(&graph);
            delete embedding.value();
        }
        std::cout << "\n";
        std::cout << "all good\n";
        delete red;
        delete blue;
        if (admitsSefe) return 1;
        return 0;
    }
}