#include "prim.h"

#include "point.h"
#include "nr-point.h"

#include "matrix.h"
#include "nr-matrix.h"

#include "scale.h"
#include "nr-scale.h"

#include "rotate.h"
#include "nr-rotate.h"
#include "nr-rotate-fns.h"

#include "translate.h"
#include "nr-translate.h"

Geom::Point NR2Geom(NR::Point p) {
    return Geom::Point(p[0], p[1]);
}

Geom::Matrix NR2Geom(NR::Matrix m) {
    return Geom::Matrix(m[0], m[1],
                        m[2], m[3],
                        m[4], m[5]);
}

Geom::Matrix NR2Geom(NR::NRMatrix m) {
    return Geom::Matrix(m[0], m[1],
                        m[2], m[3],
                        m[4], m[5]);
}

Geom::Scale NR2Geom(NR::scale s) {
    return Geom::Scale(s[0], s[1]);
}

Geom::Rotate NR2Geom(NR::rotate r) {
    return Geom::Rotate(NR::rotate_degrees(r));
}

Geom::Translate NR2Geom(NR::translate t) {
    return Geom::Translate(t[0], t[1]);
}

NR::Point Geom2NR(Geom::Point p) {
    return NR::Point(p[0], p[1]);
}

NR::Matrix Geom2NR(Geom::Matrix m) {
    return NR::Matrix(m[0], m[1],
                      m[2], m[3],
                      m[4], m[5]);
}

NR::scale Geom2NR(Geom::Scale s) {
    return NR::scale(s[0], s[1]);
}

NR::rotate Geom2NR(Geom::Rotate r) {
    return NR::rotate(Geom::rotate_degrees(r));
}

NR::translate Geom2NR(Geom::Translate t) {
    return NR::translate(t[0], t[1]);
}
