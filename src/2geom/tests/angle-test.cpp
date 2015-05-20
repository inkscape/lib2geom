/** @file
 * @brief Unit tests for Angle and AngleInterval.
 * Uses the Google Testing Framework
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2015 Authors
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

#include "testing.h"
#include <2geom/angle.h>
#include <glib.h>

using namespace Geom;

TEST(AngleTest, Containment) {
    AngleInterval a(0, M_PI, true);
    AngleInterval b(0, M_PI, false);
    AngleInterval c(M_PI, 0, true);
    AngleInterval d(M_PI, 0, false);

    EXPECT_TRUE(a.contains(1.));
    EXPECT_FALSE(a.contains(5.));
    EXPECT_EQ(a.extent(), M_PI);

    EXPECT_FALSE(b.contains(1.));
    EXPECT_TRUE(b.contains(5.));
    EXPECT_EQ(b.extent(), M_PI);

    EXPECT_FALSE(c.contains(1.));
    EXPECT_TRUE(c.contains(5.));
    EXPECT_EQ(c.extent(), M_PI);

    EXPECT_TRUE(d.contains(1.));
    EXPECT_FALSE(d.contains(5.));
    EXPECT_EQ(d.extent(), M_PI);
}

TEST(AngleTest, IntervalTimeAtAngle) {
    Coord pi32 = (3./2.)*M_PI;
    AngleInterval a(M_PI, pi32, true);
    AngleInterval b(pi32, M_PI, true);
    AngleInterval c(M_PI, 0, false);
    AngleInterval d(M_PI/2, M_PI, false);

    EXPECT_EQ(a.timeAtAngle(M_PI), 0);
    EXPECT_EQ(a.timeAtAngle(pi32), 1);
    EXPECT_EQ(a.extent(), M_PI/2);
    for (Coord t = -1; t <= 2; t += 0.125) {
        EXPECT_FLOAT_EQ(a.timeAtAngle(lerp(t, M_PI, pi32)), t);
    }

    EXPECT_EQ(b.timeAtAngle(pi32), 0);
    EXPECT_EQ(b.timeAtAngle(M_PI), 1);
    EXPECT_EQ(b.extent(), pi32);
    EXPECT_FLOAT_EQ(b.timeAtAngle(M_PI/4), 0.5);
    EXPECT_FLOAT_EQ(b.timeAtAngle(0), 1./3.);
    EXPECT_FLOAT_EQ(b.timeAtAngle((11./8)*M_PI), -1./12);
    for (Coord t = -0.125; t <= 1.125; t += 0.0625) {
        EXPECT_FLOAT_EQ(b.timeAtAngle(lerp(t, pi32, 3*M_PI)), t);
    }

    EXPECT_EQ(c.timeAtAngle(M_PI), 0);
    EXPECT_EQ(c.timeAtAngle(0), 1);
    EXPECT_EQ(c.extent(), M_PI);
    EXPECT_FLOAT_EQ(c.timeAtAngle(M_PI/2), 0.5);
    for (Coord t = -0.25; t <= 1.25; t += 0.125) {
        EXPECT_FLOAT_EQ(c.timeAtAngle(lerp(t, M_PI, 0)), t);
    }

    EXPECT_EQ(d.timeAtAngle(M_PI/2), 0);
    EXPECT_EQ(d.timeAtAngle(M_PI), 1);
    EXPECT_EQ(d.extent(), pi32);
    EXPECT_FLOAT_EQ(d.timeAtAngle(-M_PI/4), 0.5);
    for (Coord t = -0.125; t <= 1.125; t += 0.0625) {
        EXPECT_FLOAT_EQ(d.timeAtAngle(lerp(t, M_PI/2, -M_PI)), t);
    }
}

TEST(AngleTest, IntervalAngleAt) {
    Coord pi32 = (3./2.)*M_PI;
    AngleInterval a(M_PI, pi32, true);
    AngleInterval c(M_PI, 0, false);

    EXPECT_EQ(a.angleAt(0), M_PI);
    EXPECT_EQ(a.angleAt(1), pi32);
    EXPECT_EQ(a.extent(), M_PI/2);
    for (Coord t = -1; t <= 2; t += 0.125) {
        EXPECT_FLOAT_EQ(a.angleAt(t), Angle(lerp(t, M_PI, pi32)));
    }

    EXPECT_EQ(c.timeAtAngle(M_PI), 0);
    EXPECT_EQ(c.timeAtAngle(0), 1);
    EXPECT_EQ(c.extent(), M_PI);
    for (Coord t = -0.25; t <= 1.25; t += 0.0625) {
        EXPECT_FLOAT_EQ(c.angleAt(t), Angle(lerp(t, M_PI, 0)));
    }
}
