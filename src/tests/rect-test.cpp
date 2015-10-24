/** @file
 * @brief Unit tests for Rect, OptRect, IntRect, and OptIntRect.
 * Uses the Google Testing Framework
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2010 Authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

#include <gtest/gtest.h>
#include <2geom/coord.h>
#include <2geom/rect.h>

namespace Geom {

typedef ::testing::Types<Coord, IntCoord> CoordTypes;

TEST(RectTest, Upconversion) {
    IntRect ir(0, -27, 10, 202);
    Rect r_a(ir);
    Rect r_b = ir;
    OptIntRect oir_a(ir);
    OptIntRect oir_b = ir;
    OptRect or_a(oir_a);
    OptRect or_b = oir_b;

    EXPECT_EQ(r_a, ir);
    EXPECT_EQ(r_a, r_b);
    EXPECT_EQ(r_a, *oir_a);
    EXPECT_EQ(r_a, *oir_b);
    EXPECT_EQ(r_a, *or_a);
    EXPECT_EQ(r_a, *or_b);
    EXPECT_EQ(oir_a, oir_b);
    EXPECT_EQ(or_a, or_b);
}

TEST(RectTest, Rounding) {
    Rect r(-0.5, -0.5, 5.5, 5.5);
    Rect r_small(0.3, 0.0, 0.6, 10.0);
    Rect r_int(0,0,10,10);
    IntRect out(-1, -1, 6, 6);
    IntRect out_small(0, 0, 1, 10);
    IntRect out_int(0,0,10,10);
    OptIntRect in = IntRect(0, 0, 5, 5);
    EXPECT_EQ(r.roundOutwards(), out);
    EXPECT_EQ(r_small.roundOutwards(), out_small);
    EXPECT_EQ(r_int.roundOutwards(), out_int);
    EXPECT_EQ(r.roundInwards(), in);
    EXPECT_EQ(r_small.roundInwards(), OptIntRect());
}

template <typename C>
class GenericRectTest : public ::testing::Test {
public:
    typedef typename CoordTraits<C>::PointType CPoint;
    typedef typename CoordTraits<C>::RectType CRect;
    typedef typename CoordTraits<C>::OptRectType OptCRect;
    CRect a, a2, b, c, d;
    CRect int_ab, int_bc, uni_ab, uni_bc;
    GenericRectTest()
        : a(0, 0, 10, 10)
        , a2(0, 0, 10, 10)
        , b(-5, -5, 5, 5)
        , c(-10, -10, -1, -1)
        , d(1, 1, 9, 9)
        , int_ab(0, 0, 5, 5)
        , int_bc(-5, -5, -1, -1)
        , uni_ab(-5, -5, 10, 10)
        , uni_bc(-10, -10, 5, 5)
    {}
};

TYPED_TEST_CASE(GenericRectTest, CoordTypes);

TYPED_TEST(GenericRectTest, EqualityTest) {
    typename TestFixture::CRect a(0, 0, 10, 10), a2(a), b(-5, -5, 5, 5);
    typename TestFixture::OptCRect empty, oa = a;

    EXPECT_TRUE (a == a);
    EXPECT_FALSE(a != a);
    EXPECT_TRUE (a == a2);
    EXPECT_FALSE(a != a2);
    EXPECT_TRUE (empty == empty);
    EXPECT_FALSE(empty != empty);
    EXPECT_FALSE(a == empty);
    EXPECT_TRUE (a != empty);
    EXPECT_FALSE(empty == a);
    EXPECT_TRUE (empty != a);
    EXPECT_FALSE(a == b);
    EXPECT_TRUE (a != b);
    EXPECT_TRUE (a == oa);
    EXPECT_FALSE(a != oa);
}

TYPED_TEST(GenericRectTest, Intersects) {
    typename TestFixture::CRect a(0, 0, 10, 10), b(-5, -5, 5, 5), c(-10, -10, -1, -1), d(1, 1, 9, 9);
    typename TestFixture::OptCRect empty, oa(a), oc(c), od(d);
    EXPECT_TRUE(a.intersects(a));
    EXPECT_TRUE(a.intersects(b));
    EXPECT_TRUE(b.intersects(a));
    EXPECT_TRUE(b.intersects(c));
    EXPECT_TRUE(c.intersects(b));
    EXPECT_TRUE(a.intersects(d));
    EXPECT_TRUE(d.intersects(a));
    EXPECT_FALSE(a.intersects(c));
    EXPECT_FALSE(c.intersects(a));
    EXPECT_FALSE(c.intersects(d));
    EXPECT_FALSE(empty.intersects(empty));
    EXPECT_FALSE(empty.intersects(oa));
    EXPECT_FALSE(oa.intersects(empty));
    EXPECT_TRUE(oa.intersects(od));
    EXPECT_FALSE(oa.intersects(oc));
}

/**
 JonCruz failure: (10, 20)-(55,30) and (45,20)-(100,30) should intersect.
*/

TYPED_TEST(GenericRectTest, JonCruzRect) {
    typename TestFixture::CRect a(10, 20, 55, 30), b(45, 20, 100,30);
    typename TestFixture::OptCRect empty, oa(a), ob(b);
    EXPECT_TRUE(a.intersects(a));
    EXPECT_TRUE(a.intersects(b));
    EXPECT_TRUE(b.intersects(a));
    EXPECT_TRUE(oa.intersects(oa));
    EXPECT_TRUE(oa.intersects(ob));
    EXPECT_TRUE(ob.intersects(oa));
}

TYPED_TEST(GenericRectTest, Intersection) {
    typename TestFixture::CRect a(0, 0, 10, 10), b(-5, -5, 5, 5), c(-10, -10, -1, -1), d(1, 1, 9, 9);
    typename TestFixture::CRect int_ab(0, 0, 5, 5), int_bc(-5, -5, -1, -1);
    typename TestFixture::OptCRect empty, oa(a), ob(b);
    
    EXPECT_EQ(a & a, a);
    EXPECT_EQ(a & b, int_ab);
    EXPECT_EQ(b & c, int_bc);
    EXPECT_EQ(intersect(b, c), int_bc);
    EXPECT_EQ(intersect(a, a), a);
    EXPECT_EQ(a & c, empty);
    EXPECT_EQ(a & d, d);
    EXPECT_EQ(a & empty, empty);
    EXPECT_EQ(empty & empty, empty);

    oa &= ob;
    EXPECT_EQ(oa, int_ab);
    oa = a;
    oa &= b;
    EXPECT_EQ(oa, int_ab);
    oa = a;
    oa &= empty;
    EXPECT_EQ(oa, empty);
}

TYPED_TEST(GenericRectTest, Contains) {
    typename TestFixture::CRect a(0, 0, 10, 10), b(-5, -5, 5, 5), c(-10, -10, -1, -1), d(1, 1, 9, 9);
    typename TestFixture::CRect int_ab(0, 0, 5, 5), int_bc(-5, -5, -1, -1);
    typename TestFixture::OptCRect empty, oa(a), od(d);
    EXPECT_TRUE(a.contains(a));
    EXPECT_FALSE(a.contains(b));
    EXPECT_FALSE(b.contains(a));
    EXPECT_FALSE(a.contains(c));
    EXPECT_FALSE(c.contains(a));
    EXPECT_TRUE(a.contains(d));
    EXPECT_FALSE(d.contains(a));
    EXPECT_TRUE(a.contains(int_ab));
    EXPECT_TRUE(b.contains(int_ab));
    EXPECT_TRUE(b.contains(int_bc));
    EXPECT_TRUE(c.contains(int_bc));
    EXPECT_FALSE(int_ab.contains(a));
    EXPECT_FALSE(int_ab.contains(b));
    EXPECT_FALSE(int_bc.contains(b));
    EXPECT_FALSE(int_bc.contains(c));
    EXPECT_FALSE(empty.contains(empty));
    EXPECT_FALSE(empty.contains(od));
    EXPECT_TRUE(oa.contains(empty));
    EXPECT_TRUE(oa.contains(od));
    EXPECT_FALSE(od.contains(oa));
}

TYPED_TEST(GenericRectTest, Union) {
    typename TestFixture::CRect a(0, 0, 10, 10), old_a(a), b(-5, -5, 5, 5), c(-10, -10, -1, -1), d(1, 1, 9, 9);
    typename TestFixture::CRect int_ab(0, 0, 5, 5), int_bc(-5, -5, -1, -1);
    typename TestFixture::CRect uni_ab(-5, -5, 10, 10), uni_bc(-10, -10, 5, 5);
    typename TestFixture::OptCRect empty, oa(a), ob(b);
    EXPECT_EQ(a | b, uni_ab);
    EXPECT_EQ(b | c, uni_bc);
    EXPECT_EQ(a | a, a);
    EXPECT_EQ(a | d, a);
    EXPECT_EQ(a | int_ab, a);
    EXPECT_EQ(b | int_ab, b);
    EXPECT_EQ(uni_ab | a, uni_ab);
    EXPECT_EQ(uni_bc | c, uni_bc);
    EXPECT_EQ(a | empty, a);
    EXPECT_EQ(empty | empty, empty);

    a |= b;
    EXPECT_EQ(a, uni_ab);
    a = old_a;
    a |= ob;
    EXPECT_EQ(a, uni_ab);
    a = old_a;
    a |= empty;
    EXPECT_EQ(a, old_a);
    oa |= ob;
    EXPECT_EQ(oa, uni_ab);
    oa = old_a;
    oa |= b;
    EXPECT_EQ(oa, uni_ab);
}

TYPED_TEST(GenericRectTest, Area) {
    typename TestFixture::CRect a(0, 0, 10, 10), b(-5, -5, 5, 5), c(-10, -10, -1, -1), d(1, 1, 9, 9);
    typename TestFixture::CRect zero(0,0,0,0);
    EXPECT_EQ(a.area(), 100);
    EXPECT_EQ(a.area(), a.width() * a.height());
    EXPECT_EQ(b.area(), 100);
    EXPECT_EQ(c.area(), 81);
    EXPECT_EQ(d.area(), 64);
    EXPECT_FALSE(a.hasZeroArea());
    EXPECT_TRUE(zero.hasZeroArea());
}

TYPED_TEST(GenericRectTest, Emptiness) {
    typename TestFixture::OptCRect empty, oa(0, 0, 10, 10);
    EXPECT_TRUE(empty.empty());
    EXPECT_FALSE(empty);
    EXPECT_TRUE(!empty);
    EXPECT_FALSE(oa.empty());
    EXPECT_TRUE(oa);
    EXPECT_FALSE(!oa);
}

TYPED_TEST(GenericRectTest, Dimensions) {
    typedef typename TestFixture::CPoint CPoint;
    typename TestFixture::CRect a(-10, -20, 10, 20), b(-15, 30, 45, 90);
    EXPECT_EQ(a.width(), 20);
    EXPECT_EQ(a.height(), 40);
    EXPECT_EQ(a.left(), -10);
    EXPECT_EQ(a.top(), -20);
    EXPECT_EQ(a.right(), 10);
    EXPECT_EQ(a.bottom(), 20);
    EXPECT_EQ(a.min(), CPoint(-10, -20));
    EXPECT_EQ(a.max(), CPoint(10, 20));
    EXPECT_EQ(a.minExtent(), a.width());
    EXPECT_EQ(a.maxExtent(), a.height());
    EXPECT_EQ(a.dimensions(), CPoint(20, 40));
    EXPECT_EQ(a.midpoint(), CPoint(0, 0));

    EXPECT_EQ(b.width(), 60);
    EXPECT_EQ(b.height(), 60);
    EXPECT_EQ(b.left(), -15);
    EXPECT_EQ(b.top(), 30);
    EXPECT_EQ(b.right(), 45);
    EXPECT_EQ(b.bottom(), 90);
    EXPECT_EQ(b.min(), CPoint(-15, 30));
    EXPECT_EQ(b.max(), CPoint(45, 90));
    EXPECT_EQ(b.minExtent(), b.maxExtent());
    EXPECT_EQ(b.dimensions(), CPoint(60, 60));
    EXPECT_EQ(b.midpoint(), CPoint(15, 60));
}

TYPED_TEST(GenericRectTest, Modification) {
    typedef typename TestFixture::CRect CRect;
    typedef typename TestFixture::OptCRect OptCRect;
    typedef typename TestFixture::CPoint CPoint;
    CRect a(-1, -1, 1, 1);
    a.expandBy(9);
    EXPECT_EQ(a, CRect(-10, -10, 10, 10));
    a.setMin(CPoint(0, 0));
    EXPECT_EQ(a, CRect(0, 0, 10, 10));
    a.setMax(CPoint(20, 30));
    EXPECT_EQ(a, CRect(0, 0, 20, 30));
    a.setMax(CPoint(-5, -5));
    EXPECT_EQ(a, CRect(-5, -5, -5, -5));
    a.expandTo(CPoint(5, 5));
    EXPECT_EQ(a, CRect(-5, -5, 5, 5));
    a.expandTo(CPoint(0, 0));
    EXPECT_EQ(a, CRect(-5, -5, 5, 5));
    a.expandTo(CPoint(0, 15));
    EXPECT_EQ(a, CRect(-5, -5, 5, 15));
    a.expandBy(-10);
    EXPECT_EQ(a, CRect(0, 5, 0, 5));
    EXPECT_EQ(a.midpoint(), CPoint(0, 5));
    a.unionWith(CRect(-20, 0, -10, 20));
    EXPECT_EQ(a, CRect(-20, 0, 0, 20));
    OptCRect oa(a);
    oa.intersectWith(CRect(-10, -5, 5, 15));
    EXPECT_EQ(oa, OptCRect(-10, 0, 0, 15));
}

TYPED_TEST(GenericRectTest, OptRectDereference) {
    typename TestFixture::CRect a(0, 0, 5, 5);
    typename TestFixture::OptCRect oa(0, 0, 10, 10);
    EXPECT_NE(a, oa);
    a = *oa;
    EXPECT_EQ(a, oa);
}

TYPED_TEST(GenericRectTest, Offset) {
    typename TestFixture::CRect a(0, 0, 5, 5), old_a(a), app1(-5, 0, 0, 5), amp1(5, 0, 10, 5),
        app2(5, -10, 10, -5), amp2(-5, 10, 0, 15);
    typename TestFixture::CPoint p1(-5, 0), p2(5, -10);
    EXPECT_EQ(a + p1, app1);
    EXPECT_EQ(a + p2, app2);
    EXPECT_EQ(a - p1, amp1);
    EXPECT_EQ(a - p2, amp2);

    a += p1;
    EXPECT_EQ(a, app1);
    a = old_a;
    a += p2;
    EXPECT_EQ(a, app2);
    a = old_a;
    a -= p1;
    EXPECT_EQ(a, amp1);
    a = old_a;
    a -= p2;
    EXPECT_EQ(a, amp2);
}

TYPED_TEST(GenericRectTest, NearestEdgePoint) {
    typename TestFixture::CRect a(0, 0, 10, 10);
    typename TestFixture::CPoint p1(-5, 5), p2(15, 17), p3(6, 5),  p4(3, 9);
    typename TestFixture::CPoint r1(0, 5),  r2(10, 10), r3(10, 5), r4(3, 10);

    EXPECT_EQ(a.nearestEdgePoint(p1), r1);
    EXPECT_EQ(a.nearestEdgePoint(p2), r2);
    EXPECT_EQ(a.nearestEdgePoint(p3), r3);
    EXPECT_EQ(a.nearestEdgePoint(p4), r4);
}

} // end namespace Geom

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
