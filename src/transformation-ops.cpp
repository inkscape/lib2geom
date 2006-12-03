#include "point-matrix-ops.h"

#include "transformation-ops.h"

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

Point operator/(Point const &p, Matrix const &m) {
    return p * m.inverse();
}

Matrix operator/(Matrix const &a, Matrix const &b) {
    return a * b.inverse();
}


/** Multiplies a translate with a scale.
 \return a Matrix which not only translates by the scaled translation, but also includes the scaling.
 */
Matrix operator*(translate const &t, scale const &s) {
    Matrix ret(s);
    ret[4] = t[X] * s[X];
    ret[5] = t[Y] * s[Y];

    assert_close( ret, Matrix(t) * Matrix(s) );
    return ret;
}

/** Multiplies a translate with a rotate.
 \return a Matrix which not only translates by the rotated translation, but also includes the rotation.
 */
Matrix operator*(translate const &t, rotate const &r) {
    Matrix ret(r);
    ret.set_translation(t.offset * ret);

    assert_close( ret, Matrix(t) * Matrix(r) );
    return ret;
}

/** Multiplies a translate with a Matrix.
 \return a Matrix which not only translates by the transformed translation, but also includes the matrix's transformation.
 */
Matrix operator*(translate const &t, Matrix const &m) {
    Matrix ret(m);
    ret[4] += m[0] * t[X] + m[2] * t[Y];
    ret[5] += m[1] * t[X] + m[3] * t[Y];

    assert_close( ret, Matrix(t) * m );
    return ret;
}


//***************
//Scale operators

/** Multiplies a scale with a translate.
 \return a Matrix which scales, and then translates.
 */
Matrix operator*(scale const &s, translate const &t) {
    Matrix ret(s);
    ret.set_translation(t.offset);

    assert_close( ret, Matrix(s) * t );
    return ret;
}

/** Multiplies a scale with a Matrix.
 \return a Matrix where the x and y axii have been scaled.
 */
Matrix operator*(scale const &s, Matrix const &m) {
    Matrix ret(m);
    ret[0] *= s[X];
    ret[1] *= s[X];
    ret[2] *= s[Y];
    ret[3] *= s[Y];

    assert_close( ret, Matrix(s) * m );
    return ret;
}


//****************
//Rotate operators
//TODO: Rotate translate?

/** Multiplies a rotation with a Matrix. */
Matrix operator*(rotate const &a, Matrix const &b) {
    return Matrix(a) * b;
}


//****************
//Matrix operators

/** Multiplies a Matrix with a translation, effectively adding the translation to the matrix. */
Matrix operator*(Matrix const &m, translate const &t) {
    Matrix ret(m);
    ret[4] += t[X];
    ret[5] += t[Y];

    assert_close( ret, m * Matrix(t) );
    return ret;
}

/** Divides a Matrix by a scale.  This is the same thing as multiplying by the inverse of the scale. */
Matrix operator/(Matrix const &m, scale const &s) {
    Geom::Matrix ret(m);
    ret[0] /= s[X]; ret[1] /= s[Y];
    ret[2] /= s[X]; ret[3] /= s[Y];
    ret[4] /= s[X]; ret[5] /= s[Y];

    assert_close( ret, m * Matrix(s.inverse()) );
    return ret;
}

/** Multiplies a Matrix with a scale, scaling up every aspect of the transformation. */ 
Matrix operator*(Matrix const &m, scale const &s) {
    Matrix ret(m);
    ret[0] *= s[X]; ret[1] *= s[Y];
    ret[2] *= s[X]; ret[3] *= s[Y];
    ret[4] *= s[X]; ret[5] *= s[Y];

    assert_close( ret, m * Matrix(s) );
    return ret;
}

/** Multiplies a Matrix with a rotation. */
Matrix operator*(Matrix const &m, rotate const &r) {
  return m * Matrix(r);
}

}

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
