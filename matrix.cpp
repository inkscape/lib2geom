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



/**
 *  Implement NR functions and methods
 */
namespace Geom {





/**
 *  Multiply two matrices together
 */
Matrix operator*(Matrix const &m0, Matrix const &m1)
{
    Geom::Coord const d0 = m0[0] * m1[0]  +  m0[1] * m1[2];
    Geom::Coord const d1 = m0[0] * m1[1]  +  m0[1] * m1[3];
    Geom::Coord const d2 = m0[2] * m1[0]  +  m0[3] * m1[2];
    Geom::Coord const d3 = m0[2] * m1[1]  +  m0[3] * m1[3];
    Geom::Coord const d4 = m0[4] * m1[0]  +  m0[5] * m1[2]  +  m1[4];
    Geom::Coord const d5 = m0[4] * m1[1]  +  m0[5] * m1[3]  +  m1[5];

    Matrix ret( d0, d1, d2, d3, d4, d5 );

    return ret;
}





/**
 *  Multiply a matrix by another
 */
Matrix &Matrix::operator*=(Matrix const &o)
{
    *this = *this * o;
    return *this;
}





/**
 *  Multiply by a scaling matrix
 */
Matrix &Matrix::operator*=(scale const &other)
{
    /* This loop is massive overkill.  Let's unroll.
     *   o    _c[] goes from 0..5
     *   o    other[] alternates between 0 and 1
     */
    /*
     * for (unsigned i = 0; i < 3; ++i) {
     *     for (unsigned j = 0; j < 2; ++j) {
     *         this->_c[i * 2 + j] *= other[j];
     *     }
     * }
     */

    Geom::Coord const xscale = other[0];
    Geom::Coord const yscale = other[1];
    Geom::Coord *dest = _c;

    /*i=0 j=0*/  *dest++ *= xscale;
    /*i=0 j=1*/  *dest++ *= yscale;
    /*i=1 j=0*/  *dest++ *= xscale;
    /*i=1 j=1*/  *dest++ *= yscale;
    /*i=2 j=0*/  *dest++ *= xscale;
    /*i=2 j=1*/  *dest   *= yscale;

    return *this;
}





/**
 *  Return the inverse of this matrix.  If an inverse is not defined,
 *  then return the identity matrix.
 */
Matrix Matrix::inverse() const
{
    Matrix d(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    Geom::Coord const det = _c[0] * _c[3] - _c[1] * _c[2];
    if (!Geom_DF_TEST_CLOSE(det, 0.0, Geom_EPSILON)) {

        Geom::Coord const idet = 1.0 / det;
        Geom::Coord *dest = d._c;

        /*0*/ *dest++ =  _c[3] * idet;
        /*1*/ *dest++ = -_c[1] * idet;
        /*2*/ *dest++ = -_c[2] * idet;
        /*3*/ *dest++ =  _c[0] * idet;
        /*4*/ *dest++ = -_c[4] * d._c[0] - _c[5] * d._c[2];
        /*5*/ *dest   = -_c[4] * d._c[1] - _c[5] * d._c[3];

    } else {
        d.set_identity();
    }

    return d;
}





/**
 *  Set this matrix to Identity
 */
void Matrix::set_identity()
{
    Geom::Coord *dest = _c;

    *dest++ = 1.0; //0
    *dest++ = 0.0; //1
    *dest++ = 0.0; //2
    *dest++ = 1.0; //3
    // translation
    *dest++ = 0.0; //4
    *dest   = 0.0; //5
}





/**
 *  return an Identity matrix
 */
Matrix identity()
{
    Matrix ret(1.0, 0.0,
               0.0, 1.0,
               0.0, 0.0);
    return ret;
}





/**
 *
 */
Matrix from_basis(Point const x_basis, Point const y_basis, Point const offset)
{
    Matrix const ret(x_basis[X], y_basis[X],
                     x_basis[Y], y_basis[Y],
                     offset[X], offset[Y]);
    return ret;
}




/**
 * Returns a rotation matrix corresponding by the specified angle (in radians) about the origin.
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
{
}





/**
 *  Return the determinant of the Matrix
 */
Geom::Coord Matrix::det() const
{
    return _c[0] * _c[3] - _c[1] * _c[2];
}





/**
 * Return the scalar of the descriminant of the Matrix
 */
Geom::Coord Matrix::descrim2() const
{
    return fabs(det());
}





/**
 *  Return the descriminant of the Matrix
 */
Geom::Coord Matrix::descrim() const
{
    return sqrt(descrim2());
}





/**
 *  Assign a matrix to a given coordinate array
 */
Matrix &Matrix::assign(Coord const *array)
{
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





/**
 *  Copy this matrix's values to an array
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





/**
 *
 */
double expansion(Matrix const &m) {
    return sqrt(fabs(m.det()));
}





/**
 *
 */
double Matrix::expansion() const {
    return sqrt(fabs(det()));
}





/**
 *
 */
double Matrix::expansionX() const {
    return sqrt(_c[0] * _c[0] + _c[1] * _c[1]);
}





/**
 *
 */
double Matrix::expansionY() const {
    return sqrt(_c[2] * _c[2] + _c[3] * _c[3]);
}





/**
 *
 */
bool Matrix::is_translation(Coord const eps) const {
    return ( fabs(_c[0] - 1.0) < eps &&
             fabs(_c[3] - 1.0) < eps &&
             fabs(_c[1])       < eps &&
             fabs(_c[2])       < eps   );
}



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
