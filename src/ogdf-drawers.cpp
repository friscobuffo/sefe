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
        auto myGraph = OgdfUtils::ogdfGraphToMyGraph(graph);
        Embedder embedder;
        auto embeddingOpt = embedder.embedGraph(*myGraph);
        if (!embeddingOpt) {
            std::cout << "error\n";
            exit(1);
        }
        auto& embedding = *embeddingOpt.value().get();
        std::vector<int> position(embedding.size());
        for (ogdf::node n : graph.nodes) {
            const int index = n->index();
            const Node& node = embedding.getNode(index);
            auto& edges = node.getEdges();
            for (int i = 0; i < edges.size(); ++i)
                position[edges[i].to.getIndex()] = i;
            std::vector<ogdf::adjEntry> order(edges.size());
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
    const Graph& embedding_m;
public:
    AuslanderParterEmbedderSefe(const Graph& embedding) 
    : embedding_m(embedding) {}
    void doCall(ogdf::Graph& graph, ogdf::adjEntry &adjExternal) {
        std::vector<int> position(embedding_m.size());
        for (ogdf::node n : graph.nodes) {
            const int index = n->index();
            const Node& node = embedding_m.getNode(index);
            auto& edges = node.getEdges();
            for (int i = 0; i < edges.size(); ++i)
                position[edges[i].to.getIndex()] = i;
            std::vector<ogdf::adjEntry> order(edges.size());
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

void drawEmbeddingToFile(const Graph& graph, const Graph& embedding) {
    std::unique_ptr<ogdf::Graph> ogdfGraph = OgdfUtils::myGraphToOgdf(graph);
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
        std::string embeddingString = embedding.toString();
        saveStringToFile("/embedding.txt", embeddingString);
    }
    else
        std::cerr << "Error generating SVG content." << std::endl;
}

void drawSefeProjectionEmbeddingToFile(const Graph& embedding, const Graph& intersection, const std::string& color, const std::string& outputFilename) {
    std::unique_ptr<ogdf::Graph> ogdfGraph = OgdfUtils::myGraphToOgdf(embedding);
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
        if (!intersection.hasEdge(fromIndex, toIndex))
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
        std::string embeddingString = embedding.toString();
        saveStringToFile("/embedding-" + color + ".txt", embeddingString);
    } else
        std::cerr << "Error generating SVG content." << std::endl;
}

#include "drawing/polygon.hpp"

void drawSefeEmbeddingToFile(const Graph& embedding, const Graph& intersection,
const std::string& outputFilename) {
    std::unique_ptr<const SubGraph> redEmbedding = embedding.computeRedProjection();
    std::unique_ptr<ogdf::Graph> ogdfGraph = OgdfUtils::myGraphToOgdf(*redEmbedding);
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
        if (!intersection.hasEdge(fromIndex, toIndex))
            GA.strokeColor(e) = ogdf::Color(255, 0, 0);
        GA.strokeWidth(e) = 1.5;
        GA.arrowType(e) = ogdf::EdgeArrow::None;
    }

    ogdf::PlanarDrawLayout layout;
    layout.setEmbedder(new AuslanderParterEmbedderSefe(*redEmbedding));
    layout.call(GA);

    std::vector<double> xCoords;
    std::vector<double> yCoords;
    for (ogdf::node v : ogdfGraph->nodes) {
        xCoords.push_back(GA.x(v));
        yCoords.push_back(GA.y(v));
    }

    for (int nodeIndex = 0; nodeIndex < embedding.size(); ++nodeIndex) {
        const Node& node = embedding.getNode(nodeIndex);
        for (const auto& edge : node.getEdges()) {
            int neighborIndex = edge.to.getIndex();
            if (nodeIndex > neighborIndex) 
                continue;
            if (edge.color != Color::BLUE)
                continue;
            std::vector<int> boundingFace = embedding.findBoundingFace(nodeIndex, neighborIndex, Color::BLUE);
            int startingIndex = -1;
            int endingIndex = -1;
            for (int i = 0; i < boundingFace.size(); ++i) {
                if (boundingFace[i] == nodeIndex)
                    startingIndex = i;
                if (boundingFace[i] == neighborIndex)
                    endingIndex = i;
            }
            std::vector<Point2D> points;
            for (int i = 0; i < boundingFace.size(); ++i) {
                int index = boundingFace[i];
                points.push_back(Point2D(xCoords[index], yCoords[index]));
            }
            Polygon2D polygon{points};
            const auto insidePath = polygon.computePathInside(points[startingIndex], points[endingIndex]);
            ogdf::node u = ogdfGraph->newNode();
            GA.x(u) = insidePath.points[0].x;
            GA.y(u) = insidePath.points[0].y;
            GA.width(u) = 0.0;
            GA.height(u) = 0.0;
            GA.shape(u) = ogdf::Shape::Ellipse;
            for (int i = 1; i < insidePath.points.size(); ++i) {
                ogdf::node v = ogdfGraph->newNode();
                GA.x(v) = insidePath.points[i].x;
                GA.y(v) = insidePath.points[i].y;
                GA.width(v) = 0.0;
                GA.height(v) = 0.0;
                GA.shape(v) = ogdf::Shape::Ellipse;
                ogdf::edge e = ogdfGraph->newEdge(u, v);
                GA.strokeWidth(e) = 1.5;
                GA.strokeColor(e) = ogdf::Color(0, 0, 255);
                GA.arrowType(e) = ogdf::EdgeArrow::None;
                u = v;
            }
        }
    }
    std::ostringstream svgStream;
    ogdf::GraphIO::SVGSettings svgSettings;
    if (ogdf::GraphIO::drawSVG(GA, svgStream, svgSettings)) {
        std::string svgContent = svgStream.str();
        saveStringToFile(outputFilename, svgContent);
        std::string embeddingString = embedding.toString();
        saveStringToFile("/sefe-embedding.txt", embeddingString);
    } else
        std::cerr << "Error generating SVG content." << std::endl;
}