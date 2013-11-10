/*
 * Test performance of bezier-util fitting code
 *
 * Copyright (C) authors 2013
 * Authors:
 *       Johan Engelen   <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <iostream>
#include <ctime>

#include "2geom/bezier-utils.h"
#include "2geom/path.h"
using namespace Geom;

//static const Point data[] = { Point(0, 0), Point(1, 0), Point( 2, 0 ), Point( 1, 1 )};
static const Point data[] = { Point( 0, 0 ), Point( 1, 0 ), Point( 2, 0 ), Point( 1, 1 ),
                              Point( 1, 2 ), Point( 1, 3 ), Point( 3, 0 ), Point( 4, 0 ),
                              Point( 2, 0 ), Point( 1, 1 ), Point( 1, 2 ), Point( 2, 0 ),
                              Point( 1, 0 ), Point( 2, 0 ), Point( 1, 3 ), Point( 1, 4 ),
                              Point( 1, 2 ), Point( 1, 1 ), Point( 0, 0 ), Point( 1, 0 ),
                              Point( 2, 0 ), Point( 1, 1 ), Point( 1, 2 ), Point( 1, 3 ),
                              Point( 4, 0 ), Point( 2, 0 ), Point( 1, 1 ), Point( 1, 2 ),
                              Point( 2, 0 ), Point( 1, 0 ), Point( 2, 0 ), Point( 1, 3 ),
                              Point( 1, 4 ), Point( 1, 2 ), Point( 1, 1 ), Point( 2, 1 ) };

static const unsigned int data_len = sizeof(data)/sizeof(Point);



// code test with 2geom types
Path interpolateToPath(std::vector<Point> const &points, double tolerance_sq, unsigned max_beziers)
{
    Geom::Point *b = new Geom::Point[max_beziers*4];
    Geom::Point *points_array = new Geom::Point[4 * points.size()]; // for safety, do a copy into a vector. I think it
                                                                    // is possible to simply pass &points[0] as a
                                                                    // Point[], but I am not sure
    for (size_t i = 0; i < points.size(); ++i) {
        points_array[i] = points.at(i);
    }

    int const n_segs = Geom::bezier_fit_cubic_r(b, points_array, points.size(), tolerance_sq, max_beziers);

    Geom::Path fit;
    if (n_segs > 0) {
        fit.start(b[0]);
        for (int c = 0; c < n_segs; c++) {
            fit.appendNew<Geom::CubicBezier>(b[4 * c + 1], b[4 * c + 2], b[4 * c + 3]);
        }
    }

    delete[] b;
    delete[] points_array;

    return fit;
};


Path interpolateToPath2(std::vector<Point> const &points, double tolerance_sq, unsigned max_beziers)
{
    std::vector<Point> b(max_beziers * 4);

    int const n_segs = Geom::bezier_fit_cubic_r(b.data(), points.data(), points.size(), tolerance_sq, max_beziers);

    Geom::Path fit;
    if (n_segs > 0) {
        fit.start(b[0]);
        for (int c = 0; c < n_segs; c++) {
            fit.appendNew<Geom::CubicBezier>(b[4 * c + 1], b[4 * c + 2], b[4 * c + 3]);
        }
    }

    return fit;
};


int main()
{
    std::vector<Point> data_vector;
    for (unsigned int i = 0; i < data_len; i++) {
        data_vector.push_back(data[i]);
    }

    const int num_repeats = 2000;

    unsigned max_beziers = data_len*2;
    double tolerance_sq = 0.01;

    for (int rep = 0; rep < 3; rep++) {
        std::clock_t start = std::clock();
        for (int i = 0; i < num_repeats; i++) {
            Point *bezier = new Point[max_beziers*4];  // large array on stack = not good, so allocate on heap
            int n_segs = bezier_fit_cubic_r(bezier, data, data_len, tolerance_sq, max_beziers);
            (void) n_segs;
            delete[] bezier;
        }
        std::clock_t stop = std::clock();
        std::cout << "bezier_fit_cubic_r C-array (" << num_repeats << "x): " << (stop - start) * (1000. / CLOCKS_PER_SEC) << " ms "
                  << std::endl;
    }

    for (int rep = 0; rep < 3; rep++) {
        std::clock_t start = std::clock();
        for (int i = 0; i < num_repeats; i++) {
            Path path = interpolateToPath(data_vector, tolerance_sq, max_beziers);
            int n_segs = path.size();
            (void) n_segs;
        }
        std::clock_t stop = std::clock();
        std::cout << "bezier_fit_cubic_r 2Geom interoperability (" << num_repeats << "x): " << (stop - start) * (1000. / CLOCKS_PER_SEC) << " ms "
                  << std::endl;
    }

    for (int rep = 0; rep < 3; rep++) {
        std::clock_t start = std::clock();
        for (int i = 0; i < num_repeats; i++) {
            Path path = interpolateToPath2(data_vector, tolerance_sq, max_beziers);
            int n_segs = path.size();
            (void) n_segs;
        }
        std::clock_t stop = std::clock();
        std::cout << "bezier_fit_cubic_r 2Geom interoperability 2nd version (" << num_repeats
                  << "x): " << (stop - start) * (1000. / CLOCKS_PER_SEC) << " ms " << std::endl;
    }

}



//