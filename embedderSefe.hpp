#ifndef MY_EMBEDDER_SEFE_H
#define MY_EMBEDDER_SEFE_H

#include <optional>
#include <vector>
#include <string>

#include "graph.hpp"
#include "biconnectedComponent.hpp"
#include "cycle.hpp"
#include "segment.hpp"

class EmbedderSefe {
private:    
public:
    bool testSefe(const Graph* graph1, const Graph* graph2) const;
};

#endif