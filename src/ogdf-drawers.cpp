#include "ogdf-drawers.hpp"

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/planarlayout/PlanarDrawLayout.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/planarity/EmbedderModule.h>

#include <memory>

#include "basic/utils.hpp"

class AuslanderParterEmbedder : public ogdf::EmbedderModule {
public:
    void doCall(ogdf::Graph& graph, ogdf::adjEntry &adjExternal) {
        const Graph* myGraph = OgdfUtils::ogdfGraphToMyGraph(&graph);
        std::unique_ptr<const Graph> myGraphPtr(myGraph);
        Embedder embedder;
        std::optional<const Embedding*> embeddingOpt = embedder.embedGraph(myGraph);
        if (!embeddingOpt) {
            std::cout << "error\n";
            exit(1);
        }
        const Embedding* embedding = embeddingOpt.value();
        std::unique_ptr<const Embedding> embeddingPtr(embedding);
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
    }
};

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

void drawEmbeddingToFile(const Graph* graph, const Embedding* embedding) {
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
    if (ogdf::GraphIO::drawSVG(GA, svgStream, svgSettings)) {
        std::string svgContent = svgStream.str();
        saveStringToFile("/embedding.svg", svgContent);
        std::string embeddingString = embedding->toString();
        saveStringToFile("/embedding.txt", embeddingString);
    }
    else
        std::cerr << "Error generating SVG content." << std::endl;
}

void drawSefeEmbeddingToFile(const Embedding* embedding, const Graph* intersection, const std::string& color, const std::string& outputFilename) {
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