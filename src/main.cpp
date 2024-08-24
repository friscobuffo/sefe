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
            std::string path = "embedding.svg";
            embedder.embedToSvg(graph, path);
            delete embedding.value();
        }
        std::cout << "\n";
        std::cout << "all good\n";
        delete graph;
    }
}

void testGraph(std::string path) {
    EmbedderSefe embedderSefe{};
    Embedder embedder{};
    const Graph* g1 = loadFromfile(path);
    const Graph* g2 = loadFromfile(path);
    std::optional<const Embedding*> embedding = embedder.embedGraph(g1);
    std::cout << embedding.has_value() << " - " << embedderSefe.testSefe(g1, g2) << "\n";
    delete g1;
    delete g2;
    if (embedding.has_value()) delete embedding.value();
}

extern "C" {
    void sefeMainTest() {
        std::cout << std::boolalpha;
        const Graph* graph1 = loadFromfile("/example-graphs/graphs-sefe/a0.txt");
        const Graph* graph2 = loadFromfile("/example-graphs/graphs-sefe/a1.txt");
        const BicoloredGraph bicoloredGraph(graph1, graph2);
        bicoloredGraph.print();
        EmbedderSefe embedderSefe{};
        std::cout << embedderSefe.testSefe(graph1, graph2) << "\n";
        std::optional<const EmbeddingSefe*> embeddingSefe = embedderSefe.embedGraph(&bicoloredGraph);
        std::cout << embeddingSefe.has_value() << "\n";
        if (embeddingSefe.has_value()) {
            embeddingSefe.value()->print();
            delete embeddingSefe.value();
        }
        delete graph1;
        delete graph2;

        std::cout << "all graphs tests\n";
        std::cout << "(boolean values on same row must be the same)\n";
        testGraph("/example-graphs/graphs/g1.txt");
        testGraph("/example-graphs/graphs/g2.txt");
        // testGraph("/example-graphs/graphs/g3.txt");
        testGraph("/example-graphs/graphs/g4.txt");
        testGraph("/example-graphs/graphs/g5.txt");
        testGraph("/example-graphs/graphs/g6.txt");
        testGraph("/example-graphs/graphs/k5.txt");
        testGraph("/example-graphs/graphs/k33.txt");
    }
}
