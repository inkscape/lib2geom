import unittest
import math
from random import randint, uniform

import cy2geom

from cy2geom import Point, IntPoint
from cy2geom import Interval, IntInterval, OptInterval, OptIntInterval

from cy2geom import Affine
from cy2geom import Translate, Scale, Rotate, VShear, HShear, Zoom
from cy2geom import Eigen

from cy2geom import Curve
from cy2geom import Linear
from cy2geom import SBasis, SBasisCurve
from cy2geom import Bezier, BezierCurve

from cy2geom import LineSegment, QuadraticBezier, CubicBezier
from cy2geom import HLineSegment, VLineSegment

from cy2geom import EllipticalArc

from cy2geom import Path

#TODO! move drawing elsewhere, it nice to see paths, but is not very suitable for automatic testing
draw = False

try:
    import utils
except ImportError:
    print "No drawing with Tk"
    draw = False

class TestPrimitives(unittest.TestCase):
    def curves_equal(self, C1, C2):
        for i in range(101):
            t = i/100.0
            self.assertAlmostEqual(C1(t), C2(t))
    def path(self, P):
        for curve in P:
            self.assertIsInstance(curve, Curve)

        self.assertAlmostEqual(P(0), P.front()(0))
        self.curves_equal(P.front(), P[0])

        self.curves_equal(P.back_default(), P[P.size_default()-1])
        self.curves_equal(P.back_open(), P.back())
        self.assertEqual(P.size_open(), P.size())
        
        self.assertFalse(P.empty() ^ (P.size()==0))

        exact = P.bounds_exact().Rect
        exact.expand_by(1e-5)
        
        fast = P.bounds_fast().Rect
        fast.expand_by(1e-5)
        A1 = Affine(3, 1, 8, 3, 9, 9)
        A2 = Rotate(0.231)

        for i in range(100 * P.size_open() + 1):
            t = i/100.0
            self.assertTrue(exact.contains(P(t)))
            self.assertTrue(fast.contains(P(t)))
            self.assertAlmostEqual( (P*A1)(t) , P(t)*A1 )
            self.assertAlmostEqual( (P*A2)(t) , P(t)*A2 )
            
            self.assertAlmostEqual(P(t), P.point_at(t))
            self.assertAlmostEqual(P(t).x, P.value_at(t, 0))
            self.assertAlmostEqual(P(t).y, P.value_at(t, 1))

        if P.closed():
            self.curves_equal(P.back_default(), P.back_closed())
            self.assertEqual(P.size_default(), P.size_closed())
        else:
            self.curves_equal(P.back_default(), P.back_open())
            self.assertEqual(P.size_default(), P.size_open())

        for i in range(10):
            for root in P.roots(i, 0):
                if root < P.size_default():
                    self.assertAlmostEqual(P.value_at(root, 0), i)
            for root in P.roots(i, 1):
                if root < P.size_default():
                    self.assertAlmostEqual(P.value_at(root, 1), i)
        
        for t in P.all_nearest_times(P(0)):
            self.assertAlmostEqual(P(t), P(0))
        self.assertAlmostEqual(min(P.all_nearest_times( P(0) )), 0)
        self.assertAlmostEqual(P.nearest_time(P(0), 0, 0.2), 0)
        self.assertEqual( len(P.nearest_time_per_curve(Point())), P.size_default() )
        
        t, distSq = P.nearest_time_and_dist_sq(Point(-1, -1), 0, P.size())
        self.assertAlmostEqual(distSq**0.5, abs(P(t)-Point(-1, -1)) )
        
        self.assertAlmostEqual(P.portion(0.3, 0.4)(0), P(0.3))
        self.assertAlmostEqual( P.portion( interval=Interval(P.size(), P.size() * 2) / 3 )(0), 
                                P(P.size()/3.0))
        
        self.assertAlmostEqual(P(0.23), P.reverse()(P.size()-0.23))
        
        self.assertAlmostEqual(P.initial_point(), P(0))
        self.assertAlmostEqual(P.final_point(), P(P.size()))
    def test_path(self):
        a = Path()
        a.append_curve( CubicBezier( Point(-7, -3), Point(2, 8), Point(2, 1), Point(-2, 0) ) )

        self.assertEqual(a.size(), 1)
        self.assertFalse(a.closed())
        self.path(a)

        a.close(True)
        self.assertTrue(a.closed())
        self.path(a)

        a.close(False)
        a.append_curve( LineSegment(a.final_point(), Point(3, 5)) )
        self.assertEqual(a.size(), 2)
        self.path(a)
        
        a.append_SBasis( SBasis(3, 6)*SBasis(1, 0), SBasis(5, 2))
        self.path(a)
        
        a.append_curve(EllipticalArc(Point(), 1, 2, math.pi/6, True, True, Point(1, 1)), Path.STITCH_DISCONTINUOUS)
        #Stitching adds new segment
        self.assertEqual(a.size(), 5)
        
        b = Path()
        for c in a:
            b.append_curve(c)
            
        #TODO: This fails with STITCH_DISCONTINUOUS, but also does so in C++, so
        #it's either correct behaviour or bug in 2geom
        #~ self.path(b)
        
        b.insert(2, LineSegment(b[2-1](1), b[2](0))) #, Path.STITCH_DISCONTINUOUS)
        self.curves_equal(LineSegment(b[2-1](1), b[2](0)), b[2])
        #TODO! fails on root finding
        #self.path(b)
        
        b.set_initial(a[2](1))
        b.set_final(a[3](0))
        
        a.insert_slice(3, b, 0, b.size())
        self.assertEqual(a.size(), b.size()*2-1)
        
        for i in range(b.size()):
            self.curves_equal(a[3+i], b[i])
            
        #Looks like bug:
#        A = Path() 
#        A.append_curve( CubicBezier( Point(-7, -3), Point(2, 8), Point(2, 1), Point(-2, 0) ) )
#        A.append_curve(EllipticalArc(Point(), 1, 2, math.pi/6, True, True, Point(1, 1)), Path.STITCH_DISCONTINUOUS)
#        print A.roots(0, 1)

        #Roots are [1.0, 2.768305708350847, 3.25], Point at second root is
        #Point (2.32, -0.48) 
        #and third root is > 3 - it corresponds to root on closing segment, but A is open,
        #and computing A(3.25) results in RangeError - this might be bug or feature.
        
        self.path(a.portion(0.232, 3.12))
        self.path(a.portion( interval=Interval(0.1, 4.7) ))
        self.path(a.portion(0.232, 3.12).reverse())
        
        b.clear()
        self.assertTrue(b.empty())

        aa = Path()
        for c in a:
            aa.append_curve(c)
        
        a.erase(0)
        self.assertEqual(a.size(), aa.size() - 1)
        self.assertAlmostEqual(a(0), aa(1))
        
        a.erase_last()
        self.assertEqual(a.size(), aa.size() - 2)
        self.assertAlmostEqual(a.final_point(), aa[aa.size()-2](1))
        
        a.replace(3, QuadraticBezier(a(3), Point(), a(4)))
        self.assertEqual(a.size(), aa.size() - 2)
        
        cs = [LineSegment(Point(-0.5, 0), Point(0.5, 0)).transformed( Rotate(-math.pi/3 * i)*Translate(Point(0, math.sqrt(3)/2)*Rotate(-math.pi/3 * i)) ) for i in range(6)]
        
        hexagon = Path.fromList(cs, stitching = Path.STITCH_DISCONTINUOUS, closed = True)
                
        if draw:
            utils.draw(hexagon, scale = 100)
        
        #to = 5 because each corner contains one stitching segment
        half_hexagon = Path.fromPath(hexagon, fr = 0, to = 5)
        if draw:
            utils.draw(half_hexagon, scale = 100)
        
        half_hexagon.replace_slice(1, 5, LineSegment(half_hexagon(1), half_hexagon(5)))
        self.assertEqual(half_hexagon.size(), 2)
        self.assertAlmostEqual(half_hexagon(1.5), Point(0.5, 0))

        half_hexagon.stitch_to(half_hexagon(0))
        self.assertAlmostEqual(half_hexagon(2.5), Point())

        a.start(Point(2, 2))
        a.append_SBasis( SBasis(2, 6), SBasis(1, 5)*SBasis(2, 9) )
        self.assertAlmostEqual(a(1), Point(6, 5*9))
        
        l = Path.fromList([QuadraticBezier(Point(6, 5*9), Point(1, 2), Point(-2, .21))])
        a.append_path(l)
        self.assertAlmostEqual(a.final_point(), l.final_point())
        
        k = Path.fromList([QuadraticBezier(Point(), Point(2, 1), Point(-2, .21)).reverse()])
        k.append_portion_to(l, 0, 0.3)
        self.assertAlmostEqual(l.final_point(), k(0.3))
    
    def test_read_svgd(self):
        p = Path.read_svgd("../toys/spiral.svgd")
        if draw:
            utils.draw(p[0], scale=0.4)
unittest.main()
