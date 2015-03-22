#include "testing.h"
#include <iostream>

#include <2geom/convex-hull.h>
#include <vector>
#include <iterator>

using namespace std;
using namespace Geom;

class ConvexHullTest : public ::testing::Test {
protected:

    ConvexHullTest() {
        // You can do set-up work for each test here.
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
    }

    ConvexHull null, point, line, triangle, square, hexagon, antihexagon;

};

void check_convex(ConvexHull &/*ch*/) {
    // TODO
}

TEST_F(ConvexHullTest, size) {
    EXPECT_EQ(0u, null.size());
    EXPECT_TRUE(null.isDegenerate());
    EXPECT_EQ(1u, point.size());
    EXPECT_TRUE(point.isDegenerate());
    EXPECT_EQ(2u, line.size());
    EXPECT_TRUE(line.isDegenerate());
    EXPECT_EQ(3u, triangle.size());
    EXPECT_FALSE(triangle.isDegenerate());
    EXPECT_EQ(4u, square.size());
    EXPECT_FALSE(square.isDegenerate());
    EXPECT_EQ(6u, hexagon.size());
    EXPECT_FALSE(hexagon.isDegenerate());
    EXPECT_EQ(6u, antihexagon.size());
    EXPECT_FALSE(antihexagon.isDegenerate());
    check_convex(null);
    check_convex(point);
    check_convex(line);
    check_convex(triangle);
    check_convex(square);
    check_convex(hexagon);
    check_convex(antihexagon);
}


TEST_F(ConvexHullTest, Area) {
    EXPECT_EQ(0, null.area());
    EXPECT_EQ(0, point.area());
    EXPECT_EQ(0, line.area());
    EXPECT_EQ(0.5, triangle.area());
    EXPECT_EQ(1, square.area());
    EXPECT_FLOAT_EQ(6*(0.5*1*sin(M_PI/3)), hexagon.area());
    EXPECT_FLOAT_EQ(6*(0.5*1*sin(M_PI/3)), antihexagon.area());
}

TEST_F(ConvexHullTest, Bounds) {
    //Rect hexbounds(-1,sin(M_PI/3),1,-sin(M_PI/3));

    EXPECT_TRUE(OptRect() == null.bounds());
    EXPECT_TRUE(Rect(0,0,0,0) == point.bounds());
    EXPECT_TRUE(Rect(0,0,1,0) == line.bounds());
    EXPECT_TRUE(Rect(0,0,1,1) == triangle.bounds());
    EXPECT_TRUE(Rect(0,0,1,1) == square.bounds());
    //EXPECT_TRUE(hexbounds == hexagon.bounds());
    //EXPECT_TRUE(hexbounds == antihexagon.bounds());
}

TEST_F(ConvexHullTest, PointContainment) {
    Point zero(0,0), half(0.5, 0.5), x(0.25, 0.25);
    EXPECT_FALSE(null.contains(zero));
    EXPECT_TRUE(point.contains(zero));
    EXPECT_TRUE(line.contains(zero));
    EXPECT_TRUE(triangle.contains(zero));
    EXPECT_TRUE(square.contains(zero));
    EXPECT_FALSE(line.contains(half));
    EXPECT_TRUE(triangle.contains(x));
    EXPECT_TRUE(triangle.contains(half));
    EXPECT_TRUE(square.contains(half));
    EXPECT_TRUE(hexagon.contains(zero));
    EXPECT_TRUE(antihexagon.contains(zero));

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
