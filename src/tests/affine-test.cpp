/** @file
 * @brief Unit tests for Affine
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
#include <2geom/affine.h>
#include <2geom/transforms.h>

namespace Geom {

TEST(AffineTest, Equality) {
    Affine e; // identity
    Affine a(1, 2, 3, 4, 5, 6);
    EXPECT_EQ(e, e);
    EXPECT_EQ(e, Geom::identity());
    EXPECT_EQ(e, Geom::Affine::identity());
    EXPECT_NE(e, a);
}

TEST(AffineTest, Classification) {
    {
        Affine a; // identity
        EXPECT_TRUE(a.isIdentity());
        EXPECT_TRUE(a.isTranslation());
        EXPECT_TRUE(a.isScale());
        EXPECT_TRUE(a.isUniformScale());
        EXPECT_TRUE(a.isRotation());
        EXPECT_TRUE(a.isHShear());
        EXPECT_TRUE(a.isVShear());
        EXPECT_TRUE(a.isZoom());
        EXPECT_FALSE(a.isNonzeroTranslation());
        EXPECT_FALSE(a.isNonzeroScale());
        EXPECT_FALSE(a.isNonzeroUniformScale());
        EXPECT_FALSE(a.isNonzeroRotation());
        EXPECT_FALSE(a.isNonzeroNonpureRotation());
        EXPECT_FALSE(a.isNonzeroHShear());
        EXPECT_FALSE(a.isNonzeroVShear());
        EXPECT_TRUE(a.preservesArea());
        EXPECT_TRUE(a.preservesAngles());
        EXPECT_TRUE(a.preservesDistances());
        EXPECT_FALSE(a.flips());
        EXPECT_FALSE(a.isSingular());
    }
    {
        Affine a = Translate(10, 15); // pure translation
        EXPECT_FALSE(a.isIdentity());
        EXPECT_TRUE(a.isTranslation());
        EXPECT_FALSE(a.isScale());
        EXPECT_FALSE(a.isUniformScale());
        EXPECT_FALSE(a.isRotation());
        EXPECT_FALSE(a.isHShear());
        EXPECT_FALSE(a.isVShear());
        EXPECT_TRUE(a.isZoom());
        EXPECT_TRUE(a.isNonzeroTranslation());
        EXPECT_FALSE(a.isNonzeroScale());
        EXPECT_FALSE(a.isNonzeroUniformScale());
        EXPECT_FALSE(a.isNonzeroRotation());
        EXPECT_FALSE(a.isNonzeroNonpureRotation());
        EXPECT_FALSE(a.isNonzeroHShear());
        EXPECT_FALSE(a.isNonzeroVShear());
        EXPECT_TRUE(a.preservesArea());
        EXPECT_TRUE(a.preservesAngles());
        EXPECT_TRUE(a.preservesDistances());
        EXPECT_FALSE(a.flips());
        EXPECT_FALSE(a.isSingular());
    }
    {
        Affine a = Scale(-1.0, 1.0); // flip on the X axis
        EXPECT_FALSE(a.isIdentity());
        EXPECT_FALSE(a.isTranslation());
        EXPECT_TRUE(a.isScale());
        EXPECT_TRUE(a.isUniformScale());
        EXPECT_FALSE(a.isRotation());
        EXPECT_FALSE(a.isHShear());
        EXPECT_FALSE(a.isVShear());
        EXPECT_FALSE(a.isZoom()); // zoom must be non-flipping
        EXPECT_FALSE(a.isNonzeroTranslation());
        EXPECT_TRUE(a.isNonzeroScale());
        EXPECT_TRUE(a.isNonzeroUniformScale());
        EXPECT_FALSE(a.isNonzeroRotation());
        EXPECT_FALSE(a.isNonzeroNonpureRotation());
        EXPECT_FALSE(a.isNonzeroHShear());
        EXPECT_FALSE(a.isNonzeroVShear());
        EXPECT_TRUE(a.preservesArea());
        EXPECT_TRUE(a.preservesAngles());
        EXPECT_TRUE(a.preservesDistances());
        EXPECT_TRUE(a.flips());
        EXPECT_FALSE(a.isSingular());
    }
    {
        Affine a = Scale(0.5, 0.5); // pure uniform scale
        EXPECT_FALSE(a.isIdentity());
        EXPECT_FALSE(a.isTranslation());
        EXPECT_TRUE(a.isScale());
        EXPECT_TRUE(a.isUniformScale());
        EXPECT_FALSE(a.isRotation());
        EXPECT_FALSE(a.isHShear());
        EXPECT_FALSE(a.isVShear());
        EXPECT_TRUE(a.isZoom());
        EXPECT_FALSE(a.isNonzeroTranslation());
        EXPECT_TRUE(a.isNonzeroScale());
        EXPECT_TRUE(a.isNonzeroUniformScale());
        EXPECT_FALSE(a.isNonzeroRotation());
        EXPECT_FALSE(a.isNonzeroNonpureRotation());
        EXPECT_FALSE(a.isNonzeroHShear());
        EXPECT_FALSE(a.isNonzeroVShear());
        EXPECT_FALSE(a.preservesArea());
        EXPECT_TRUE(a.preservesAngles());
        EXPECT_FALSE(a.preservesDistances());
        EXPECT_FALSE(a.flips());
        EXPECT_FALSE(a.isSingular());
    }
    {
        Affine a = Scale(0.5, -0.5); // pure uniform flipping scale
        EXPECT_FALSE(a.isIdentity());
        EXPECT_FALSE(a.isTranslation());
        EXPECT_TRUE(a.isScale());
        EXPECT_TRUE(a.isUniformScale());
        EXPECT_FALSE(a.isRotation());
        EXPECT_FALSE(a.isHShear());
        EXPECT_FALSE(a.isVShear());
        EXPECT_FALSE(a.isZoom()); // zoom must be non-flipping
        EXPECT_FALSE(a.isNonzeroTranslation());
        EXPECT_TRUE(a.isNonzeroScale());
        EXPECT_TRUE(a.isNonzeroUniformScale());
        EXPECT_FALSE(a.isNonzeroRotation());
        EXPECT_FALSE(a.isNonzeroNonpureRotation());
        EXPECT_FALSE(a.isNonzeroHShear());
        EXPECT_FALSE(a.isNonzeroVShear());
        EXPECT_FALSE(a.preservesArea());
        EXPECT_TRUE(a.preservesAngles());
        EXPECT_FALSE(a.preservesDistances());
        EXPECT_TRUE(a.flips());
        EXPECT_FALSE(a.isSingular());
    }
    {
        Affine a = Scale(0.5, 0.7); // pure non-uniform scale
        EXPECT_FALSE(a.isIdentity());
        EXPECT_FALSE(a.isTranslation());
        EXPECT_TRUE(a.isScale());
        EXPECT_FALSE(a.isUniformScale());
        EXPECT_FALSE(a.isRotation());
        EXPECT_FALSE(a.isHShear());
        EXPECT_FALSE(a.isVShear());
        EXPECT_FALSE(a.isZoom());
        EXPECT_FALSE(a.isNonzeroTranslation());
        EXPECT_TRUE(a.isNonzeroScale());
        EXPECT_FALSE(a.isNonzeroUniformScale());
        EXPECT_FALSE(a.isNonzeroRotation());
        EXPECT_FALSE(a.isNonzeroNonpureRotation());
        EXPECT_FALSE(a.isNonzeroHShear());
        EXPECT_FALSE(a.isNonzeroVShear());
        EXPECT_FALSE(a.preservesArea());
        EXPECT_FALSE(a.preservesAngles());
        EXPECT_FALSE(a.preservesDistances());
        EXPECT_FALSE(a.flips());
        EXPECT_FALSE(a.isSingular());
    }
    {
        Affine a = Scale(0.5, 2.0); // "squeeze" transform (non-uniform scale with det=1)
        EXPECT_FALSE(a.isIdentity());
        EXPECT_FALSE(a.isTranslation());
        EXPECT_TRUE(a.isScale());
        EXPECT_FALSE(a.isUniformScale());
        EXPECT_FALSE(a.isRotation());
        EXPECT_FALSE(a.isHShear());
        EXPECT_FALSE(a.isVShear());
        EXPECT_FALSE(a.isZoom());
        EXPECT_FALSE(a.isNonzeroTranslation());
        EXPECT_TRUE(a.isNonzeroScale());
        EXPECT_FALSE(a.isNonzeroUniformScale());
        EXPECT_FALSE(a.isNonzeroRotation());
        EXPECT_FALSE(a.isNonzeroNonpureRotation());
        EXPECT_FALSE(a.isNonzeroHShear());
        EXPECT_FALSE(a.isNonzeroVShear());
        EXPECT_TRUE(a.preservesArea());
        EXPECT_FALSE(a.preservesAngles());
        EXPECT_FALSE(a.preservesDistances());
        EXPECT_FALSE(a.flips());
        EXPECT_FALSE(a.isSingular());
    }
    {
        Affine a = Rotate(0.7); // pure rotation
        EXPECT_FALSE(a.isIdentity());
        EXPECT_FALSE(a.isTranslation());
        EXPECT_FALSE(a.isScale());
        EXPECT_FALSE(a.isUniformScale());
        EXPECT_TRUE(a.isRotation());
        EXPECT_FALSE(a.isHShear());
        EXPECT_FALSE(a.isVShear());
        EXPECT_FALSE(a.isZoom());
        EXPECT_FALSE(a.isNonzeroTranslation());
        EXPECT_FALSE(a.isNonzeroScale());
        EXPECT_FALSE(a.isNonzeroUniformScale());
        EXPECT_TRUE(a.isNonzeroRotation());
        EXPECT_TRUE(a.isNonzeroNonpureRotation());
        EXPECT_EQ(a.rotationCenter(), Point(0.0,0.0));
        EXPECT_FALSE(a.isNonzeroHShear());
        EXPECT_FALSE(a.isNonzeroVShear());
        EXPECT_TRUE(a.preservesArea());
        EXPECT_TRUE(a.preservesAngles());
        EXPECT_TRUE(a.preservesDistances());
        EXPECT_FALSE(a.flips());
        EXPECT_FALSE(a.isSingular());
    }
    {
        Point rotation_center(1.23,4.56);
        Affine a = Translate(-rotation_center) * Rotate(0.7) * Translate(rotation_center); // rotation around (1.23,4.56)
        EXPECT_FALSE(a.isIdentity());
        EXPECT_FALSE(a.isTranslation());
        EXPECT_FALSE(a.isScale());
        EXPECT_FALSE(a.isUniformScale());
        EXPECT_FALSE(a.isRotation());
        EXPECT_FALSE(a.isHShear());
        EXPECT_FALSE(a.isVShear());
        EXPECT_FALSE(a.isZoom());
        EXPECT_FALSE(a.isNonzeroTranslation());
        EXPECT_FALSE(a.isNonzeroScale());
        EXPECT_FALSE(a.isNonzeroUniformScale());
        EXPECT_FALSE(a.isNonzeroRotation());
        EXPECT_TRUE(a.isNonzeroNonpureRotation());
        EXPECT_TRUE(are_near(a.rotationCenter(), rotation_center, 1e-7));
        EXPECT_FALSE(a.isNonzeroHShear());
        EXPECT_FALSE(a.isNonzeroVShear());
        EXPECT_TRUE(a.preservesArea());
        EXPECT_TRUE(a.preservesAngles());
        EXPECT_TRUE(a.preservesDistances());
        EXPECT_FALSE(a.flips());
        EXPECT_FALSE(a.isSingular());
    }
    {
        Affine a = HShear(0.5); // pure horizontal shear
        EXPECT_FALSE(a.isIdentity());
        EXPECT_FALSE(a.isTranslation());
        EXPECT_FALSE(a.isScale());
        EXPECT_FALSE(a.isUniformScale());
        EXPECT_FALSE(a.isRotation());
        EXPECT_TRUE(a.isHShear());
        EXPECT_FALSE(a.isVShear());
        EXPECT_FALSE(a.isZoom());
        EXPECT_FALSE(a.isNonzeroTranslation());
        EXPECT_FALSE(a.isNonzeroScale());
        EXPECT_FALSE(a.isNonzeroUniformScale());
        EXPECT_FALSE(a.isNonzeroRotation());
        EXPECT_FALSE(a.isNonzeroNonpureRotation());
        EXPECT_TRUE(a.isNonzeroHShear());
        EXPECT_FALSE(a.isNonzeroVShear());
        EXPECT_TRUE(a.preservesArea());
        EXPECT_FALSE(a.preservesAngles());
        EXPECT_FALSE(a.preservesDistances());
        EXPECT_FALSE(a.flips());
        EXPECT_FALSE(a.isSingular());
    }
    {
        Affine a = VShear(0.5); // pure vertical shear
        EXPECT_FALSE(a.isIdentity());
        EXPECT_FALSE(a.isTranslation());
        EXPECT_FALSE(a.isScale());
        EXPECT_FALSE(a.isUniformScale());
        EXPECT_FALSE(a.isRotation());
        EXPECT_FALSE(a.isHShear());
        EXPECT_TRUE(a.isVShear());
        EXPECT_FALSE(a.isZoom());
        EXPECT_FALSE(a.isNonzeroTranslation());
        EXPECT_FALSE(a.isNonzeroScale());
        EXPECT_FALSE(a.isNonzeroUniformScale());
        EXPECT_FALSE(a.isNonzeroRotation());
        EXPECT_FALSE(a.isNonzeroNonpureRotation());
        EXPECT_FALSE(a.isNonzeroHShear());
        EXPECT_TRUE(a.isNonzeroVShear());
        EXPECT_TRUE(a.preservesArea());
        EXPECT_FALSE(a.preservesAngles());
        EXPECT_FALSE(a.preservesDistances());
        EXPECT_FALSE(a.flips());
        EXPECT_FALSE(a.isSingular());
    }
    {
        Affine a = Zoom(3.0, Translate(10, 15)); // zoom
        EXPECT_FALSE(a.isIdentity());
        EXPECT_FALSE(a.isTranslation());
        EXPECT_FALSE(a.isScale());
        EXPECT_FALSE(a.isUniformScale());
        EXPECT_FALSE(a.isRotation());
        EXPECT_FALSE(a.isHShear());
        EXPECT_FALSE(a.isVShear());
        EXPECT_TRUE(a.isZoom());
        EXPECT_FALSE(a.isNonzeroTranslation());
        EXPECT_FALSE(a.isNonzeroScale());
        EXPECT_FALSE(a.isNonzeroUniformScale());
        EXPECT_FALSE(a.isNonzeroRotation());
        EXPECT_FALSE(a.isNonzeroNonpureRotation());
        EXPECT_FALSE(a.isNonzeroHShear());
        EXPECT_FALSE(a.isNonzeroVShear());
        EXPECT_FALSE(a.preservesArea());
        EXPECT_TRUE(a.preservesAngles());
        EXPECT_FALSE(a.preservesDistances());
        EXPECT_FALSE(a.flips());
        EXPECT_FALSE(a.isSingular());
        
        EXPECT_TRUE(a.withoutTranslation().isUniformScale());
        EXPECT_TRUE(a.withoutTranslation().isNonzeroUniformScale());
    }
    {
        Affine a(0, 0, 0, 0, 0, 0); // zero matrix (singular)
        EXPECT_FALSE(a.isIdentity());
        EXPECT_FALSE(a.isTranslation());
        EXPECT_FALSE(a.isScale());
        EXPECT_FALSE(a.isUniformScale());
        EXPECT_FALSE(a.isRotation());
        EXPECT_FALSE(a.isHShear());
        EXPECT_FALSE(a.isVShear());
        EXPECT_FALSE(a.isZoom());
        EXPECT_FALSE(a.isNonzeroTranslation());
        EXPECT_FALSE(a.isNonzeroScale());
        EXPECT_FALSE(a.isNonzeroUniformScale());
        EXPECT_FALSE(a.isNonzeroRotation());
        EXPECT_FALSE(a.isNonzeroNonpureRotation());
        EXPECT_FALSE(a.isNonzeroHShear());
        EXPECT_FALSE(a.isNonzeroVShear());
        EXPECT_FALSE(a.preservesArea());
        EXPECT_FALSE(a.preservesAngles());
        EXPECT_FALSE(a.preservesDistances());
        EXPECT_FALSE(a.flips());
        EXPECT_TRUE(a.isSingular());
    }
    {
        Affine a(0, 1, 0, 1, 10, 10); // another singular matrix
        EXPECT_FALSE(a.isIdentity());
        EXPECT_FALSE(a.isTranslation());
        EXPECT_FALSE(a.isScale());
        EXPECT_FALSE(a.isUniformScale());
        EXPECT_FALSE(a.isRotation());
        EXPECT_FALSE(a.isHShear());
        EXPECT_FALSE(a.isVShear());
        EXPECT_FALSE(a.isZoom());
        EXPECT_FALSE(a.isNonzeroTranslation());
        EXPECT_FALSE(a.isNonzeroScale());
        EXPECT_FALSE(a.isNonzeroUniformScale());
        EXPECT_FALSE(a.isNonzeroRotation());
        EXPECT_FALSE(a.isNonzeroNonpureRotation());
        EXPECT_FALSE(a.isNonzeroHShear());
        EXPECT_FALSE(a.isNonzeroVShear());
        EXPECT_FALSE(a.preservesArea());
        EXPECT_FALSE(a.preservesAngles());
        EXPECT_FALSE(a.preservesDistances());
        EXPECT_FALSE(a.flips());
        EXPECT_TRUE(a.isSingular());
    }
}

TEST(AffineTest, Inversion) {
    Affine i(1, 2, 1, -2, 10, 15); // invertible
    Affine n(1, 2, 1, 2, 15, 30); // non-invertible
    Affine e; // identity
    EXPECT_EQ(i * i.inverse(), e);
    EXPECT_EQ(i.inverse().inverse(), i);
    EXPECT_EQ(n.inverse(), e);
    EXPECT_EQ(e.inverse(), e);
}

TEST(AffineTest, CoordinateAccess) {
    Affine a(0, 1, 2, 3, 4, 5);
    for (int i=0; i<6; ++i) {
        EXPECT_EQ(a[i], i);
    }
    for (int i=0; i<6; ++i) {
        a[i] = 5*i;
    }
    for (int i=0; i<6; ++i) {
        EXPECT_EQ(a[i], 5*i);
    }
}

TEST(AffineTest, Nearness) {
    Affine a1(1, 0, 1, 2, 1e-8, 1e-8);
    Affine a2(1+1e-8, 0, 1, 2-1e-8, -1e-8, -1e-8);
    EXPECT_TRUE(are_near(a1, a2, 1e-7));
    EXPECT_FALSE(are_near(a1, a2, 1e-9));
}

TEST(AffineTest, Multiplication) {
    // test whether noncommutative multiplications work correctly
    Affine a1 = Scale(0.1), a2 = Translate(10, 10), a3 = Scale(10.0);
    Affine t1 = Translate(1, 1), t100 = Translate(100, 100);
    EXPECT_EQ(a1 * a2 * a3, t100);
    EXPECT_EQ(a3 * a2 * a1, t1);
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
