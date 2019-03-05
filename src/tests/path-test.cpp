#include "testing.h"
#include <iostream>

#include <2geom/bezier.h>
#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <2geom/path-intersection.h>
#include <2geom/svg-path-parser.h>
#include <2geom/svg-path-writer.h>
#include <vector>
#include <iterator>

using namespace std;
using namespace Geom;

Path string_to_path(const char* s) {
    PathVector pv = parse_svg_path(s);
    assert(pv.size() == 1);
    return pv[0];
}

// Path fixture
class PathTest : public ::testing::Test {
protected:
    PathTest() {
        line.append(LineSegment(Point(0,0), Point(1,0)));
        square = string_to_path("M 0,0 1,0 1,1 0,1 z");
        circle = string_to_path("M 0,0 a 4.5,4.5 0 1 1 -9,0 4.5,4.5 0 1 1 9,0 z");
        arcs = string_to_path("M 0,0 a 5,10 45 0 1 10,10 a 5,10 45 0 1 0,0 z");
        diederik = string_to_path("m 262.6037,35.824151 c 0,0 -92.64892,-187.405851 30,-149.999981 104.06976,31.739531 170,109.9999815 170,109.9999815 l -10,-59.9999905 c 0,0 40,79.99999 -40,79.99999 -80,0 -70,-129.999981 -70,-129.999981 l 50,0 C 435.13571,-131.5667 652.76275,126.44872 505.74322,108.05672 358.73876,89.666591 292.6037,-14.175849 292.6037,15.824151 c 0,30 -30,20 -30,20 z");
        cmds = string_to_path("M 0,0 V 100 H 100 Q 100,0 0,0 L 200,0 C 200,100 300,100 300,0 S 200,-100 200,0");
        
        p_open = string_to_path("M 0,0 L 0,5 5,5 5,0");
        p_closed = p_open;
        p_closed.close(true);
        p_add = string_to_path("M -1,6 L 6,6");

        p_open.setStitching(true);
        p_closed.setStitching(true);
    }

    // Objects declared here can be used by all tests in the test case for Foo.
    Path line, square, circle, arcs, diederik, cmds;
    Path p_open, p_closed, p_add;
};

TEST_F(PathTest, CopyConstruction) {
    Path pa = p_closed;
    Path pc(p_closed);
    EXPECT_EQ(pa, p_closed);
    EXPECT_EQ(pa.closed(), p_closed.closed());
    EXPECT_EQ(pc, p_closed);
    EXPECT_EQ(pc.closed(), p_closed.closed());
    
    Path poa = cmds;
    Path poc(cmds);
    EXPECT_EQ(poa, cmds);
    EXPECT_EQ(poa.closed(), cmds.closed());
    EXPECT_EQ(poc, cmds);
    EXPECT_EQ(poc.closed(), cmds.closed());
    
    PathVector pvc(pa);
    EXPECT_EQ(pvc[0], pa);
    PathVector pva((Geom::Path()));
    pva[0] = pa;
    EXPECT_EQ(pva[0], pa);
}

TEST_F(PathTest, PathInterval) {
    PathTime n2_before(1, 0.9995), n2_after(2, 0.0005),
                 n3_before(2, 0.9995), n3_after(3, 0.0005),
                 mid2(2, 0.5), mid3(3, 0.5);

    // ival[x][0] - normal
    // ival[x][1] - reversed
    // ival[x][2] - crosses start
    // ival[x][3] - reversed, crosses start
    PathInterval ival[5][4];

    ival[0][0] = PathInterval(n2_before, n2_after, false, 4);
    ival[0][1] = PathInterval(n2_after, n2_before, false, 4);
    ival[0][2] = PathInterval(n2_before, n2_after, true, 4);
    ival[0][3] = PathInterval(n2_after, n2_before, true, 4);
    ival[1][0] = PathInterval(n2_before, n3_after, false, 4);
    ival[1][1] = PathInterval(n3_after, n2_before, false, 4);
    ival[1][2] = PathInterval(n2_before, n3_after, true, 4);
    ival[1][3] = PathInterval(n3_after, n2_before, true, 4);
    ival[2][0] = PathInterval(n2_before, mid2, false, 4);
    ival[2][1] = PathInterval(mid2, n2_before, false, 4);
    ival[2][2] = PathInterval(n2_before, mid2, true, 4);
    ival[2][3] = PathInterval(mid2, n2_before, true, 4);
    ival[3][0] = PathInterval(mid2, mid3, false, 4);
    ival[3][1] = PathInterval(mid3, mid2, false, 4);
    ival[3][2] = PathInterval(mid2, mid3, true, 4);
    ival[3][3] = PathInterval(mid3, mid2, true, 4);
    ival[4][0] = PathInterval(n2_after, n3_before, false, 4);
    ival[4][1] = PathInterval(n3_before, n2_after, false, 4);
    ival[4][2] = PathInterval(n2_after, n3_before, true, 4);
    ival[4][3] = PathInterval(n3_before, n2_after, true, 4);

    EXPECT_TRUE(ival[0][0].contains(n2_before));
    EXPECT_TRUE(ival[0][0].contains(n2_after));
    EXPECT_TRUE(ival[0][1].contains(n2_before));
    EXPECT_TRUE(ival[0][1].contains(n2_after));

    for (unsigned i = 0; i <= 4; ++i) {
        EXPECT_FALSE(ival[i][0].reverse());
        EXPECT_TRUE(ival[i][1].reverse());
        EXPECT_TRUE(ival[i][2].reverse());
        EXPECT_FALSE(ival[i][3].reverse());
    }

    for (unsigned i = 0; i <= 4; ++i) {
        for (unsigned j = 0; j <= 3; ++j) {
            //std::cout << i << " " << j << " " << ival[i][j] << std::endl;
            EXPECT_TRUE(ival[i][j].contains(ival[i][j].inside(1e-3)));
        }
    }

    PathTime n1(1, 0.0), n1x(0, 1.0),
                 n2(2, 0.0), n2x(1, 1.0),
                 n3(3, 0.0), n3x(2, 1.0);
    PathTime tests[8] = { n1, n1x, n2, n2x, n3, n3x, mid2, mid3 };

    // 0: false for both
    // 1: true for normal, false for cross_start
    // 2: false for normal, true for cross_start
    // 3: true for both

    int const NORMAL = 1, CROSS = 2, BOTH = 3;

    int includes[5][8] = {
        { CROSS,  CROSS,  NORMAL, NORMAL, CROSS,  CROSS,  CROSS,  CROSS  },
        { CROSS,  CROSS,  NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, CROSS  },
        { CROSS,  CROSS,  NORMAL, NORMAL, CROSS,  CROSS,  BOTH,   CROSS  },
        { CROSS,  CROSS,  CROSS,  CROSS,  NORMAL, NORMAL, BOTH,   BOTH   },
        { CROSS,  CROSS,  CROSS,  CROSS,  CROSS,  CROSS,  NORMAL, CROSS  }
    };
    unsigned sizes[5][2] = {
        { 2, 4 },
        { 3, 3 },
        { 2, 4 },
        { 2, 4 },
        { 1, 5 }
    };

    for (unsigned i = 0; i < 5; ++i) {
        for (unsigned j = 0; j < 8; ++j) {
            EXPECT_EQ(ival[i][0].contains(tests[j]), bool(includes[i][j] & NORMAL));
            EXPECT_EQ(ival[i][1].contains(tests[j]), bool(includes[i][j] & NORMAL));
            EXPECT_EQ(ival[i][2].contains(tests[j]), bool(includes[i][j] & CROSS));
            EXPECT_EQ(ival[i][3].contains(tests[j]), bool(includes[i][j] & CROSS));
        }
        EXPECT_EQ(ival[i][0].curveCount(), sizes[i][0]);
        EXPECT_EQ(ival[i][1].curveCount(), sizes[i][0]);
        EXPECT_EQ(ival[i][2].curveCount(), sizes[i][1]);
        EXPECT_EQ(ival[i][3].curveCount(), sizes[i][1]);
    }
}

TEST_F(PathTest, Continuity) {
    line.checkContinuity();
    square.checkContinuity();
    circle.checkContinuity();
    diederik.checkContinuity();
    cmds.checkContinuity();
}

TEST_F(PathTest, RectConstructor) {
    Rect r(Point(0,0), Point(10,10));
    Path rpath(r);

    EXPECT_EQ(rpath.size(), 4u);
    EXPECT_TRUE(rpath.closed());
    for (unsigned i = 0; i < 4; ++i) {
        EXPECT_TRUE(dynamic_cast<LineSegment const *>(&rpath[i]) != NULL);
        EXPECT_EQ(rpath[i].initialPoint(), r.corner(i));
    }
}

TEST_F(PathTest, Reversed) {
    std::vector<Path> a, r;
    a.push_back(p_open);
    a.push_back(p_closed);
    a.push_back(circle);
    a.push_back(diederik);
    a.push_back(cmds);

    for (unsigned i = 0; i < a.size(); ++i) {
        r.push_back(a[i].reversed());
    }

    for (unsigned i = 0; i < a.size(); ++i) {
        EXPECT_EQ(r[i].size(), a[i].size());
        EXPECT_EQ(r[i].initialPoint(), a[i].finalPoint());
        EXPECT_EQ(r[i].finalPoint(), a[i].initialPoint());
        EXPECT_EQ(r[i].reversed(), a[i]);
        Point p1 = r[i].pointAt(0.75);
        Point p2 = a[i].pointAt(a[i].size() - 0.75);
        EXPECT_FLOAT_EQ(p1[X], p2[X]);
        EXPECT_FLOAT_EQ(p1[Y], p2[Y]);
        EXPECT_EQ(r[i].closed(), a[i].closed());
        a[i].checkContinuity();
    }
}

TEST_F(PathTest, ValueAt) {
    EXPECT_EQ(Point(0,0), line.initialPoint());
    EXPECT_EQ(Point(1,0), line.finalPoint());

    EXPECT_EQ(Point(0.5, 0.0), line.pointAt(0.5));

    EXPECT_EQ(Point(0,0), square.initialPoint());
    EXPECT_EQ(Point(0,0), square.finalPoint());
    EXPECT_EQ(Point(1,0), square.pointAt(1));
    EXPECT_EQ(Point(0.5,1), square.pointAt(2.5));
    EXPECT_EQ(Point(0,0.5), square.pointAt(3.5));
    EXPECT_EQ(Point(0,0), square.pointAt(4));
}

TEST_F(PathTest, NearestPoint) {
    EXPECT_EQ(0, line.nearestTime(Point(0,0)).asFlatTime());
    EXPECT_EQ(0.5, line.nearestTime(Point(0.5,0)).asFlatTime());
    EXPECT_EQ(0.5, line.nearestTime(Point(0.5,1)).asFlatTime());
    EXPECT_EQ(1, line.nearestTime(Point(100,0)).asFlatTime());
    EXPECT_EQ(0, line.nearestTime(Point(-100,1000)).asFlatTime());

    EXPECT_EQ(0, square.nearestTime(Point(0,0)).asFlatTime());
    EXPECT_EQ(1, square.nearestTime(Point(1,0)).asFlatTime());
    EXPECT_EQ(3, square.nearestTime(Point(0,1)).asFlatTime());
    
    //cout << diederik.nearestTime(Point(247.32293,-43.339507)) << endl;

    Point p(511.75,40.85);
    EXPECT_FLOAT_EQ(6.5814033, diederik.nearestTime(p).asFlatTime());
    /*cout << diederik.pointAt(diederik.nearestTime(p)) << endl
         << diederik.pointAt(6.5814033) << endl
         << distance(diederik.pointAt(diederik.nearestTime(p)), p) << "  "
         << distance(diederik.pointAt(6.5814033), p) << endl;*/

}

TEST_F(PathTest, Winding) {
    // test points in special positions
    EXPECT_EQ(line.winding(Point(-1, 0)), 0);
    EXPECT_EQ(line.winding(Point(2, 0)), 0);
    EXPECT_EQ(line.winding(Point(0, 1)), 0);
    EXPECT_EQ(line.winding(Point(0, -1)), 0);
    EXPECT_EQ(line.winding(Point(1, 1)), 0);
    EXPECT_EQ(line.winding(Point(1, -1)), 0);

    EXPECT_EQ(square.winding(Point(0, -1)), 0);
    EXPECT_EQ(square.winding(Point(1, -1)), 0);
    EXPECT_EQ(square.winding(Point(0, 2)), 0);
    EXPECT_EQ(square.winding(Point(1, 2)), 0);
    EXPECT_EQ(square.winding(Point(-1, 0)), 0);
    EXPECT_EQ(square.winding(Point(-1, 1)), 0);
    EXPECT_EQ(square.winding(Point(2, 0)), 0);
    EXPECT_EQ(square.winding(Point(2, 1)), 0);
    EXPECT_EQ(square.winding(Point(0.5, 0.5)), 1);

    EXPECT_EQ(circle.winding(Point(-4.5,0)), 1);
    EXPECT_EQ(circle.winding(Point(-3.5,0)), 1);
    EXPECT_EQ(circle.winding(Point(-4.5,1)), 1);
    EXPECT_EQ(circle.winding(Point(-10,0)), 0);
    EXPECT_EQ(circle.winding(Point(1,0)), 0);

    Path yellipse = string_to_path("M 0,0 A 40 20 90 0 0 0,-80 40 20 90 0 0 0,0 z");
    EXPECT_EQ(yellipse.winding(Point(-1, 0)), 0);
    EXPECT_EQ(yellipse.winding(Point(-1, -80)), 0);
    EXPECT_EQ(yellipse.winding(Point(1, 0)), 0);
    EXPECT_EQ(yellipse.winding(Point(1, -80)), 0);
    EXPECT_EQ(yellipse.winding(Point(0, -40)), -1);
    std::vector<double> r[4];
    r[0] = yellipse[0].roots(0, Y);
    r[1] = yellipse[0].roots(-80, Y);
    r[2] = yellipse[1].roots(0, Y);
    r[3] = yellipse[1].roots(-80, Y);
    for (unsigned i = 0; i < 4; ++i) {
        for (unsigned j = 0; j < r[i].size(); ++j) {
            std::cout << format_coord_nice(r[i][j]) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << yellipse[0].unitTangentAt(0) << " "
              << yellipse[0].unitTangentAt(1) << " "
              << yellipse[1].unitTangentAt(0) << " "
              << yellipse[1].unitTangentAt(1) << std::endl;

    Path half_ellipse = string_to_path("M 0,0 A 40 20 90 0 0 0,-80 L -20,-40 z");
    EXPECT_EQ(half_ellipse.winding(Point(-1, 0)), 0);
    EXPECT_EQ(half_ellipse.winding(Point(-1, -80)), 0);
    EXPECT_EQ(half_ellipse.winding(Point(1, 0)), 0);
    EXPECT_EQ(half_ellipse.winding(Point(1, -80)), 0);
    EXPECT_EQ(half_ellipse.winding(Point(0, -40)), -1);

    // extra nasty cases with exact double roots
    Path hump = string_to_path("M 0,0 Q 1,1 2,0 L 2,2 0,2 Z");
    EXPECT_EQ(hump.winding(Point(0.25, 0.5)), 1);
    EXPECT_EQ(hump.winding(Point(1.75, 0.5)), 1);

    Path hump2 = string_to_path("M 0,0 L 2,0 2,2 Q 1,1 0,2 Z");
    EXPECT_EQ(hump2.winding(Point(0.25, 1.5)), 1);
    EXPECT_EQ(hump2.winding(Point(1.75, 1.5)), 1);
}

TEST_F(PathTest, SVGRoundtrip) {
    SVGPathWriter sw;

    Path transformed = diederik * (Rotate(1.23456789) * Scale(1e-8) * Translate(1e-9, 1e-9));

    for (unsigned i = 0; i < 4; ++i) {
        sw.setOptimize(i & 1);
        sw.setUseShorthands(i & 2);

        sw.feed(line);
        //cout << sw.str() << endl;
        Path line_svg = string_to_path(sw.str().c_str());
        EXPECT_TRUE(line_svg == line);
        sw.clear();

        sw.feed(square);
        //cout << sw.str() << endl;
        Path square_svg = string_to_path(sw.str().c_str());
        EXPECT_TRUE(square_svg == square);
        sw.clear();

        sw.feed(circle);
        //cout << sw.str() << endl;
        Path circle_svg = string_to_path(sw.str().c_str());
        EXPECT_TRUE(circle_svg == circle);
        sw.clear();

        sw.feed(arcs);
        //cout << sw.str() << endl;
        Path arcs_svg = string_to_path(sw.str().c_str());
        EXPECT_TRUE(arcs_svg == arcs);
        sw.clear();

        sw.feed(diederik);
        //cout << sw.str() << endl;
        Path diederik_svg = string_to_path(sw.str().c_str());
        EXPECT_TRUE(diederik_svg == diederik);
        sw.clear();

        sw.feed(transformed);
        //cout << sw.str() << endl;
        Path transformed_svg = string_to_path(sw.str().c_str());
        EXPECT_TRUE(transformed_svg == transformed);
        sw.clear();

        sw.feed(cmds);
        //cout << sw.str() << endl;
        Path cmds_svg = string_to_path(sw.str().c_str());
        EXPECT_TRUE(cmds_svg == cmds);
        sw.clear();
    }
}

TEST_F(PathTest, Portion) {
    PathTime a(0, 0.5), b(3, 0.5);
    PathTime c(1, 0.25), d(1, 0.75);

    EXPECT_EQ(square.portion(a, b), string_to_path("M 0.5, 0 L 1,0 1,1 0,1 0,0.5"));
    EXPECT_EQ(square.portion(b, a), string_to_path("M 0,0.5 L 0,1 1,1 1,0 0.5,0"));
    EXPECT_EQ(square.portion(a, b, true), string_to_path("M 0.5,0 L 0,0 0,0.5"));
    EXPECT_EQ(square.portion(b, a, true), string_to_path("M 0,0.5 L 0,0 0.5,0"));
    EXPECT_EQ(square.portion(c, d), string_to_path("M 1,0.25 L 1,0.75"));
    EXPECT_EQ(square.portion(d, c), string_to_path("M 1,0.75 L 1,0.25"));
    EXPECT_EQ(square.portion(c, d, true), string_to_path("M 1,0.25 L 1,0 0,0 0,1 1,1 1,0.75"));
    EXPECT_EQ(square.portion(d, c, true), string_to_path("M 1,0.75 L 1,1 0,1 0,0 1,0 1,0.25"));

    // verify that no matter how an endpoint is specified, the result is the same
    PathTime a1(0, 1.0), a2(1, 0.0);
    PathTime b1(2, 1.0), b2(3, 0.0);
    Path result = string_to_path("M 1,0 L 1,1 0,1");
    EXPECT_EQ(square.portion(a1, b1), result);
    EXPECT_EQ(square.portion(a1, b2), result);
    EXPECT_EQ(square.portion(a2, b1), result);
    EXPECT_EQ(square.portion(a2, b2), result);
}

TEST_F(PathTest, AppendSegment) {
    Path p_open = line, p_closed = line;
    p_open.setStitching(true);
    p_open.append(new LineSegment(Point(10,20), Point(10,25)));
    EXPECT_EQ(p_open.size(), 3u);
    EXPECT_NO_THROW(p_open.checkContinuity());
    
    p_closed.setStitching(true);
    p_closed.close(true);
    p_closed.append(new LineSegment(Point(10,20), Point(10,25)));
    EXPECT_EQ(p_closed.size(), 4u);
    EXPECT_NO_THROW(p_closed.checkContinuity());
}

TEST_F(PathTest, AppendPath) {
    p_open.append(p_add);
    Path p_expected = string_to_path("M 0,0 L 0,5 5,5 5,0 -1,6 6,6");
    EXPECT_EQ(p_open.size(), 5u);
    EXPECT_EQ(p_open, p_expected);
    EXPECT_NO_THROW(p_open.checkContinuity());

    p_expected.close(true);
    p_closed.append(p_add);
    EXPECT_EQ(p_closed.size(), 6u);
    EXPECT_EQ(p_closed, p_expected);
    EXPECT_NO_THROW(p_closed.checkContinuity());
}

TEST_F(PathTest, ReplaceMiddle) {
    p_open.replace(p_open.begin() + 1, p_open.begin() + 2, p_add);
    EXPECT_EQ(p_open.size(), 5u);
    EXPECT_NO_THROW(p_open.checkContinuity());
    
    p_closed.replace(p_closed.begin() + 1, p_closed.begin() + 2, p_add);
    EXPECT_EQ(p_closed.size(), 6u);
    EXPECT_NO_THROW(p_closed.checkContinuity());
}

TEST_F(PathTest, ReplaceStart) {
    p_open.replace(p_open.begin(), p_open.begin() + 2, p_add);
    EXPECT_EQ(p_open.size(), 3u);
    EXPECT_NO_THROW(p_open.checkContinuity());
    
    p_closed.replace(p_closed.begin(), p_closed.begin() + 2, p_add);
    EXPECT_EQ(p_closed.size(), 5u);
    EXPECT_NO_THROW(p_closed.checkContinuity());
}

TEST_F(PathTest, ReplaceEnd) {
    p_open.replace(p_open.begin() + 1, p_open.begin() + 3, p_add);
    EXPECT_EQ(p_open.size(), 3u);
    EXPECT_NO_THROW(p_open.checkContinuity());
    
    p_closed.replace(p_closed.begin() + 1, p_closed.begin() + 3, p_add);
    EXPECT_EQ(p_closed.size(), 5u);
    EXPECT_NO_THROW(p_closed.checkContinuity());
}

TEST_F(PathTest, ReplaceClosing) {
    p_open.replace(p_open.begin() + 1, p_open.begin() + 4, p_add);
    EXPECT_EQ(p_open.size(), 3u);
    EXPECT_NO_THROW(p_open.checkContinuity());
    
    p_closed.replace(p_closed.begin() + 1, p_closed.begin() + 4, p_add);
    EXPECT_EQ(p_closed.size(), 4u);
    EXPECT_NO_THROW(p_closed.checkContinuity());
}

TEST_F(PathTest, ReplaceEverything) {
    p_open.replace(p_open.begin(), p_open.end(), p_add);
    EXPECT_EQ(p_open.size(), 1u);
    EXPECT_NO_THROW(p_open.checkContinuity());

    // TODO: in this specific case, it may make sense to set the path to open...
    // Need to investigate what behavior is sensible here
    p_closed.replace(p_closed.begin(), p_closed.end(), p_add);
    EXPECT_EQ(p_closed.size(), 2u);
    EXPECT_NO_THROW(p_closed.checkContinuity());
}

TEST_F(PathTest, EraseLast) {
    p_open.erase_last();
    Path p_expected = string_to_path("M 0,0 L 0,5 5,5");
    EXPECT_EQ(p_open, p_expected);
    EXPECT_NO_THROW(p_open.checkContinuity());
}

TEST_F(PathTest, AreNear) {
    Path nudged_arcs1 = string_to_path("M 0,0 a 5,10 45 0 1 10,10.0000005 a 5,10 45 0 1 0,0 z");
    Path nudged_arcs2 = string_to_path("M 0,0 a 5,10 45 0 1 10,10.00005 a 5,10 45 0 1 0,0 z");
    EXPECT_EQ(are_near(diederik, diederik, 0), true);
    EXPECT_EQ(are_near(cmds, diederik, 1e-6), false);
    EXPECT_EQ(are_near(arcs, nudged_arcs1, 1e-6), true);
    EXPECT_EQ(are_near(arcs, nudged_arcs2, 1e-6), false);
}

TEST_F(PathTest, Roots) {
    Path path;
    path.start(Point(0, 0));
    path.appendNew<Geom::LineSegment>(Point(1, 1));
    path.appendNew<Geom::LineSegment>(Point(2, 0));

    EXPECT_FALSE(path.closed());

    // Trivial case: make sure that path is not closed
    std::vector<PathTime> roots = path.roots(0.5, Geom::X);
    EXPECT_EQ(roots.size(), 1u);
    EXPECT_EQ(path.valueAt(roots[0], Geom::Y), 0.5);

    // Now check that it is closed if we make it so
    path.close(true);
    roots = path.roots(0.5, Geom::X);
    EXPECT_EQ(roots.size(), 2u);
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
