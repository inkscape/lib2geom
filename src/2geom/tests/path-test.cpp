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
        diederik = string_to_path("m 262.6037,35.824151 c 0,0 -92.64892,-187.405851 30,-149.999981 104.06976,31.739531 170,109.9999815 170,109.9999815 l -10,-59.9999905 c 0,0 40,79.99999 -40,79.99999 -80,0 -70,-129.999981 -70,-129.999981 l 50,0 C 435.13571,-131.5667 652.76275,126.44872 505.74322,108.05672 358.73876,89.666591 292.6037,-14.175849 292.6037,15.824151 c 0,30 -30,20 -30,20 z");
        cmds = string_to_path("M 0,0 V 100 H 100 Q 100,0 0,0 L 200,0 C 200,100 300,100 300,0 S 200,-100 200,0");
    }

    // Objects declared here can be used by all tests in the test case for Foo.
    Path line, square, circle, diederik, cmds;
};

TEST_F(PathTest, PathInterval) {
    PathPosition n2_before(1, 0.9995), n2_after(2, 0.0005),
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

    PathPosition n1(1, 0.0), n1x(0, 1.0),
                 n2(2, 0.0), n2x(1, 1.0),
                 n3(3, 0.0), n3x(2, 1.0);
    PathPosition tests[8] = { n1, n1x, n2, n2x, n3, n3x, mid2, mid3 };

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

    for (unsigned i = 0; i < 5; ++i) {
        for (unsigned j = 0; j < 8; ++j) {
            EXPECT_EQ(ival[i][0].contains(tests[j]), bool(includes[i][j] & NORMAL));
            EXPECT_EQ(ival[i][1].contains(tests[j]), bool(includes[i][j] & NORMAL));
            EXPECT_EQ(ival[i][2].contains(tests[j]), bool(includes[i][j] & CROSS));
            EXPECT_EQ(ival[i][3].contains(tests[j]), bool(includes[i][j] & CROSS));
        }
    }
}

TEST_F(PathTest, Continuity) {
    line.checkContinuity();
    square.checkContinuity();
    circle.checkContinuity();
    diederik.checkContinuity();
    cmds.checkContinuity();
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
    EXPECT_EQ(0, line.nearestTime(Point(0,0)));
    EXPECT_EQ(0.5, line.nearestTime(Point(0.5,0)));
    EXPECT_EQ(0.5, line.nearestTime(Point(0.5,1)));
    EXPECT_EQ(1, line.nearestTime(Point(100,0)));
    EXPECT_EQ(0, line.nearestTime(Point(-100,1000)));

    EXPECT_EQ(0, square.nearestTime(Point(0,0)));
    EXPECT_EQ(1, square.nearestTime(Point(1,0)));
    EXPECT_EQ(3, square.nearestTime(Point(0,1)));
    
    //cout << diederik.nearestTime(Point(247.32293,-43.339507)) << endl;

    EXPECT_FLOAT_EQ(6.5814033, diederik.nearestTime(Point(511.75,40.85)));
    //cout << diederik.pointAt(diederik.nearestTime(Point(511.75,40.85))) << endl;

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
    PathPosition a(0, 0.5), b(3, 0.5);
    PathPosition c(1, 0.25), d(1, 0.75);

    EXPECT_EQ(square.portion(a, b), string_to_path("M 0.5, 0 L 1,0 1,1 0,1 0,0.5"));
    EXPECT_EQ(square.portion(b, a), string_to_path("M 0,0.5 L 0,1 1,1 1,0 0.5,0"));
    EXPECT_EQ(square.portion(a, b, true), string_to_path("M 0.5,0 L 0,0 0,0.5"));
    EXPECT_EQ(square.portion(b, a, true), string_to_path("M 0,0.5 L 0,0 0.5,0"));
    EXPECT_EQ(square.portion(c, d), string_to_path("M 1,0.25 L 1,0.75"));
    EXPECT_EQ(square.portion(d, c), string_to_path("M 1,0.75 L 1,0.25"));
    EXPECT_EQ(square.portion(c, d, true), string_to_path("M 1,0.25 L 1,0 0,0 0,1 1,1 1,0.75"));
    EXPECT_EQ(square.portion(d, c, true), string_to_path("M 1,0.75 L 1,1 0,1 0,0 1,0 1,0.25"));

    // verify that no matter how an endpoint is specified, the result is the same
    PathPosition a1(0, 1.0), a2(1, 0.0);
    PathPosition b1(2, 1.0), b2(3, 0.0);
    Path result = string_to_path("M 1,0 L 1,1 0,1");
    EXPECT_EQ(square.portion(a1, b1), result);
    EXPECT_EQ(square.portion(a1, b2), result);
    EXPECT_EQ(square.portion(a2, b1), result);
    EXPECT_EQ(square.portion(a2, b2), result);
}

    /*TEST_F(PathTest,Operators) {
    cout << "scalar operators\n";
    cout << hump + 3 << endl;
    cout << hump - 3 << endl;
    cout << hump*3 << endl;
    cout << hump/3 << endl;

    Bezier reverse_wiggle = reverse(wiggle);
    EXPECT_EQ(reverse_wiggle[0], wiggle[wiggle.size()-1]);
    EXPECT_TRUE(are_equal(reverse(reverse_wiggle), wiggle));

    cout << "Bezier portion(const Bezier & a, double from, double to);\n";
    cout << portion(Bezier(0.0,2.0), 0.5, 1) << endl;

// std::vector<Point> bezier_points(const D2<Bezier > & a) {

    cout << "Bezier derivative(const Bezier & a);\n";
    std::cout << derivative(hump) <<std::endl;
    std::cout << integral(hump) <<std::endl;

    EXPECT_TRUE(are_equal(derivative(integral(wiggle)), wiggle));
    std::cout << derivative(integral(hump)) <<std::endl;
    expect_array((const double []){0.5}, derivative(hump).roots());

    EXPECT_TRUE(bounds_fast(hump)->contains(Interval(0,hump.valueAt(0.5))));

    EXPECT_EQ(Interval(0,hump.valueAt(0.5)), *bounds_exact(hump));

    Interval tight_local_bounds(min(hump.valueAt(0.3),hump.valueAt(0.6)),
             hump.valueAt(0.5));
    EXPECT_TRUE(bounds_local(hump, Interval(0.3, 0.6))->contains(tight_local_bounds));

    Bezier Bs[] = {unit, hump, wiggle};
    for(unsigned i = 0; i < sizeof(Bs)/sizeof(Bezier); i++) {
        Bezier B = Bs[i];
        Bezier product = multiply(B, B);
        for(int i = 0; i <= 16; i++) {
            double t = i/16.0;
            double b = B.valueAt(t);
            EXPECT_FLOAT_EQ(b*b, product.valueAt(t));
        }
    }
    }*/

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
