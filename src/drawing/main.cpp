#include "polygon.hpp"
#include "svg_drawer.hpp"

#include <iostream>

int main() {
    std::vector<Point2D> points = {
        Point2D(10, 10),
        Point2D(50, 10),
        Point2D(60, 40),
        Point2D(30, 60),
        Point2D(30, 40),
    };
    Polygon2D polygon(points);
    SvgDrawer svgDrawer(100, 100);
    svgDrawer.add(polygon);
    auto path = polygon.computePathInside(points[0], points[3]);
    svgDrawer.add(path, "red");
    svgDrawer.saveToFile("polygon.svg");
    auto convexHull = polygon.computeConvexHull();
    for (auto point : convexHull) {
        std::cout << (point) << std::endl;
    }
    return 0;
}
