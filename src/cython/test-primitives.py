import unittest
from math import pi, sqrt

import cy2geom
from cy2geom import Angle, AngleInterval, Point, IntPoint, Line, Ray
from cy2geom import LineSegment, Curve

class TestPrimitives(unittest.TestCase):

    def test_angle(self):
        self.assertAlmostEqual(Angle.rad_from_deg(45), pi/4)
        self.assertAlmostEqual(Angle.deg_from_rad(pi/6), 30)

        p = Point(1, sqrt(3))
        alpha = Angle.from_Point(p)
        self.assertAlmostEqual(alpha.degrees(), 60)

        beta = Angle.from_radians(pi/5)
        gamma = Angle.from_degrees(36)
        self.assertAlmostEqual(beta.radians0(), gamma.radians0())
        self.assertTrue(beta==gamma)
        omega = Angle.from_degrees_clock(0)
        self.assertAlmostEqual(omega.radians(), pi/2)

        delta = Angle(-pi * 0.5)
        self.assertAlmostEqual(delta.degrees(), -90)
        self.assertAlmostEqual(delta.radians0(), 1.5*pi)
        #degreesClock roughly means [ 90 - Angle.degrees() ] mod 360
        self.assertAlmostEqual(delta.degrees_clock(), 180)

        self.assertAlmostEqual(
            (beta + gamma).radians(),
            beta.radians()+gamma.radians() )
        self.assertAlmostEqual( (beta - gamma).degrees(), 0)

    def test_angleInterval(self):
        A = AngleInterval(Angle(pi/6), Angle(pi/4))
        B = AngleInterval( 0, pi/4, cw = True )
        self.assertEqual(A(0), Angle(pi/6))
        self.assertEqual(A(0.5), A.angle_at(0.5))
        self.assertEqual(A(0), A.initial_angle())
        self.assertEqual(B(1), B.final_angle())
        self.assertFalse(B.is_degenerate())
        self.assertTrue(B.contains(Angle(pi/6)))
        self.assertTrue(A.contains(Angle(pi)))

        self.assertAlmostEqual( B.extent(), pi/4 )
    def test_point(self):
        p = Point(3, 4)
        q = Point(8, 16)
        p_inf = Point(float('inf'), 1)
        #y axis points downwards
        p_ccw = Point(4, -3)

        self.assertAlmostEqual(p.length(), 5)
        self.assertAlmostEqual(p.ccw(), p_ccw)
        self.assertAlmostEqual(p_ccw.cw(), p)
        self.assertAlmostEqual(p[0], 3)
        self.assertAlmostEqual(p[1], 4)

        self.assertFalse(p_inf.isFinite())
        self.assertTrue(p.isFinite())
        self.assertFalse(p.isNormalized())
        self.assertTrue((p/p.length()).isNormalized())
        self.assertFalse(p.isZero())
        self.assertTrue((p*0).isZero())

        self.assertTrue( (p + p.ccw().ccw()).isZero)
        self.assertAlmostEqual(  (q-p).length(), 13)

        self.assertGreater(q, p)
        self.assertGreaterEqual(q, p)
        self.assertEqual(p, p)
        self.assertNotEqual(p, q)
        self.assertLess( Point(1, 1), Point(1, 2) )
        self.assertLessEqual(p, p)

        self.assertTrue( Point.are_near(
            Point.polar(pi/4, sqrt(2)),
            Point(1, 1) ))
        self.assertAlmostEqual(sqrt(2), Point.L2(Point(1, 1)))
        self.assertAlmostEqual(2, Point.L2sq(Point(1, 1)))
        self.assertAlmostEqual( Point.middle_point(Point(), q), q/2 )
        self.assertAlmostEqual( Point.rot90(p), p.cw() )
        self.assertAlmostEqual(
            Point.lerp(0.2, Point(), Point(3,4)).length(),
            1)
        self.assertAlmostEqual(Point.dot(p, p_ccw), 0)
        self.assertAlmostEqual(Point.dot(p, p_inf), float('inf'))
        self.assertAlmostEqual(Point.dot(p, q), 88)
        #TODO this might be implemented incorrectly in lib2geom!
        self.assertAlmostEqual(Point.cross(p, q), -16)

        self.assertAlmostEqual(Point.distance(p, q), 13)
        self.assertAlmostEqual(Point.distanceSq(p, p_ccw), 50)
        self.assertAlmostEqual(Point.unit_vector(p), p/5)

        self.assertAlmostEqual(Point.L1(p), 7)
        self.assertAlmostEqual(Point.L1(p_inf), float('inf'))
        self.assertAlmostEqual(Point.LInfty(q), 16)
        self.assertAlmostEqual(Point.LInfty(p_inf), float('inf'))

        self.assertTrue(Point.is_zero(Point()))
        self.assertFalse(Point.is_zero(p))
        self.assertTrue(Point.is_unit_vector(p/5))
        self.assertFalse(Point.is_unit_vector(q))

        self.assertAlmostEqual(Point.atan2(Point(1, 1)), pi/4)
        self.assertAlmostEqual(Point.angle_between(p, p_ccw), -pi/2)
        self.assertAlmostEqual(Point.abs(-p), p)
    #TODO I have no idea what should this function do
    #    self.assertAlmostEqual(
    #        Point.constrain_angle(Point(1, 0), Point(0, 1), 1, Point(sqrt(2)/2, sqrt(2)/2)),
    #
    #   ))

    def test_intPoint(self):
        p = Point(4.89, 3.21)
        self.assertEqual(p.round(), IntPoint(5, 3))
        self.assertEqual(p.floor(), IntPoint(4, 3))
        self.assertEqual(p.ceil(), IntPoint(5, 4))

        self.assertEqual(p.ceil().x, 5)
        self.assertEqual(p.floor().y, 3)
        self.assertEqual(IntPoint(), p.floor()-p.floor())

        a = IntPoint(2, -5)
        b = IntPoint(5, 3)
        self.assertEqual(IntPoint(7, -2), a+b)
        self.assertEqual(IntPoint(3, 8), b-a)
        self.assertGreater(b, a)
        self.assertGreaterEqual(b, b)
        self.assertNotEqual(a, b)

    def test_line(self):

        l = Line(Point(), pi/4)
        self.assertAlmostEqual( l.origin(), Point() )
        self.assertAlmostEqual( l.versor(), Point(1, 1)/sqrt(2) )
        self.assertAlmostEqual( l.angle(), pi/4 )

        k = Line.from_points(Point(), Point(2, 1))
        self.assertFalse(k.is_degenerate())
        self.assertFalse(Line().is_degenerate())
        self.assertAlmostEqual( l.point_at(sqrt(2)), Point(1,1) )
        self.assertAlmostEqual(
            k.point_at(43),
            Point(k.value_at(43, 0), k.value_at(43, 1)))
        self.assertAlmostEqual(k.time_at(Point(4, 2)), sqrt(20))
        self.assertAlmostEqual(
            k.time_at_projection(Point(4, 2) + Point(2, -4)),
            sqrt(20))
        self.assertAlmostEqual(
            k.point_at(k.nearest_time(Point(4, 2) + Point(2, -4))),
            Point(4,2))
        self.assertAlmostEqual(
            k.time_at_projection(Point(3, 3)),
            -k.reverse().time_at_projection(Point(3, 3)))
        self.assertAlmostEqual( k.derivative().origin(), k.versor())
        self.assertAlmostEqual(k.normal(), k.versor().cw())

        roots = k.roots( 3, 0 )
        for root in roots:
            self.assertAlmostEqual( k.value_at(root, 0), 3)

        self.assertAlmostEqual(l.normal(), l.normal_and_dist()[0])
        self.assertAlmostEqual(Line.distance(Point(), l), l.normal_and_dist()[1])

        self.assertAlmostEqual(Line.distance(Point(-1, 1), l), sqrt(2))
        self.assertTrue(Line.are_near(Point(0), l))
        self.assertFalse(Line.are_near(Point(1, 1), k))
        self.assertTrue(Line.are_near(Point(1, 1), k, 2))

        p = Line(Point(1, 1))
        p_orto = Line(Point(2, 3), pi/2)
        p_para = Line(Point(2, 3))
        p_same = Line.from_points(Point(1, 1), Point(5, 1))

        self.assertTrue(Line.are_orthogonal(p, p_orto))
        self.assertFalse(Line.are_orthogonal(p, p_para))
        self.assertTrue(Line.are_parallel(p, p_para))
        self.assertFalse(Line.are_parallel(p, p_orto))
        self.assertTrue(Line.are_same(p, p_same))
        self.assertFalse(Line.are_same(p, p_para))

        self.assertTrue(Line.are_collinear(
            Point(1,1),
            Point(2, 3),
            Point(4, 7)))
        self.assertAlmostEqual(Line.angle_between(p, p_orto), pi/2)

        m = Line.from_normal_distance(Point(1, -1), 1)
        self.assertAlmostEqual(m.angle(), pi/4)

        m = Line.from_LineSegment( LineSegment( Point(2, 2), Point(4, 4) ) )
        self.assertAlmostEqual(m.angle(), pi/4)

        m = Line.from_Ray( Ray(Point(2, 3), 0.2) )
        self.assertAlmostEqual(m.angle(), 0.2)
        self.assertAlmostEqual(m.origin(), Point(2, 3))

        self.assertIsInstance(m.portion(2, 4), Curve)
        self.assertAlmostEqual(m.portion(2, 4)(0), m.point_at(2))

        self.assertIsInstance(m.segment(1, 5), LineSegment)
        self.assertAlmostEqual(m.segment(1, 5)(1), m.point_at(5))

        self.assertAlmostEqual(m.ray(4).origin(), m.point_at(4))

        m.set_origin(Point())
        self.assertAlmostEqual(m.origin(), Point())

        m.set_angle(0.2)
        self.assertAlmostEqual(m.angle(), 0.2)

        m.set_versor(Point())
        self.assertTrue(m.is_degenerate())

        m.set_points(Point(2, 9), Point(1, 8))
        self.assertAlmostEqual(m.versor(), Point.unit_vector(Point(1, 8) - Point(2, 9)))

    def test_ray(self):
        r = Ray(Point(1,1), pi/4)
        self.assertAlmostEqual(r.origin(), Point(1, 1))
        self.assertAlmostEqual(r.versor(), Point(1, 1)/sqrt(2))
        self.assertAlmostEqual(r.angle(), pi/4)

        r.set_origin(Point(4, 3))
        #TODO this should maybe normalize the versor!
        r.set_versor(Point(1, -1)/sqrt(2))
        self.assertAlmostEqual(r.origin(), Point(4, 3))
        self.assertAlmostEqual(r.versor(), Point(1, -1)/sqrt(2))
        self.assertAlmostEqual(r.angle(), -pi/4)

        r.set_points(Point(1, 1), Point(1, 3))
        self.assertFalse(r.is_degenerate())
        self.assertFalse(Ray().is_degenerate())
        self.assertAlmostEqual(r.point_at(4), Point(1, 5))

        #TODO I think this should be expected behaviour
#        self.assertAlmostEqual(
#            r.pointAt(-3),
#            Point(1, 1)))
        self.assertAlmostEqual(r.value_at(4, 0), 1)
        self.assertAlmostEqual(r.value_at(4, 1), 5)

        roots = r.roots( 3, 1 )
        for root in roots:
            self.assertAlmostEqual( r.value_at(root, 1), 3)

        self.assertAlmostEqual(
            r.point_at(3) - r.origin(),
            r.origin()-r.reverse().point_at(3))

        self.assertAlmostEqual(Ray.distance(Point(), r), sqrt(2))
        self.assertAlmostEqual(Ray.distance(Point()+r.versor(), r), 1)

        self.assertTrue(Ray.are_near(Point(), r, 2))
        self.assertFalse(Ray.are_near(Point(), r))
        self.assertTrue(Ray.are_same(r, r))

        q = Ray(r.origin(), r.angle())
        self.assertTrue(Ray.are_same(r, q))

        q.set_origin(r.origin()+Point(0, 1))
        self.assertFalse(Ray.are_same(r, q))
        #TODO shouldn't this really be 0?
        self.assertAlmostEqual(Ray.angle_between(r, q), 2*pi)

        q.set_versor(Point(1, 0))
        q.set_origin(r.origin())
        self.assertAlmostEqual(
            Point(1, 1)/sqrt(2),
            Ray.make_angle_bisector_ray(q, r).versor())

        q.set_angle(pi/7)
        self.assertAlmostEqual(q.angle(), pi/7)

        self.assertIsInstance(q.portion(2, 4), Curve)
        self.assertAlmostEqual(q.portion(2, 4)(0), q.point_at(2))

        self.assertIsInstance(q.segment(1, 5), LineSegment)

        self.assertAlmostEqual(q.segment(1, 5)(1), q.point_at(5))


unittest.main()

