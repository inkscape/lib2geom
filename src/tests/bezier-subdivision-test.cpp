/** @file
 * @brief Unit tests for Affine
 * Uses the Google Testing Framework
 *//*
 * Authors:
 *   Alexander Brock <Brock.Alexander@web.de>
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
#include <2geom/bezier.h>
#include <2geom/path.h>
#include <cstdlib>

namespace Geom {

double asymmetric_furthest_distance(Path const& a, Path const& b) {

    double result = 0;
    for (auto const& curve : a) {
        for (double t = 0; t <= 1; t += .05) {
            double current_dist = 0;
            b.nearestTime(curve.pointAt(t), &current_dist);
            result = std::max(result, current_dist);
        }
    }

    return result;
}

double symmetric_furthest_distance(Path const& a, Path const& b) {
    return std::max(
                asymmetric_furthest_distance(a,b),
                asymmetric_furthest_distance(b,a)
                );
}

Geom::Path subdivide(CubicBezier const& bez, std::vector<double> const& times_in) {
    Path result;
    // First we need to sort the times ascending.
    std::vector<CubicBezier> curves = bez.subdivide(times_in);
    for (const auto& c : curves) {
        result.append(c);
    }
    return result;
}


double rand_uniform() {
    return static_cast<double>(rand()) / RAND_MAX;
}

double rand_uniform(double const lower, double const upper) {
    return rand_uniform() * (upper - lower) + lower;
}

std::vector<double> random_times(size_t const size) {
    std::vector<double> result(size, 0.0);
    for (size_t ii = 0; ii < size; ++ii) {
        result[ii] = rand_uniform();
    }
    return result;
}

Point random_point() {
    return Point(rand_uniform(-1,1), rand_uniform(-1,1));
}

CubicBezier random_bezier() {
    return CubicBezier(random_point(), random_point(), random_point(), random_point());
}

void print_vector(std::vector<double> const& in) {
    for (auto const val : in) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}


#define NUM_TESTS 10
#define MAX_SUBDIVISIONS 15

TEST(Bezier, Subdivision) {

    srand(0xAFFEAFFE);
    for (size_t num_divisions = 0; num_divisions < MAX_SUBDIVISIONS; num_divisions++) {
        double worst_error = 0;
        std::vector<double> worst_subdivision;
        Path worst_bezier;
        for (size_t ii = 0; ii < NUM_TESTS; ++ii) {
            CubicBezier current_curve = random_bezier();
            Path current_path;
            current_path.append(current_curve);
            std::vector<double> times = random_times(num_divisions);
            Path divided = subdivide(current_curve, times);
            double current_error = symmetric_furthest_distance(divided, current_path);
            if (current_error > worst_error) {
                worst_error = current_error;
                worst_bezier = current_path;
                worst_subdivision = times;
            }
        }
        if (worst_error > 1e-3) {
            std::cout << std::endl << "subdivisions: " << num_divisions << std::endl;
            std::cout << "Worst error: " << worst_error << " with path: " << std::endl
                      << worst_bezier << std::endl
                      << "and times: ";
            print_vector(worst_subdivision);
        }
        EXPECT_LE(worst_error, 1e-3);
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
