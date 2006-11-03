#define __Geom_MATRIX_C__

/** \file
 * Various matrix routines.  Currently includes some Geom::rotate etc. routines too.
 */

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include "matrix.h"
#include "matrix-fns.h"

namespace Geom {

/** Multiplies two matrices together, effectively combining their transformations.*/
Matrix operator*(Matrix const &m0, Matrix const &m1) {
    Matrix ret;
    int n = 0;
    for(int a = 0; a < 5; a += 2) {
        for(int b = 0; b < 2; b++) {
            ret[n] = m0[a] * m1[b] + m0[a + 1] * m1[b + 2];
            n++;
        }
    }
    ret[4] += m1[4];
    ret[5] += m1[5];
    return ret;
}

/** Multiplies a Matrix with another.*/
Matrix &Matrix::operator*=(Matrix const &o)
{
    *this = *this * o;
    return *this;
}

/** Scales a Matrix by multiplication with a scale transform. */
Matrix &Matrix::operator*=(scale const &other) {
    for(int i = 0; i < 6; i += 2) {
        _c[i] *= other[X];
    }
    for(int i = 1; i < 6; i += 2) {
        _c[i] *= other[Y];
    }

    return *this;
}

/** Translates a Matrix by multiplication with a translate transform. */
Matrix &Matrix::operator*=(translate const &other) {
    _c[4] += other[X];
    _c[5] += other[Y];
    return *this;
}


/** Attempts to calculate the inverse of a matrix.
 *  This is a Matrix such that m * m.inverse() is very near (hopefully < epsilon difference) the identity Matrix.
 *  \textbf{The Identity Matrix is returned if the matrix has no inverse.}
 \return The inverse of the Matrix if defined, otherwise the Identity Matrix.
 */
Matrix Matrix::inverse() const {
    Matrix d;

    Geom::Coord const determ = det();
    if (!Geom_DF_TEST_CLOSE(determ, 0.0, Geom_EPSILON)) {
        Geom::Coord const ideterm = 1.0 / determ;

        d._c[0] =  _c[3] * ideterm;
        d._c[1] = -_c[1] * ideterm;
        d._c[2] = -_c[2] * ideterm;
        d._c[3] =  _c[0] * ideterm;
        d._c[4] = -_c[4] * d._c[0] - _c[5] * d._c[2];
        d._c[5] = -_c[4] * d._c[1] - _c[5] * d._c[3];
    } else {
        d.set_identity();
    }

    return d;
}

/** Sets this matrix to be the Identity Matrix. */
void Matrix::set_identity() {
    _c[0] = 1.0;
    _c[1] = 0.0;
    _c[2] = 0.0;
    _c[3] = 1.0;
    _c[4] = 0.0;
    _c[5] = 0.0;
}

/** Returns the Identity Matrix. */
Matrix identity() {
    return Matrix(1.0, 0.0,
                  0.0, 1.0,
                  0.0, 0.0);
}

/** Creates a Matrix given an axis and origin point.
 *  The axis is represented as two vectors, which represent skew, rotation, and scaling in two dimensions.
 *  from_basis(Point(1, 0), Point(0, 1), Point(0, 0)) would return the identity matrix.

 \param x_basis the vector for the x-axis.
 \param y_basis the vector for the y-axis.
 \param offset the translation applied by the matrix.
 \return The new Matrix.
 */
Matrix from_basis(Point const x_basis, Point const y_basis, Point const offset) {
    return Matrix(x_basis[X], x_basis[Y],
                  y_basis[X], y_basis[Y],
                  offset [X], offset [Y]);
}

/** Constructs a rotate transformation corresponding to an angle.
 \param theta the rotation angle in radians about the origin
 *
 * \see Geom::rotate_degrees
 *
 * Angle direction in Inkscape code: If you use the traditional mathematics convention that y
 * increases upwards, then positive angles are anticlockwise as per the mathematics convention.  If
 * you take the common non-mathematical convention that y increases downwards, then positive angles
 * are clockwise, as is common outside of mathematics.
 */
rotate::rotate(Geom::Coord const theta) :
    vec(cos(theta),
        sin(theta))
{}

/** Calculates the determinant of a Matrix. */
Geom::Coord Matrix::det() const {
    return _c[0] * _c[3] - _c[1] * _c[2];
}

/** Calculates the scalar of the descriminant of the Matrix.
 *  This is simply the absolute value of the determinant.
 */
Geom::Coord Matrix::descrim2() const {
    return fabs(det());
}

/** Calculates the descriminant of the Matrix. */
Geom::Coord Matrix::descrim() const {
    return sqrt(descrim2());
}

/** [DEPRECATED] Assigns a matrix to a given coordinate array.
 \param array a pointer to the source array, must be 6 Coord long.
 \return The resulting matrix.
 */
Matrix &Matrix::assign(Coord const *array) {
    assert(array != NULL);

    Coord const *src = array;
    Coord *dest = _c;

    *dest++ = *src++;  //0
    *dest++ = *src++;  //1
    *dest++ = *src++;  //2
    *dest++ = *src++;  //3
    *dest++ = *src++;  //4
    *dest   = *src  ;  //5

    return *this;
}

/** [DEPRECATED] Copies this matrix's values to an array.
 \param array a pointer to the target array.
 */
Geom::Coord *Matrix::copyto(Geom::Coord *array) const {
    assert(array != NULL);

    Coord const *src = _c;
    Coord *dest = array;

    *dest++ = *src++;  //0
    *dest++ = *src++;  //1
    *dest++ = *src++;  //2
    *dest++ = *src++;  //3
    *dest++ = *src++;  //4
    *dest   = *src  ;  //5

    return array;
}

/** [DEPRECATED] - use descrim. */
double expansion(Matrix const &m) {
    return sqrt(fabs(m.det()));
}

/** [DEPRECATED] - use descrim. */
double Matrix::expansion() const {
    return sqrt(fabs(det()));
}

/** Calculates the amount of x-scaling imparted by the Matrix.  This is the scaling applied before
 *  the other transformations.  It is \emph{not} the overall y-scaling of the transformation.
 */
double Matrix::expansionX() const {
    return sqrt(_c[0] * _c[0] + _c[1] * _c[1]);
}

/** Calculates the amount of y-scaling imparted by the Matrix.  This is the scaling applied before
 *  the other transformations.  It is \emph{not} the overall y-scaling of the transformation. 
 */
double Matrix::expansionY() const {
    return sqrt(_c[2] * _c[2] + _c[3] * _c[3]);
}

Point Matrix::get_x_axis() const {
    return Point(_c[0], _c[1]);
}

Point Matrix::get_y_axis() const {
    return Point(_c[2], _c[3]);
}

void Matrix::set_x_axis(Point const &vec) {
    for(int i = 0; i < 2; i++)
        _c[i] = vec[i];
}

void Matrix::set_y_axis(Point const &vec) {
    for(int i = 0; i < 2; i++)
        _c[i + 2] = vec[i];
}

/** Gets the translation imparted by the Matrix.
 */
Point Matrix::get_translation() const {
    return Point(_c[4], _c[5]);
}

/** Sets the translation imparted by the Matrix.
 */
void Matrix::set_translation(Point const &loc) {
    for(int i = 0; i < 2; i++)
        _c[i + 4] = loc[i];
}

/** Answers the question "Does this matrix perform \em{only} a translation?"
 \param eps an epsilon value defaulting to Geom_EPSILON
 \return A bool representing yes/no.
 */
bool Matrix::is_translation(Coord const eps) const {
    return ( fabs(_c[0] - 1.0) < eps &&
             fabs(_c[1])       < eps &&
             fabs(_c[2])       < eps &&
             fabs(_c[3] - 1.0) < eps &&
             _c[4] != 0 && _c[5] != 0);
}

/** Answers the question "Does this matrix perform \em{only} a scale?"
 \param eps an epsilon value defaulting to Geom_EPSILON
 \return A bool representing yes/no.
 */
bool Matrix::is_scale(Coord const eps) const {
    return ( (_c[0] != 1 || _c[3] != 1) &&
             fabs(_c[1]) < eps &&
             fabs(_c[2]) < eps &&
             fabs(_c[4]) < eps &&
             fabs(_c[5]) < eps);
}

/** Answers the question "Does this matrix perform a uniform scale?"
 \param eps an epsilon value defaulting to Geom_EPSILON
 \return A bool representing yes/no.
 */
bool Matrix::is_uniform_scale(Coord const eps) const {
    return ( _c[0] != 1              &&
             fabs(_c[0]-_c[3]) < eps &&
             fabs(_c[1])       < eps &&
             fabs(_c[2])       < eps &&
             fabs(_c[4])       < eps &&
             fabs(_c[5])       < eps);
}

/** Answers the question "Does this matrix perform \em{only} a rotation?"
 \param eps an epsilon value defaulting to Geom_EPSILON
 \return A bool representing yes/no.
 */
bool Matrix::is_rotation(Coord const eps) const {
    return ( fabs(_c[0] - _c[3]) < eps &&
             fabs(_c[1] + _c[2]) < eps &&
             fabs(_c[4])         < eps &&
             fabs(_c[5])         < eps &&
             fabs(L2(get_x_axis()) - 1) < eps &&
             fabs(L2(get_y_axis()) - 1) < eps);
}

//TODO: Remove all this crap!

#define nr_matrix_test_equal(m0,m1,e) ((!(m0) && !(m1)) || ((m0) && (m1) && Geom_MATRIX_DF_TEST_CLOSE(m0, m1, e)))
#define nr_matrix_test_transform_equal(m0,m1,e) ((!(m0) && !(m1)) || ((m0) && (m1) && Geom_MATRIX_DF_TEST_TRANSFORM_CLOSE(m0, m1, e)))
#define nr_matrix_test_translate_equal(m0,m1,e) ((!(m0) && !(m1)) || ((m0) && (m1) && Geom_MATRIX_DF_TEST_TRANSLATE_CLOSE(m0, m1, e)))

/**
 *
 */
bool Matrix::test_identity() const {
    static Matrix const identity(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    
    return Geom_MATRIX_DF_TEST_CLOSE(this, &identity, Geom_EPSILON);
}

/**
 *
 */
bool transform_equalp(Matrix const &m0, Matrix const &m1, Geom::Coord const epsilon) {
    return Geom_MATRIX_DF_TEST_TRANSFORM_CLOSE(&m0, &m1, epsilon);
}

/**
 *
 */
bool translate_equalp(Matrix const &m0, Matrix const &m1, Geom::Coord const epsilon) {
    return Geom_MATRIX_DF_TEST_TRANSLATE_CLOSE(&m0, &m1, epsilon);
}

/**
 *
 */
bool matrix_equalp(Matrix const &m0, Matrix const &m1, Geom::Coord const epsilon)
{
    return ( Geom_MATRIX_DF_TEST_TRANSFORM_CLOSE(&m0, &m1, epsilon) &&
             Geom_MATRIX_DF_TEST_TRANSLATE_CLOSE(&m0, &m1, epsilon)   );
}

/**
 *  A home-made assertion.  Stop if the two matrixes are not 'close' to
 *  each other.
 */
void assert_close(Matrix const &a, Matrix const &b)
{
    if (!matrix_equalp(a, b, 1e-3)) {
        fprintf(stderr,
                "a = | %g %g |,\tb = | %g %g |\n"
                "    | %g %g | \t    | %g %g |\n"
                "    | %g %g | \t    | %g %g |\n",
                a[0], a[1], b[0], b[1],
                a[2], a[3], b[2], b[3],
                a[4], a[5], b[4], b[5]);
        abort();
    }
}

}  //namespace Geom

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
