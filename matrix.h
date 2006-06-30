#ifndef __Geom_MATRIX_H__
#define __Geom_MATRIX_H__

/** \file
 * Definition of Geom::Matrix types.
 *
 * \note Operator functions (e.g. Matrix * Matrix etc.) are mostly in
 * libnr/matrix-ops.h.  See end of file for discussion.
 *
 * Main authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>:
 *     Original NRMatrix definition and related macros.
 *
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>:
 *     Geom::Matrix class version of the above.
 *
 * This code is in public domain.
 */

//#include <glib/gmessages.h>

#include "coord.h"
#include "values.h"
#include "rotate.h"
#include "scale.h"
#include "translate.h"

namespace Geom {

/**
 * The Matrix class.
 * 
 * For purposes of multiplication, points should be thought of as row vectors
 *
 *    p = ( p[X] p[Y]  1  )
 *
 * to be right-multiplied by transformation matrices
 * \verbatim
    c[] = | c[0] c[1]  0  |
          | c[2] c[3]  0  |
          | c[4] c[5]  1  |                           \endverbatim
 *
 * (so the columns of the matrix correspond to the columns (elements) of the result,
 * and the rows of the matrix correspond to columns (elements) of the "input").
 */
class Matrix {
    Geom::Coord _c[6];

  public:

    //TODO: I'd prefer it default to identity matrix - Botty
    explicit Matrix() { }

    Matrix(Matrix const &m) {
        for(int i = 0; i < 6; i++) {
            _c[i] = m[i];
        }
    }

    Matrix(double c0, double c1, double c2, double c3, double c4, double c5) {
        _c[0] = c0;
        _c[1] = c1;
        _c[2] = c2;
        _c[3] = c3;
        _c[4] = c4;
        _c[5] = c5;
    }

    Matrix &operator=(Matrix const &m) {
        for(int i = 0; i < 6; i++) {
            _c[i] = m._c[i];
        }
        return *this;
    }

    explicit Matrix(scale const &sm) {
        _c[0] = sm[X];
        _c[3] = sm[Y];
    }

    Point get_x_axis() const;
    Point get_y_axis() const;
    Point get_translation() const;
    Point set_x_axis(Point const &vec);
    Point set_y_axis(Point const &vec);
    Point set_translation(Point const &loc);

    explicit Matrix(rotate const &r) {
        set_x_axis(r.vec);
        set_y_axis(r.vec.cw());
    }

    explicit Matrix(translate const &tm) {
        set_translation(tm.offset);
    }

    //TODO: Remove testing code from production code! matrix.cpp as well
    bool test_identity() const;

    bool is_translation(Coord const eps = 1e-6) const;
    bool is_rotation(double const eps = 1e-6) const;
    bool is_scale(double const eps = 1e-6) const;
    bool is_uniform_scale(double const eps = 1e-6) const;

    Matrix inverse() const;

    Matrix &operator*=(Matrix const &other);
    Matrix &operator*=(scale const &other);
    Matrix &operator*=(translate const &other);

    inline Coord &operator[](int const i) {
        return _c[i];
    }

    inline Coord operator[](int const i) const {
        return _c[i];
    }

    //TODO: change name to reset? just create a new matrix?
    void set_identity();
	
    Coord det() const;
    Coord descrim2() const;
    Coord descrim() const;

    double expansion() const;
    //TODO: change to get/set_x/y_length?
    double expansionX() const;
    double expansionY() const;
	
    // legacy
    //TODO: Remove
    Matrix &assign(Coord const *array);
    Coord *copyto(Coord *array) const;
};

/** A function to print out the Matrix (for debugging) */
inline std::ostream &operator<< (std::ostream &out_file, const Geom::Matrix &m) {
    out_file << "A: " << m[0] << "  C: " << m[2] << "  E: " << m[4] << "\n";
    out_file << "B: " << m[1] << "  D: " << m[3] << "  F: " << m[5] << "\n";
    return out_file;
}

extern void assert_close(Matrix const &a, Matrix const &b);

} /* namespace Geom */

/** \note
 * Discussion of splitting up matrix.h into lots of little files:
 *
 *   Advantages:
 *
 *    - Reducing amount of recompilation necessary when anything changes.
 *
 *    - Hopefully also reducing compilation time by reducing the number of inline
 *      function definitions encountered by the compiler for a given .o file.
 *      (No timing comparisons done yet.  On systems without much memory available
 *      for caching, this may be outweighed by additional I/O costs.)
 *
 *   Disadvantages:
 *
 *    - More #include lines necessary per file.  If a compile fails due to
 *      not having all the necessary #include lines, then the developer needs
 *      to spend some time working out what #include to add.
 */

#endif /* !__Geom_MATRIX_H__ */


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
