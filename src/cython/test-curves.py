import unittest
import math
from random import randint, uniform

import cy2geom

from cy2geom import Angle
from cy2geom import Point, IntPoint
from cy2geom import Line, Ray, Rect
from cy2geom import Interval, IntInterval, OptInterval, OptIntInterval


from cy2geom import Affine
from cy2geom import Translate, Scale, Rotate, VShear, HShear, Zoom
from cy2geom import Eigen

from cy2geom import Linear
from cy2geom import SBasis, SBasisCurve
from cy2geom import Bezier, BezierCurve
from cy2geom import lerp

from cy2geom import LineSegment, QuadraticBezier, CubicBezier
from cy2geom import HLineSegment, VLineSegment

from cy2geom import EllipticalArc

class TestPrimitives(unittest.TestCase):
    def test_linear(self):
        L = Linear(0, 1)
        M = Linear(2)
        N = Linear()
        self.assertEqual( (L+M), L+2 )
        self.assertEqual( (L-M), L-2 )
        self.assertAlmostEqual(L(0.5), lerp(.5, 0, 1))
        #~ self.assertTrue(N.is_zero())
        self.assertTrue(M.is_constant())
        self.assertTrue(L.is_finite())
        self.assertAlmostEqual(L(0), L.at0())
        self.assertAlmostEqual(L(1), L.at1())
        self.assertAlmostEqual(L.value_at(0.3), L(0.3))
        self.assertTrue( isinstance(M.to_SBasis(), SBasis ))
        
        self.assertAlmostEqual(L.tri(), L(1) - L(0))
        self.assertAlmostEqual(L.hat(), (L(1) + L(0))/2)
        
        for i in range(11):
            t = i/10.0
            self.assertTrue(L.bounds_exact().Interval.contains(L(t)))
            self.assertTrue(L.bounds_fast().Interval.contains(L(t)))
            self.assertTrue(L.bounds_local(t-0.05, t+0.05).Interval.contains(L(t)))
            self.assertAlmostEqual(lerp(t, 0, 4), t*4)
            self.assertAlmostEqual(L(t), cy2geom.reverse(L)(1-t))
            self.assertAlmostEqual( L(t)*t, (L*t)(t) )
            self.assertAlmostEqual( L(t)+t, (L+t)(t) )
            self.assertAlmostEqual( L(t)-t, (L-t)(t) )
            self.assertAlmostEqual( -( L(t) ), (-L)(t) )
            self.assertAlmostEqual( (L/2)(t), L(t)/2 )
        
    def test_sBasis(self):
        S = SBasis()
        T = SBasis(2)
        U = SBasis(1, 7)
        V = SBasis.from_linear( Linear(2, 8) )
        
        self.assertEqual(V[0], Linear(2, 8))
        self.assertEqual(V.back(), Linear(2, 8))
        
        #~ self.assertTrue(S.empty())
        self.assertFalse(T.empty())
        
        T.pop_back()
        self.assertTrue(T.empty())
        
        self.assertEqual(S.size(), 0)
        self.assertEqual(U.size(), 1)
        self.assertEqual((U*V).size(), 2)
        
        T.resize(1, Linear(2, 3))
        self.assertEqual(T[0], Linear(2, 3))
        T.clear()
        self.assertTrue(T.empty())
        #TODO
        #~ T.reserve(5)
        #~ print T.size()
        self.assertEqual(V.at(0), V[0])
        self.assertEqual(V, U+1)
        self.assertNotEqual(V, U)
        self.assertTrue(T.is_zero())
        self.assertTrue(SBasis(1).is_constant())
        def f(A, B):
            return (-A)*(A+B*2.2)*(A*B-B*B/3)
        W = f(U, V)
        self.assertAlmostEqual(W(0), W.at0())
        self.assertAlmostEqual(W(1), W.at1())
        
        for i in range(11):
            t = i/10.0
            self.assertAlmostEqual(W(t), W.value_at(t))
            self.assertAlmostEqual(W(t), f(U(t), V(t)))
            
            vd_UV = (U*V).value_and_derivatives(t, 1)
            vd_U = U.value_and_derivatives(t, 1)
            vd_V = V.value_and_derivatives(t, 1)
            self.assertAlmostEqual( vd_UV[1], vd_U[1]*V(t)+U(t)*vd_V[1] )
            
            self.assertAlmostEqual( U(V)(t), U(V(t)) )
        self.assertEqual(T.degrees_of_freedom(), 0)
        self.assertEqual(U.degrees_of_freedom(), 2)
        
        self.assertEqual(T, T.to_SBasis())
        
        U2 = SBasis(U(0), U(1))
        U2.resize(10)
        self.assertNotEqual(U2, U)
        U2.truncate(U.size())
        self.assertEqual(U2, U)
        #TODO: normalize()
        sL = Linear.sin(Linear(0, 1), 3)
        cL = Linear.cos(Linear(0, 1), 3)
        sqrtU = SBasis.sqrt( U, 3 )
        rL = Linear.reciprocal(Linear(1,2), 3)
        # cy2geom.inverse seems to return nans for degrees > 1
        #~ asin = cy2geom.inverse( cy2geom.sqrt( SBasis(Linear(0, 1)), 3 ), 1)
        for i in range(11):
            t = i/10.0
            self.assertAlmostEqual(sL(t), math.sin(t))
            self.assertAlmostEqual(cL(t), math.cos(t))
            #cy2geom.sqrt is not that precise 
            self.assertAlmostEqual(sqrtU(t), math.sqrt(U(t)), places = 1)
            self.assertAlmostEqual(rL(t), 1/(1+t), places = 1 )
            #~ self.assertAlmostEqual( asin(t), math.asin(t) )
            self.assertAlmostEqual( SBasis.compose(U, V)(t), U(V)(t) )
            self.assertAlmostEqual( SBasis.divide(U, V, 3)(t), U(t)/V(t), places = 1)
            
            self.assertAlmostEqual( SBasis.derivative(SBasis.integral(W))(t), W(t))
            self.assertAlmostEqual( cy2geom.reverse(W)(t), W(1-t) )
            self.assertAlmostEqual( SBasis.multiply(U, V)(t), (U*V)(t))
            #TODO looks like bug in 2geom
            #~ print cy2geom.multiply_add(U, V, W)(t), (U*V+W)(t)
            self.assertAlmostEqual( SBasis.multiply_add(U, W, V)(t), (U*W+V)(t))
            
            self.assertTrue( SBasis.bounds_exact(U).Interval.contains(U(t)) )
            self.assertTrue( SBasis.bounds_fast(U).Interval.contains(U(t)) )
            self.assertTrue( SBasis.bounds_local(U, OptInterval(t-0.05, t+0.05)).Interval.contains(U(t)) )
        
        
        for r in SBasis.roots(W):
            self.assertAlmostEqual(W(r), 0)
        for r in SBasis.roots(W, Interval(0, 0.7)):
            self.assertAlmostEqual(W(r), 0)
            self.assertTrue(Interval(0, 0.7).contains(r))

        levels = [0, 3, 22, -21]
        for i, roots in enumerate( SBasis.multi_roots(W, levels) ):
            level = levels[i]
            for r in roots:
                self.assertAlmostEqual(W(r), level)

        self.assertEqual(SBasis.valuation(W), 0)
        #TODO: why is this still 0?
        #~ print cy2geom.valuation(cy2geom.shift(W, 6))
        self.assertEqual( U[0], SBasis.shift(U, 2)[2] )
        
        for I in SBasis.level_set(W, 2, tol = 1e-7):
            self.assertAlmostEqual( W(I.mid()), 2 )
        for I in SBasis.level_set(W, Interval(0, 1), tol = 1e-7, vtol = 1e-7):
            self.assertTrue( 0 <= W(I.begin()) <= 1 )
            self.assertTrue( 0 <= W(I.mid()) <= 1 )
            self.assertTrue( 0 <= W(I.end()) <= 1 )

    def test_bezier(self):
        B = Bezier()
        C = Bezier(2)
        D = Bezier(2, 4)
        E = Bezier(1, 3, 9)
        F = Bezier(-2, 5, -1, 2)
        self.assertTrue( B.is_zero() )
        self.assertTrue( C.is_constant() )
        self.assertTrue( D.is_finite() )
        C.clear()
        self.assertEqual(D.degree(), 1)
        self.assertEqual(E.at0(), 1)
        self.assertEqual(E.at1(), 9)
        self.assertEqual(E[2], 9)
        for i in range(11):
            t = i/10.0
            self.assertAlmostEqual( D(t), lerp(t, 2, 4) )
            self.assertAlmostEqual( D(t), D.value_at(t))
            self.assertAlmostEqual( D.value_and_derivatives(t, 0)[0], D(t) )
            self.assertAlmostEqual( D.value_and_derivatives(t, 1)[1], Bezier.derivative(D)(t) )
            self.assertAlmostEqual( Bezier.integral(D).value_and_derivatives(t, 1)[1], D(t) )
            #~ self.assertAlmostEqual( D.elevate_degree().reduce_degree()(t), D(t) )
            self.assertAlmostEqual( (D+2)(t), D(t)+2 )
            self.assertAlmostEqual( (D-1)(t), D(t)-1 )
            self.assertAlmostEqual( (D*2)(t), D(t)*2 )
            self.assertAlmostEqual( (D/4)(t), D(t)/4 )
            self.assertTrue( Bezier.bounds_fast(F).Interval.contains(F(t)) )
            self.assertTrue( Bezier.bounds_exact(F).Interval.contains(F(t)) )
            self.assertTrue( Bezier.bounds_local(F, OptInterval(t-0.05, t+0.05)).Interval.contains(F(t)) )
        for r in F.roots():
            self.assertAlmostEqual(F(r), 0)
        #TODO: bug in 2geom?
        #~ for r in F.roots(Interval(0.1, 0.8)):
            #~ self.assertAlmostEqual(F(r), 0)
            #~ self.assertTrue( 0.1 <= r <= 0.8 )
        self.assertIsInstance(F.forward_difference(1), Bezier)
        self.assertIsInstance(F.elevate_degree(), Bezier)
        self.assertIsInstance(E.reduce_degree(), Bezier)
        #F.reduce_degree() fails with
        # *** glibc detected *** python2: malloc(): memory corruption:
        self.assertIsInstance(F.elevate_to_degree(4), Bezier)
        self.assertIsInstance(F.deflate(), Bezier)
        S = F.to_SBasis()
        self.assertIsInstance(S, SBasis)
        for i in range(11):
            t = i/10.0
            self.assertAlmostEqual(S(t), F(t))
        
    def curve(self, C): 
        self.assertAlmostEqual(C.initial_point(), C(0))
        self.assertAlmostEqual(C.final_point(), C.point_at(1))
        #Doesn't have to be true
        #~ if C.length() > 0.01:
            #~ self.assertFalse(C.is_degenerate())

        if C.is_degenerate():
            #trivial special case
            return
        
        for i in range(11):
            t = i/10.0
            self.assertAlmostEqual(C(t).x, C.point_at(t).x)
            self.assertAlmostEqual(C(t).y, C.value_at(t, 1))
            self.assertEqual( C(t), C.point_and_derivatives(t, 1)[0] )
            self.assertTrue( C.bounds_exact().contains(C(t)) )
            self.assertTrue( C.bounds_fast().contains(C(t)) )
            #TODO why this works only with degree = 0?
            if      C.bounds_local(OptInterval(t-0.05, t+0.05), 0
                ) and  (
                    C.bounds_local(OptInterval(t-0.05, t+0.05), 0).Rect.area() > 1e-10):
                #ruling out too small rectangles, they have problems with precision
                self.assertTrue( C.bounds_local( OptInterval(t-0.05, t+0.05), 0 ).Rect.contains(C(t)))
        D = C.duplicate()
        
        D.set_initial(Point())
        self.assertAlmostEqual(D.initial_point(), Point())
        
        D.set_final(Point(1, 1))
        self.assertAlmostEqual(D.final_point(), Point(1, 1))
        
        A = Affine( uniform(-10, 10),
                    uniform(-10, 10),
                    uniform(-10, 10),
                    uniform(-10, 10),
                    uniform(-10, 10),
                    uniform(-10, 10))
        E = C.transformed(A)
        for i in range(11):
            t = i/10.0
      #      self.assertAlmostEqual( E(t), C(t)*A )
        G1 = C.portion(0.2, 0.8)
        G2 = C.portion( interval=Interval(2, 8)/10 )
        self.assertAlmostEqual( G1(0), C(0.2) )
        self.assertAlmostEqual( G2(0.5), C( lerp(0.5, 0.2, 0.8) ))
        self.assertAlmostEqual( G1(1), G2(1) )
        
        for i in range(11):
            t = i/10.0
            self.assertAlmostEqual( C.reverse()(t), C(1-t) )
        self.assertAlmostEqual( C.point_and_derivatives(0.3, 1)[1], C.derivative()(0.3) )
        
        self.assertAlmostEqual( C.nearest_time(C(0)), 0 )
        self.assertAlmostEqual( C( C.nearest_time(C(0.5), interval=Interval(0.2, 0.5)) ), C(0.5) )
        self.assertAlmostEqual( C( C.nearest_time(C(0.5), 0.2, 0.5) ), C(0.5) )
        for p in C.all_nearest_times( C(0), 0, 1):
            self.assertEqual(C(p), C(0))
        for p in C.all_nearest_times( C(1), interval=Interval(0, 1)):
            self.assertEqual(C(p), C(1))
        for r in C.roots(0, 0):
            self.assertAlmostEqual(C.value_at(r, 0), 0)
        
        self.assertGreaterEqual(C.length(), abs(C(1) - C(0)))
        self.assertEqual(C.winding(Point()), int(C.winding(Point())) )
        self.assertAlmostEqual( C.unit_tangent_at(0.5), 
                                Point.unit_vector(C.derivative()(0.5)) )
        self.assertTrue(isinstance(C.to_SBasis()[0], SBasis))

    def test_sBasisCurve(self):
        S = SBasisCurve(SBasis(0, 2), SBasis(3, 7)*SBasis(1, 8))
        a = SBasis(3, 9)*SBasis(4, 6)
        b = SBasis(2, 0)
        c = a(b)
        self.curve(S)
        self.curve(S.derivative())
        self.curve(S.reverse())
        self.curve(S.transformed( Scale(4) ))
        self.curve(S.transformed( Zoom(9, Translate(3, 6)) ))
        self.curve(SBasisCurve(a*b*c, a+b+c))
        self.curve(S.derivative().derivative())

    def test_bezierCurve(self):
        B = BezierCurve.create( [ Point(0, 5), Point(3, 65), Point(-3, 2), Point(1, 9) ] )
        C = BezierCurve.create( [ Point(0,1), Point(1, 0) ] )
        self.curve(B)
        self.curve(C)
        self.curve(C.reverse())        
        self.curve(B.portion(0, 2))
        self.curve(B.transformed(Zoom(9, Translate(3, 6))))
        self.curve(B.derivative())
    
    def ntest_lineSegment(self):
        L = LineSegment(Point(2, 8), Point(1, 9))
        K = LineSegment.from_beziers(Bezier(2, 8), Bezier(-1, 9)) 
        self.curve(L)
        self.curve(K)
        self.curve(L.reverse())
        self.curve(L.portion(Interval(0.2, 0.4)))
        self.curve(L.subdivide(0.3)[0])
        self.curve(L.subdivide(0.3)[1])
        self.curve(L.derivative())
        self.curve(L.transformed(Scale(30)*Translate(3, 9)))
        
        self.curve(LineSegment())
        
    def test_quadraticBezier(self):
        Q = QuadraticBezier(Point(2, 8), Point(1, 9), Point(-2, 3))
        R = QuadraticBezier.from_beziers(Bezier(2, 8, 4), Bezier(-1, 9, 9)) 
        self.curve(Q)
        self.curve(R)
        self.curve(Q.reverse())
        self.curve(Q.portion(interval=Interval(0.1, 0.9)))
        self.curve(Q.subdivide(0.8)[0])
        self.curve(Q.subdivide(0.8)[1])
        self.curve(Q.derivative())
        self.curve(Q.transformed(Scale(-3)*Translate(4, 8)))
        
        self.curve(QuadraticBezier())

    def test_cubicBezier(self):
        C = CubicBezier(Point(2, 0), Point(-1, 2.9), Point(-2, 3), Point(3, 1))
        D = CubicBezier.from_beziers(Bezier(2, 8, 4, 7), Bezier(-1, 9, 9, 8)) 
        print 343
        self.curve(C)
        self.curve(D)
        self.curve(C.reverse())
        #Some kind of numerical instability imo
        #~ self.curve(C.portion(Interval(0.1, 0.9)))
        self.curve(C.subdivide(0.8)[0])
        self.curve(C.subdivide(0.8)[1])
        self.curve(C.derivative())
        self.curve(C.transformed(Scale(-3)*Translate(4, 8)))
        
        self.curve(CubicBezier())

    def test_hLineSegment(self):
        H = HLineSegment(Point(3, 9), Point(9, 9))
        I = HLineSegment(Point(1, 3), Point(92, 3))
        J = HLineSegment.from_point_length( Point(2, 4), 1)
        self.curve( H )
        self.curve( I )
        self.curve( J )
        self.curve( H.portion(0, .25) )
        self.curve( H.derivative() )
        self.curve( H.transformed(Rotate(20)) )
        self.curve( HLineSegment() )
        self.curve( I.reverse() )
        map(self.curve, I.subdivide(0.8))
        
        self.assertAlmostEqual(I.get_Y(), 3)
        J.set_Y(2)
        J.set_initial_X(0)
        J.set_final_X(1)
        self.assertAlmostEqual( J(0), Point(0, 2) )
        self.assertAlmostEqual( J(1), Point(1, 2) )
        
    def test_vLineSegment(self):
        V = VLineSegment(Point(2, 9), Point(2, 6))
        W = VLineSegment(Point(1, 2), Point(1, 8))
        X = VLineSegment.from_point_length( Point(2, 4), 1)
        #~ self.curve( V )
        #~ self.curve( W )
        #~ self.curve( X )
        #~ self.curve( V.portion(0, .25) )
        #~ self.curve( V.derivative() )
        #~ self.curve( V.transformed(Rotate(20)) )
        #~ self.curve( VLineSegment() )
        #~ self.curve( W.reverse() )
        #~ map(self.curve, W.subdivide(0.8))
        #~ 
        #~ self.assertAlmostEqual(I.get_Y(), 3)
        #~ X.set_Y(2)
        #~ X.set_initialX(0)
        #~ X.set_finalX(1)
        #~ self.assertAlmostEqual( X(0), Point(0, 2) )
        #~ self.assertAlmostEqual( X(1), Point(1, 2) )
        #~ print V(0.5)
        #~ print V.nearest_time(V(0.5), 0.1, 0.4   )
        #~ print V.nearest_time(V(0.5), Interval(0.2, 0.5))
        #~ print V(0.5), V(0.2)
    #TODO:
    #this is likely a bug in 2geom, following code
    
        #~ VLineSegment V(Point(0, 0), 2);
        #~ printf("%f\n", V.nearest_time(V(0.5), 0.2, 0.5));
    
    #prints
        #0.2
    
    def test_ellipticalArc(self):
        E = EllipticalArc()
        self.curve(E)
        F = EllipticalArc(Point(), 1, 2, math.pi/6, True, True, Point(1, 1))

        self.assertTrue(F.sweep())
        self.assertTrue(F.large_arc())
        self.assertAlmostEqual(F.chord()(0), Point())
        self.assertAlmostEqual(F.chord()(1), Point(1, 1))
        
        F.set_extremes(Point(1, 1), Point(-1, 1))
        self.assertAlmostEqual(F.initial_point(), Point(1, 1))
        self.assertAlmostEqual(F.final_point(), Point(-1, 1))
        self.assertEqual(F.initial_angle(), F.angle_at(0))
        self.assertEqual(F.final_angle(), F.angle_at(1))
        self.assertTrue(F.contains(F.angle_at(0.5)))
        
        G = EllipticalArc(Point(), 1, 1, 0, True, True, Point(2, 0))
        for i in range(11):
            t = i/10.0
            print G(t)
        self.assertAlmostEqual(G.extent(), math.pi)
        self.assertAlmostEqual(G.extent(), G.sweep_angle())
        self.assertAlmostEqual(float(G.angle_at(0.5)), -math.pi/2)
        
        self.assertAlmostEqual(Point(1, 1), G.rays())
        self.assertAlmostEqual(1, G.ray(1))
        self.assertAlmostEqual(0, float(G.rotation_angle()))
        
        self.assertAlmostEqual(G.extent(), G.angle_interval().extent())
        
        self.assertAlmostEqual(G.center(), Point(1, 0))
        #unit half-circle
        U = EllipticalArc(Point(1, 0), 1, 1, 0, True, True, Point(-1, 0))
        
        G.set(Point(), 1, 1, 0, True, False, Point(1, 0))
        
        A = G.unit_circle_transform()
        
        self.assertAlmostEqual( G(0.5), U.transformed(A)(0.5) )
        self.assertAlmostEqual( G.value_at_angle(G.angle_at(0.32), 0), G.value_at(0.32, 0) )
        
        self.assertTrue(G.contains_angle(Angle(math.pi/4)))
        self.assertFalse(G.is_SVG_compliant())
        #~ self.curve(F)
        #TODO:
        #F.point_and_derivatives(t, 1)[0] differs from F(0) and F.bounds_exact, 
        #F.bounds_fast doesn't contain F(1)
        
unittest.main()
