#include "testing.h"
#include <iostream>

#include <2geom/bezier.h>
#include <2geom/bezier-curve.h>

#include <2geom/shape.h>
#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <2geom/svg-path-parser.h>
#include <vector>
#include <iterator>

using namespace std;
using namespace Geom;

bool are_equal(Bezier const &A, Bezier const &B) {
    int maxSize = max(A.size(), B.size());
    double t = 0., dt = 1./maxSize;
    
    for(int i = 0; i <= maxSize; i++) {
        EXPECT_FLOAT_EQ(A.valueAt(t), B.valueAt(t));// return false;
        t += dt;
    }
    return true;
}

class Chain {
public:
  Path p;
  
  int lowerBound;
  double lowerBoundY;
  /** computes the bounds of all curves in the range[s, e]
   */
  OptRect bounds(int s, int e) {
    if(s >= e) return OptRect();
    Rect out = p[s].boundsFast();
    for(int i = s+1; i < e; i++) {
      out.unionWith(p[i].boundsFast());
    }
    return out;
  }
  
  void trim(double y) {
    unsigned i = lowerBound;
    for(; i < p.size(); i++) {
      Rect bounds = p[i].boundsFast();
      if(bounds[Geom::Y].max() < y) {
  break;
      }
    }
    lowerBound = i;
  }
};

class ChainSweep {
public:
  vector<Chain> chains;
  double sweepY;
  
  void findSweepY() {
    sweepY = chains[0].p.finalPoint()[Geom::Y];
    for(unsigned i = 0; i < chains.size(); i++) {
      double y = chains[i].p.finalPoint()[Geom::Y];
      if(y < sweepY) {
  sweepY = y;
      }
    }
  }
};

// The fixture for testing class Foo.
class ChainTest : public ::testing::Test {
protected:
  ChainSweep cs;
  
    ChainTest() {
      const char *path_b_svgd="m 307,259 c 0,0 8,8 -3,11 -5,1 -5,9 -5,9 m 4,-17 1,3 m -6,-8 3,12 m -9,-11 1,4 2,3 -1,4 3,3 -2,5 m -9,-19 4,14";

      PathVector pv = parse_svg_path(path_b_svgd);
      EXPECT_EQ(5u, pv.size());
      
      vector<PathVector> segs;
      segs.push_back(parse_svg_path("m 296,280 5,-11"));
      segs.push_back(parse_svg_path("m 309,283 -5,-18"));

      for (unsigned i = 0; i < pv.size(); i++) {
          cs.chains.push_back(Chain());
          Chain &c = cs.chains.back();
          for (unsigned j = 0; j < pv[i].size(); j++) {
              c.p.append(pv[i][j]);
          }
          if (c.p.initialPoint()[Y] > c.p.finalPoint()[Y]) {
              c.p = c.p.reversed();
          }
          cout << c.p.initialPoint() << endl;
          cout << c.p.finalPoint() << endl;
      }
      EXPECT_FLOAT_EQ(0, cs.sweepY);
      cs.findSweepY();
      EXPECT_EQ(265, cs.sweepY);
      cs.chains[1].p.append(segs[0][0].reversed());
      cs.chains[1].p.append(segs[1][0].reversed());
    }
};

TEST_F(ChainTest, UnitTests) {
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
