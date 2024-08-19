#include "graphLoader.hpp"

#include <fstream>
#include <iostream>
#include <utility>
#include <string>
#include <sstream>

const Graph* GraphLoader::loadFromFile(const char* path) const {
    std::ifstream inputFile(path);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file " << path << std::endl;
        exit(1);
    }
    int nodesNumber{};
    inputFile >> nodesNumber;
    Graph* graph = new Graph(nodesNumber);
    int fromIndex, toIndex;
    std::string line;
    while (std::getline(inputFile, line)) {
        if (line.find("//") == 0)
            continue;
        std::istringstream iss(line);
        if (iss >> fromIndex >> toIndex)
            graph->addEdge(fromIndex, toIndex);
    }
    inputFile.close();
    return graph;
}