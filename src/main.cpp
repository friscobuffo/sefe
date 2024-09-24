#include <iostream>
#include <optional>
#include <string>
#include <fstream>
#include <sstream>

#include "basic/graph.hpp"
#include "auslander-parter/embedder.hpp"

#include "sefe/bicoloredGraph.hpp"
#include "sefe/embedderSefe.hpp"

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/planarlayout/PlanarDrawLayout.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/planarity/EmbedderModule.h>

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

class AuslanderParterEmbedder : public ogdf::EmbedderModule {
public:
    void doCall(ogdf::Graph& graph, ogdf::adjEntry &adjExternal) {
        const Graph* myGraph = OgdfUtils::ogdfGraphToMyGraph(&graph);
        Embedder embedder;
        std::optional<const Embedding*> embeddingOpt = embedder.embedGraph(myGraph);
        if (!embeddingOpt) {
            std::cout << "error\n";
            exit(1);
        }
        const Embedding* embedding = embeddingOpt.value();
        std::vector<int> position(embedding->size());
        for (ogdf::node n : graph.nodes) {
            const int index = n->index();
            const Node* node = embedding->getNode(index);
            const std::vector<const Node*>& neighbors = node->getNeighbors();
            for (int i = 0; i < neighbors.size(); ++i)
                position[neighbors[i]->getIndex()] = i;
            std::vector<ogdf::adjEntry> order(neighbors.size());
            for (ogdf::adjEntry& adj : n->adjEntries) {
                const int neighbor = adj->twinNode()->index();
                order[position[neighbor]] = adj;
            }
            ogdf::List<ogdf::adjEntry> newOrder;
            for (ogdf::adjEntry& adj : order)
                newOrder.pushBack(adj);
            graph.sort(n, newOrder);
        }
        delete myGraph;
        delete embedding;
    }
};

extern "C" {
    int embedLoadedFile() {
        const Graph* graph = loadFromfile("input.txt");
        std::cout << "graph:\n";
        graph->print();
        if (!graph->isConnected()) {
            std::cerr << "Graph is not connected." << std::endl;
            delete graph;
            return -1;
        }
        Embedder embedder{};
        std::optional<const Embedding*> embedding = embedder.embedGraph(graph);
        std::cout << std::boolalpha << "graph is planar: " << embedding.has_value() << ".\n";
        if (embedding.has_value()) {
            std::cout << "embedding:\n";
            embedding.value()->print();

            std::unique_ptr<ogdf::Graph> ogdfGraph = std::unique_ptr<ogdf::Graph>(OgdfUtils::myGraphToOgdf(graph));
            ogdf::GraphAttributes GA(*ogdfGraph, ogdf::GraphAttributes::nodeGraphics | ogdf::GraphAttributes::edgeGraphics |
                                ogdf::GraphAttributes::nodeLabel | ogdf::GraphAttributes::edgeStyle |
                                ogdf::GraphAttributes::nodeStyle | ogdf::GraphAttributes::edgeArrow);
            for (ogdf::node v : ogdfGraph->nodes) {
                GA.label(v) = std::to_string(v->index());
                GA.shape(v) = ogdf::Shape::Ellipse;
            }
            for (ogdf::edge e : ogdfGraph->edges) {
                GA.strokeWidth(e) = 1.5;
                GA.arrowType(e) = ogdf::EdgeArrow::None;
            }
            ogdf::PlanarDrawLayout layout;
            layout.setEmbedder(new AuslanderParterEmbedder);
            layout.call(GA);

            std::ostringstream svgStream;
            ogdf::GraphIO::SVGSettings svgSettings;
            delete embedding.value();
            delete graph;
            if (ogdf::GraphIO::drawSVG(GA, svgStream, svgSettings)) {
                std::string svgContent = svgStream.str();
                saveStringToFile("/embedding.svg", svgContent);
                std::string embeddingString = embedding.value()->toString();
                saveStringToFile("/embedding.txt", embeddingString);
                return 1;
            }
            else {
                std::cerr << "Error generating SVG content." << std::endl;
                return -2;
            }
        }
        delete graph;
        return 0;
    }
}

class AuslanderParterEmbedderSefe : public ogdf::EmbedderModule {
private:
    const Embedding* embedding_m;
public:
    AuslanderParterEmbedderSefe(const Embedding* embedding) 
    : embedding_m(embedding) {}
    void doCall(ogdf::Graph& graph, ogdf::adjEntry &adjExternal) {
        std::vector<int> position(embedding_m->size());
        for (ogdf::node n : graph.nodes) {
            const int index = n->index();
            const Node* node = embedding_m->getNode(index);
            const std::vector<const Node*>& neighbors = node->getNeighbors();
            for (int i = 0; i < neighbors.size(); ++i)
                position[neighbors[i]->getIndex()] = i;
            std::vector<ogdf::adjEntry> order(neighbors.size());
            for (ogdf::adjEntry& adj : n->adjEntries) {
                const int neighbor = adj->twinNode()->index();
                order[position[neighbor]] = adj;
            }
            ogdf::List<ogdf::adjEntry> newOrder;
            for (ogdf::adjEntry& adj : order)
                newOrder.pushBack(adj);
            graph.sort(n, newOrder);
        }
    }
};

void processEmbeddingFromSefe(const Embedding* embedding, const Graph* intersection, const std::string& color, const std::string& outputFilename) {
    std::unique_ptr<ogdf::Graph> ogdfGraph = std::unique_ptr<ogdf::Graph>(OgdfUtils::myGraphToOgdf(embedding));
    ogdf::GraphAttributes GA(*ogdfGraph, ogdf::GraphAttributes::nodeGraphics |
                            ogdf::GraphAttributes::edgeGraphics |
                            ogdf::GraphAttributes::nodeLabel | ogdf::GraphAttributes::edgeStyle |
                            ogdf::GraphAttributes::nodeStyle | ogdf::GraphAttributes::edgeArrow);
    for (ogdf::node v : ogdfGraph->nodes) {
        GA.label(v) = std::to_string(v->index());
        GA.shape(v) = ogdf::Shape::Ellipse;
    }
    for (ogdf::edge e : ogdfGraph->edges) {
        ogdf::node from = e->source();
        ogdf::node to = e->target();
        int fromIndex = from->index();
        int toIndex = to->index();
        if (!intersection->hasEdge(fromIndex, toIndex))
            GA.strokeColor(e) = (color == "red") ? ogdf::Color(255, 0, 0) : ogdf::Color(0, 0, 255);
        GA.strokeWidth(e) = 1.5;
        GA.arrowType(e) = ogdf::EdgeArrow::None;
    }

    ogdf::PlanarDrawLayout layout;
    layout.setEmbedder(new AuslanderParterEmbedderSefe(embedding));
    layout.call(GA);

    std::ostringstream svgStream;
    ogdf::GraphIO::SVGSettings svgSettings;
    if (ogdf::GraphIO::drawSVG(GA, svgStream, svgSettings)) {
        std::string svgContent = svgStream.str();
        saveStringToFile(outputFilename, svgContent);
        std::string embeddingString = embedding->toString();
        saveStringToFile("/embedding-" + color + ".txt", embeddingString);
    } else
        std::cerr << "Error generating SVG content." << std::endl;
}

extern "C" {
    int sefeLoadedFiles() {
        const Graph* red = loadFromfile("red.txt");
        const Graph* blue = loadFromfile("blue.txt");
        if (red->size() != blue->size()) {
            std::cerr << "Error: graphs have different number of nodes." << std::endl;
            delete red;
            delete blue;
            return -2;
        }
        const BicoloredGraph graph(red, blue);
        const Graph* intersection = graph.getIntersection();
        std::cout << "graph:\n";
        graph.print();
        delete red;
        delete blue;
        BiconnectedComponentsHandler biconnectedComponentsHandler{intersection};
        if (biconnectedComponentsHandler.size() > 1) {
            std::cerr << "Error: intersection graph is not biconnected." << std::endl;
            return -1;
        }
        EmbedderSefe embedder{};
        std::optional<const EmbeddingSefe*> embedding = embedder.embedGraph(&graph);
        if (!embedding.has_value())
            return 0;
        // red embedding
        const Graph* redGraph = graph.computeRed();
        const Embedding* redEmbedding = embedding.value()->computeRedEmbedding(redGraph);
        std::cout << "red embedding:\n";
        redEmbedding->print();
        processEmbeddingFromSefe(redEmbedding, intersection, "red", "/embedding-red.svg");
        delete redGraph;
        delete redEmbedding;

        // blue embedding
        const Graph* blueGraph = graph.computeBlue();
        const Embedding* blueEmbedding = embedding.value()->computeBlueEmbedding(blueGraph);
        std::cout << "blue embedding:\n";
        blueEmbedding->print();
        processEmbeddingFromSefe(blueEmbedding, intersection, "blue", "/embedding-blue.svg");
        delete blueGraph;
        delete blueEmbedding;

        delete embedding.value();
        return 1;
    }
}