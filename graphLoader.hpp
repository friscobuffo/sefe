#ifndef MY_GRAPH_LOADER_H
#define MY_GRAPH_LOADER_H

#include "graph.hpp"

class GraphLoader {
public:
    const Graph* loadFromFile(char* path);
};

#endif