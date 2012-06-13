import unittest
from math import pi, sqrt

import cy2geom
from cy2geom import Angle, Point, Line, Ray, GenericInterval

class TestPrimitives(unittest.TestCase):
    def test_angle(self):
        self.assertAlmostEqual(Angle.deg_to_rad(45), pi/4)
        self.assertAlmostEqual(Angle.rad_to_deg(pi/6), 30)
        self.assertAlmostEqual(
            Angle.map_circular_arc_on_unit_interval(pi/6, 0, pi/2),
            1/3.)
        self.assertAlmostEqual(
            Angle.map_unit_interval_on_circular_arc(0.4, 0, pi),
            2*pi/5)
        self.assertTrue(Angle.arc_contains(pi/3, pi/4, pi/2, pi))
        self.assertFalse(Angle.arc_contains(pi/6, pi/4, pi/2, pi))

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
        self.assertAlmostEqual(delta.degreesClock(), 180)

        self.assertAlmostEqual(
            (beta + gamma).radians(),
            beta.radians()+gamma.radians() )
        self.assertAlmostEqual( (beta - gamma).degrees(), 0)

    def test_point(self):
        p = Point(3, 4)
        q = Point(8, 16)
        p_inf = Point(float('inf'), 1)
        #y axis points downwards
        p_ccw = Point(4, -3)

        self.assertAlmostEqual(p.length(), 5)
        self.assertTrue(Point.are_near(p.ccw(), p_ccw))
        self.assertTrue(Point.are_near(p_ccw.cw(), p))
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
        self.assertTrue(Point.are_near( Point.middle_point(Point(), q), q/2 ))
        self.assertTrue(Point.are_near( Point.rot90(p), p.cw() ))
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
        self.assertTrue(Point.are_near(Point.unit_vector(p), p/5))
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
        self.assertTrue(Point.are_near(Point.abs(-p), p))
    #TODO I have no idea what should this funcion do
    #    self.assertTrue(Point.are_near(
    #        Point.constrain_angle(Point(1, 0), Point(0, 1), 1, Point(sqrt(2)/2, sqrt(2)/2)),
    #
    #   ))

    def test_line(self):
        l = Line(Point(), pi/4)
        self.assertTrue(Point.are_near(
            l.origin(),
            Point()))
        self.assertTrue(Point.are_near(
            l.versor(),
            Point(1, 1)/sqrt(2)))
        self.assertAlmostEqual(l.angle(), pi/4)
        k = Line.fromPoints(Point(), Point(2, 1))
        self.assertFalse(k.isDegenerate())
        self.assertFalse(Line().isDegenerate())
        self.assertTrue(Point.are_near(
            l.pointAt(sqrt(2)),
            Point(1,1)))
        self.assertTrue(Point.are_near(
            k.pointAt(43),
            Point(k.valueAt(43, 0), k.valueAt(43, 1))))
        self.assertAlmostEqual(k.timeAt(Point(4, 2)), sqrt(20))
        self.assertAlmostEqual(
            k.timeAtProjection(Point(4, 2) + Point(2, -4)),
            sqrt(20))
        self.assertTrue(Point.are_near(
            k.pointAt(k.nearestPoint(Point(4, 2) + Point(2, -4))),
            Point(4,2)))
        self.assertAlmostEqual(
            k.timeAtProjection(Point(3, 3)),
            -k.reverse().timeAtProjection(Point(3, 3)))
        self.assertTrue(Point.are_near(
            k.derivative().origin(),
            k.versor()))
        self.assertTrue(Point.are_near(k.normal(), k.versor().cw()))
        #normalAndDist does something strange
        self.assertAlmostEqual(Line.distance(Point(-1, 1), l), sqrt(2))
        self.assertTrue(Line.are_near(Point(0), l))
        self.assertFalse(Line.are_near(Point(1, 1), k))
        self.assertTrue(Line.are_near(Point(1, 1), k, 2))
        p = Line(Point(1, 1))
        p_orto = Line(Point(2, 3), pi/2)
        p_para = Line(Point(2, 3))
        p_same = Line.fromPoints(Point(1, 1), Point(5, 1))
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

    def test_ray(self):
        r = Ray(Point(1,1), pi/4)
        self.assertTrue(Point.are_near(
            r.origin(),
            Point(1, 1)))
        self.assertTrue(Point.are_near(
            r.versor(),
            Point(1, 1)/sqrt(2)))
        self.assertAlmostEqual(r.angle(), pi/4)
        r.setOrigin(Point(4, 3))
        #TODO this should maybe normalize the versor!
        r.setVersor(Point(1, -1)/sqrt(2))
        self.assertTrue(Point.are_near(
            r.origin(),
            Point(4, 3)))
        self.assertTrue(Point.are_near(
            r.versor(),
            Point(1, -1)/sqrt(2)))
        self.assertAlmostEqual(r.angle(), -pi/4)
        r.setPoints(Point(1, 1), Point(1, 3))
        self.assertFalse(r.isDegenerate())
        self.assertFalse(Ray().isDegenerate())
        self.assertTrue(Point.are_near(
            r.pointAt(4),
            Point(1, 5)))
        #TODO I think this should be expected behaviour
#        self.assertTrue(Point.are_near(
#            r.pointAt(-3),
#            Point(1, 1)))
        self.assertAlmostEqual(r.valueAt(4, 0), 1)
        self.assertAlmostEqual(r.valueAt(4, 1), 5)
        self.assertTrue(Point.are_near(
            r.pointAt(3) - r.origin(),
            r.origin()-r.reverse().pointAt(3)))
        self.assertAlmostEqual(Ray.distance(Point(), r), sqrt(2))
        self.assertAlmostEqual(Ray.distance(Point()+r.versor(), r), 1)
        self.assertTrue(Ray.are_near(Point(), r, 2))
        self.assertFalse(Ray.are_near(Point(), r))
        self.assertTrue(Ray.are_same(r, r))
        q = Ray(r.origin(), r.angle())
        self.assertTrue(Ray.are_same(r, q))
        q.setOrigin(r.origin()+Point(0, 1))
        self.assertFalse(Ray.are_same(r, q))
        #TODO shouldn't this really be 0?
        self.assertAlmostEqual(Ray.angle_between(r, q), 2*pi)
        q.setVersor(Point(1, 0))
        q.setOrigin(r.origin())
        self.assertTrue(Point.are_near(
            Point(1, 1)/sqrt(2),
            Ray.make_angle_bisector_ray(q, r).versor()))

    def test_genericInterval(self):
        a = 1
        b = 5
        I = GenericInterval(a, b)
        self.assertEqual(I.min(), a)
        self.assertEqual(I.max(), b)
        self.assertEqual(I.middle(), (b+a)/2)
        self.assertEqual(I.extent(), b-a)
        self.assertFalse(I.isSingular())
        self.assertTrue(I.contains(a + (b-a)/3))
        self.assertFalse(I.contains(a-1))
        I.setMin(b)
        I.setMax(8+b)
        self.assertTrue(I.containsInterval(GenericInterval(b+1, b+4)))
        self.assertFalse(I.containsInterval(I+1))
        #I in now [5, 13]
        J = GenericInterval(2, 9)
        self.assertEqual((I+J).min(), 7)
        self.assertEqual((I|J).min(), 2)
        self.assertEqual((I|J), GenericInterval.unify(I, J))
        I.expandBy(-3)
        self.assertEqual(I, GenericInterval(8, 10))
        I.expandTo(1)
        self.assertEqual(I, GenericInterval(1, 10))
        
        p = [1, 2, 3.443, 3]
        K = GenericInterval.from_list(p)
        self.assertAlmostEqual(K.max(), max(p))
        self.assertAlmostEqual((K+GenericInterval(1.0)).min(), min(p)+1)
        L = GenericInterval(10/3.0)
        for i in range(3):
            K+=L
        self.assertAlmostEqual(K.max(), max(p)+10)
        #TODO This 2geom behaviour is a bit strange
        self.assertEqual(GenericInterval(3.0)|GenericInterval(5.0), 
                    GenericInterval(3.0, 5.0))
        self.assertAlmostEqual((K-L).max(), (K-10/3.0).max())
unittest.main()

