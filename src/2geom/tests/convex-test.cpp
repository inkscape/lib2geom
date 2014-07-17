#include "testing.h"
#include <iostream>

#include <2geom/convex-cover.h>
#include <vector>
#include <iterator>

using namespace std;


using namespace Geom;

namespace {

// The fixture for testing class Foo.
class ConvexHullTest : public ::testing::Test {
protected:
    // You can remove any or all of the following functions if its body
    // is empty.

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

    virtual ~ConvexHullTest() {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    virtual void SetUp() {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    virtual void TearDown() {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }

    // Objects declared here can be used by all tests in the test case for Foo.
    ConvexHull null, point, line, triangle, square, hexagon, antihexagon;

};

void check_convex(ConvexHull &ch) {
    EXPECT_TRUE(ch.is_clockwise());
    EXPECT_TRUE(ch.top_point_first());
    if(!ch.meets_invariants()) {
	cout << ch << endl;
    }
}

TEST_F(ConvexHullTest, size) {
    EXPECT_EQ(0u, null.size());
    EXPECT_TRUE(null.is_degenerate());
    EXPECT_EQ(1u, point.size());
    EXPECT_TRUE(point.is_degenerate());
    EXPECT_EQ(2u, line.size());
    EXPECT_TRUE(line.is_degenerate());
    EXPECT_EQ(3u, triangle.size());
    EXPECT_FALSE(triangle.is_degenerate());
    EXPECT_EQ(4u, square.size());
    EXPECT_FALSE(square.is_degenerate());
    EXPECT_EQ(6u, hexagon.size());
    EXPECT_FALSE(hexagon.is_degenerate());
    EXPECT_EQ(6u, antihexagon.size());
    EXPECT_FALSE(antihexagon.is_degenerate());
    check_convex(null);
    check_convex(point);
    check_convex(line);
    check_convex(triangle);
    check_convex(square);
    check_convex(hexagon);
    check_convex(antihexagon);
}

TEST_F(ConvexHullTest, area) {
    EXPECT_EQ(0, null.area());
    EXPECT_EQ(0, point.area());
    EXPECT_EQ(0, line.area());
    EXPECT_EQ(0.5, triangle.area());
    EXPECT_EQ(1, square.area());
    EXPECT_FLOAT_EQ(6*(0.5*1*sin(M_PI/3)), hexagon.area());
    EXPECT_FLOAT_EQ(6*(0.5*1*sin(M_PI/3)), antihexagon.area());
}

TEST_F(ConvexHullTest, PointContainment) {
    Point zero(0,0), half(0.5, 0.5);
    EXPECT_FALSE(null.contains_point(zero));
    EXPECT_TRUE(point.contains_point(zero));
    EXPECT_TRUE(line.contains_point(zero));
    EXPECT_TRUE(triangle.contains_point(zero));
    EXPECT_TRUE(square.contains_point(zero));
    EXPECT_FALSE(line.contains_point(half));
    EXPECT_TRUE(triangle.contains_point(half));
    EXPECT_TRUE(square.contains_point(half));

    EXPECT_FALSE(null.strict_contains_point(zero));
    EXPECT_FALSE(point.strict_contains_point(zero));
    EXPECT_FALSE(line.strict_contains_point(zero));
    EXPECT_FALSE(triangle.strict_contains_point(zero));
    EXPECT_FALSE(square.strict_contains_point(zero));
    EXPECT_FALSE(line.strict_contains_point(half));
    EXPECT_FALSE(triangle.strict_contains_point(Point(0,0.5)));
    EXPECT_FALSE(triangle.strict_contains_point(half));
    EXPECT_TRUE(square.strict_contains_point(half));
}

TEST_F(ConvexHullTest, PointMerging) {
    Point zero(0,0), half(0.5, 0.5);
    null.merge(zero);
    EXPECT_EQ(1u, null.size());
    check_convex(null);
    point.merge(zero);
    EXPECT_EQ(1u, point.size());
    check_convex(point);
    //cout << line << endl;
    line.merge(zero);
    check_convex(line);
    EXPECT_EQ(2u, line.size());
    line.merge(half);
    check_convex(line);
    EXPECT_EQ(3u, line.size());
    line.merge(half);
    check_convex(line);
    EXPECT_EQ(3u, line.size());
    ConvexHull trySquare = square;
    trySquare.merge(half);
    check_convex(trySquare);
    EXPECT_EQ(4u, trySquare.size());
    trySquare.merge(Point(1e12, 0));
    check_convex(trySquare);
    EXPECT_EQ(5u, trySquare.size());
    trySquare.merge(Point(1e12, 1));
    cout << trySquare << endl;
    check_convex(trySquare);
    EXPECT_EQ(4u, trySquare.size());
    trySquare = square;
    
    trySquare.merge(Point(-1e12, -1e12));
    cout << trySquare << endl;
    check_convex(trySquare);
    EXPECT_EQ(4u, trySquare.size());
}
    
TEST_F(ConvexHullTest, Merging) {
    ConvexHull dodecagon = graham_merge(hexagon, antihexagon);
    EXPECT_EQ(12u, dodecagon.size());
    
    dodecagon = andrew_merge(hexagon, antihexagon);
    EXPECT_EQ(12u, dodecagon.size());
    
    //dodecagon = merge(hexagon, antihexagon);
    //EXPECT_EQ(12, dodecagon.size());
}



}  // namespace


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
