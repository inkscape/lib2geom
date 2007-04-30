/*
 * concepts.h - Declares various mathematical concepts, for restriction of template parameters
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

#ifndef SEEN_CONCEPTS_H
#define SEEN_CONCEPTS_H

#include "s-basis.h"
#include "interval.h"
#include "point.h"

#include <boost/concept_check.hpp>
using namespace boost;

namespace Geom {

//forward decls
template <typename T> class D2;
class SBasis;
class Interval;
class Point;

template <typename T> struct ResultTraits;

template <> struct ResultTraits<double> {
  typedef Interval bounds_type;
  typedef SBasis sb_type;
};

template <> struct ResultTraits<Point > {
  typedef D2<Interval> bounds_type;
  typedef D2<SBasis> sb_type;
};

//A concept for one-dimensional functions defined on [0,1]
template <typename T>
struct FragmentConcept {
    typedef typename T::output_type                        OutputType;
    typedef typename ResultTraits<OutputType>::bounds_type BoundsType;
    typedef typename ResultTraits<OutputType>::sb_type     SbType;
    T t;
    double d;
    OutputType o;
    bool b;
    BoundsType i;
    Interval dom;
    void constraints() {
        b = t.isZero();
        b = t.isFinite();
        o = t.at0();
        o = t.at1();
        o = t.pointAt(d);
        o = t(d);
        SbType sb = t.toSBasis();
        t = reverse(t);
        i = boundsFast(t);
        i = boundsExact(t);
        i = boundsLocal(t, dom);
    }
};

template <typename T>
struct OffsetableConcept {
    T t;
    typename T::output_type d;
    void constraints() {
        t = t + d; t += d;
        t = t - d; t -= d;
    }
};

template <typename T>
struct ScalableConcept {
    T t;
    typename T::output_type d;
    void constraints() {
        t = -t;
        t = t * d; t *= d;
        t = t / d; t /= d;
    }
};

template <class T>
struct AddableConcept {
    T i, j;
    void constraints() {
        i += j; i = i + j;
        i -= j; i = i - j;
    }
};

template <class T>
struct MultiplicableConcept {
    T i, j;
    void constraints() {
        i *= j; i = i * j;
    }
};

};

#endif //SEEN_CONCEPTS_H
