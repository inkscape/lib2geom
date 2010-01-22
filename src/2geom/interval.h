/**
 * \file
 * \brief  Simple closed interval class
 *
 * Copyright 2007 Michael Sloan <mgsloan@gmail.com>
 *
 * Original Rect/Range code by:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   bulia byak <buliabyak@users.sf.net>
 *   MenTaLguY <mental@rydia.net>
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
#ifndef LIB2GEOM_SEEN_INTERVAL_H
#define LIB2GEOM_SEEN_INTERVAL_H

#include <assert.h>
#include <boost/optional/optional.hpp>
#include <boost/operators.hpp>
#include <2geom/coord.h>
#include <2geom/isnan.h>

namespace Geom {

class OptInterval;

/** 
 * @brief Range of numbers that is never empty.
 *
 * Intervals are closed ranges \f$[a, b]\f$, which means they include their endpoints.
 * To use them as open ranges, you can use the interiorContains() methods.
 *
 * @ingroup Primitives
 */
class Interval
    : boost::equality_comparable<Interval>
{
private:
    /// @invariant _b[0] <= _b[1]
    Coord _b[2];

public:
    /// @name Create intervals.
    /// @{
    /** @brief Create an interval that contains only zero. */
    explicit Interval() { _b[0] = 0;  _b[1] = 0; }
    /** @brief Create an interval that contains a single point. */
    explicit Interval(Coord u) { _b[0] = _b[1] = u; }
    /** @brief Create an interval that contains all points between @c u and @c v. */
    Interval(Coord u, Coord v) {
        if (u <= v) {
            _b[0] = u; _b[1] = v;
        } else {
            _b[0] = v; _b[1] = u;
        }
    }

    /** @brief Create an interval containing a range of values.
     * The resulting interval will contain all values from the given range.
     * The return type of iterators must be convertible to double. The given range
     * must not be empty. For potentially empty ranges, see OptInterval.
     * @param start Beginning of the range
     * @param end   End of the range
     * @return Interval that contains all values from [start, end). */
    template <typename InputIterator>
    static Interval from_range(InputIterator start, InputIterator end) {
        assert(start != end);
        Interval result(*start++);
        for (; start != end; ++start) result.extendTo(*start);
        return result;
    }
    /** @brief Create an interval from a C-style array of values it should contain. */
    static Interval from_array(Coord const *c, int n) {
        Interval result = from_range(c, c+n);
        return result;
    }
    /// @}

    /// @name Inspect endpoints.
    /// @{
    Coord operator[](unsigned i) const { return _b[i]; }
    Coord& operator[](unsigned i) { return _b[i]; }

    Coord min() const { return _b[0]; }
    Coord max() const { return _b[1]; }
    Coord extent() const { return _b[1] - _b[0]; }
    Coord middle() const { return (_b[1] + _b[0]) * 0.5; }
    bool isSingular() const { return _b[0] == _b[1]; }
    bool isFinite() const {
        return IS_FINITE(_b[0]) && IS_FINITE(_b[1]);
    }
    /// @}

    /// @name Test coordinates and other intervals for inclusion.
    /// @{
    /** @brief Check whether the interval includes this number. */
    bool contains(Coord val) const { return _b[0] <= val && val <= _b[1]; }
    /** @brief Check whether the interior of the interval includes this number.
     * Interior means all numbers in the interval except its ends. */
    bool interiorContains(Coord val) const { return _b[0] < val && val < _b[1]; }
    /** @brief Check whether the interval includes the given interval. */
    bool contains(const Interval & val) const { return _b[0] <= val._b[0] && val._b[1] <= _b[1]; }
    /** @brief Check whether the interior of the interval includes the given interval.
     * Interior means all numbers in the interval except its ends. */
    bool interiorContains(const Interval & val) const { return _b[0] < val._b[0] && val._b[1] < _b[1]; }
    /** @brief Check whether the intervals have any common elements. */
    bool intersects(const Interval & val) const {
        return contains(val._b[0]) || contains(val._b[1]) || val.contains(*this);
    }
    /// @}
    
    bool operator==(Interval const &other) const { return _b[0] == other._b[0] && _b[1] == other._b[1]; }
    
    //IMPL: OffsetableConcept
    //TODO: rename output_type to something else in the concept
    typedef Coord output_type;
    Interval operator+(Coord amnt) {
        return Interval(_b[0] + amnt, _b[1] + amnt);
    }
    Interval operator-(Coord amnt) {
        return Interval(_b[0] - amnt, _b[1] - amnt);
    }
    Interval operator+=(Coord amnt) {
        _b[0] += amnt; _b[1] += amnt;
        return *this;
    }
    Interval operator-=(Coord amnt) {
        _b[0] -= amnt; _b[1] -= amnt;
        return *this;
    }
    
    //IMPL: ScalableConcept
    inline Interval operator-() const { return Interval(*this); }
    inline Interval operator*(Coord s) const { return Interval(_b[0]*s, _b[1]*s); }
    inline Interval operator/(Coord s) const { return Interval(_b[0]/s, _b[1]/s); }
    Interval operator*=(Coord s) {
        if(s < 0) {
            Coord temp = _b[0];
            _b[0] = _b[1]*s;
            _b[1] = temp*s;
        } else {
            _b[0] *= s;
            _b[1] *= s;
        }
        return *this;
    }
    Interval operator/=(Coord s) {
        //TODO: what about s=0?
        if(s < 0) {
            Coord temp = _b[0];
            _b[0] = _b[1]/s;
            _b[1] = temp/s;
        } else {
            _b[0] /= s;
            _b[1] /= s;
        }
        return *this;
    }
    /// @name Modify the interval.
    /// @{
    //TODO: NaN handleage for the next two?
    /** @brief Set the lower boundary of the interval.
     * When the given number is larger than the interval's largest element,
     * it will be reduced to the single number @c val. */
    void setMin(Coord val) {
        if(val > _b[1]) {
            _b[0] = _b[1] = val;
        } else {
            _b[0] = val;
        }
    }
    /** @brief Set the upper boundary of the interval.
     * When the given number is smaller than the interval's smallest element,
     * it will be reduced to the single number @c val. */
    void setMax(Coord val) {
        if(val < _b[0]) {
            _b[1] = _b[0] = val;
        } else {
            _b[1] = val;
        }
    }
    /** @brief Extend the interval to include the given number. */
    void extendTo(Coord val) {
       if(val < _b[0]) _b[0] = val;
       if(val > _b[1]) _b[1] = val;  //no else, as we want to handle NaN
    }
    /** @brief Expand or shrink the interval in both directions by the given amount.
     * After this method, the interval's length (extent) will be reduced by
     * <code>amount * 2</code>. Negative values can be given; they will shrink the interval.
     * Shrinking by a value larger than half the interval's length will create a degenerate
     * interval containing only the midpoint of the original. */
    void expandBy(double amount) {
        _b[0] -= amount;
        _b[1] += amount;
        if (_b[0] > _b[1]) {
            Coord halfway = (_b[0]+_b[1])/2;
            _b[0] = _b[1] = halfway;
        }
    }
    /** @brief Union the interval with another one.
     * The resulting interval will contain all points of both intervals.
     * It might also contain some points which didn't belong to either - this happens
     * when the intervals did not have any common elements. */
    void unionWith(const Interval & a) {
        if(a._b[0] < _b[0]) _b[0] = a._b[0];
        if(a._b[1] > _b[1]) _b[1] = a._b[1];
    }
    /// @}
    inline operator OptInterval();
};

//IMPL: AddableConcept
inline Interval operator+(const Interval & a, const Interval & b) {
    return Interval(a.min() + b.min(), a.max() + b.max());
}
inline Interval operator-(const Interval & a, const Interval & b) {
    return Interval(a.min() - b.max(), a.max() - b.min());
}
inline Interval operator+=(Interval & a, const Interval & b) { a = a + b; return a; }
inline Interval operator-=(Interval & a, const Interval & b) { a = a - b; return a; }

//There might be impls of this based off sign checks
inline Interval operator*(const Interval & a, const Interval & b) {
    Interval res(a.min() * b.min());
    res.extendTo(a.min() * b.max());
    res.extendTo(a.max() * b.min());
    res.extendTo(a.max() * b.max());
    return res;
}
inline Interval operator*=(Interval & a, const Interval & b) { a = a * b; return a; }

// 'union' conflicts with C keyword
inline Interval unify(const Interval & a, const Interval & b) {
    return Interval(std::min(a.min(), b.min()),
                    std::max(a.max(), b.max()));
}
inline Interval operator|(const Interval & a, const Interval & b) {
    return unify(a, b);
}
inline Interval operator|=(Interval & a, const Interval & b) {
    return a = unify(a, b);
}

/**
 * @brief An range of numbers that can be empty.
 * @ingroup Primitives
 */
class OptInterval : public boost::optional<Interval> {
public:
    /// @name Crate optionally empty intervals.
    /// @{
    /** @brief Create an empty interval. */
    OptInterval() : boost::optional<Interval>() {};
    /** @brief Wrap an existing interval. */
    OptInterval(Interval const &a) : boost::optional<Interval>(a) {};
    /** @brief Create an interval containing a single point. */
    OptInterval(Coord u) : boost::optional<Interval>(Interval(u)) {};
    /** @brief Create an interval containing a range of numbers. */
    OptInterval(Coord u, Coord v) : boost::optional<Interval>(Interval(u,v)) {};

    /** @brief Create a possibly empty interval containing a range of values.
     * The resulting interval will contain all values from the given range.
     * The return type of iterators must be convertible to double. The given range
     * may be empty.
     * @param start Beginning of the range
     * @param end   End of the range
     * @return Interval that contains all values from [start, end), or nothing if the range
     *         is empty. */
    template <typename InputIterator>
    static OptInterval from_range(InputIterator start, InputIterator end) {
        if (start == end) {
            OptInterval ret;
            return ret;
        }
        OptInterval ret(Interval::from_array(start, end));
        return ret;
    }
    /// @}

    /** @brief Check whether this OptInterval is empty. */
    inline bool isEmpty() { return (*this == false); };
    
    /** @brief Union with another interval, gracefully handling empty ones. */
    inline void unionWith(const OptInterval & a) {
        if (a) {
            if (*this) { // check that we are not empty
                (*this)->unionWith(*a);
            } else {
                *this = a;
            }
        }
    }
};

inline OptInterval intersect(const Interval & a, const Interval & b) {
    Coord u = std::max(a.min(), b.min()),
          v = std::min(a.max(), b.max());
    //technically >= might be incorrect, but singulars suck
    return u > v ? OptInterval()
                  : OptInterval(Interval(u, v));
}

/*inline Interval operator&(const Interval & a, const Interval & b) {
    return intersect(a, b);
}
inline Interval operator&=(Interval & a, const Interval & b) {
    return a = intersect(a, b);
}*/

inline Interval::operator OptInterval() {
    return OptInterval(*this);
}

#ifdef _GLIBCXX_IOSTREAM
inline std::ostream &operator<< (std::ostream &os, 
                                 const Geom::Interval &I) {
    os << "Interval("<<I[0] << ", "<<I[1] << ")";
    return os;
}
#endif

}
#endif //SEEN_INTERVAL_H

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
