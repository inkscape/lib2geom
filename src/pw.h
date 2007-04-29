/*
 * pw.h - Piecewise function class
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

#ifndef SEEN_GEOM_PW_SB_H
#define SEEN_GEOM_PW_SB_H

#include "s-basis.h"
#include <vector>

#include <boost/concept_check.hpp>
#include "concepts.h"

using namespace std;
using namespace boost;

namespace Geom {

template <typename T>
class Piecewise {
  BOOST_CLASS_REQUIRE(T, Geom, FragmentConcept);

  public:
    vector<double> cuts;
    vector<T> segs;
    //segs[i] stretches from cuts[i] to cuts[i+1].

    Piecewise() {}

    explicit Piecewise(const T &s) {
        push_cut(0.);
        push_seg(s);
        push_cut(1.);
    }

    typedef typename T::output_type output_type;

    inline T operator[](unsigned i) const { return segs[i]; }
    inline T &operator[](unsigned i) { return segs[i]; }
    inline typename T::output_type operator()(double t) const {
        int n = segN(t);
        return segs[n](segT(t, n));
    }
    inline unsigned size() const { return segs.size(); }
    inline bool empty() const { return segs.empty(); }

    /**Convenience/implementation hiding function to add segment/cut pairs.
     * Asserts that basic size and order invariants are correct
     */
    inline void push(const T &s, double to) {
        assert(cuts.size() - segs.size() == 1);
        push_seg(s);
        push_cut(to);
    }
    //Convenience/implementation hiding function to add cuts.
    inline void push_cut(double c) {
        assert(cuts.empty() || c > cuts.back()); 
        cuts.push_back(c);
    }//IMPL: OffsetableConcept
    //Convenience/implementation hiding function to add segments.
    inline void push_seg(const T &s) { segs.push_back(s); }

    /**Returns the segment index which corresponds to a 'global' piecewise time.
     * Also takes optional low/high parameters to expedite the search for the segment.
     */
    inline int segN(double t, int low = 0, int high = -1) const {
        high = (high == -1) ? size() : high;
        if(t < cuts[0]) return 0;
        if(t >= cuts[size()]) return size() - 1;
        while(low < high) {
            int mid = (high + low) / 2; //Lets not plan on having huge (> INT_MAX / 2) cut sequences
            double mv = cuts[mid];
            if(mv < t) {
                if(t < cuts[mid + 1]) return mid; else low = mid + 1;
            } else if(t < mv) {
                if(cuts[mid - 1] < t) return mid - 1; else high = mid - 1;
            } else {
                return mid;
            }
        }
        return low;
    }

    /**Returns the time within a segment, given the 'global' piecewise time.
     * Also takes an optional index parameter which may be used for efficiency or to find the time on a
     * segment outside its range.  If it is left to its default, -1, it will call segN to find the index.
     */
    inline double segT(double t, int i = -1) const {
        if(i == -1) i = segN(t);
        return (t - cuts[i]) / (cuts[i+1] - cuts[i]);
    }

    //Offsets the piecewise domain
    inline void offsetDomain(double o) {
        if(o != 0)
            for(int i = 0; i <= size(); i++)
                cuts[i] += o;
    }

    //Scales the domain of the function by a value. 0 will result in an empty Piecewise.
    inline void scaleDomain(double s) {
        if(s == 0) {
            cuts.clear(); segs.clear();
            return;
        }
        for(int i = 0; i <= size(); i++)
            cuts[i] *= s;
    }

    //Retrieves the domain in interval form
    inline void domain() { return Interval(cuts.front(), cuts.back()); }

    //Transforms the domain into another interval
    inline void setDomain(Interval dom) {
        if(empty()) return;
        if(dom.singular()) {
            cuts.clear(); segs.clear();
            return;
        }
        double cf = cuts.front();
        double o = dom.min() - cf, s = dom.size() / (cuts.back() - cf);
        for(int i = 0; i <= size(); i++)
            cuts[i] = (cuts[i] - cf) * s + o;
    }

    //Concatenates this Piecewise function with another, offseting time of the other to match the end.
    inline void concat(const Piecewise<T> &other) {
        if(other.empty()) return;

        if(empty()) { 
            cuts = other.cuts; segs = other.segs;
            return;
        }

        segs.insert(segs.end(), other.segs.begin(), other.segs.end());
        double t = cuts.back() - other.cuts.front();        
        for(int i = 0; i < other.size(); i++)
            push_cut(other.cuts[i + 1] + t);
    }

    //Like concat, but ensures continuity.
    inline void continuousConcat(const Piecewise<T> &other) {
        function_requires<AddableConcept<typename T::output_type> >();
        if(other.empty()) return;
        typename T::output_type y = segs.back().at1() - other.segs.front().at0();

        if(empty()) {
            for(int i = 0; i < other.size(); i++)
                push_seg(y + other[i]);
            cuts = other.cuts;
            return;
        }

        double t = cuts.back() - other.cuts.front();
        for(int i = 0; i < other.size(); i++)
            push(y + other[i], other.cuts[i + 1] + t);
    }

    //returns true if the Piecewise<T> meets some basic invariants.
    inline bool invariants() const {
        // segs between cuts
        if(!(segs.size() + 1 == cuts.size() || (segs.empty() && cuts.empty())))
            return false;
        // cuts in order
        for(int i = 0; i < segs.size(); i++)
            if(cuts[i] >= cuts[i+1])
                return false;
        return true;
    }

    inline Interval boundsFast() {
        if(empty()) return Interval(0);
        Interval ret(segs[0].boundsFast());
        for(int i = 1; i < size(); i++)
            ret.unionWith(segs[i].boundsFast()); 
        return ret;
    }

    inline Interval boundsExact() {
        if(empty()) return Interval(0);
        Interval ret(segs[0].boundsExact());
        for(int i = 1; i < size(); i++)
            ret.unionWith(segs[i].boundsExact()); 
        return ret;
    }

    inline Interval boundsLocal(Interval m) {
        boundsLocal(m.min(), m.max());
    }
    inline Interval boundsLocal(double f, double t) {
        if(empty()) return Interval(0);
        if(f == t) return Interval((*this)(f));

        int fi = segN(f), ti = segN(t);
        double ft = segT(f, fi), tt = segT(t, ti);

        if(fi == ti) return segs[fi].boundsLocal(ft, tt);

        Interval ret(segs[fi].boundsLocal(ft, 1.));
        for(int i = fi + 1; i < ti; i++)
            ret.unionWith(segs[i].boundsExact());
        if(tt != 0.) ret.unionWith(segs[ti].boundsLocal(0., tt));

        return ret;
    }
};

//returns a portion of a piece of a Piecewise<T>, given the piece's index and a to/from time.
template<typename T>
T elem_portion(const Piecewise<T> &a, int i, double from, double to) {
    assert(i < a.size());
    double rwidth = 1 / (a.cuts[i+1] - a.cuts[i]);
    return portion( a[i], (from - a.cuts[i]) * rwidth, (to - a.cuts[i]) * rwidth );
}

/**Piecewise<T> partition(const Piecewise<T> &pw, vector<double> const &c);
 * Further subdivides the Piecewise<T> such that there is a cut at every value in c.
 * Precondition: c sorted lower to higher.
 * 
 * //Given Piecewise<T> a and b:
 * Piecewise<T> ac = a.partition(b.cuts);
 * Piecewise<T> bc = b.partition(a.cuts);
 * //ac.cuts should be equivalent to bc.cuts
 */
template<typename T>
Piecewise<T> partition(const Piecewise<T> &pw, vector<double> const &c) {
    if(c.empty()) return Piecewise<T>(pw);

    Piecewise<T> ret = Piecewise<T>();
    ret.cuts.reserve(c.size() + pw.cuts.size());
    ret.segs.reserve(c.size() + pw.cuts.size() - 1);

    if(pw.empty()) {
        ret.cuts = c;
        for(int i = 0; i < c.size() - 1; i++)
            ret.push_seg(T());
        return ret;
    }

    int si = 0, ci = 0;     //Segment index, Cut index

    //if the cuts have something earlier than the Piecewise<T>, add portions of the first segment
    while(c[ci] < pw.cuts.front() && ci < c.size()) {
        bool isLast = (ci == c.size()-1 || c[ci + 1] >= pw.cuts.front());
        ret.push_cut(c[ci]);
        ret.push_seg( elem_portion(pw, 0, c[ci], isLast ? pw.cuts.front() : c[ci + 1]) );
        ci++;
    }

    ret.push_cut(pw.cuts[0]);
    double prev = pw.cuts[0];    //previous cut
    //Loop which handles cuts within the Piecewise<T> domain
    //Should have the cuts = segs + 1 invariant
    while(si < pw.size() && ci <= c.size()) {
        if(ci == c.size() && prev <= pw.cuts[si]) { //cuts exhausted, straight copy the rest
            ret.segs.insert(ret.segs.end(), pw.segs.begin() + si, pw.segs.end());
            ret.cuts.insert(ret.cuts.end(), pw.cuts.begin() + si + 1, pw.cuts.end());
            return ret;
        }else if(ci == c.size() || c[ci] >= pw.cuts[si + 1]) {  //no more cuts within this segment, finalize
            if(prev > pw.cuts[si]) {      //segment already has cuts, so portion is required
                ret.push_seg(portion(pw[si], pw.segT(prev, si), 1.0));
            } else {                     //plain copy is fine
                ret.push_seg(pw[si]);
            }
            ret.push_cut(pw.cuts[si + 1]);
            prev = pw.cuts[si + 1];
            si++;
        } else if(c[ci] == pw.cuts[si]){                  //coincident
            //Already finalized the seg with the code immediately above
            ci++;
        } else {                                         //plain old subdivision
            ret.push(elem_portion(pw, si, prev, c[ci]), c[ci]);
            prev = c[ci];
            ci++;
        }
    }
    
    //input cuts extend further than this Piecewise<T>, extend the last segment.
    while(ci < c.size()) {
        if(c[ci] > prev) {
            ret.push(elem_portion(pw, pw.size() - 1, prev, c[ci]), c[ci]);
            prev = c[ci];
        }
        ci++;
    }
    return ret;
}

/**Piecewise<T> portion(const Piecewise<T> &pw, double from, double to);
 * Returns a Piecewise<T> with a defined domain of [min(from, to), max(from, to)].
 */
template<typename T>
Piecewise<T> portion(const Piecewise<T> &pw, double from, double to) {
    if(pw.empty()) return Piecewise<T>();

    Piecewise<T> ret;

    double temp = from;
    from = min(from, to);
    to = max(temp, to);
    
    int i = pw.segN(from);
    ret.push_cut(from);
    if(to < pw.cuts[i + 1]) {    //to/from inhabit the same segment
        ret.push(elem_portion(pw, i, from, to), to);
        return ret;
    }
    ret.push(portion( pw[i], pw.segT(from, i), 1.0 ), pw.cuts[i + 1]);
    i++;
    int fi = pw.segN(to, i);

    ret.segs.insert(ret.segs.end(), pw.segs.begin() + i, pw.segs.begin() + fi - 1);  //copy segs
    ret.cuts.insert(ret.cuts.end(), pw.cuts.begin() + i + 1, pw.cuts.begin() + fi);  //and their ends

    ret.push( portion(pw[fi], 0.0, pw.segT(to, fi)), to);

    return ret;
}

template<typename T>
vector<double> roots(const Piecewise<T> &pw) {
    vector<double> ret;
    for(int i = 0; i < pw.size(); i++) {
        vector<double> sr = roots(pw[i]);
        for (int j = 0; j < sr.size(); j++) ret.push_back(sr[j] * (pw.cuts[i + 1] - pw.cuts[i]) + pw.cuts[i]);

    }
    return ret;
}

//IMPL: OffsetableConcept
template<typename T>
Piecewise<T> operator+(Piecewise<T> const &a, double b) {
    function_requires<OffsetableConcept<T> >();

    Piecewise<T> ret = Piecewise<T>();
    ret.cuts = a.cuts;
    for(int i = 0; i < a.size();i++)
        ret.push_seg(a[i] + b);
    return ret;
}
template<typename T>
Piecewise<T> operator-(Piecewise<T> const &a, double b) {
    function_requires<OffsetableConcept<T> >();

    Piecewise<T> ret = Piecewise<T>();
    ret.cuts = a.cuts;
    for(int i = 0; i < a.size();i++)
        ret.push_seg(a[i] - b);
    return ret;
}
template<typename T>
Piecewise<T> operator+=(Piecewise<T>& a, double b) {
    function_requires<OffsetableConcept<T> >();

    if(a.empty()) { a.push_cut(0.); a.push(Linear(b), 1.); return a; }

    for(int i = 0; i < a.size();i++)
        a[i] += b;
    return a;
}
template<typename T>
Piecewise<T> operator-=(Piecewise<T>& a, double b) {
    function_requires<OffsetableConcept<T> >();

    if(a.empty()) { a.push_cut(0.); a.push(Linear(b), 1.); return a; }

    for(int i = 0;i < a.size();i++)
        a[i] -= b;
    return a;
}

//IMPL: ScalableConcept
template<typename T>
Piecewise<T> operator-(Piecewise<T> const &a) {
    function_requires<ScalableConcept<T> >();

    Piecewise<T> ret = Piecewise<T>();
    ret.cuts = a.cuts;
    for(int i = 0; i < a.size();i++)
        ret.push_seg(- a[i]);
    return ret;
}
template<typename T>
Piecewise<T> operator*(Piecewise<T> const &a, double b) {
    function_requires<ScalableConcept<T> >();

    if(a.empty()) return Piecewise<T>();

    Piecewise<T> ret = Piecewise<T>();
    ret.cuts = a.cuts;
    for(int i = 0; i < a.size();i++)
        ret.push_seg(a[i] * b);
    return ret;
}
template<typename T>
Piecewise<T> operator/(Piecewise<T> const &a, double b) {
    function_requires<ScalableConcept<T> >();

    //FIXME: b == 0?
    if(a.empty()) return Piecewise<T>();

    Piecewise<T> ret = Piecewise<T>();
    ret.cuts = a.cuts;
    for(int i = 0; i < a.size();i++)
        ret.push_seg(a[i] / b);
    return ret;
}
template<typename T>
Piecewise<T> operator*=(Piecewise<T>& a, double b) {
    function_requires<ScalableConcept<T> >();

    if(a.empty()) return Piecewise<T>();

    for(int i = 0; i < a.size();i++)
        a[i] *= b;
    return a;
}
template<typename T>
Piecewise<T> operator/=(Piecewise<T>& a, double b) {
    function_requires<ScalableConcept<T> >();

    //FIXME: b == 0?
    if(a.empty()) return Piecewise<T>();

    for(int i = 0; i < a.size();i++)
        a[i] /= b;
    return a;
}

//IMPL: AddableConcept
template<typename T>
Piecewise<T> operator+(Piecewise<T> const &a, Piecewise<T> const &b) {
    function_requires<AddableConcept<T> >();

    Piecewise<T> pa = partition(a, b.cuts), pb = partition(b, a.cuts);
    Piecewise<T> ret = Piecewise<T>();
    assert(pa.size() == pb.size());
    ret.cuts = pa.cuts;
    for (int i = 0; i < pa.size(); i++)
        ret.push_seg(pa[i] + pb[i]);
    return ret;
}
template<typename T>
Piecewise<T> operator-(Piecewise<T> const &a, Piecewise<T> const &b) {
    function_requires<AddableConcept<T> >();

    Piecewise<T> pa = partition(a, b.cuts), pb = partition(b, a.cuts);
    Piecewise<T> ret = Piecewise<T>();
    assert(pa.size() == pb.size());
    ret.cuts = pa.cuts;
    for (int i = 0; i < pa.size(); i++)
        ret.push_seg(pa[i] - pb[i]);
    return ret;
}
template<typename T>
inline Piecewise<T> operator+=(Piecewise<T> &a, Piecewise<T> const &b) {
    a = a+b;
    return a;
}
template<typename T>
inline Piecewise<T> operator-=(Piecewise<T> &a, Piecewise<T> const &b) {
    a = a-b;
    return a;
}

template<typename T>
Piecewise<T> operator*(Piecewise<T> const &a, Piecewise<T> const &b) {
    function_requires<MultiplicableConcept<T> >();

    Piecewise<T> pa = partition(a, b.cuts), pb = partition(b, a.cuts);
    Piecewise<T> ret = Piecewise<T>();
    assert(pa.size() == pb.size());
    ret.cuts = pa.cuts;
    for (int i = 0; i < pa.size(); i++)
        ret.push_seg(pa[i] * pb[i]);
    return ret;
}

//TODO: operator/(pw, pw)

template<typename T>
inline Piecewise<T> operator*=(Piecewise<T> &a, Piecewise<T> const &b) {
    a = a * b;
    return a;
}

//TODO: operator/=(pw, pw)

Piecewise<SBasis> divide(Piecewise<SBasis> const &a, Piecewise<SBasis> const &b, unsigned k);

Piecewise<SBasis> compose(Piecewise<SBasis> const &a, SBasis const &b);
Piecewise<SBasis> compose(Piecewise<SBasis> const &a, Piecewise<SBasis> const &b);

Piecewise<SBasis> integral(Piecewise<SBasis> const &a);
Piecewise<SBasis> derivative(Piecewise<SBasis> const &a);

}

#endif //SEEN_GEOM_PW_SB_H
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
