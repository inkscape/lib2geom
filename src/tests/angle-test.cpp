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

#include <2geom/angle.h>
#include <glib.h>
#include "testing.h"

using namespace Geom;

TEST(AngleIntervalTest, InnerAngleConstrutor) {
    std::vector<AngleInterval> ivs;

    ivs.push_back(AngleInterval(0, M_PI, true));
    ivs.push_back(AngleInterval(0, M_PI, false));
    ivs.push_back(AngleInterval(M_PI, 0, true));
    ivs.push_back(AngleInterval(M_PI, 0, false));
    ivs.push_back(AngleInterval(Angle(0), Angle(0), Angle(M_PI)));

    for (auto & iv : ivs) {
        AngleInterval inner(iv.angleAt(0), iv.angleAt(0.5), iv.angleAt(1));
        EXPECT_EQ(inner, iv);
    }
}

TEST(AngleIntervalTest, Containment) {
    AngleInterval a(0, M_PI, true);
    AngleInterval b(0, M_PI, false);
    AngleInterval c(M_PI, 0, true);
    AngleInterval d(M_PI, 0, false);
    AngleInterval e = AngleInterval::create_full(M_PI, true);

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

    EXPECT_TRUE(e.contains(1.));
    EXPECT_TRUE(e.contains(5.));
    EXPECT_EQ(e.extent(), 2*M_PI);
}

TEST(AngleIntervalTest, TimeAtAngle) {
    Coord pi32 = (3./2.)*M_PI;
    AngleInterval a(M_PI, pi32, true);
    AngleInterval b(pi32, M_PI, true);
    AngleInterval c(M_PI, 0, false);
    AngleInterval d(M_PI/2, M_PI, false);
    AngleInterval e = AngleInterval::create_full(M_PI, true);
    AngleInterval f = AngleInterval::create_full(M_PI, false);
    Interval unit(0, 1);

    EXPECT_EQ(a.timeAtAngle(M_PI), 0);
    EXPECT_EQ(a.timeAtAngle(pi32), 1);
    EXPECT_EQ(a.extent(), M_PI/2);
    for (Coord t = -1; t <= 2; t += 0.125) {
        Coord angle = lerp(t, M_PI, pi32);
        Coord ti = a.timeAtAngle(angle);
        EXPECT_EQ(unit.contains(ti), a.contains(angle));
        EXPECT_FLOAT_EQ(ti, t);
    }

    EXPECT_EQ(b.timeAtAngle(pi32), 0);
    EXPECT_EQ(b.timeAtAngle(M_PI), 1);
    EXPECT_EQ(b.extent(), pi32);
    EXPECT_FLOAT_EQ(b.timeAtAngle(M_PI/4), 0.5);
    EXPECT_FLOAT_EQ(b.timeAtAngle(0), 1./3.);
    EXPECT_FLOAT_EQ(b.timeAtAngle((11./8)*M_PI), -1./12);
    for (Coord t = -0.125; t <= 1.125; t += 0.0625) {
        Coord angle = lerp(t, pi32, 3*M_PI);
        Coord ti = b.timeAtAngle(angle);
        EXPECT_EQ(unit.contains(ti), b.contains(angle));
        EXPECT_FLOAT_EQ(ti, t);
    }

    EXPECT_EQ(c.timeAtAngle(M_PI), 0);
    EXPECT_EQ(c.timeAtAngle(0), 1);
    EXPECT_EQ(c.extent(), M_PI);
    EXPECT_FLOAT_EQ(c.timeAtAngle(M_PI/2), 0.5);
    for (Coord t = -0.25; t <= 1.25; t += 0.125) {
        Coord angle = lerp(t, M_PI, 0);
        Coord ti = c.timeAtAngle(angle);
        EXPECT_EQ(unit.contains(ti), c.contains(angle));
        EXPECT_FLOAT_EQ(ti, t);
    }

    EXPECT_EQ(d.timeAtAngle(M_PI/2), 0);
    EXPECT_EQ(d.timeAtAngle(M_PI), 1);
    EXPECT_EQ(d.extent(), pi32);
    EXPECT_FLOAT_EQ(d.timeAtAngle(-M_PI/4), 0.5);
    for (Coord t = -0.125; t <= 1.125; t += 0.0625) {
        Coord angle = lerp(t, M_PI/2, -M_PI);
        Coord ti = d.timeAtAngle(angle);
        EXPECT_EQ(unit.contains(ti), d.contains(angle));
        EXPECT_FLOAT_EQ(ti, t);
    }

    EXPECT_EQ(e.timeAtAngle(M_PI), 0);
    EXPECT_EQ(e.extent(), 2*M_PI);
    EXPECT_FLOAT_EQ(e.timeAtAngle(0), 0.5);
    for (Coord t = 0; t < 1; t += 0.125) {
        Coord angle = lerp(t, M_PI, 3*M_PI);
        Coord ti = e.timeAtAngle(angle);
        EXPECT_EQ(unit.contains(ti), true);
        EXPECT_EQ(e.contains(angle), true);
        EXPECT_FLOAT_EQ(ti, t);
    }

    EXPECT_EQ(f.timeAtAngle(M_PI), 0);
    EXPECT_EQ(f.extent(), 2*M_PI);
    EXPECT_FLOAT_EQ(e.timeAtAngle(0), 0.5);
    for (Coord t = 0; t < 1; t += 0.125) {
        Coord angle = lerp(t, M_PI, -M_PI);
        Coord ti = f.timeAtAngle(angle);
        EXPECT_EQ(unit.contains(ti), true);
        EXPECT_EQ(f.contains(angle), true);
        EXPECT_FLOAT_EQ(ti, t);
    }
}

TEST(AngleIntervalTest, AngleAt) {
    Coord pi32 = (3./2.)*M_PI;
    AngleInterval a(M_PI, pi32, true);
    AngleInterval c(M_PI, 0, false);
    AngleInterval f1 = AngleInterval::create_full(0, true);
    AngleInterval f2 = AngleInterval::create_full(M_PI, false);

    EXPECT_EQ(a.angleAt(0), M_PI);
    EXPECT_EQ(a.angleAt(1), pi32);
    EXPECT_EQ(a.extent(), M_PI/2);
    for (Coord t = -1; t <= 2; t += 0.125) {
        EXPECT_FLOAT_EQ(a.angleAt(t), Angle(lerp(t, M_PI, pi32)));
    }

    EXPECT_EQ(c.angleAt(0), M_PI);
    EXPECT_EQ(c.angleAt(1), 0.);
    EXPECT_EQ(c.extent(), M_PI);
    for (Coord t = -0.25; t <= 1.25; t += 0.0625) {
        EXPECT_FLOAT_EQ(c.angleAt(t), Angle(lerp(t, M_PI, 0)));
    }

    EXPECT_EQ(f1.angleAt(0), 0.);
    EXPECT_EQ(f1.angleAt(1), 0.);
    for (Coord t = 0; t < 1; t += 0.125) {
        EXPECT_FLOAT_EQ(f1.angleAt(t), Angle(lerp(t, 0, 2*M_PI)));
    }
    EXPECT_EQ(f2.angleAt(0), M_PI);
    EXPECT_EQ(f2.angleAt(1), M_PI);
    for (Coord t = 0; t < 1; t += 0.125) {
        EXPECT_FLOAT_EQ(f2.angleAt(t), Angle(lerp(t, M_PI, -M_PI)));
    }
}

TEST(AngleIntervalTest, Extent) {
     Coord pi32 = (3./2.)*M_PI;
    AngleInterval a(M_PI, pi32, true);
    AngleInterval b(pi32, M_PI, true);
    AngleInterval c(M_PI, 0, false);
    AngleInterval d(M_PI/2, M_PI, false);

    EXPECT_EQ(a.extent(), M_PI/2);
    EXPECT_EQ(a.sweepAngle(), M_PI/2);
    EXPECT_EQ(b.extent(), pi32);
    EXPECT_EQ(b.sweepAngle(), pi32);
    EXPECT_EQ(c.extent(), M_PI);
    EXPECT_EQ(c.sweepAngle(), -M_PI);
    EXPECT_EQ(d.extent(), pi32);
    EXPECT_EQ(d.sweepAngle(), -pi32);
}
