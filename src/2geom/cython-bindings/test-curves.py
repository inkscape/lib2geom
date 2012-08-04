import unittest
import math
from random import randint, uniform

import cy2geom

from cy2geom import Angle
from cy2geom import Point, IntPoint
from cy2geom import Line, Ray, Rect
from cy2geom import Interval, IntInterval, OptInterval, OptIntInterval
#from cy2geom import GenericInterval, GenericOptInterval

from cy2geom import Rect#, OptRect, IntRect, OptIntRect
#from cy2geom import GenericRect

from cy2geom import Affine
from cy2geom import Translate, Scale, Rotate, VShear, HShear, Zoom
from cy2geom import Eigen

from cy2geom import Linear
from cy2geom import SBasis, SBasisCurve
from cy2geom import Bezier, BezierCurve
from cy2geom import lerp, reverse

from cy2geom import LineSegment, QuadraticBezier, CubicBezier
from cy2geom import HLineSegment, VLineSegment

from cy2geom import EllipticalArc, SVGEllipticalArc

class TestPrimitives(unittest.TestCase):
    def test_linear(self):
        L = Linear(0, 1)
        M = Linear(2)
        N = Linear()
        self.assertEqual( (L+M), L+2 )
        self.assertEqual( (L-M), L-2 )
        self.assertAlmostEqual(L(0.5), lerp(.5, 0, 1))
        #~ self.assertTrue(N.isZero())
        self.assertTrue(M.isConstant())
        self.assertTrue(L.isFinite())
        self.assertAlmostEqual(L(0), L.at0())
        self.assertAlmostEqual(L(1), L.at1())
        self.assertAlmostEqual(L.valueAt(0.3), L(0.3))
        self.assertTrue( isinstance(M.toSBasis(), SBasis ))
        
        self.assertAlmostEqual(L.tri(), L(1) - L(0))
        self.assertAlmostEqual(L.hat(), (L(1) + L(0))/2)
        
        for i in range(11):
            t = i/10.0
            self.assertTrue(L.bounds_exact().Interval.contains(L(t)))
            self.assertTrue(L.bounds_fast().Interval.contains(L(t)))
            self.assertTrue(L.bounds_local(t-0.05, t+0.05).Interval.contains(L(t)))
            self.assertAlmostEqual(lerp(t, 0, 4), t*4)
            self.assertAlmostEqual(L(t), reverse(L)(1-t))
            self.assertAlmostEqual( L(t)*t, (L*t)(t) )
            self.assertAlmostEqual( L(t)+t, (L+t)(t) )
            self.assertAlmostEqual( L(t)-t, (L-t)(t) )
            self.assertAlmostEqual( -( L(t) ), (-L)(t) )
            self.assertAlmostEqual( (L/2)(t), L(t)/2 )
        
    def test_sBasis(self):
        S = SBasis()
        T = SBasis(2)
        U = SBasis(1, 7)
        V = SBasis(Linear(2, 8))
        
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
        self.assertTrue(T.isZero())
        self.assertTrue(SBasis(1).isConstant())
        def f(A, B):
            return (-A)*(A+B*2.2)*(A*B-B*B/3)
        W = f(U, V)
        self.assertAlmostEqual(W(0), W.at0())
        self.assertAlmostEqual(W(1), W.at1())
        
        for i in range(11):
            t = i/10.0
            self.assertAlmostEqual(W(t), W.valueAt(t))
            self.assertAlmostEqual(W(t), f(U(t), V(t)))
            
            vd_UV = (U*V).valueAndDerivatives(t, 1)
            vd_U = U.valueAndDerivatives(t, 1)
            vd_V = V.valueAndDerivatives(t, 1)
            self.assertAlmostEqual( vd_UV[1], vd_U[1]*V(t)+U(t)*vd_V[1] )
            
            self.assertAlmostEqual( U(V)(t), U(V(t)) )
        self.assertEqual(T.degreesOfFreedom(), 0)
        self.assertEqual(U.degreesOfFreedom(), 2)
        
        self.assertEqual(T, T.toSBasis())
        
        U2 = SBasis(U)
        U2.resize(10)
        self.assertNotEqual(U2, U)
        U2.truncate(U.size())
        self.assertEqual(U2, U)
        #TODO: normalize()
        sL = cy2geom.sin(Linear(0, 1), 3)
        cL = cy2geom.cos(Linear(0, 1), 3)
        sqrtU = cy2geom.sqrt( U, 3 )
        rL = cy2geom.reciprocal(Linear(1,2), 3)
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
            self.assertAlmostEqual( cy2geom.compose(U, V)(t), U(V)(t) )
            self.assertAlmostEqual( cy2geom.divide(U, V, 3)(t), U(t)/V(t), places = 1)
            
            self.assertAlmostEqual( cy2geom.derivative(cy2geom.integral(W))(t), W(t))
            self.assertAlmostEqual( cy2geom.reverse(W)(t), W(1-t) )
            self.assertAlmostEqual( cy2geom.multiply(U, V)(t), (U*V)(t))
            #TODO looks like bug in 2geom
            #~ print cy2geom.multiply_add(U, V, W)(t), (U*V+W)(t)
            self.assertAlmostEqual( cy2geom.multiply_add(U, W, V)(t), (U*W+V)(t))
            
            self.assertTrue( cy2geom.bounds_exact(U).Interval.contains(U(t)) )
            self.assertTrue( cy2geom.bounds_fast(U).Interval.contains(U(t)) )
            self.assertTrue( cy2geom.bounds_local(U, OptInterval(t-0.05, t+0.05)).Interval.contains(U(t)) )
        
        
        for r in cy2geom.roots(W):
            self.assertAlmostEqual(W(r), 0)
        for r in cy2geom.roots(W, Interval(0, 0.7)):
            self.assertAlmostEqual(W(r), 0)
            self.assertTrue(Interval(0, 0.7).contains(r))

        levels = [0, 3, 22, -21]
        for i, roots in enumerate( cy2geom.multi_roots(W, levels) ):
            level = levels[i]
            for r in roots:
                self.assertAlmostEqual(W(r), level)

        self.assertEqual(cy2geom.valuation(W), 0)
        #TODO: why is this still 0?
        #~ print cy2geom.valuation(cy2geom.shift(W, 6))
        self.assertEqual( U[0], cy2geom.shift(U, 2)[2] )
        
        for I in cy2geom.level_set(W, 2, tol = 1e-7):
            self.assertAlmostEqual( W(I.mid()), 2 )
        for I in cy2geom.level_set(W, Interval(0, 1), tol = 1e-7, vtol = 1e-7):
            self.assertTrue( 0 <= W(I.begin()) <= 1 )
            self.assertTrue( 0 <= W(I.mid()) <= 1 )
            self.assertTrue( 0 <= W(I.end()) <= 1 )

    def test_bezier(self):
        B = Bezier()
        C = Bezier(2)
        D = Bezier(2, 4)
        E = Bezier(1, 3, 9)
        F = Bezier(-2, 5, -1, 2)
        self.assertTrue( B.isZero() )
        self.assertTrue( C.isConstant() )
        self.assertTrue( D.isFinite() )
        C.clear()
        self.assertEqual(D.degree(), 1)
        self.assertEqual(E.at0(), 1)
        self.assertEqual(E.at1(), 9)
        self.assertEqual(E[2], 9)
        for i in range(11):
            t = i/10.0
            self.assertAlmostEqual( D(t), lerp(t, 2, 4) )
            self.assertAlmostEqual( D(t), D.valueAt(t))
            self.assertAlmostEqual( D.valueAndDerivatives(t, 0)[0], D(t) )
            self.assertAlmostEqual( D.valueAndDerivatives(t, 1)[1], cy2geom.derivative(D)(t) )
            self.assertAlmostEqual( cy2geom.integral(D).valueAndDerivatives(t, 1)[1], D(t) )
            #~ self.assertAlmostEqual( D.elevate_degree().reduce_degree()(t), D(t) )
            self.assertAlmostEqual( (D+2)(t), D(t)+2 )
            self.assertAlmostEqual( (D-1)(t), D(t)-1 )
            self.assertAlmostEqual( (D*2)(t), D(t)*2 )
            self.assertAlmostEqual( (D/4)(t), D(t)/4 )
            self.assertTrue( cy2geom.bounds_fast(F).Interval.contains(F(t)) )
            self.assertTrue( cy2geom.bounds_exact(F).Interval.contains(F(t)) )
            self.assertTrue( cy2geom.bounds_local(F, OptInterval(t-0.05, t+0.05)).Interval.contains(F(t)) )
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
        S = SBasis()
        cy2geom.bezier_to_sbasis(S, F)
        self.assertIsInstance(S, SBasis)
        for i in range(11):
            t = i/10.0
            self.assertAlmostEqual(S(t), F(t))
        
    def curve(self, C): 
        self.assertAlmostEqual(C.initialPoint(), C(0))
        self.assertAlmostEqual(C.finalPoint(), C.pointAt(1))
        #Doesn't have to be true
        #~ if C.length() > 0.01:
            #~ self.assertFalse(C.isDegenerate())

        if C.isDegenerate():
            #trivial special case
            return
        
        for i in range(11):
            t = i/10.0
            self.assertAlmostEqual(C(t).x, C.pointAt(t).x)
            self.assertAlmostEqual(C(t).y, C.valueAt(t, 1))
            self.assertEqual( C(t), C.pointAndDerivatives(t, 1)[0] )
            self.assertTrue( C.boundsExact().contains(C(t)) )
            self.assertTrue( C.boundsFast().contains(C(t)) )
            #TODO why this works only with degree = 0?
            if      C.boundsLocal(OptInterval(t-0.05, t+0.05), 0
                ) and  (
                    C.boundsLocal(OptInterval(t-0.05, t+0.05), 0).Rect.area() > 1e-10):
                #ruling out too small rectangles, they have problems with precision
                self.assertTrue( C.boundsLocal( OptInterval(t-0.05, t+0.05), 0 ).Rect.contains(C(t)))
        D = C.duplicate()
        
        D.setInitial(Point())
        self.assertAlmostEqual(D.initialPoint(), Point())
        
        D.setFinal(Point(1, 1))
        self.assertAlmostEqual(D.finalPoint(), Point(1, 1))
        
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
        G2 = C.portion(Interval(2, 8)/10)
        self.assertAlmostEqual( G1(0), C(0.2) )
        self.assertAlmostEqual( G2(0.5), C( lerp(0.5, 0.2, 0.8) ))
        self.assertAlmostEqual( G1(1), G2(1) )
        
        for i in range(11):
            t = i/10.0
            self.assertAlmostEqual( C.reverse()(t), C(1-t) )
        self.assertAlmostEqual( C.pointAndDerivatives(0.3, 1)[1], C.derivative()(0.3) )
        
        self.assertAlmostEqual( C.nearestPoint(C(0)), 0 )
        self.assertAlmostEqual( C( C.nearestPoint(C(0.5), Interval(0.2, 0.5)) ), C(0.5) )
        self.assertAlmostEqual( C( C.nearestPoint(C(0.5), 0.2, 0.5) ), C(0.5) )
        for p in C.allNearestPoints( C(0), 0, 1):
            self.assertEqual(C(p), C(0))
        for p in C.allNearestPoints( C(1), Interval(0, 1)):
            self.assertEqual(C(p), C(1))
        for r in C.roots(0, 0):
            self.assertAlmostEqual(C.valueAt(r, 0), 0)
        
        self.assertGreaterEqual(C.length(), abs(C(1) - C(0)))
        self.assertEqual(C.winding(Point()), int(C.winding(Point())) )
        self.assertAlmostEqual( C.unitTangentAt(0.5), 
                                Point.unit_vector(C.derivative()(0.5)) )
        self.assertTrue(isinstance(C.toSBasis()[0], SBasis))

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
        B = BezierCurve( [ Point(0, 5), Point(3, 65), Point(-3, 2), Point(1, 9) ] )
        C = BezierCurve( [ Point(0,1), Point(1, 0) ] )
        self.curve(B)
        self.curve(C)
        self.curve(C.reverse())        
        self.curve(B.portion(0, 2))
        self.curve(B.transformed(Zoom(9, Translate(3, 6))))
        self.curve(B.derivative())
    
    def test_lineSegment(self):
        L = LineSegment(Point(2, 8), Point(1, 9))
        K = LineSegment(Bezier(2, 8), Bezier(-1, 9)) 
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
        R = QuadraticBezier(Bezier(2, 8, 4), Bezier(-1, 9, 9)) 
        self.curve(Q)
        self.curve(R)
        self.curve(Q.reverse())
        self.curve(Q.portion(Interval(0.1, 0.9)))
        self.curve(Q.subdivide(0.8)[0])
        self.curve(Q.subdivide(0.8)[1])
        self.curve(Q.derivative())
        self.curve(Q.transformed(Scale(-3)*Translate(4, 8)))
        
        self.curve(QuadraticBezier())

    def test_cubicBezier(self):
        C = CubicBezier(Point(2, 0), Point(-1, 2.9), Point(-2, 3), Point(3, 1))
        D = CubicBezier(Bezier(2, 8, 4, 7), Bezier(-1, 9, 9, 8)) 
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
        H = HLineSegment(Point(3, 9), 9)
        I = HLineSegment(Point(1, 3), Point(92, 3))
        J = HLineSegment( 2, 4, 1)
        self.curve( H )
        self.curve( I )
        self.curve( J )
        self.curve( H.portion(0, .25) )
        self.curve( H.derivative() )
        self.curve( H.transformed(Rotate(20)) )
        self.curve( HLineSegment() )
        self.curve( I.reverse() )
        map(self.curve, I.subdivide(0.8))
        
        self.assertAlmostEqual(I.getY(), 3)
        J.setY(2)
        J.setInitialX(0)
        J.setFinalX(1)
        self.assertAlmostEqual( J(0), Point(0, 2) )
        self.assertAlmostEqual( J(1), Point(1, 2) )
        
    def test_vLineSegment(self):
        V = VLineSegment(Point(2, 9), 9)
        #~ W = VLineSegment(Point(1, 2), Point(1, 8))
        #~ X = VLineSegment( 2, 4, 1)
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
        #~ self.assertAlmostEqual(I.getY(), 3)
        #~ X.setY(2)
        #~ X.setInitialX(0)
        #~ X.setFinalX(1)
        #~ self.assertAlmostEqual( X(0), Point(0, 2) )
        #~ self.assertAlmostEqual( X(1), Point(1, 2) )
        #~ print V(0.5)
        #~ print V.nearestPoint(V(0.5), 0.1, 0.4   )
        #~ print V.nearestPoint(V(0.5), Interval(0.2, 0.5))
        #~ print V(0.5), V(0.2)
    #TODO:
    #this is likely a bug in 2geom, following code
    
        #~ VLineSegment V(Point(0, 0), 2);
        #~ printf("%f\n", V.nearestPoint(V(0.5), 0.2, 0.5));
    
    #prints
        #0.2
    
    def test_ellipticalArc(self):
        E = EllipticalArc()
        self.curve(E)
        F = EllipticalArc(Point(), 1, 2, math.pi/6, True, True, Point(1, 1))

        self.assertTrue(F.sweep())
        self.assertTrue(F.largeArc())
        self.assertAlmostEqual(F.chord()(0), Point())
        self.assertAlmostEqual(F.chord()(1), Point(1, 1))
        
        F.setExtremes(Point(1, 1), Point(-1, 1))
        self.assertAlmostEqual(F.initialPoint(), Point(1, 1))
        self.assertAlmostEqual(F.finalPoint(), Point(-1, 1))
        self.assertEqual(F.initialAngle(), F.angleAt(0))
        self.assertEqual(F.finalAngle(), F.angleAt(1))
        self.assertTrue(F.contains(F.angleAt(0.5)))
        
        G = EllipticalArc(Point(), 1, 1, 0, True, True, Point(2, 0))
        for i in range(11):
            t = i/10.0
            print G(t)
        self.assertAlmostEqual(G.extent(), math.pi)
        self.assertAlmostEqual(G.extent(), G.sweepAngle())
        self.assertAlmostEqual(float(G.angleAt(0.5)), -math.pi/2)
        
        self.assertAlmostEqual(Point(1, 1), G.rays())
        self.assertAlmostEqual(1, G.ray(1))
        self.assertAlmostEqual(0, float(G.rotationAngle()))
        
        self.assertAlmostEqual(G.extent(), G.angleInterval().extent())
        
        self.assertAlmostEqual(G.center(), Point(1, 0))
        #unit half-circle
        U = EllipticalArc(Point(1, 0), 1, 1, 0, True, True, Point(-1, 0))
        
        G.set(Point(), 1, 1, 0, True, False, Point(1, 0))
        
        A = G.unitCircleTransform()
        
        self.assertAlmostEqual( G(0.5), U.transformed(A)(0.5) )
        self.assertAlmostEqual( G.valueAtAngle(G.angleAt(0.32), 0), G.valueAt(0.32, 0) )
        
        self.assertTrue(G.containsAngle(Angle(math.pi/4)))
        self.assertFalse(G.isSVGCompliant())
        #~ self.curve(F)
        #TODO:
        #F.pointAndDerivatives(t, 1)[0] differs from F(0) and F.boundsExact, 
        #F.boundsFast doesn't contain F(1)
    def test_sVGEllipticalArc(self):
        F1 = EllipticalArc(Point(), 1, 2, math.pi/6, True, True, Point(1, 1))
        F2 = SVGEllipticalArc(Point(), 1, 2, math.pi/6, True, True, Point(1, 1))
        for i in range(11):
            t = i/10.0
            self.assertAlmostEqual(F1(t), F2(t))
        #degenerate ellipse
        D = SVGEllipticalArc(Point(0, 0), 0.5, 0, 0, False, False, Point(1, 0))
        self.assertTrue(D.isDegenerate())
        for i in range(11):
            t = i/10.0
            self.assertAlmostEqual(D(t), D.chord()(t))
            self.assertAlmostEqual(D.valueAt(t, 0), t)
            self.assertAlmostEqual(D.roots(t, 0)[0], t)
        self.assertIsInstance(D.derivative(), LineSegment)
        
unittest.main()
