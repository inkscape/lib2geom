#include "testing.h"
#include <iostream>

#include <2geom/convex-hull.h>
#include <vector>
#include <iterator>

using namespace std;
using namespace Geom;

void points_from_shape(std::vector<Point> &pts, std::string const &shape) {
    pts.clear();
    int x = 0, y = 0;
    for (unsigned c = 0; c < shape.size(); ++c) {
        if (shape[c] == '\n') {
            x = 0; ++y;
            continue;
        }
        if (shape[c] == ' ') {
            ++x;
            continue;
        }
        pts.push_back(Point(x, y));
        ++x;
    }
}

class ConvexHullTest : public ::testing::Test {
protected:
    ConvexHullTest()
        : null(hulls[0])
        , point(hulls[1])
        , line(hulls[2])
        , triangle(hulls[3])
        , square(hulls[4])
        , hexagon(hulls[5])
        , antihexagon(hulls[6])
        , gem(hulls[7])
        , diamond(hulls[8])
    {
        null = ConvexHull();

        std::vector<Point> pts;

        pts.push_back(Point(0,0));
        point = ConvexHull(pts);
        pts.push_back(Point(1,0));
        line = ConvexHull(pts);
        pts.push_back(Point(0,1));
        triangle = ConvexHull(pts);
        pts.push_back(Point(1,1));
        square = ConvexHull(pts);
        pts.clear();

        for(int i = 0; i < 6; i++) {
            pts.push_back(Point(cos(i*M_PI*2/6), sin(i*M_PI*2/6)));
        }
        hexagon = ConvexHull(pts);
        pts.clear();

        for(int i = 0; i < 6; i++) {
            pts.push_back(Point(cos((1-i*2)*M_PI/6), sin((1-i*2)*M_PI/6)));
        }
        antihexagon = ConvexHull(pts);
        pts.clear();

        gem_shape = 
            " ++++  \n"
            "++++++ \n"
            "++++++ \n"
            "++++++ \n"
            " ++++  \n";
        points_from_shape(pts, gem_shape);
        gem.swap(pts);

        diamond_shape =
            "   +    \n"
            " +++++  \n"
            " +++++  \n"
            "+++++++ \n"
            " +++++  \n"
            " +++++  \n"
            "   +    \n";
        points_from_shape(pts, diamond_shape);
        diamond.swap(pts);
    }

    ConvexHull hulls[9];
    ConvexHull &null, &point, &line, &triangle, &square, &hexagon, &antihexagon, &gem, &diamond;
    std::string gem_shape, diamond_shape;
};

void check_convex(ConvexHull &/*ch*/) {
    // TODO
}

TEST_F(ConvexHullTest, SizeAndDegeneracy) {
    EXPECT_EQ(0u, null.size());
    EXPECT_TRUE(null.empty());
    EXPECT_TRUE(null.isDegenerate());
    EXPECT_FALSE(null.isSingular());
    EXPECT_FALSE(null.isLinear());

    EXPECT_EQ(1u, point.size());
    EXPECT_FALSE(point.empty());
    EXPECT_TRUE(point.isDegenerate());
    EXPECT_TRUE(point.isSingular());
    EXPECT_FALSE(point.isLinear());

    EXPECT_EQ(2u, line.size());
    EXPECT_FALSE(line.empty());
    EXPECT_TRUE(line.isDegenerate());
    EXPECT_FALSE(line.isSingular());
    EXPECT_TRUE(line.isLinear());

    EXPECT_EQ(3u, triangle.size());
    EXPECT_FALSE(triangle.empty());
    EXPECT_FALSE(triangle.isDegenerate());
    EXPECT_FALSE(triangle.isSingular());
    EXPECT_FALSE(triangle.isLinear());

    EXPECT_EQ(4u, square.size());
    EXPECT_FALSE(square.empty());
    EXPECT_FALSE(square.isDegenerate());
    EXPECT_FALSE(square.isSingular());
    EXPECT_FALSE(square.isLinear());

    EXPECT_EQ(6u, hexagon.size());
    EXPECT_FALSE(hexagon.empty());
    EXPECT_FALSE(hexagon.isDegenerate());
    EXPECT_FALSE(hexagon.isSingular());
    EXPECT_FALSE(hexagon.isLinear());

    EXPECT_EQ(6u, antihexagon.size());
    EXPECT_FALSE(antihexagon.empty());
    EXPECT_FALSE(antihexagon.isDegenerate());
    EXPECT_FALSE(antihexagon.isSingular());
    EXPECT_FALSE(antihexagon.isLinear());

    EXPECT_EQ(8u, gem.size());
    EXPECT_FALSE(gem.empty());
    EXPECT_FALSE(gem.isDegenerate());
    EXPECT_FALSE(gem.isSingular());
    EXPECT_FALSE(gem.isLinear());

    EXPECT_EQ(8u, diamond.size());
    EXPECT_FALSE(diamond.empty());
    EXPECT_FALSE(diamond.isDegenerate());
    EXPECT_FALSE(diamond.isSingular());
    EXPECT_FALSE(diamond.isLinear());
}


TEST_F(ConvexHullTest, Area) {
    EXPECT_EQ(0, null.area());
    EXPECT_EQ(0, point.area());
    EXPECT_EQ(0, line.area());
    EXPECT_EQ(0.5, triangle.area());
    EXPECT_EQ(1, square.area());
    EXPECT_EQ(18, gem.area());
    EXPECT_EQ(24, diamond.area());
    EXPECT_FLOAT_EQ(6*(0.5*1*sin(M_PI/3)), hexagon.area());
    EXPECT_FLOAT_EQ(6*(0.5*1*sin(M_PI/3)), antihexagon.area());
}

TEST_F(ConvexHullTest, Bounds) {
    //Rect hexbounds(-1,sin(M_PI/3),1,-sin(M_PI/3));

    EXPECT_EQ(OptRect(), null.bounds());
    EXPECT_EQ(OptRect(0,0,0,0), point.bounds());
    EXPECT_EQ(OptRect(0,0,1,0), line.bounds());
    EXPECT_EQ(OptRect(0,0,1,1), triangle.bounds());
    EXPECT_EQ(OptRect(0,0,1,1), square.bounds());
    EXPECT_EQ(OptRect(0,0,5,4), gem.bounds());
    EXPECT_EQ(OptRect(0,0,6,6), diamond.bounds());
    //EXPECT_TRUE(hexbounds == hexagon.bounds());
    //EXPECT_TRUE(hexbounds == antihexagon.bounds());
}

::testing::AssertionResult HullContainsPoint(ConvexHull const &h, Point const &p) {
    if (h.contains(p)) {
        return ::testing::AssertionSuccess();
    } else {
        return ::testing::AssertionFailure()
            << "Convex hull:\n"
            << h << "\ndoes not contain " << p;
    }
}

TEST_F(ConvexHullTest, PointContainment) {
    Point zero(0,0), half(0.5, 0.5), x(0.25, 0.25);
    EXPECT_FALSE(HullContainsPoint(null, zero));
    EXPECT_TRUE(HullContainsPoint(point, zero));
    EXPECT_TRUE(HullContainsPoint(line, zero));
    EXPECT_TRUE(HullContainsPoint(triangle, zero));
    EXPECT_TRUE(HullContainsPoint(square, zero));
    EXPECT_FALSE(HullContainsPoint(line, half));
    EXPECT_TRUE(HullContainsPoint(triangle, x));
    EXPECT_TRUE(HullContainsPoint(triangle, half));
    EXPECT_TRUE(HullContainsPoint(square, half));
    EXPECT_TRUE(HullContainsPoint(hexagon, zero));
    EXPECT_TRUE(HullContainsPoint(antihexagon, zero));

    std::vector<Point> pts;

    points_from_shape(pts, gem_shape);
    for (unsigned i = 0; i < pts.size(); ++i) {
        EXPECT_TRUE(HullContainsPoint(gem, pts[i]));
    }

    points_from_shape(pts, diamond_shape);
    for (unsigned i = 0; i < pts.size(); ++i) {
        EXPECT_TRUE(HullContainsPoint(diamond, pts[i]));
    }

    /*EXPECT_FALSE(null.interiorContains(zero));
    EXPECT_FALSE(point.interiorContains(zero));
    EXPECT_FALSE(line.interiorContains(zero));
    EXPECT_FALSE(triangle.interiorContains(zero));
    EXPECT_FALSE(square.interiorContains(zero));
    EXPECT_FALSE(line.interiorContains(half));
    EXPECT_FALSE(triangle.interiorContains(Point(0,0.5)));
    EXPECT_FALSE(triangle.interiorContains(half));
    EXPECT_TRUE(square.interiorContains(half));*/
}

TEST_F(ConvexHullTest, ExtremePoints) {
    Point zero(0,0);
    EXPECT_EQ(0., point.top());
    EXPECT_EQ(0., point.right());
    EXPECT_EQ(0., point.bottom());
    EXPECT_EQ(0., point.left());
    EXPECT_EQ(zero, point.topPoint());
    EXPECT_EQ(zero, point.rightPoint());
    EXPECT_EQ(zero, point.bottomPoint());
    EXPECT_EQ(zero, point.leftPoint());

    // line from 0,0 to 1,0
    EXPECT_EQ(0., line.top());
    EXPECT_EQ(1., line.right());
    EXPECT_EQ(0., line.bottom());
    EXPECT_EQ(0., line.left());
    EXPECT_EQ(Point(1,0), line.topPoint());
    EXPECT_EQ(Point(1,0), line.rightPoint());
    EXPECT_EQ(Point(0,0), line.bottomPoint());
    EXPECT_EQ(Point(0,0), line.leftPoint());

    // triangle 0,0 1,0 0,1
    EXPECT_EQ(0., triangle.top());
    EXPECT_EQ(1., triangle.right());
    EXPECT_EQ(1., triangle.bottom());
    EXPECT_EQ(0., triangle.left());
    EXPECT_EQ(Point(1,0), triangle.topPoint());
    EXPECT_EQ(Point(1,0), triangle.rightPoint());
    EXPECT_EQ(Point(0,1), triangle.bottomPoint());
    EXPECT_EQ(Point(0,0), triangle.leftPoint());

    // square 0,0 to 1,1
    EXPECT_EQ(0., square.top());
    EXPECT_EQ(1., square.right());
    EXPECT_EQ(1., square.bottom());
    EXPECT_EQ(0., square.left());
    EXPECT_EQ(Point(1,0), square.topPoint());
    EXPECT_EQ(Point(1,1), square.rightPoint());
    EXPECT_EQ(Point(0,1), square.bottomPoint());
    EXPECT_EQ(Point(0,0), square.leftPoint());

    EXPECT_EQ(0., gem.top());
    EXPECT_EQ(5., gem.right());
    EXPECT_EQ(4., gem.bottom());
    EXPECT_EQ(0., gem.left());
    EXPECT_EQ(Point(4,0), gem.topPoint());
    EXPECT_EQ(Point(5,3), gem.rightPoint());
    EXPECT_EQ(Point(1,4), gem.bottomPoint());
    EXPECT_EQ(Point(0,1), gem.leftPoint());

    EXPECT_EQ(0., diamond.top());
    EXPECT_EQ(6., diamond.right());
    EXPECT_EQ(6., diamond.bottom());
    EXPECT_EQ(0., diamond.left());
    EXPECT_EQ(Point(3,0), diamond.topPoint());
    EXPECT_EQ(Point(6,3), diamond.rightPoint());
    EXPECT_EQ(Point(3,6), diamond.bottomPoint());
    EXPECT_EQ(Point(0,3), diamond.leftPoint());
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
