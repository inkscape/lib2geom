/** @file
 * @brief Unit tests for Affine
 * Uses the Google Testing Framework
 *//*
 * Authors:
 *   Alexander Brock <Brock.Alexander@web.de>
 *
 * Copyright 2017 Authors
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

/**
 * @brief Find the largest distance between points on a and the Path b.
 * The method is asymmetric in the sense that it will report a low distance if Path a is a subset of Path b.
 * @param a Path on which points are selected for comparison with Path b
 * @param b Path on which points closest to given points on a are searched
 * @return Largest distance found in the comparison.
 */
double asymmetric_furthest_distance(Path const& a, Path const& b) {

    double result = 0;
    for (size_t ii = 0; ii < a.size(); ++ii) {
        Curve const& curve = a[ii];
        for (double t = 0; t <= 1; t += .05) {
            double current_dist = 0;
            b.nearestTime(curve.pointAt(t), &current_dist);
            result = std::max(result, current_dist);
        }
    }

    return result;
}

/**
 * @brief Find the largest distance between Path a and Path b
 * @param a Path a
 * @param b Path b
 * @return Largest distance found.
 */
double symmetric_furthest_distance(Path const& a, Path const& b) {
    return std::max(
                asymmetric_furthest_distance(a,b),
                asymmetric_furthest_distance(b,a)
                );
}

template<class C>
/**
 * @brief Find the largest distance between CubicBezier a_in and Path b
 * @param a_in
 * @param b
 * @return Largest distance found.
 */
double symmetric_furthest_distance(C const& a_in, Path const& b) {
    Path a;
    a.append(a_in);
    return std::max(
                asymmetric_furthest_distance(a,b),
                asymmetric_furthest_distance(b,a)
                );
}

template<class C>
/**
 * @brief Create a Path from a single curve and a set of subdivision times.
 * @param bez Original bezier path.
 * @param times_in Times for splitting the path.
 * @return A Geom::Path with the subdivied beziers.
 */
Geom::Path subdivide(C const& bez, std::vector<double> const& times_in) {
    Path result;
    // First we need to sort the times ascending.
    std::vector<C> curves = bez.subdivide(times_in);
    for (size_t ii = 0; ii < curves.size(); ++ii) {
        result.append(curves[ii]);
    }
    return result;
}

/**
 * @brief Create a pseudo random number in the closed interval [0,1]
 * @return
 */
double random_uniform() {
    return static_cast<double>(rand()) / RAND_MAX;
}

/**
 * @brief Create a pseudo random number in the closed interval [lower, upper]
 * @param lower Lower bound for the generated numbers
 * @param upper Upper bound for the generated numbers
 * @return A pseudo random number in [lower, upper]
 */
double random_uniform(double const lower, double const upper) {
    if (upper < lower) {
        return random_uniform(upper, lower);
    }
    return random_uniform() * (upper - lower) + lower;
}

/**
 * @brief Create a vector of pseudo random numbers in [0,1] with a given number of elements.
 * @param size Desired number of elements.
 * @return Vector of pseudo random numbers.
 */
std::vector<double> random_times(size_t const size) {
    std::vector<double> result(size, 0.0);
    for (size_t ii = 0; ii < size; ++ii) {
        result[ii] = random_uniform();
    }
    return result;
}

/**
 * @brief Create a pseudo random point uniformly drawn from the rectangle [-1,1]x[-1,1]
 */
Point random_point() {
    return Point(random_uniform(-1,1), random_uniform(-1,1));
}

/**
 * @brief Create a CubicBezier with pseudo random control points.
 */
CubicBezier random_cubic_bezier() {
    return CubicBezier(random_point(), random_point(), random_point(), random_point());
}

/**
 * @brief Create a QuadraticBezier with pseudo random control points.
 * @return
 */
QuadraticBezier random_quadratic_bezier() {
    return QuadraticBezier(random_point(), random_point(), random_point());
}

/**
 * @brief Create a LineSegment pseudo random start and end points.
 */
LineSegment random_line_segment() {
    return LineSegment(random_point(), random_point());
}

template<class C>
C random_curve() {
    throw std::runtime_error("No specialisation of random_curve() available for desired type");
    return C();
}

template<>
LineSegment random_curve() {
    return LineSegment(random_point(), random_point());
}

template<>
QuadraticBezier random_curve() {
    return QuadraticBezier(random_point(), random_point(), random_point());
}

template<>
CubicBezier random_curve() {
    return CubicBezier(random_point(), random_point(), random_point(), random_point());
}


/**
 * @brief Print a vector of doubles to std::cout
 */
void print_vector(std::vector<double> const& in) {
    for (size_t ii = 0; ii < in.size(); ++ii) {
        std::cout << in[ii] << " ";
    }
    std::cout << std::endl;
}


#define NUM_TESTS 10
#define MAX_SUBDIVISIONS 15

#define PRNG_SEED (time(NULL))


template <typename T>
class BezierSubdivision : public ::testing::Test {
};
typedef ::testing::Types<CubicBezier, QuadraticBezier, LineSegment> BezierTypes;

TYPED_TEST_CASE(BezierSubdivision, BezierTypes);

/**
 * @brief Devide random Bezier curves of order 1,2,3 at random points into sets of shorter curves and test if the
 * set represents the same curve as the original curve.
 */
TYPED_TEST(BezierSubdivision, Subdivision) {
    double global_worst_error = 0;
    std::vector<double> global_worst_subdivision;
    Path global_worst_bezier;

    srand(PRNG_SEED);
    for (size_t num_divisions = 0; num_divisions < MAX_SUBDIVISIONS; num_divisions++) {
        double worst_error = 0;
        std::vector<double> worst_subdivision;
        Path worst_bezier;
        for (size_t ii = 0; ii < NUM_TESTS; ++ii) {
            TypeParam current_curve = random_curve<TypeParam>();
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
        if (worst_error > global_worst_error) {
            global_worst_error = worst_error;
            global_worst_subdivision = worst_subdivision;
            global_worst_bezier = worst_bezier;
        }
        EXPECT_LE(worst_error, 1e-3);
    }
    std::cout << "Worst error: " << global_worst_error << " with path: " << std::endl
              << global_worst_bezier << std::endl
              << "and times: ";
    print_vector(global_worst_subdivision);
}

template<class C>
/**
 * @brief Add uniformly distributed pseudo random numbers to control points of Bezier curves
 * @param[in] curve Original curve (won't be modified)
 * @param[in] scale Maximum difference between original and result.
 * @return Copy of the original with modified control points
 */
C add_noise(C curve, double const scale = .1) {
    double sum = 0;
    double max_val = 0;
    for (size_t ii = 0; ii < curve.size(); ++ii) {
        curve[ii] += scale * random_point();
        sum += curve[ii].length();
        max_val = std::max(max_val, curve[ii].length());
    }
    for (size_t ii = 0; ii < 4; ++ii) {
        curve[ii] /= max_val;
    }
    return curve;
}

/**
 * @brief Add uniformly distributed pseudo random numbers to vector of curve times (i.e. numbers in [0,1])
 * @param[in] times Original vector of times, this won't be modified.
 * @param[in] scale Maximum difference between original entries and modified entries.
 * @return Copy of the original vector with modified entries.
 */
std::vector<double> add_noise(std::vector<double> times, double const scale = .1) {
    for (size_t ii = 0; ii < times.size(); ++ii) {
        double& t = times[ii];
        double const noise = random_uniform(-scale, scale);
        t += noise;
        if (t < 0) {
            t = std::abs(t);
        }
        if (t > 1) {
            t = 1.0 - std::abs(t-1);
        }
    }
    return times;
}

/**
 * @brief Search for curves and subdivision times for which the error of the subdivision method is maximized
 * by randomly changing an initial random curve and initial random subdivision times vector.
 */
TYPED_TEST(BezierSubdivision, SearchWorstSubdivision) {

    TypeParam worst_bez = add_noise(random_curve<TypeParam>());
    std::vector<double> worst_times = random_times(3);
    Path divided = subdivide(worst_bez, worst_times);
    double worst_error = symmetric_furthest_distance(worst_bez, divided);

    srand(PRNG_SEED);

    double scale = .1;
    size_t iteration_counter = 0;
    size_t const max_it = 5e3;
    while (scale > .01 && iteration_counter < max_it) {
        for (size_t ii = 0; ii < 2e2 && iteration_counter < max_it; ++ii) {
            iteration_counter++;
            TypeParam current_bez = add_noise(worst_bez, scale);
            std::vector<double> current_times = add_noise(worst_times, scale);
            Path divided = subdivide(current_bez, current_times);
            double current_error = symmetric_furthest_distance(current_bez, divided);
            if (current_error > worst_error) {
                worst_error = current_error;
                worst_bez = current_bez;
                worst_times = current_times;
                Path worst_path;
                worst_path.append(worst_bez);
            }
        }
        scale /= 2;
    }
    EXPECT_LE(worst_error, 1e-3);

    Path worst_path;
    worst_path.append(worst_bez);
    std::cout << std::endl << "#iterations: " << iteration_counter << std::endl;
    std::cout << "Worst error: " << worst_error << " with path: " << std::endl
              << worst_path << std::endl
              << "and times: ";
    print_vector(worst_times);

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
