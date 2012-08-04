import unittest
from math import pi, sqrt
from random import randint

import cy2geom

from cy2geom import Point, IntPoint

from cy2geom import Interval, IntInterval, OptInterval, OptIntInterval
from cy2geom import GenericInterval, GenericOptInterval

from cy2geom import Rect, OptRect, IntRect, OptIntRect
from cy2geom import GenericRect

from fractions import Fraction
        

class TestPrimitives(unittest.TestCase):
    def interval_basic(self, I, J):
        #for simplicity
        self.assertTrue(I.min() >= 0)
        self.assertTrue(J.min() >= 0)
        a = I.min()
        b = I.max();
        self.assertAlmostEqual(I.middle(), (a+b)/2); 
        self.assertAlmostEqual(I.extent(), (b-a)); 
        if a != b:
            self.assertFalse(I.isSingular())
        else:
            self.assertTrue(I.isSingular())
            
        I.expandBy(a)
        self.assertAlmostEqual(I.min(), 0); 
        self.assertAlmostEqual(I.max(), a+b)
        I.setMin(a)
        I.setMax(b)
        
        self.assertTrue(I.contains(a+ (b-a)/3 ))
        self.assertTrue(I.contains(a))
        self.assertTrue(I.contains(I))
        
        if (not I.isSingular()) or I.min() != 0 :
            self.assertFalse(I.contains(I+I))
        self.assertFalse(I.contains(a-1))
        
        c = J.min()
        d = J.max()
        self.assertAlmostEqual( (I+J).min(), a+c )
        self.assertAlmostEqual((I|J).min(), min(a, c))
        J.setMin(a+2)
        J.setMax(b+2)
        self.assertEqual(I+2, J)
        I.expandTo(2*b)
        self.assertAlmostEqual(I.max(), 2*b)
        
    def test_interval(self):
        I = Interval(1.2, 5)
        J = Interval(0, 0.3)
        self.interval_basic(I, J)
        
        self.assertTrue(I.interiorContains(I.middle()))
        self.assertFalse(I.interiorContains(I.min()))
        self.assertFalse(I.interiorContains(I))
        self.assertTrue(I.interiorContains(Interval(I.min()+1, I.max()-1)))
        
        self.assertTrue(I.interiorIntersects(I))
        self.assertFalse(I.interiorIntersects(-I))
        p = [1, 2, 3.442, 3]
        K = Interval.from_list(p)
        self.assertAlmostEqual(K.max(), max(p))
        self.assertAlmostEqual((K+Interval(1.0)).min(), min(p)+1)
        L = Interval(10/3.0)
        for i in range(3):
            K+=L
        self.assertAlmostEqual(K.max(), max(p)+10)
        
        #TODO This 2geom behaviour is a bit strange
        self.assertEqual(Interval(3.0)|Interval(5.0), 
                    Interval(3.0, 5.0))
        self.assertAlmostEqual((K-L).max(), (K-10/3.0).max())
        
        self.assertAlmostEqual((K*3.4).max(), 3.4*K.max())
        self.assertAlmostEqual((K/3).extent(), K.extent()/3)
        
    def test_optInterval(self):
        I = OptInterval(2.2, 9.3)
        J = Interval(3, 13)
        K = OptInterval.from_Interval(J)
        self.assertEqual(K.Interval, J)
        self.interval_basic(K.Interval, I.Interval)
        
        L = OptInterval()
        
        self.assertFalse(L)
        self.assertTrue( (L&I).isEmpty() )
        L.intersectWith(I)
        self.assertFalse(L)
        
        L |= I
        
        self.assertEqual(L.Interval, I.Interval)
        
        self.assertEqual((I & K).Interval, Interval(3, 9.3))
        
    def test_intInterval(self):
        I = IntInterval(2, 6)
        J = IntInterval(0, 1)
        self.interval_basic(I, J)
        p = [3, 2.3, 65.3, 43]
        K = IntInterval.from_list(p)
        self.assertAlmostEqual(K.max(), int(max(p)))
        self.assertAlmostEqual(int((K+IntInterval(1.0)).min()), int(min(p)+1))
        L = IntInterval(3)
        for i in range(3):
            K+=L
        self.assertAlmostEqual(K.max(), int(max(p))+9)
        
        self.assertEqual(Interval(3)|Interval(5), 
                    Interval(3, 5))
        self.assertAlmostEqual((K-L).max(), (K-3).max())

    def test_optIntInterval(self):
        I = OptIntInterval(2, 9)
        J = IntInterval(3, 13)
        K = OptIntInterval.from_Interval(J)
        self.assertEqual(K.Interval, J)
        self.interval_basic(K.Interval, I.Interval)
        
        L = OptIntInterval()
        
        self.assertFalse(L)
        self.assertTrue( (L&I).isEmpty() )
        L.intersectWith(I)
        self.assertFalse(L)
        
        L |= I
        
        self.assertEqual(L.Interval, I.Interval)
        
        self.assertEqual((I & K).Interval, IntInterval(3, 9))
        
    def test_genericInterval(self):
        maxv = 100000
        test_vars = [
            ( (randint(0, maxv), randint(0, maxv)), (randint(0, maxv), randint(0, maxv)) ),
            ( (3,), (2, 0) ),
            ( (0.0, 9), (4, 1.3)),
            ((2.98, sqrt(2)), (sqrt(7),)),
            ( (Fraction(1,2), Fraction(3, 7)), ( Fraction(2, 1), ) )
            ]
        for a,b in test_vars:
            self.interval_basic( GenericInterval(*a), GenericInterval(*b) )
            
    def test_genericOptInterval(self):
        test_vars = [
            ( (3,), (2, 0) ),
            ( (0.0, 9), (4, 1.3)),
            ((2.98, sqrt(2)), (sqrt(7),)),
            ( (Fraction(1,2), Fraction(3, 7)), ( Fraction(2, 1), ) )
            ]
            
        for a, b in test_vars:
            I = GenericOptInterval(*a)
            J = GenericInterval(*b)
            K = GenericOptInterval.from_Interval(J)
            
            self.assertEqual(I, GenericOptInterval.from_Interval(I))
            
            self.assertEqual(K.Interval, J)
            self.interval_basic(K.Interval, I.Interval)
            
            L = GenericOptInterval()
            
            self.assertFalse(L)
            self.assertTrue( (L&I).isEmpty() )
            L.intersectWith(I)
            self.assertFalse(L)
            
            L |= I
            
            self.assertEqual(L.Interval, I.Interval)
            
            if I.intersectWith(K):
                if I.Interval.min() <= K.Interval.min():
                    if I.Interval.max() >= K.Interval.max():
                        self.assertEqual( I & K, K)
                    else:
                        self.assertEqual( I & K, GenericInterval(K.min(), I.max()))
                else:
                    if I.Interval.max() >= K.Interval.max():
                        self.assertEqual( I & K, GenericInterval(I.min(), K.max()))
                    else:
                        self.assertEqual( I & K, I)
    
    def test_genericRect(self):
        A = GenericRect(1, 1, 4, 4)
        self.assertEqual( A.min(), (1, 1) )
        B = GenericRect(Fraction(1,4), Fraction(9, 94), Fraction(2, 3), Fraction(23, 37))

        amin = A.min()
        amax = A.max()
        
        self.assertAlmostEqual(amin[0], A[0].min())
        self.assertAlmostEqual(amax[1], A[1].max())
        self.assertEqual(amin, A.corner(0))
        
        self.assertEqual(amin, (A.left(), A.top()))
        self.assertEqual(amax, (A.right(), A.bottom()))
        
        self.assertAlmostEqual( A.width(), A[0].extent() )
        self.assertAlmostEqual( A.height(), A[1].extent() )
        
        #~ self.assertAlmostEqual( A.aspectRatio(), A.width()/A.height() )
        self.assertEqual( A.dimensions(), ( A.width(), A.height() ) )
        #~ self.assertEqual( A.midpoint(), (A.min() + A.max())/2 )
        self.assertAlmostEqual(A.area(), A.width()*A.height())
        #TODO export EPSILON from 2geom
        print A.area()
        if A.area() > 0:
            self.assertFalse(A.hasZeroArea())
        else:
            self.assertTrue(A.hasZeroArea())
        self.assertAlmostEqual(A.maxExtent(), max(A.width(), A.height()))
        self.assertGreaterEqual(A.maxExtent(), A.minExtent())
        
        bmin = B.min()
        bmax = B.max()

        pdiag = sqrt((amax[0]-amin[0])**2+(amax[1]-amin[1])**2)

        B.setMin(A.midpoint())
        B.setMax(A.midpoint())

        self.assertTrue(B.hasZeroArea())
        
        #print P,B
        B.expandBy(A.minExtent()/3.0)
         
        #print P, B
        
        self.assertTrue(A.contains(B))
        self.assertTrue(A.intersects(B))
        self.assertTrue(B.intersects(A))
        self.assertFalse(B.contains(A))
        

        
        self.assertTrue(A.contains(A.midpoint()))
        self.assertFalse(A.contains( (A.midpoint()[0]*3, A.midpoint()[1]*3) ))
        
        A.unionWith(B)
        
        self.assertEqual( A.min(), amin )
        
        B.setLeft(bmin[0])
        B.setTop(bmin[1])
        B.setRight(bmax[0])
        B.setBottom(bmax[1])
        
        self.assertEqual(B.min(), bmin)
        self.assertEqual(B.max(), bmax)
        
        B.expandTo( (0, 0) )
        self.assertEqual((0, 0), B.min())
        print B.min()
        B.expandBy(*bmax)
        #~ B.expandBy((3, 6))
        print bmax
        print B.max(), B.min()
        self.assertEqual(bmax, (- (B.min()[0]), - (B.min()[1])) )
        
        B.expandBy(-bmax[0], -bmax[1])
        self.assertEqual(B.max(), bmax)
        
        self.assertEqual( (A+B.min()).max()[0], A.max()[0] + B.min()[0] )
        
        #~ self.assertEqual( (A-B.max()).min(), A.min() - B.max() )
        
        self.assertEqual( A|A, A )
        
        self.assertFalse( A != A )

        B.setLeft(bmin[0])
        B.setTop(bmin[1])
        B.setRight(bmax[0])
        B.setBottom(bmax[1])
        
        #~ self.assertAlmostEqual(Rect.distance(Point(), A), A.min().length())
        #~ self.assertAlmostEqual(Rect.distanceSq(B.min(), A), Rect.distance(B.min(), A)**2 )


        
    def rect_basic(self, P, Q):
        pmin = P.min()
        pmax = P.max()
        #for simplicity
        self.assertTrue(pmin.x > 0)
        self.assertTrue(pmin.y > 0)
        
        self.assertAlmostEqual(pmin.x, P[0].min())
        self.assertAlmostEqual(pmax.y, P[1].max())
        self.assertEqual(pmin, P.corner(0))
        
        self.assertEqual(pmin, Point(P.left(), P.top()))
        self.assertEqual(pmax, Point(P.right(), P.bottom()))
        
        self.assertAlmostEqual( P.width(), P[0].extent() )
        self.assertAlmostEqual( P.height(), P[1].extent() )
        
        self.assertAlmostEqual( P.aspectRatio(), P.width()/P.height() )
        self.assertEqual( P.dimensions(), Point( P.width(), P.height() ) )
        self.assertEqual( P.midpoint(), (P.min() + P.max())/2 )
        self.assertAlmostEqual(P.area(), P.width()*P.height())
        #TODO export EPSILON from 2geom
        if P.area() > 1e-7:
            self.assertFalse(P.hasZeroArea())
            self.assertTrue(P.hasZeroArea(P.area()))
        else:
            self.assertTrue(P.hasZeroArea())
        self.assertAlmostEqual(P.maxExtent(), max(P.width(), P.height()))
        self.assertGreaterEqual(P.maxExtent(), P.minExtent())
        
        qmin = Q.min()
        qmax = Q.max()
        
        pdiag = (pmax-pmin).length()
        
        Q.setMin(P.midpoint())
        Q.setMax(P.midpoint())
        self.assertTrue(Q.hasZeroArea())
        
        #print P,Q
        Q.expandBy(P.minExtent()/3.0)
         
        #print P, Q
        
        self.assertTrue(P.contains(Q))
        self.assertTrue(P.intersects(Q))
        self.assertTrue(Q.intersects(P))
        self.assertFalse(Q.contains(P))
        
        self.assertTrue(P.interiorContains(Q))
        self.assertFalse(P.interiorContains(P))
        self.assertTrue(P.interiorIntersects(Q))
        self.assertTrue(P.interiorIntersects(P))
        
        self.assertTrue(P.contains(P.midpoint()))
        self.assertFalse(P.contains(P.midpoint()*3))
        
        P.unionWith(Q)
        
        self.assertEqual( P.min(), pmin )
        
        Q.setLeft(qmin.x)
        Q.setTop(qmin.y)
        Q.setRight(qmax.x)
        Q.setBottom(qmax.y)
        
        self.assertEqual(Q.min(), qmin)
        self.assertEqual(Q.max(), qmax)
        
        Q.expandTo( Point() )
        self.assertEqual(Point(), Q.min())
        Q.expandBy(qmax)
        self.assertEqual(qmax, -Q.min())
        
        Q.expandBy(-qmax.x, -qmax.y)
        self.assertEqual(Q.max(), qmax)
        
        self.assertEqual( (P+Q.min()).max(), P.max() + Q.min() )
        
        self.assertEqual( (P-Q.max()).min(), P.min() - Q.max() )
        
        self.assertEqual( P|P, P )
        
        self.assertFalse( P != P )

        Q.setLeft(qmin.x)
        Q.setTop(qmin.y)
        Q.setRight(qmax.x)
        Q.setBottom(qmax.y)
        
        self.assertAlmostEqual(Rect.distance(Point(), P), P.min().length())
        self.assertAlmostEqual(Rect.distanceSq(Q.min(), P), Rect.distance(Q.min(), P)**2 )
        
        self.assertEqual(P.roundOutwards()[0], P[0].roundOutwards())
        if P.roundInwards():
            self.assertEqual(P.roundInwards().Rect[1], P[1].roundInwards().Interval)
        
        
    def intrect_basic(self, P, Q):
        pmin = P.min()
        pmax = P.max()
        #for simplicity
        self.assertTrue(pmin.x > 0)
        self.assertTrue(pmin.y > 0)
        
        self.assertAlmostEqual(pmin.x, P[0].min())
        self.assertAlmostEqual(pmax.y, P[1].max())
        self.assertEqual(pmin, P.corner(0))
        
        self.assertEqual(pmin, IntPoint(P.left(), P.top()))
        self.assertEqual(pmax, IntPoint(P.right(), P.bottom()))
        
        self.assertAlmostEqual( P.width(), P[0].extent() )
        self.assertAlmostEqual( P.height(), P[1].extent() )
        
        self.assertAlmostEqual( P.aspectRatio(), float(P.width())/float(P.height()) )
        self.assertEqual( P.dimensions(), IntPoint( P.width(), P.height() ) )
        self.assertEqual( P.midpoint().x, (P.min() + P.max()).x/2 )
        self.assertAlmostEqual(P.area(), P.width()*P.height())
        #TODO export EPSILON from 2geom
        if P.area() > 0:
            self.assertFalse(P.hasZeroArea())
        else:
            self.assertTrue(P.hasZeroArea())
        self.assertAlmostEqual(P.maxExtent(), max(P.width(), P.height()))
        self.assertGreaterEqual(P.maxExtent(), P.minExtent())
        
        qmin = Q.min()
        qmax = Q.max()
        
        #pdiag = (pmax-pmin).length()
        
        Q.setMin(P.midpoint())
        Q.setMax(P.midpoint())
        self.assertTrue(Q.hasZeroArea())
        
        #print P,Q
        Q.expandBy(P.minExtent()/3.0)
         
        #print P, Q
        
        self.assertTrue(P.contains(Q))
        self.assertTrue(P.intersects(Q))
        self.assertTrue(Q.intersects(P))
        self.assertFalse(Q.contains(P))

        self.assertTrue(P.contains(P.midpoint()))
        self.assertFalse(P.contains(P.midpoint()+P.midpoint()+P.midpoint()))
        
        P.unionWith(Q)
        
        self.assertEqual( P.min(), pmin )
        
        Q.setLeft(qmin.x)
        Q.setTop(qmin.y)
        Q.setRight(qmax.x)
        Q.setBottom(qmax.y)
        
        self.assertEqual(Q.min(), qmin)
        self.assertEqual(Q.max(), qmax)
        
        Q.expandTo( IntPoint() )
        self.assertEqual(IntPoint(), Q.min())
        Q.expandBy(qmax)
        self.assertEqual(qmax, IntPoint()-Q.min())
        
        Q.expandBy(-qmax.x, -qmax.y)
        self.assertEqual(Q.max(), qmax)
        
        self.assertEqual( (P+Q.min()).max(), P.max() + Q.min() )
        
        self.assertEqual( (P-Q.max()).min(), P.min() - Q.max() )
        
        self.assertEqual( P|P, P )
        
        self.assertFalse( P != P )

        Q.setLeft(qmin.x)
        Q.setTop(qmin.y)
        Q.setRight(qmax.x)
        Q.setBottom(qmax.y)



    def test_rect(self):
        P = Rect(0.298, 2, 4, 5)
        self.interval_basic(P[0], P[1])
        G = Rect(sqrt(2), sqrt(2), sqrt(3), sqrt(3))
        H = Rect.from_xywh(3.43232, 9.23214, 21.523, -0.31232)
        
        self.rect_basic(P, G)
        self.rect_basic(G, H)
        
        lst = [Point(randint(-100, 100), randint(-100, 100)) for i in range(10)]
   
        R = Rect.from_list(lst)
        
        for p in lst:
            self.assertTrue(R.contains(p))
            
        self.assertAlmostEqual(min(lst).y, R.min().y)
        self.assertAlmostEqual(max(lst).y, R.max().y)
        
        
        
    def test_optRect(self):
        P = OptRect(0.298, 2, 4, 5)
        self.interval_basic(P.Rect[0], P.Rect[1])
        
        
        G = Rect(sqrt(2), sqrt(2), sqrt(3), sqrt(3))
        H = OptRect.from_Rect(G)
        
        self.rect_basic(P.Rect, G)
        
        lst = [Point(randint(-100, 100), randint(-100, 100)) for i in range(10)]

        R = OptRect.from_list(lst)
        
        for p in lst:
            self.assertTrue(R.Rect.contains(p))
            
        self.assertAlmostEqual(min(lst).y, R.Rect.min().y)
        self.assertAlmostEqual(max(lst).y, R.Rect.max().y)
        
        Q = OptRect()
        self.assertFalse(Q)
        self.assertTrue(P)

        self.assertTrue(Q.isEmpty())
        self.assertFalse(P.isEmpty())

        self.assertTrue(P.contains( P ))
        self.assertTrue(P.contains(Q))
        self.assertFalse(Q.contains(P))
        self.assertFalse(P.intersects(Q))
        self.assertTrue(P.contains(P.Rect))
        self.assertTrue(P.contains(P.Rect.midpoint())) 
        
        self.assertEqual(P, OptRect.from_Rect(P))
        
        P.unionWith(G)
        P.unionWith(H)
        self.assertTrue(P.contains(H))
        
        P.intersectWith(G)
        self.assertEqual(P, G)
        
        self.assertEqual( P|H, G )
        self.assertEqual( (P|R).Rect.min().x , min( P.Rect.min().x, R.Rect.min().x ))
        
        self.assertFalse(P & Q)
        self.assertEqual(P, P&P)
        
        self.assertEqual( P & (R | H), (P & R) | (P & H)  )
        
    def test_intRect(self):
        A = IntRect(2, 6, 9, 23)
        B = IntRect(IntInterval(1, 5), IntInterval(8, 9))
        C = IntRect(IntPoint(1, 8), IntPoint(5, 9))
        
        self.assertEqual(B, C)
        
        self.intrect_basic(A, B)
        self.intrect_basic(B, C)
        self.intrect_basic(C, A)
        
    def test_optIntRect(self):
        P = OptIntRect(1, 2, 4, 5)
        self.interval_basic(P.Rect[0], P.Rect[1])
        
        
        G = IntRect(2, 2, 3, 3)
        H = OptIntRect.from_Rect(G)
        
        self.intrect_basic(P.Rect, G)
        
        lst = [IntPoint(randint(-100, 100), randint(-100, 100)) for i in range(10)]

        R = OptIntRect.from_list(lst)
        
        for p in lst:
            self.assertTrue(R.Rect.contains(p))
            
        self.assertAlmostEqual(min(lst).y, R.Rect.min().y)
        self.assertAlmostEqual(max(lst).y, R.Rect.max().y)
        
        Q = OptIntRect()
        self.assertFalse(Q)
        self.assertTrue(P)

        self.assertTrue(Q.isEmpty())
        self.assertFalse(P.isEmpty())

        self.assertTrue(P.contains( P ))
        self.assertTrue(P.contains( P.Rect ))
        self.assertTrue(P.contains(Q))
        self.assertFalse(Q.contains(P))
        self.assertFalse(P.intersects(Q))
        self.assertTrue(P.contains(P.Rect))
        self.assertTrue(P.contains(P.Rect.midpoint())) 
        
        self.assertEqual(P, OptIntRect.from_Rect(P))
        
        P.unionWith(G)
        P.unionWith(H)
        self.assertTrue(P.contains(H))
        
        P.intersectWith(G)
        self.assertEqual(P, G)
        
        self.assertEqual( P|H, G )
        self.assertEqual( (P|R).Rect.min().x , min( P.Rect.min().x, R.Rect.min().x ))
        
        self.assertFalse(P & Q)
        self.assertEqual(P, P&P)
        
        self.assertEqual( P & (R | H), (P & R) | (P & H)  )

unittest.main()
