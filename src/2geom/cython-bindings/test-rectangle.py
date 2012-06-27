import unittest
from math import pi, sqrt

import cy2geom
from cy2geom import Interval, IntInterval, OptInterval, OptIntInterval

class TestPrimitives(unittest.TestCase):
    def interval_basic(self, I, J):
        #for simplicity
        self.assertTrue(I.min() >= 0)
        self.assertTrue(J.min() >= 0)
        a = I.min()
        b = I.max()
        self.assertAlmostEqual(I.middle(), (a+b)/2)
        self.assertAlmostEqual(I.extent(), (b-a))
        if a != b:
            self.assertFalse(I.isSingular())
        else:
            self.assertTrue(I.isSingular())
        I.expandBy(a)
        self.assertAlmostEqual(I.min(), 0)
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
        self.assertEqual(K.IntInterval, J)
        self.interval_basic(K.IntInterval, I.IntInterval)
        
        L = OptIntInterval()
        
        self.assertFalse(L)
        self.assertTrue( (L&I).isEmpty() )
        L.intersectWith(I)
        self.assertFalse(L)
        
        L |= I
        
        self.assertEqual(L.IntInterval, I.IntInterval)
        
        self.assertEqual((I & K).IntInterval, IntInterval(3, 9))
        
unittest.main()
