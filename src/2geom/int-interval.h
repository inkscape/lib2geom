/**
 *  \file
 *  \brief Closed interval of integer values
 *//*
 * Copyright 2011 Krzysztof Kosiński <tweenk.pl@gmail.com>
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
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
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
 */

#ifndef LIB2GEOM_SEEN_INT_INTERVAL_H
#define LIB2GEOM_SEEN_INT_INTERVAL_H

#include <cassert>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/operators.hpp>
#include <2geom/coord.h>

namespace Geom {

class OptIntInterval;

class IntInterval
    : boost::equality_comparable< IntInterval
    , boost::additive< IntInterval
    , boost::additive< IntInterval, IntCoord
    , boost::orable< IntInterval
      > > > >
{
    IntCoord _b[2];
public:
    /// @name Create intervals.
    /// @{
    /** @brief Create an interval that contains only zero. */
    IntInterval() { _b[0] = 0;  _b[1] = 0; }
    /** @brief Create an interval that contains a single point. */
    explicit IntInterval(IntCoord u) { _b[0] = _b[1] = u; }
    /** @brief Create an interval that contains all points between @c u and @c v. */
    IntInterval(Coord u, Coord v) {
        if (u <= v) {
            _b[0] = u; _b[1] = v;
        } else {
            _b[0] = v; _b[1] = u;
        }
    }

    /** @brief Create an interval containing a range of values.
     * The resulting interval will contain all values from the given range.
     * The return type of iterators must be convertible to IntCoord. The given range
     * must not be empty. For potentially empty ranges, see OptIntInterval.
     * @param start Beginning of the range
     * @param end   End of the range
     * @return Interval that contains all values from [start, end). */
    template <typename InputIterator>
    static IntInterval from_range(InputIterator start, InputIterator end) {
        assert(start != end);
        IntInterval result(*start++);
        for (; start != end; ++start) result.expandTo(*start);
        return result;
    }
    /** @brief Create an interval from a C-style array of values it should contain. */
    static IntInterval from_array(IntCoord const *c, unsigned n) {
        IntInterval result = from_range(c, c+n);
        return result;
    }
    /// @}

    /// @name Inspect endpoints.
    /// @{
    IntCoord min() const { return _b[0]; }
    IntCoord max() const { return _b[1]; }
    IntCoord extent() const { return max() - min(); }
    IntCoord middle() const { return (max() + min()) * 0.5; }
    bool isSingular() const { return min() == max(); }
    /// @}

    /// @name Test coordinates and other intervals for inclusion.
    /// @{
    /** @brief Check whether the interval includes this number. */
    bool contains(IntCoord val) const { return min() <= val && val <= max(); }
    /** @brief Check whether the interval includes the given interval. */
    bool contains(IntInterval const &val) const { return min() <= val.min() && val.max() <= max(); }
    /** @brief Check whether the intervals have any common elements. */
    bool intersects(IntInterval const &val) const {
        return contains(val.min()) || contains(val.max()) || val.contains(*this);
    }
    /// @}

    /// @name Modify the interval.
    /// @{
    //TODO: NaN handleage for the next two?
    /** @brief Set the lower boundary of the interval.
     * When the given number is larger than the interval's largest element,
     * it will be reduced to the single number @c val. */
    void setMin(IntCoord val) {
        if(val > _b[1]) {
            _b[0] = _b[1] = val;
        } else {
            _b[0] = val;
        }
    }
    /** @brief Set the upper boundary of the interval.
     * When the given number is smaller than the interval's smallest element,
     * it will be reduced to the single number @c val. */
    void setMax(IntCoord val) {
        if(val < _b[0]) {
            _b[1] = _b[0] = val;
        } else {
            _b[1] = val;
        }
    }
    /** @brief Extend the interval to include the given number. */
    void expandTo(IntCoord val) {
       if(val < _b[0]) _b[0] = val;
       if(val > _b[1]) _b[1] = val;  //no else, as we want to handle NaN
    }
    /** @brief Expand or shrink the interval in both directions by the given amount.
     * After this method, the interval's length (extent) will be increased by
     * <code>amount * 2</code>. Negative values can be given; they will shrink the interval.
     * Shrinking by a value larger than half the interval's length will create a degenerate
     * interval containing only the midpoint of the original. */
    void expandBy(IntCoord amount) {
        _b[0] -= amount;
        _b[1] += amount;
        if (_b[0] > _b[1]) {
            IntCoord halfway = (_b[0]+_b[1])/2;
            _b[0] = _b[1] = halfway;
        }
    }
    /** @brief Union the interval with another one.
     * The resulting interval will contain all points of both intervals.
     * It might also contain some points which didn't belong to either - this happens
     * when the intervals did not have any common elements. */
    void unionWith(IntInterval const &a) {
        if(a._b[0] < _b[0]) _b[0] = a._b[0];
        if(a._b[1] > _b[1]) _b[1] = a._b[1];
    }
    /// @}

    /// @name Operators
    /// @{
    inline operator OptIntInterval();
    bool operator==(IntInterval const &other) const { return min() == other.min() && max() == other.max(); }
    
    //IMPL: OffsetableConcept
    //TODO: rename output_type to something else in the concept
    typedef IntCoord output_type;
    /** @brief Offset the interval by a specified amount */
    IntInterval &operator+=(IntCoord amnt) {
        _b[0] += amnt; _b[1] += amnt;
        return *this;
    }
    /** @brief Offset the interval by the negation of the specified amount */
    IntInterval &operator-=(IntCoord amnt) {
        _b[0] -= amnt; _b[1] -= amnt;
        return *this;
    }
    
    /** @brief Return an interval mirrored about 0 */
    IntInterval operator-() const { IntInterval r(-_b[1], -_b[0]); return r; }
    // IMPL: AddableConcept
    /** @brief Add two intervals.
     * Sum is defined as the set of points that can be obtained by adding any two values
     * from both operands: \f$S = \{x \in A, y \in B: x + y\}\f$ */
    IntInterval &operator+=(IntInterval const &o) {
        _b[0] += o._b[0];
        _b[1] += o._b[1];
        return *this;
    }
    /** @brief Subtract two intervals.
     * Difference is defined as the set of points that can be obtained by subtracting
     * any value from the second operand from any value from the first operand:
     * \f$S = \{x \in A, y \in B: x - y\}\f$ */
    IntInterval &operator-=(IntInterval const &o) {
        // equal to *this += -o
        _b[0] -= o._b[1];
        _b[1] -= o._b[0];
        return *this;
    }
    /** @brief Union two intervals.
     * Note that the intersection-and-assignment operator is not defined for IntInterval,
     * because the result of an intersection can be empty, while an IntInterval cannot. */
    IntInterval &operator|=(IntInterval const &o) {
        unionWith(o);
        return *this;
    }
    /// @}
};

/** @brief Union two intervals
 * @relates IntInterval */
inline IntInterval unify(IntInterval const &a, IntInterval const &b) {
    return a | b;
}

/**
 * @brief A range of integers that can be empty.
 * @ingroup Primitives
 */
class OptIntInterval
    : public boost::optional<IntInterval>
    , boost::orable< OptIntInterval
    , boost::andable< OptIntInterval
      > >
{
public:
    /// @name Create optionally empty intervals of integers.
    /// @{
    /** @brief Create an empty interval. */
    OptIntInterval() : boost::optional<IntInterval>() {};
    /** @brief Wrap an existing interval. */
    OptIntInterval(IntInterval const &a) : boost::optional<IntInterval>(a) {};
    /** @brief Create an interval containing a single point. */
    OptIntInterval(Coord u) : boost::optional<IntInterval>(IntInterval(u)) {};
    /** @brief Create an interval containing a range of numbers. */
    OptIntInterval(Coord u, Coord v) : boost::optional<IntInterval>(IntInterval(u,v)) {};

    /** @brief Create a possibly empty interval containing a range of values.
     * The resulting interval will contain all values from the given range.
     * The return type of iterators must be convertible to IntCoord. The given range
     * may be empty.
     * @param start Beginning of the range
     * @param end   End of the range
     * @return Interval that contains all values from [start, end), or nothing if the range
     *         is empty. */
    template <typename InputIterator>
    static OptIntInterval from_range(InputIterator start, InputIterator end) {
        if (start == end) {
            OptIntInterval ret;
            return ret;
        }
        OptIntInterval ret(IntInterval::from_range(start, end));
        return ret;
    }
    /// @}

    /** @brief Check whether this OptInterval is empty. */
    bool isEmpty() { return !*this; };

    /** @brief Union with another interval, gracefully handling empty ones. */
    inline void unionWith(OptIntInterval const &a) {
        if (a) {
            if (*this) { // check that we are not empty
                (*this)->unionWith(*a);
            } else {
                *this = a;
            }
        }
    }
    inline void intersectWith(OptIntInterval const &o) {
        if (o && *this) {
            Coord u, v;
            u = std::max((*this)->min(), o->min());
            v = std::min((*this)->max(), o->max());
            if (u <= v) {
                *this = IntInterval(u, v);
                return;
            }
        }
        (*static_cast<boost::optional<IntInterval>*>(this)) = boost::none;
    }
    OptIntInterval &operator|=(OptIntInterval const &o) {
        unionWith(o);
        return *this;
    }
    OptIntInterval &operator&=(OptIntInterval const &o) {
        intersectWith(o);
        return *this;
    }
};

/** @brief Intersect two intervals and return a possibly empty range of numbers
 * @relates OptInterval */
inline OptIntInterval intersect(IntInterval const &a, IntInterval const &b) {
    return OptIntInterval(a) & OptIntInterval(b);
}
/** @brief Intersect two intervals and return a possibly empty range of numbers
 * @relates OptInterval */
inline OptIntInterval operator&(IntInterval const &a, IntInterval const &b) {
    return OptIntInterval(a) & OptIntInterval(b);
}

inline IntInterval::operator OptIntInterval() {
    return OptIntInterval(*this);
}

#ifdef _GLIBCXX_IOSTREAM
inline std::ostream &operator<< (std::ostream &os, 
                                 Geom::IntInterval const &I) {
    os << "Interval("<<I.min() << ", "<<I.max() << ")";
    return os;
}
#endif

} // namespace Geom
#endif // !LIB2GEOM_SEEN_INT_RECT_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
