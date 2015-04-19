#include "testing.h"
#include <iostream>

#include <2geom/intersection-graph.h>
#include <2geom/pathvector.h>
#include <2geom/svg-path-parser.h>
#include <2geom/svg-path-writer.h>

using namespace std;
using namespace Geom;

Path string_to_path(const char* s) {
    PathVector pv = parse_svg_path(s);
    assert(pv.size() == 1);
    return pv[0];
}

class IntersectionGraphTest : public ::testing::Test {
protected:
    IntersectionGraphTest() {
        rectangle = string_to_path("M 0,0 L 5,0 5,8 0,8 Z");
        bigrect = string_to_path("M -3,-4 L 7,-4 7,12 -3,12 Z");
        bigh = string_to_path("M 2,-3 L 3,-2 1,2 3,4 4,2 6,3 2,11 0,10 2,5 1,4 -1,6 -2,5 Z");
        smallrect = string_to_path("M 7,4 L 9,4 9,7 7,7 Z");
    }

    // Objects declared here can be used by all tests in the test case for Foo.
    Path rectangle, bigrect, bigh, smallrect;
};

TEST_F(IntersectionGraphTest, Union) {
    PathIntersectionGraph graph(rectangle, bigh);

    PathVector r = graph.getUnion();
    EXPECT_EQ(r.size(), 1);

    /*SVGPathWriter wr;
    wr.feed(r);
    std::cout << wr.str() << std::endl;*/
}

TEST_F(IntersectionGraphTest, DisjointUnion) {
    PathIntersectionGraph graph(rectangle, smallrect);

    PathVector r = graph.getUnion();
    EXPECT_EQ(r.size(), 2);
}

TEST_F(IntersectionGraphTest, CoverUnion) {
    PathIntersectionGraph graph(bigrect, bigh);
    PathVector r = graph.getUnion();
    EXPECT_EQ(r.size(), 1);
    EXPECT_EQ(r, bigrect);
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
