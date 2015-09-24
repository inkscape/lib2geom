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

from cy2geom import Circle, Ellipse


class TestPrimitives(unittest.TestCase):
    def test_circle(self):
        C = Circle()
        self.assertEqual(C.center(), Point())
        
        D = Circle(Point(2, 4), 2)
        Dp = D.getPath()
        self.assertEqual(D.center(), Point(2, 4))
        self.assertEqual(D.ray(), 2)

        for i in range(11):
            t = i/10.0
            #Circle approximated by SBasis is not perfect
            self.assertAlmostEqual( abs(D.center()-Dp(t)), D.ray(), delta=0.1 )

        half_circle = D.arc(Dp(0), Dp(0.3), Dp(0.5))

        self.assertTrue(half_circle.is_SVG_compliant())

        self.assertAlmostEqual(Dp(0.25), half_circle(0.5), delta=0.1)

        points = [Point(2, 5), Point(1, 4), Point(9, 0)]
        D.set_points(points)
        for p in points:
            self.assertAlmostEqual( abs(p-D.center()), D.ray() )
        Dc = Circle.from_points(points)
        self.assertAlmostEqual(Dc.center(), D.center())
        self.assertAlmostEqual(Dc.ray(), D.ray())
        
        coeffs = (2, 4, 1, -4)
        E = Circle.from_coefficients(*coeffs)
        def param(x, y):
            A, B, C, D = coeffs
            return A*x**2 + A*y**2 + B*x + C*y + D
        Ec = E.arc(E.center()+Point(E.ray(), 0), E.center()-Point(E.ray(), 0), E.center()+Point(E.ray(), 0) )
        for i in range(11):
            t = i/10.0
            self.assertAlmostEqual(param(Ec.value_at(t, 0), Ec.value_at(t, 1)), 0)
        
        E.set(3, 5, 9)
        self.assertAlmostEqual(E.center(), Point(3, 5))
        self.assertAlmostEqual(E.ray(), 9)
        
        E.set_coefficients(*coeffs)
        #radius and center from parametric equation 
        ca = float(coeffs[1])/coeffs[0]
        cb = float(coeffs[2])/coeffs[0]
        cc = float(coeffs[3])/coeffs[0]
        self.assertAlmostEqual( 4*E.ray()**2 , ca**2 + cb**2 -4*cc )
        self.assertAlmostEqual( E.center(), -Point(ca, cb)/2)
    
    def test_ellipse(self):
        #TODO: maybe a bug in arc? get_curve(F) returns different ellipse than F
        def get_curve(ellipse):
            p = Point(ellipse.ray(0), 0)*Rotate(ellipse.rot_angle())
            return ellipse.arc(ellipse.center()+p, ellipse.center()-p, ellipse.center()+p*(1-1e-7))
        E = Ellipse()
        self.assertAlmostEqual(E.center(), Point())
        self.assertAlmostEqual(E.ray(0), 0)
        self.assertAlmostEqual(E.ray(1), 0)
        
        F = Ellipse(Point(), 3, 2, 0)
        self.assertAlmostEqual(F.center(), Point())
        self.assertAlmostEqual(F.ray(0), 3)
        self.assertAlmostEqual(F.ray(1), 2)
        self.assertAlmostEqual(F.rot_angle(), 0)
        # x**2/9 + y**2/4 = 1
        self.assertAlmostEqual(F.implicit_form_coefficients()[0], 1/9.0)
        self.assertAlmostEqual(F.implicit_form_coefficients()[2], 1/4.0)
        self.assertAlmostEqual(F.implicit_form_coefficients()[5], -1)
        
        coeffs = (1/3.0, 0, 1/16.0, 1, 0, -1/4.0)
        G = Ellipse.from_coefficients(*coeffs)
        self.assertAlmostEqual(G.center(), Point(-3/2.0, 0))
        self.assertAlmostEqual(G.ray(0), math.sqrt(3))
        self.assertAlmostEqual(G.ray(1), 4)
        self.assertAlmostEqual(G.rot_angle(), 0)
        
        points = [Point(1, 2), Point(2 ,9), Point(0, 3), Point(-3, 8), Point(5, 8)]
        G.set_points(points)
        coeffs_G = tuple(G.implicit_form_coefficients())
        def paramG(x, y):
            A, B, C, D, E, F = coeffs_G
            return A*x**2 + B*x*y + C*y**2 + D*x + E*y + F
        for p in points:
            self.assertAlmostEqual(paramG(p.x, p.y), 0)

        G2 = Ellipse.from_points(points)
        coeffs_G2 = tuple(G.implicit_form_coefficients())
        def paramG2(x, y):
            A, B, C, D, E, F = coeffs_G2
            return A*x**2 + B*x*y + C*y**2 + D*x + E*y + F
        for p in points:
            self.assertAlmostEqual(paramG2(p.x, p.y), 0)
        
        E.set_coefficients(*coeffs_G2)
        for a1, a2 in zip(E.implicit_form_coefficients(), G2.implicit_form_coefficients()):
            self.assertAlmostEqual(a1, a2)

        H = Ellipse.from_circle(Circle(Point(2, 8), 5))
        self.assertAlmostEqual(H.center(), Point(2, 8))
        self.assertAlmostEqual(H.ray(0), 5)
        self.assertAlmostEqual(H.ray(1), 5)
        
        Ft = F.transformed( Rotate(math.pi/2) )
        self.assertAlmostEqual(F.ray(0), Ft.ray(1))
        self.assertAlmostEqual(F.ray(1), Ft.ray(0))
        
unittest.main()
