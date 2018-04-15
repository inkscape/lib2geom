/** @file
 * @brief Unit tests for functions related to Coord.
 * Uses the Google Testing Framework
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2014 Authors
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
#include <climits>
#include <cstdint>
#include <glib.h>
#include <iostream>

namespace Geom {

TEST(CoordTest, StringRoundtripShortest) {
    union {
        uint64_t u;
        double d;
    };
    for (unsigned i = 0; i < 100000; ++i) {
        u = uint64_t(g_random_int()) | (uint64_t(g_random_int()) << 32);
        if (!std::isfinite(d)) continue;

        std::string str = format_coord_shortest(d);
        double x = parse_coord(str);
        if (x != d) {
            std::cout << std::endl << d << " -> " << str << " -> " << x << std::endl;
        }
        EXPECT_EQ(d, x);
    }
}

TEST(CoordTest, StringRoundtripNice) {
    union {
        uint64_t u;
        double d;
    };
    for (unsigned i = 0; i < 100000; ++i) {
        u = uint64_t(g_random_int()) | (uint64_t(g_random_int()) << 32);
        if (!std::isfinite(d)) continue;

        std::string str = format_coord_nice(d);
        double x = parse_coord(str);
        if (x != d) {
            std::cout << std::endl << d << " -> " << str << " -> " << x << std::endl;
        }
        EXPECT_EQ(d, x);
    }
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
