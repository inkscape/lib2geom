/*
 * d2.cpp - Lifts one dimensional objects into 2d 
 *
 * Copyright 2007 Michael Sloan <mgsloan@gmail.com>
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
 * in the file COPYING-LGPL-2.1; if not, output to the Free Software
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
 *
 */

#include "d2.h"
#include "rect.h"

namespace Geom {

D2<SBasis> derivative(D2<SBasis> const & a) {
    D2<SBasis> r;
    for(unsigned i = 0; i < 2; i++)
        r[i]=derivative(a[i]);
    return r;
}

D2<SBasis> integral(D2<SBasis> const & a) {
    D2<SBasis> r;
    for(unsigned i = 0; i < 2; i++)
        r[i]=integral(a[i]);
    return r;
}

SBasis L2(D2<SBasis> const & a, int k) { return sqrt(dot(a, a), k); }
double L2(D2<double> const & a) { return hypot(a[0], a[1]); }

D2<SBasis> multiply(Linear const & a, D2<SBasis> const & b) {
    D2<SBasis> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = multiply(a, b[i]);
    return r;
}

D2<SBasis> operator*(Linear const & a, D2<SBasis> const & b) {
    return multiply(a, b);
}

D2<SBasis> operator+(D2<SBasis> const & a, Point b) {
    D2<SBasis> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a[i] + Linear(b[i]);
    return r;
}

D2<SBasis> multiply(SBasis const & a, D2<SBasis> const & b) {
    D2<SBasis> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = multiply(a, b[i]);
    return r;
}

D2<SBasis> truncate(D2<SBasis> const & a, unsigned terms) {
    D2<SBasis> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = truncate(a[i], terms);
    return r;
}

unsigned sbasisSize(D2<SBasis> const & a) {
    return std::max((unsigned) a[0].size(), (unsigned) a[1].size());
}

//TODO: Is this sensical? shouldn't it be like pythagorean or something?
double tailError(D2<SBasis> const & a, unsigned tail) {
    return std::max(a[0].tailError(tail), a[1].tailError(tail));
}

bool isFinite(D2<SBasis> const & a) {
    for(unsigned i = 0; i < 2; i++)
        if(!a[i].isFinite())
            return false;
    return true;
}

Piecewise<D2<SBasis> > sectionize(D2<Piecewise<SBasis> > const &a) {
    Piecewise<SBasis> x = partition(a[0], a[1].cuts), y = partition(a[1], a[0].cuts);
    assert(x.size() == y.size());
    Piecewise<D2<SBasis> > ret;
    for(int i = 0; i < x.size(); i++)
        ret.push_seg(D2<SBasis>(x[i], y[i]));
    ret.cuts.insert(ret.cuts.end(), x.cuts.begin(), x.cuts.end());
    return ret;
}

Rect boundsExact(D2<SBasis> const & s, int order) {
    Interval res[2];
    for(int d = 0; d < 2; d++)
        res[d] = s[d].boundsExact(order);
    //TODO: rewrite with interval constructor
    return Rect(Point(res[0].min(), res[1].min()),
                Point(res[0].max(), res[1].max()));
}

Rect boundsLocal(D2<SBasis> const & s, double t0, double t1, int order) {
    Interval res[2];
    for(int d = 0; d < 2; d++)
        res[d] = s[d].boundsLocal(t0, t1, order);
    //TODO: rewrite with interval constructor
    return Rect(Point(res[0].min(), res[1].min()),
                Point(res[0].max(), res[1].max()));
}

};
