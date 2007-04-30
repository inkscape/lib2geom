/*
 * d2.h - Lifts one dimensional objects into 2d 
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

#ifndef _2GEOM_D2
#define _2GEOM_D2

#include "point.h"
#include "interval.h"

#include <boost/concept_check.hpp>
#include "concepts.h"

using namespace boost;

namespace Geom{

template <class T>
class D2{
    //BOOST_CLASS_REQUIRE(T, boost, AssignableConcept);
  private:
    T f[2];

  public:
    D2() {}

    D2(T const &a, T const &b) {
        f[X] = a;
        f[Y] = b;
    }

    T& operator[](unsigned i)              { return f[i]; }
    T const & operator[](unsigned i) const { return f[i]; }

    //TODO: implements a Fragment2D
    typedef Point output_type;
    //TODO: doesn't make much sense in d2 context
    bool isZero() const {
        function_requires<FragmentConcept<T> >();
        return f[X].isZero() && f[Y].isZero();
    }
    bool isFinite() const {
        function_requires<FragmentConcept<T> >();
        return f[X].isFinite() && f[Y].isFinite();
    }
    Point at0() const { 
        function_requires<FragmentConcept<T> >();
        return Point(f[X].at0(), f[Y].at0());
    }
    Point at1() const {
        function_requires<FragmentConcept<T> >();
        return Point(f[X].at1(), f[Y].at1());
    }
    Point pointAt(double t) const {
        function_requires<FragmentConcept<T> >();
        return (*this)(t);
    }
    D2<SBasis> toSBasis() const {
        function_requires<FragmentConcept<T> >();
        return D2<SBasis>(f[X].toSBasis(), f[Y].toSBasis());
    }

    /*Rect reverse() const {
        function_requires<FragmentConcept<T> >();
        return D2<T>(f[0].reverse(), f[1].reverse());
    }*/

    Point operator()(double t) const;
    Point operator()(double x, double y) const;
};

template <typename T>
D2<T> reverse(const D2<T> &a) {
    function_requires<FragmentConcept<T> >();
    return D2<T>(reverse(a[X]), reverse(a[Y]));
}

//IMPL: AddableConcept
template <typename T>
inline D2<T>
operator+(D2<T> const &a, D2<T> const &b) {
    function_requires<AddableConcept<T> >();

    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a[i] + b[i];
    return r;
}
template <typename T>
inline D2<T>
operator-(D2<T> const &a, D2<T> const &b) {
    function_requires<AddableConcept<T> >();

    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a[i] - b[i];
    return r;
}
template <typename T>
inline D2<T>
operator+=(D2<T> &a, D2<T> const &b) {
    function_requires<AddableConcept<T> >();

    for(unsigned i = 0; i < 2; i++)
        a[i] += b[i];
    return a;
}
template <typename T>
inline D2<T>
operator-=(D2<T> &a, D2<T> const & b) {
    function_requires<AddableConcept<T> >();

    for(unsigned i = 0; i < 2; i++)
        a[i] -= b[i];
    return a;
}

//IMPL: ScalableConcept
template <typename T>
inline D2<T>
operator-(D2<T> const & a) {
    function_requires<ScalableConcept<T> >();
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = -a[i];
    return r;
}
template <typename T>
inline D2<T>
operator*(D2<T> const & a, Point const & b) {
    function_requires<ScalableConcept<T> >();

    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a[i] * b[i];
    return r;
}
template <typename T>
inline D2<T>
operator/(D2<T> const & a, Point const & b) {
    function_requires<ScalableConcept<T> >();
    //TODO: b==0?
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a[i] / b[i];
    return r;
}
template <typename T>
inline D2<T>
operator*=(D2<T> &a, Point const & b) {
    function_requires<ScalableConcept<T> >();

    for(unsigned i = 0; i < 2; i++)
        a[i] *= b[i];
    return a;
}
template <typename T>
inline D2<T>
operator/=(D2<T> &a, Point const & b) {
    function_requires<ScalableConcept<T> >();
    //TODO: b==0?
    for(unsigned i = 0; i < 2; i++)
        a[i] /= b[i];
    return a;
}

template <typename T> inline D2<T> operator*(D2<T> const & a, double b) { return a * Point(b,b); }
template <typename T> inline D2<T> operator*=(D2<T> & a, double b) { return a *= Point(b,b); }

//IMPL: OffsetableConcept
template <typename T>
inline D2<T>
operator+(D2<T> const & a, Point b) {
    function_requires<OffsetableConcept<T> >();
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a[i] + b[i];
    return r;
}
template <typename T>
inline D2<T>
operator-(D2<T> const & a, Point b) {
    function_requires<OffsetableConcept<T> >();
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = a[i] - b[i];
    return r;
}
template <typename T>
inline D2<T>
operator+=(D2<T> & a, Point b) {
    function_requires<OffsetableConcept<T> >();
    for(unsigned i = 0; i < 2; i++)
        a[i] += b[i];
    return a;
}
template <typename T>
inline D2<T>
operator-=(D2<T> & a, Point b) {
    function_requires<OffsetableConcept<T> >();
    for(unsigned i = 0; i < 2; i++)
        a[i] -= b[i];
    return a;
}

template <typename T>
inline T
dot(D2<T> const & a, D2<T> const & b) {
    function_requires<AddableConcept<T> >();
    function_requires<MultiplicableConcept<T> >();

    T r;
    for(unsigned i = 0; i < 2; i++)
        r += a[i] * b[i];
    return r;
}

/* Doesn't match composition
template <typename T>
inline D2<T>
compose(T const & a, D2<T> const & b) {
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = compose(a,b[i]);
    return r;
}
*/

template <typename T>
inline D2<T>
rot90(D2<T> const & a) {
    function_requires<ScalableConcept<T> >();
    return D2<T>(-a[Y], a[X]);
}

template <typename T>
inline T
cross(D2<T> const & a, D2<T> const & b) {
    function_requires<ScalableConcept<T> >();
    function_requires<MultiplicableConcept<T> >();

    //TODO: check sign conventions...
    return a[0] * b[1] - a[1] * b[0];
}

//TODO: encode with concepts
template <typename T>
inline D2<T>
compose(D2<T> const & a, T const & b) {
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = compose(a[i],b);
    return r;
}

//TODO: encode with concepts
template <typename T>
inline D2<T>
composeEach(D2<T> const & a, D2<T> const & b) {
    D2<T> r;
    for(unsigned i = 0; i < 2; i++)
        r[i] = compose(a[i],b[i]);
    return r;
}

template<typename T>
inline Point
D2<T>::operator()(double t) const {
    //TODO: restrict to 1D pw or fragment
    Point p;
    for(int i = 0; i < 2; i++)
       p[i] = (*this)[i](t);
    return p;
}

template<typename T>
inline Point
D2<T>::operator()(double x, double y) const {
    //TODO: restrict to 2D pw or fragment
    Point p;
    for(int i = 0; i < 2; i++)
       p[i] = (*this)[i](x, y);
    return p;
}

} //end namespace Geom

//D2<Interval> specialization:

 /* Authors of original rect class:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   bulia byak <buliabyak@users.sf.net>
 *   MenTaLguY <mental@rydia.net>
 */

#include "matrix.h"

namespace Geom {

typedef D2<Interval> Rect;

template<>
class D2<Interval> {
  private:
    Interval f[2];  
    D2<Interval>();

  public:
    D2<Interval>(Interval const &a, Interval const &b) {
        f[X] = a;
        f[Y] = b;
    }

    D2<Interval>(Point const & a, Point const & b) {
        f[X] = Interval(a[X], b[X]);
        f[Y] = Interval(a[Y], b[Y]);
    }

    Interval& operator[](unsigned i)              { return f[i]; }
    Interval const & operator[](unsigned i) const { return f[i]; }


    inline Point min() const { return Point(f[X].min(), f[Y].min()); }
    inline Point max() const { return Point(f[X].max(), f[Y].max()); }

    /** returns the four corners of the rectangle in order
     *  (clockwise if +Y is up, anticlockwise if +Y is down) */
    Point corner(unsigned i) const {
        switch(i % 4) {
	case 0: return Point(f[X].min(), f[Y].min());
	case 1: return Point(f[X].max(), f[Y].min());
	case 2: return Point(f[X].max(), f[Y].max());
	case 3: return Point(f[X].min(), f[Y].max());
	}
    }

    /** returns a vector from min to max. */
    Point dimensions() const { return Point(f[X].extent(), f[Y].extent()); }
    Point midpoint() const { return Point(f[X].middle(), f[Y].middle()); }

    double area() const { return f[X].extent() * f[Y].extent(); }
    double maxExtent() const { return MAX(f[X].extent(), f[Y].extent()); }

    bool isEmpty()                 const { return f[X].isEmpty()        && f[Y].isEmpty(); }
    bool intersects(Rect const &r) const { return f[X].intersects(r[X]) && f[Y].intersects(r[Y]); }
    bool contains(Rect const &r)   const { return f[X].contains(r[X])   && f[Y].contains(r[Y]); }
    bool contains(Point const &p)  const { return f[X].contains(p[X])   && f[Y].contains(p[Y]); }

    void expandTo(Point p)        { f[X].extendTo(p[X]);  f[Y].extendTo(p[Y]); }
    void unionWith(Rect const &b) { f[X].unionWith(b[X]); f[Y].unionWith(b[Y]); }

    void expandBy(double amnt)    { f[X].expandBy(amnt);  f[Y].expandBy(amnt); }
    void expandBy(Point const p)  { f[X].expandBy(p[X]);  f[Y].expandBy(p[Y]); }

    /** Transforms the rect by m. Note that it gives correct results only for scales and translates */
    inline Rect operator*(Matrix const m) const { return Rect(min() * m, max() * m); }

    inline bool operator==(Rect const &b) { return f[X] == b[X] && f[Y] == b[Y]; }
};

//TODO: implement intersect

//D2 fragment usage of Rect:

template <typename T>
Rect boundsFast(const D2<T> &a) {
    function_requires<FragmentConcept<T> >();        
    return Rect(boundsFast(a[X]), boundsFast(a[Y]));
}
template <typename T>
Rect boundsExact(const D2<T> &a) {
    function_requires<FragmentConcept<T> >();        
    return Rect(boundsExact(a[X]), boundsExact(a[Y]));
}
template <typename T>
Rect boundsLocal(const D2<T> &a, const Interval &t) {
    function_requires<FragmentConcept<T> >();        
    return Rect(boundsLocal(a[X], t), boundsLocal(a[Y], t));
}

} //end namespace decl

//D2<SBasis> specific decls:

#include "s-basis.h"
#include "s-basis-2d.h"
#include "pw.h"

namespace Geom{

inline D2<SBasis> compose(D2<SBasis> const & a, SBasis const & b) {
    return D2<SBasis>(compose(a[X], b), compose(a[Y], b));
}

D2<SBasis> derivative(D2<SBasis> const & a);
D2<SBasis> integral(D2<SBasis> const & a);

SBasis L2(D2<SBasis> const & a, int k);
double L2(D2<double> const & a);

D2<SBasis> multiply(Linear const & a, D2<SBasis> const & b);
inline D2<SBasis> operator*(Linear const & a, D2<SBasis> const & b) { return multiply(a, b); }
D2<SBasis> multiply(SBasis const & a, D2<SBasis> const & b);
D2<SBasis> truncate(D2<SBasis> const & a, unsigned terms);

unsigned sbasisSize(D2<SBasis> const & a);
double tailError(D2<SBasis> const & a, unsigned tail);
bool isFinite(D2<SBasis> const & a);

Piecewise<D2<SBasis> > sectionize(D2<Piecewise<SBasis> > const &a);

inline Rect boundsFast(D2<SBasis> const & s, int order=0) {
    return Rect(boundsFast(s[X], order),
                boundsFast(s[Y], order));
}
inline Rect boundsLocal(D2<SBasis> const & s, Interval i, int order=0) {
    return Rect(boundsLocal(s[X], i, order),
                boundsLocal(s[Y], i, order));
}
};

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
#endif
