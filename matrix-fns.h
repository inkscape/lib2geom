#ifndef SEEN_Geom_MATRIX_FNS_H
#define SEEN_Geom_MATRIX_FNS_H

#include "matrix.h"

namespace Geom {

/** Given a matrix m such that unit_circle = m*x, this returns the
 * quadratic form x*A*x = 1. */
Matrix elliptic_quadratic_form(Matrix const &m);

/** Given a matrix (ignoring the translation) this returns the eigen
 * values and vectors. */
class Eigen{
public:
    Point vectors[2];
    Point values;
    Eigen(Matrix const &m);
};

// Matrix factories
Matrix from_basis(const Point x_basis, const Point y_basis, const Point offset=Point(0,0));

Matrix identity();

double expansion(Matrix const &m);

bool transform_equalp(Matrix const &m0, Matrix const &m1, Geom::Coord const epsilon);
bool translate_equalp(Matrix const &m0, Matrix const &m1, Geom::Coord const epsilon);
bool matrix_equalp(Matrix const &m0, Matrix const &m1, Geom::Coord const epsilon);

Matrix transform(Matrix const &m);
translate get_translation(Matrix const &m);

void matrix_print(const char *say, Matrix const &m);

}  // namespace Geom

#endif /* !SEEN_Geom_MATRIX_FNS_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
