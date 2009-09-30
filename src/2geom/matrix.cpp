#define __Geom_MATRIX_C__

/** \file
 * Various matrix routines.  Currently includes some Geom::Rotate etc. routines too.
 */

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Michael G. Sloan <mgsloan@gmail.com>
 *
 * This code is in public domain
 */

#include <2geom/utils.h>
#include <2geom/matrix.h>
#include <2geom/point.h>

namespace Geom {

/** Creates a Matrix given an axis and origin point.
 *  The axis is represented as two vectors, which represent skew, rotation, and scaling in two dimensions.
 *  from_basis(Point(1, 0), Point(0, 1), Point(0, 0)) would return the identity matrix.

 \param x_basis the vector for the x-axis.
 \param y_basis the vector for the y-axis.
 \param offset the translation applied by the matrix.
 \return The new Matrix.
 */
//NOTE: Inkscape's version is broken, so when including this version, you'll have to search for code with this func
Matrix from_basis(Point const x_basis, Point const y_basis, Point const offset) {
    return Matrix(x_basis[X], x_basis[Y],
                  y_basis[X], y_basis[Y],
                  offset [X], offset [Y]);
}

Point Matrix::xAxis() const {
    return Point(_c[0], _c[1]);
}

Point Matrix::yAxis() const {
    return Point(_c[2], _c[3]);
}

/** Gets the translation imparted by the Matrix.
 */
Point Matrix::translation() const {
    return Point(_c[4], _c[5]);
}

void Matrix::setXAxis(Point const &vec) {
    for(int i = 0; i < 2; i++)
        _c[i] = vec[i];
}

void Matrix::setYAxis(Point const &vec) {
    for(int i = 0; i < 2; i++)
        _c[i + 2] = vec[i];
}

/** Sets the translation imparted by the Matrix.
 */
void Matrix::setTranslation(Point const &loc) {
    for(int i = 0; i < 2; i++)
        _c[i + 4] = loc[i];
}

/** Calculates the amount of x-scaling imparted by the Matrix.  This is the scaling applied to
 *  the original x-axis region.  It is \emph{not} the overall x-scaling of the transformation.
 *  Equivalent to L2(m.xAxis())
 */
double Matrix::expansionX() const {
    return sqrt(_c[0] * _c[0] + _c[1] * _c[1]);
}

/** Calculates the amount of y-scaling imparted by the Matrix.  This is the scaling applied before
 *  the other transformations.  It is \emph{not} the overall y-scaling of the transformation. 
 *  Equivalent to L2(m.yAxis())
 */
double Matrix::expansionY() const {
    return sqrt(_c[2] * _c[2] + _c[3] * _c[3]);
}

void Matrix::setExpansionX(double val) {
    double exp_x = expansionX();
    if(!are_near(exp_x, 0.0)) {  //TODO: best way to deal with it is to skip op?
        double coef = val / expansionX();
        for(unsigned i=0;i<2;i++) _c[i] *= coef;
    }
}

void Matrix::setExpansionY(double val) {
    double exp_y = expansionY();
    if(!are_near(exp_y, 0.0)) {  //TODO: best way to deal with it is to skip op?
        double coef = val / expansionY();
        for(unsigned i=2; i<4; i++) _c[i] *= coef;
    }
}

/** Sets this matrix to be the Identity Matrix. */
void Matrix::setIdentity() {
    _c[0] = 1.0; _c[1] = 0.0;
    _c[2] = 0.0; _c[3] = 1.0;
    _c[4] = 0.0; _c[5] = 0.0;
}

/** @brief Check whether this matrix is an identity matrix.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           1 & 0 & 0 \\
           0 & 1 & 0 \\
           0 & 0 & 1 \end{array}\right]\f$ */
bool Matrix::isIdentity(Coord eps) const {
    return are_near(_c[0], 1.0, eps) && are_near(_c[1], 0.0, eps) &&
           are_near(_c[2], 0.0, eps) && are_near(_c[3], 1.0, eps) &&
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents a pure translation.
 * Will return true for the identity matrix, which represents a zero translation.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           1 & 0 & 0 \\
           0 & 1 & 0 \\
           a & b & 1 \end{array}\right]\f$ */
bool Matrix::isTranslation(Coord eps) const {
    return are_near(_c[0], 1.0, eps) && are_near(_c[1], 0.0, eps) &&
           are_near(_c[2], 0.0, eps) && are_near(_c[3], 1.0, eps);
}
/** @brief Check whether this matrix represents a pure nonzero translation.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           1 & 0 & 0 \\
           0 & 1 & 0 \\
           a & b & 1 \end{array}\right]\f$ and \f$a, b \neq 0\f$ */
bool Matrix::isNonzeroTranslation(Coord eps) const {
    return are_near(_c[0], 1.0, eps) && are_near(_c[1], 0.0, eps) &&
           are_near(_c[2], 0.0, eps) && are_near(_c[3], 1.0, eps) &&
           (!are_near(_c[4], 0.0, eps) || !are_near(_c[5], 0.0, eps));
}

/** @brief Check whether this matrix represents pure scaling.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a & 0 & 0 \\
           0 & b & 0 \\
           0 & 0 & 1 \end{array}\right]\f$. */
bool Matrix::isScale(Coord eps) const {
    return are_near(_c[1], 0.0, eps) && are_near(_c[2], 0.0, eps) && 
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents pure, nonzero scaling.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a & 0 & 0 \\
           0 & b & 0 \\
           0 & 0 & 1 \end{array}\right]\f$ and \f$a, b \neq 1\f$. */
bool Matrix::isNonzeroScale(Coord eps) const {
    return (!are_near(_c[0], 1.0, eps) || !are_near(_c[3], 1.0, eps)) &&  //NOTE: these are the diags, and the next line opposite diags
           are_near(_c[1], 0.0, eps) && are_near(_c[2], 0.0, eps) && 
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents pure uniform scaling.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a & 0 & 0 \\
           0 & a & 0 \\
           0 & 0 & 1 \end{array}\right]\f$. */
bool Matrix::isUniformScale(Coord eps) const {
    return are_near(_c[0], _c[3], eps) &&
           are_near(_c[1], 0.0, eps) && are_near(_c[2], 0.0, eps) &&  
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents pure, nonzero uniform scaling.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a & 0 & 0 \\
           0 & a & 0 \\
           0 & 0 & 1 \end{array}\right]\f$ and \f$a \neq 1\f$. */
bool Matrix::isNonzeroUniformScale(Coord eps) const {
    return !are_near(_c[0], 1.0, eps) && are_near(_c[0], _c[3], eps) &&
           are_near(_c[1], 0.0, eps) && are_near(_c[2], 0.0, eps) &&  
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents a pure rotation.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a & b & 0 \\
           -b & a & 0 \\
           0 & 0 & 1 \end{array}\right]\f$ and \f$a^2 + b^2 = 1\f$. */
bool Matrix::isRotation(Coord eps) const {
    return are_near(_c[0], _c[3], eps) && are_near(_c[1], -_c[2], eps) &&
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps) &&
           are_near(_c[0]*_c[0] + _c[1]*_c[1], 1.0, eps);
}

/** @brief Check whether this matrix represents a pure, nonzero rotation.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a & b & 0 \\
           -b & a & 0 \\
           0 & 0 & 1 \end{array}\right]\f$, \f$a^2 + b^2 = 1\f$ and \f$a \neq 1\f$. */
bool Matrix::isNonzeroRotation(Coord eps) const {
    return !are_near(_c[0], 1.0, eps) &&
           are_near(_c[0], _c[3], eps) && are_near(_c[1], -_c[2], eps) &&
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps) &&
           are_near(_c[0]*_c[0] + _c[1]*_c[1], 1.0, eps);
}

/** @brief Check whether this matrix represents pure horizontal shearing.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           1 & 0 & 0 \\
           k & 1 & 0 \\
           0 & 0 & 1 \end{array}\right]\f$. */
bool Matrix::isHShear(Coord eps) const {
    return are_near(_c[0], 1.0, eps) && are_near(_c[1], 0.0, eps) &&
           are_near(_c[3], 1.0, eps) && are_near(_c[4], 0.0, eps) &&
           are_near(_c[5], 0.0, eps);
}
/** @brief Check whether this matrix represents pure, nonzero horizontal shearing.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           1 & 0 & 0 \\
           k & 1 & 0 \\
           0 & 0 & 1 \end{array}\right]\f$ and \f$k \neq 0\f$. */
bool Matrix::isNonzeroHShear(Coord eps) const {
    return are_near(_c[0], 1.0, eps) && are_near(_c[1], 0.0, eps) &&
          !are_near(_c[2], 0.0, eps) && are_near(_c[3], 1.0, eps) &&
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents pure vertical shearing.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           1 & k & 0 \\
           0 & 1 & 0 \\
           0 & 0 & 1 \end{array}\right]\f$. */
bool Matrix::isVShear(Coord eps) const {
    return are_near(_c[0], 1.0, eps) && are_near(_c[2], 0.0, eps) &&
           are_near(_c[3], 1.0, eps) && are_near(_c[4], 0.0, eps) &&
           are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents pure, nonzero vertical shearing.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           1 & k & 0 \\
           0 & 1 & 0 \\
           0 & 0 & 1 \end{array}\right]\f$ and \f$k \neq 0\f$. */
bool Matrix::isNonzeroVShear(Coord eps) const {
    return are_near(_c[0], 1.0, eps) && !are_near(_c[1], 0.0, eps) &&
           are_near(_c[2], 0.0, eps) && are_near(_c[3], 1.0, eps) &&
           are_near(_c[4], 0.0, eps) && are_near(_c[5], 0.0, eps);
}

/** @brief Check whether this matrix represents zooming.
 * Zooming is any combination of translation and uniform scaling. It preserves angles, ratios
 * of distances between arbitrary points and unit vectors of line segments.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a & 0 & 0 \\
           0 & a & 0 \\
           b & c & 1 \end{array}\right]\f$. */
bool Matrix::isZoom(Coord eps) const {
    return are_near(_c[0], _c[3], eps) && are_near(_c[1], 0, eps) && are_near(_c[2], 0, eps);
}

/** @brief Check whether the transformation preserves areas of polygons.
 * This means that the transformation can be any combination of translation, rotation,
 * shearing and squeezing (non-uniform scaling such that the absolute value of the product
 * of Y-scale and X-scale is 1).
 * @param eps Numerical tolerance
 * @return True iff \f$|\det A| = 1\f$. */
bool Matrix::preservesArea(Coord eps) const
{
    return are_near(descrim2(), 1.0, eps);
}

/** @brief Check whether the transformation preserves angles between lines.
 * This means that the transformation can be any combination of translation, uniform scaling
 * and rotation.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a & b & 0 \\
           -b & a & 0 \\
           c & d & 1 \end{array}\right]\f$. */
bool Matrix::preservesAngles(Coord eps) const
{
    return are_near(_c[0], _c[3], eps) && are_near(_c[1], -_c[2], eps);
}

/** @brief Check whether the transformation preserves distances between points.
 * This means that the transformation can be any combination of translation and rotation.
 * @param eps Numerical tolerance
 * @return True iff the matrix is of the form
 *         \f$\left[\begin{array}{ccc}
           a & b & 0 \\
           -b & a & 0 \\
           c & d & 1 \end{array}\right]\f$ and \f$a^2 + b^2 = 1\f$. */
bool Matrix::preservesDistances(Coord eps) const
{
    return are_near(_c[0], _c[3], eps) && are_near(_c[1], -_c[2], eps) &&
           are_near(_c[0] * _c[0] + _c[1] * _c[1], 1.0, eps);
}

/** @brief Check whether this transformation flips objects.
 * A transformation flips objects if it has a negative scaling component. */
bool Matrix::flips() const {
    // TODO shouldn't this be det() < 0?
    return cross(xAxis(), yAxis()) > 0;
}

/** @brief Check whether this matrix is singular.
 * Singular matrices have no inverse, which means that applying them to a set of points
 * results in a loss of information.
 * @param eps Numerical tolerance
 * @return True iff the determinant is near zero. */
bool Matrix::isSingular(Coord eps) const {
    return are_near(det(), 0.0, eps);
}

/** @brief Compute the inverse matrix.
 * Inverse is a matrix (denoted \f$A^{-1}) such that \f$AA^{-1} = A^{-1}A = I\f$.
 * Singular matrices have no inverse (for example a matrix that has two of its columns equal).
 * For such matrices, the identity matrix will be returned instead.
 * @param eps Numerical tolerance
 * @return Inverse of the matrix, or the identity matrix if the inverse is undefined.
 * @post (m * m.inverse()).isIdentity() == true */
Matrix Matrix::inverse() const {
    Matrix d;
    
    double mx = std::max(fabs(_c[0]) + fabs(_c[1]), 
                         fabs(_c[2]) + fabs(_c[3])); // a random matrix norm (either l1 or linfty
    if(mx > 0) {
        Geom::Coord const determ = det();
        if (!rel_error_bound(determ, mx*mx)) {
            Geom::Coord const ideterm = 1.0 / (determ);
            
            d._c[0] =  _c[3] * ideterm;
            d._c[1] = -_c[1] * ideterm;
            d._c[2] = -_c[2] * ideterm;
            d._c[3] =  _c[0] * ideterm;
            d._c[4] = (-_c[4] * d._c[0] - _c[5] * d._c[2]);
            d._c[5] = (-_c[4] * d._c[1] - _c[5] * d._c[3]);
        } else {
            d.setIdentity();
        }
    } else {
        d.setIdentity();
    }

    return d;
}

/** @brief Calculate the determinant.
 * @return \f$\det A\f$. */
Geom::Coord Matrix::det() const {
    // TODO this can overflow
    return _c[0] * _c[3] - _c[1] * _c[2];
}

/** @brief Calculate the square of the descriminant.
 * This is simply the absolute value of the determinant.
 * @return \f$|\det A|\f$. */
Geom::Coord Matrix::descrim2() const {
    return fabs(det());
}

/** @brief Calculate the descriminant.
 * If the matrix doesn't contain a non-uniform scaling or shearing component, this value says
 * how will the length any line segment change after applying this transformation
 * to arbitrary objects on a plane (the new length will be
 * @code line_seg.length() * m.descrim()) @endcode.
 * @return \f$\sqrt{|\det A|}\f$. */
Geom::Coord Matrix::descrim() const {
    return sqrt(descrim2());
}

/** @brief Combine this transformation with another one.
 * After this operation, the matrix will correspond to the transformation
 * obtained by first applying the original version of this matrix, and then
 * applying @a m. */
Matrix &Matrix::operator*=(Matrix const &o) {
    Matrix A;
    for(int a = 0; a < 5; a += 2) {
        for(int b = 0; b < 2; b++) {
            A._c[a + b] = _c[a] * o._c[b] + _c[a + 1] * o._c[b + 2];
        }
    }
    A._c[4] += o._c[4];
    A._c[5] += o._c[5];
    *this = A;
    return *this;
}

//TODO: What's this!?!
Matrix elliptic_quadratic_form(Matrix const &m) {
    double od = m[0] * m[1]  +  m[2] * m[3];
    Matrix ret (m[0]*m[0] + m[1]*m[1], od,
                od, m[2]*m[2] + m[3]*m[3],
                0, 0);
    return ret; // allow NRVO
}

Eigen::Eigen(Matrix const &m) {
    double const B = -m[0] - m[3];
    double const C = m[0]*m[3] - m[1]*m[2];
    double const center = -B/2.0;
    double const delta = sqrt(B*B-4*C)/2.0;
    values[0] = center + delta; values[1] = center - delta;
    for (int i = 0; i < 2; i++) {
        vectors[i] = unit_vector(rot90(Point(m[0]-values[i], m[1])));
    }
}

static void quadratic_roots(double q0, double q1, double q2, int &n, double&r0, double&r1) {
    std::vector<double> r;
    if(q2 == 0) {
        if(q1 == 0) { // zero or infinite roots
            n = 0;
        } else {
            n = 1;
            r0 = -q0/q1;
        }
    } else {
        double desc = q1*q1 - 4*q2*q0;
        if (desc < 0)
            n = 0;
        else if (desc == 0) {
            n = 1;
            r0 = -q1/(2*q2);
        } else {
            n = 2;
            desc = std::sqrt(desc);
            double t = -0.5*(q1+sgn(q1)*desc);
            r0 = t/q2;
            r1 = q0/t;
        }
    }
}

Eigen::Eigen(double m[2][2]) {
    double const B = -m[0][0] - m[1][1];
    double const C = m[0][0]*m[1][1] - m[1][0]*m[0][1];
    //double const desc = B*B-4*C;
    //double t = -0.5*(B+sgn(B)*desc);
    int n;
    values[0] = values[1] = 0;
    quadratic_roots(C, B, 1, n, values[0], values[1]);
    for (int i = 0; i < n; i++)
        vectors[i] = unit_vector(rot90(Point(m[0][0]-values[i], m[0][1])));
    for (int i = n; i < 2; i++) 
        vectors[i] = Point(0,0);
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
