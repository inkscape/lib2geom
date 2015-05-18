/** @file
 * @brief Unit tests for Polynomial and related functions.
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
#include <iostream>
#include <glib.h>

#include <2geom/polynomial.h>

using namespace Geom;

TEST(PolynomialTest, SolveQuadratic) {
    for (unsigned i = 0; i < 1000; ++i) {
        Coord x1 = g_random_double_range(-100, 100);
        Coord x2 = g_random_double_range(-100, 100);

        Coord a = g_random_double_range(-10, 10);
        Coord b = -a * (x1 + x2);
        Coord c = a * x1 * x2;

        std::vector<Coord> result = solve_quadratic(a, b, c);

        EXPECT_EQ(result.size(), 2);
        if (x1 < x2) {
            EXPECT_FLOAT_EQ(result[0], x1);
            EXPECT_FLOAT_EQ(result[1], x2);
        } else {
            EXPECT_FLOAT_EQ(result[0], x2);
            EXPECT_FLOAT_EQ(result[1], x1);
        }
    }
}

TEST(PolynomialTest, SolveCubic) {
    for (unsigned i = 0; i < 1000; ++i) {
        Coord x1 = g_random_double_range(-100, 100);
        Coord x2 = g_random_double_range(-100, 100);
        Coord x3 = g_random_double_range(-100, 100);

        Coord a = g_random_double_range(-10, 10);
        Coord b = -a * (x1 + x2 + x3);
        Coord c = a * (x1*x2 + x2*x3 + x1*x3);
        Coord d = -a * x1 * x2 * x3;

        std::vector<Coord> result = solve_cubic(a, b, c, d);
        std::vector<Coord> x(3); x[0] = x1; x[1] = x2; x[2] = x3;
        std::sort(x.begin(), x.end());

        EXPECT_EQ(result.size(), 3);
        EXPECT_FLOAT_EQ(result[0], x[0]);
        EXPECT_FLOAT_EQ(result[1], x[1]);
        EXPECT_FLOAT_EQ(result[2], x[2]);
    }
}
