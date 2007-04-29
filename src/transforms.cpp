#include "transforms.h"

namespace Geom {

Matrix operator*(Translate const &t, Scale const &s) {
    Matrix ret(s);
    ret[4] = t[X] * s[X];
    ret[5] = t[Y] * s[Y];
    return ret;
}

Matrix operator*(Translate const &t, Rotate const &r) {
    Matrix ret(r);
    ret.setTranslation(t.vec * ret);

    assert_close( ret, Matrix(t) * Matrix(r) );
    return ret;
}

Matrix operator*(Scale const &s, Translate const &t) {
    return Matrix(s[0], 0,
                  0   , s[1],
                  t[0], t[1]);
}

Matrix operator*(Scale const &s, Matrix const &m) {
    Matrix ret(m);
    ret[0] *= s[X];
    ret[1] *= s[X];
    ret[2] *= s[Y];
    ret[3] *= s[Y];
    return ret;
}

Matrix operator*(Matrix const &m, Translate const &t) {
    Matrix ret(m);
    ret[4] += t[X];
    ret[5] += t[Y];
    return ret;
}

Matrix operator*(Matrix const &m, Scale const &s) {
    Matrix ret(m);
    ret[0] *= s[X]; ret[1] *= s[Y];
    ret[2] *= s[X]; ret[3] *= s[Y];
    ret[4] *= s[X]; ret[5] *= s[Y];
    return ret;
}

Matrix operator*(Matrix const &m0, Matrix const &m1) {
    Matrix ret;
    for(int a = 0; a < 5; a += 2) {
        for(int b = 0; b < 2; b++) {
            ret[a + b] = m0[a] * m1[b] + m0[a + 1] * m1[b + 2];
        }
    }
    ret[4] += m1[4];
    ret[5] += m1[5];
    return ret;
}

}
