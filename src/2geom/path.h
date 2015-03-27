/** @file
 * @brief Path - a sequence of contiguous curves
 *//*
  * Authors:
  *   MenTaLguY <mental@rydia.net>
  *   Marco Cecchetti <mrcekets at gmail.com>
  *   Krzysztof Kosiński <tweenk.pl@gmail.com>
  *
  * Copyright 2007-2014 Authors
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

#ifndef LIB2GEOM_SEEN_PATH_H
#define LIB2GEOM_SEEN_PATH_H

#include <iterator>
#include <algorithm>
#include <boost/operators.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>
#include <2geom/intersection.h>
#include <2geom/curve.h>
#include <2geom/bezier-curve.h>
#include <2geom/transforms.h>

namespace Geom {

class Path;
class ConvexHull;

namespace PathInternal {

typedef boost::ptr_vector<Curve> Sequence;

template <typename P>
class BaseIterator
    : public boost::random_access_iterator_helper
        < BaseIterator<P>
        , Curve const
        , std::ptrdiff_t
        , Curve const *
        , Curve const &
        >
{
  protected:
    BaseIterator(P &p, unsigned i) : path(&p), index(i) {}
    // default copy, default assign
    typedef BaseIterator<P> Self;

  public:
    BaseIterator() : path(NULL), index(0) {}

    bool operator<(BaseIterator const &other) const {
        return path == other.path && index < other.index;
    }
    bool operator==(BaseIterator const &other) const {
        return path == other.path && index == other.index;
    }
    Curve const &operator*() const {
        return (*path)[index];
    }

    Self &operator++() {
        ++index;
        return *this;
    }
    Self &operator--() {
        --index;
        return *this;
    }
    Self &operator+=(std::ptrdiff_t d) {
        index += d;
        return *this;
    }
    Self &operator-=(std::ptrdiff_t d) {
        index -= d;
        return *this;
    }

  private:
    P *path;
    unsigned index;

    friend class ::Geom::Path;
};

}

/** @brief Position (generalized time value) in the path.
 *
 * This class exists because when mapping the range of multiple curves onto the same interval
 * as the curve index, we lose some precision. For instance, a path with 16 curves will
 * have 4 bits less precision than a path with 1 curve. If you need high precision results
 * in long paths, either use this class and related methods instead of the standard methods
 * pointAt(), nearestTime() and so on, or use curveAt() to first obtain the curve, then
 * call the method again to obtain a high precision result.
 * 
 * @ingroup Paths */
struct PathPosition
    : boost::totally_ordered<PathPosition>
{
    typedef PathInternal::Sequence::size_type size_type;

    Coord t; ///< Time value in the curve
    size_type curve_index; ///< Index of the curve in the path

    PathPosition() : t(0), curve_index(0) {}
    PathPosition(size_type idx, Coord tval) : t(tval), curve_index(idx) {}

    bool operator<(PathPosition const &other) const {
        if (curve_index < other.curve_index) return true;
        if (curve_index == other.curve_index) {
            return t < other.t;
        }
        return false;
    }
    bool operator==(PathPosition const &other) const {
        return curve_index == other.curve_index && t == other.t;
    }
};

template <>
struct ShapeTraits<Path> {
    typedef PathPosition TimeType;
    typedef GenericInterval<PathPosition> IntervalType;
};

/** @brief Sequence of contiguous curves, aka spline.
 *
 * Path represents a sequence of contiguous curves, also known as a spline.
 * It corresponds to a "subpath" in SVG terminology. It can represent both
 * open and closed subpaths. The final point of each curve is exactly
 * equal to the initial point of the next curve.
 *
 * The path always contains a linear closing segment that connects
 * the final point of the last "real" curve to the initial point of the
 * first curve. This way the curves form a closed loop even for open paths.
 * If the closing segment has nonzero length and the path is closed, it is
 * considered a normal part of the path data.
 *
 * Since the methods for inserting, erasing and replacing curves can cause a path
 * to become non-contiguous, they have an additional parameter, called @a stitching,
 * which determines whether non-contiguous segments are joined with additional
 * linear segments that fill the created gaps.
 *
 * Internally, Path uses copy-on-write data. This is done for two reasons: first,
 * copying a Curve requires calling a virtual function, so it's a little more expensive
 * that normal copying; and second, it reduces the memory cost of copying the path.
 * Therefore you can return Path and PathVector from functions without worrying
 * about temporary copies.
 *
 * Note that this class cannot represent arbitrary shapes, which may contain holes.
 * To do that, use PathVector, which is more generic.
 *
 * It's not very convenient to create a Path directly. To construct paths more easily,
 * use PathBuilder.
 *
 * @ingroup Paths */
class Path
    : boost::equality_comparable1< Path
    , MultipliableNoncommutative< Path, Affine
      > >
{
public:
    typedef PathPosition Position;
    typedef PathInternal::Sequence Sequence;
    typedef PathInternal::BaseIterator<Path> iterator;
    typedef PathInternal::BaseIterator<Path const> const_iterator;
    typedef Sequence::size_type size_type;
    typedef Sequence::difference_type difference_type;

    class ClosingSegment : public LineSegment {
      public:
        ClosingSegment() : LineSegment() {}
        ClosingSegment(Point const &p1, Point const &p2) : LineSegment(p1, p2) {}
        virtual Curve *duplicate() const { return new ClosingSegment(*this); }
        virtual Curve *reverse() const { return new ClosingSegment((*this)[1], (*this)[0]); }
    };

    enum Stitching {
        NO_STITCHING = 0,
        STITCH_DISCONTINUOUS
    };

    class StitchSegment : public LineSegment {
      public:
        StitchSegment() : LineSegment() {}
        StitchSegment(Point const &p1, Point const &p2) : LineSegment(p1, p2) {}
        virtual Curve *duplicate() const { return new StitchSegment(*this); }
        virtual Curve *reverse() const { return new StitchSegment((*this)[1], (*this)[0]); }
    };

    // Path(Path const &other) - use default copy constructor

    /** @brief Construct an empty path starting at the specified point. */
    explicit Path(Point p = Point())
        : _curves(new Sequence())
        , _closing_seg(new ClosingSegment(p, p))
        , _closed(false)
    {
        _curves->push_back(_closing_seg);
    }

    /** @brief Construct a path containing a range of curves. */
    template <typename Iter>
    Path(Iter first, Iter last, bool closed = false)
        : _curves(new Sequence())
        , _closed(closed)
    {
        for (Iter i = first; i != last; ++i) {
            _curves->push_back(i->duplicate());
        }
        if (!_curves->empty()) {
            _closing_seg = new ClosingSegment(_curves->back().finalPoint(),
                                              _curves->front().initialPoint());
        } else {
            _closing_seg = new ClosingSegment();
        }
        _curves->push_back(_closing_seg);
    }

    /// Construct a path from a convex hull.
    Path(ConvexHull const &);

    virtual ~Path() {}

    // Path &operator=(Path const &other) - use default assignment operator

    /** @brief Swap contents with another path
     * @todo Add noexcept specifiers for C++11 */
    void swap(Path &other) throw() {
        using std::swap;
        swap(other._curves, _curves);
        swap(other._closing_seg, _closing_seg);
        swap(other._closed, _closed);
    }
    /** @brief Swap contents of two paths.
     * @relates Path */
    friend inline void swap(Path &a, Path &b) throw() { a.swap(b); }

    /** @brief Access a curve by index */
    Curve const &operator[](size_type i) const { return (*_curves)[i]; }
    /** @brief Access a curve by index */
    Curve const &at(size_type i) const { return _curves->at(i); }

    /** @brief Access the first curve in the path.
     * Since the curve always contains at least a degenerate closing segment,
     * it is always safe to use this method. */
    Curve const &front() const { return _curves->front(); }
    /** @brief Access the last curve in the path.
     * Since the curve always contains at least a degenerate closing segment,
     * it is always safe to use this method. */
    Curve const &back() const { return back_default(); }
    Curve const &back_open() const {
        if (empty()) {
            THROW_RANGEERROR("Path contains not enough segments");
        }
        return (*_curves)[_curves->size() - 2];
    }
    Curve const &back_closed() const { return (*_curves)[_curves->size() - 1]; }
    Curve const &back_default() const { return (_closed ? back_closed() : back_open()); }

    const_iterator begin() const { return const_iterator(*this, 0); }
    const_iterator end() const { return end_default(); }
    const_iterator end_default() const { return const_iterator(*this, size_default()); }
    const_iterator end_open() const { return const_iterator(*this, size_open()); }
    const_iterator end_closed() const { return const_iterator(*this, size_closed()); }
    iterator begin() { return iterator(*this, 0); }
    iterator end() { return end_default(); }
    iterator end_default() { return iterator(*this, size_default()); }
    iterator end_open() { return iterator(*this, size_open()); }
    iterator end_closed() { return iterator(*this, size_closed()); }

    size_type size_open() const { return _curves->size() - 1; }
    size_type size_closed() const { return _curves->size(); }
    size_type size_default() const {
        return _includesClosingSegment() ? size_closed() : size_open();
    }
    size_type size() const { return size_default(); }

    size_type max_size() const { return _curves->max_size() - 1; }

    /** @brief Check whether path is empty.
     * The path is empty if it contains only the closing segment, which according
     * to the continuity invariant must be degenerate. Note that unlike standard
     * containers, two empty paths are not necessarily identical, because the
     * degenerate closing segment may be at a different point, affecting the operation
     * of methods such as appendNew(). */
    bool empty() const { return (_curves->size() == 1); }
    /** @brief Check whether the path is closed. */
    bool closed() const { return _closed; }
    /** @brief Set whether the path is closed. */
    void close(bool closed = true) { _closed = closed; }

    /** @brief Remove all curves from the path.
     * The initial and final points of the closing segment are set to (0,0). */
    void clear();

    /** @brief Get the approximate bounding box.
     * The rectangle returned by this method will contain all the curves, but it's not
     * guaranteed to be the smallest possible one */
    OptRect boundsFast() const;
    /** @brief Get a tight-fitting bounding box.
     * This will return the smallest possible axis-aligned rectangle containing
     * all the curves in the path. */
    OptRect boundsExact() const;

    Piecewise<D2<SBasis> > toPwSb() const;

    bool operator==(Path const &other) const;

    Path &operator*=(Affine const &m);

    /** @brief Get the allowed range of time values.
     * @return Values for which pointAt() and valueAt() yield valid results. */
    Interval timeRange() const;

    /** Get the curve at the specified time value.
     * @param t Time value
     * @param rest Optional storage for the corresponding time value in the curve */
    Curve const &curveAt(Coord t, Coord *rest = NULL) const;
    /** @brief Get the point at the specified time value.
     * Note that this method has reduced precision with respect to calling pointAt()
     * directly on the curve. If you want high precision results, use the version
     * that takes a Position parameter.
     * 
     * Allowed time values range from zero to the number of curves; you can retrieve
     * the allowed range of values with timeRange(). */
    Point pointAt(Coord t) const;
    /// @brief Get one coordinate (X or Y) at the specified time value.
    Coord valueAt(Coord t, Dim2 d) const;

    /// Get the curve at the specified position.
    Curve const &curveAt(Position const &pos) const;
    /// Get the point at the specified position.
    Point pointAt(Position const &pos) const;
    /// Get one coordinate at the specified position.
    Coord valueAt(Position const &pos, Dim2 d) const;

    Point operator()(Coord t) const { return pointAt(t); }

    std::vector<Coord> roots(Coord v, Dim2 d) const;

    /** @brief Determine the winding number at the specified point.
     * 
     * The winding number is the number of full turns made by a ray that connects the passed
     * point and the path's value (i.e. the result of the pointAt() method) as the time increases
     * from 0 to the maximum valid value. Positive numbers indicate turns in the direction
     * of increasing angles.
     *
     * Winding numbers are often used as the definition of what is considered "inside"
     * the shape. Typically points with either nonzero winding or odd winding are
     * considered to be inside the path. */
    int winding(Point const &p) const;

    std::vector<Coord> allNearestTimes(Point const &p, Coord from, Coord to) const;
    std::vector<Coord> allNearestTimes(Point const &p) const {
        return allNearestTimes(p, 0, size_default());
    }

    Coord nearestTime(Point const &p, Coord *dist = NULL) const;
    std::vector<Coord> nearestTimePerCurve(Point const &p) const;
    Position nearestPosition(Point const &p, Coord *dist = NULL) const;

    void appendPortionTo(Path &p, Coord f, Coord t) const;
    /** @brief Append a subset of this path to another path.
     * An extra linear segment will be inserted if the start point of the portion
     * and the final point of the target path do not match exactly.
     * The closing segment of the target path will be modified. */
    void appendPortionTo(Path &p, Position const &from, Position const &to, bool cross_start = false) const;

    /** @brief Get a subset of the current path.
     * Note that @a f can be larger than @a t, in which case the returned part of the path
     * will go in the opposite direction.
     * @param f Time value specifying the initial point of the returned path
     * @param t Time value specifying the final point of the returned path
     * @return Portion of the path */
    Path portion(Coord f, Coord t) const {
        Path ret;
        ret.close(false);
        appendPortionTo(ret, f, t);
        return ret;
    }
    /** @brief Get a subset of the current path.
     * This version takes an Interval. */
    Path portion(Interval const &i) const { return portion(i.min(), i.max()); }

    /** @brief Get a subset of the current path with full precision.
     * When @a from is larger (later in the path) than @a to, the returned portion
     * will be reversed. If @a cross_start is true, the portion will be reversed
     * and will cross the initial point of the path. Therefore, when @a from is larger
     * than @a to and @a cross_start is true, the returned portion will not be reversed,
     * but will "wrap around" the end of the path. */
    Path portion(Position const &from, Position const &to, bool cross_start = false) const {
        Path ret;
        ret.close(false);
        appendPortionTo(ret, from, to, cross_start);
        return ret;
    }

    /** @brief Obtain a reversed version of the current path.
     * The final point of the current path will become the initial point
     * of the reversed path, unless it is closed and has a non-degenerate
     * closing segment. In that case, the new initial point will be the final point
     * of the last "real" segment. */
    Path reversed() const;

    void insert(iterator pos, Curve const &curve, Stitching stitching = NO_STITCHING);
    void insert(iterator pos, const_iterator first, const_iterator last,
                Stitching stitching = NO_STITCHING);

    void erase(iterator pos, Stitching stitching = NO_STITCHING);
    void erase(iterator first, iterator last, Stitching stitching = NO_STITCHING);

    // erase last segment of path
    void erase_last() { erase(iterator(*this, size() - 1)); }

    void start(Point const &p);

    /** @brief Get the first point in the path. */
    Point initialPoint() const { return (*_closing_seg)[1]; }
    /** @brief Get the last point in the path.
     * If the path is closed, this is always the same as the initial point. */
    Point finalPoint() const { return (*_closing_seg)[_closed ? 1 : 0]; }

    void append(Curve *curve) {
        _unshare();
        stitchTo(curve->initialPoint());
        do_append(curve);
    }

    void append(Curve const &curve, Stitching stitching = NO_STITCHING) {
        _unshare();
        if (stitching)
            stitchTo(curve.initialPoint());
        do_append(curve.duplicate());
    }
    void append(D2<SBasis> const &curve, Stitching stitching = NO_STITCHING) {
        _unshare();
        if (stitching)
            stitchTo(Point(curve[X][0][0], curve[Y][0][0]));
        do_append(new SBasisCurve(curve));
    }
    void append(Path const &other, Stitching stitching = NO_STITCHING) {
        insert(end(), other.begin(), other.end(), stitching);
    }

    /** @brief Append a stitching segment ending at the specified point. */
    void stitchTo(Point const &p) {
        if (!empty() && finalPoint() != p) {
            _unshare();
            do_append(new StitchSegment(finalPoint(), p));
        }
    }

    void replace(iterator replaced, Curve const &curve, Stitching stitching = NO_STITCHING);
    void replace(iterator first_replaced, iterator last_replaced, Curve const &curve,
                 Stitching stitching = NO_STITCHING);
    void replace(iterator replaced, const_iterator first, const_iterator last,
                 Stitching stitching = NO_STITCHING);
    void replace(iterator first_replaced, iterator last_replaced, const_iterator first,
                 const_iterator last, Stitching stitching = NO_STITCHING);

    /** @brief Append a new curve to the path.
     *
     * This family of methods will automaticaly use the current final point of the path
     * as the first argument of the new curve's constructor. To call this method,
     * you'll need to write e.g.:
     * @code
       path.template appendNew<CubicBezier>(control1, control2, end_point);
       @endcode
     * It is important to note that the coordinates passed to appendNew should be finite!
     * If one of the coordinates is infinite, 2geom will throw a ContinuityError exception.
     */
    template <typename CurveType, typename A>
    void appendNew(A a) {
        _unshare();
        do_append(new CurveType(finalPoint(), a));
    }

    template <typename CurveType, typename A, typename B>
    void appendNew(A a, B b) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b));
    }

    template <typename CurveType, typename A, typename B, typename C>
    void appendNew(A a, B b, C c) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b, c));
    }

    template <typename CurveType, typename A, typename B, typename C, typename D>
    void appendNew(A a, B b, C c, D d) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b, c, d));
    }

    template <typename CurveType, typename A, typename B, typename C, typename D, typename E>
    void appendNew(A a, B b, C c, D d, E e) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b, c, d, e));
    }

    template <typename CurveType, typename A, typename B, typename C, typename D, typename E, typename F>
    void appendNew(A a, B b, C c, D d, E e, F f) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b, c, d, e, f));
    }

    template <typename CurveType, typename A, typename B, typename C, typename D, typename E, typename F, typename G>
    void appendNew(A a, B b, C c, D d, E e, F f, G g) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b, c, d, e, f, g));
    }

    template <typename CurveType, typename A, typename B, typename C, typename D, typename E, typename F, typename G,
              typename H>
    void appendNew(A a, B b, C c, D d, E e, F f, G g, H h) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b, c, d, e, f, g, h));
    }

    template <typename CurveType, typename A, typename B, typename C, typename D, typename E, typename F, typename G,
              typename H, typename I>
    void appendNew(A a, B b, C c, D d, E e, F f, G g, H h, I i) {
        _unshare();
        do_append(new CurveType(finalPoint(), a, b, c, d, e, f, g, h, i));
    }

    /** @brief Verify the continuity invariant.
     * If the path is not contiguous, this will throw a CountinuityError. */
    void checkContinuity() const;

private:
    static Sequence::iterator seq_iter(iterator const &iter) {
        return iter.path->_curves->begin() + iter.index;
    }
    static Sequence::const_iterator seq_iter(const_iterator const &iter) {
        return iter.path->_curves->begin() + iter.index;
    }

    bool _includesClosingSegment() const {
        return _closed && !_closing_seg->isDegenerate();
    }
    void _unshare() {
        if (!_curves.unique()) {
            _curves.reset(new Sequence(*_curves));
            _closing_seg = static_cast<ClosingSegment*>(&_curves->back());
        }
    }
    Position _getPosition(Coord t) const;

    void stitch(Sequence::iterator first_replaced, Sequence::iterator last_replaced, Sequence &sequence);

    void do_update(Sequence::iterator first_replaced, Sequence::iterator last_replaced, Sequence::iterator first,
                   Sequence::iterator last, Sequence &source);

    // n.b. takes ownership of curve object
    void do_append(Curve *curve);

    boost::shared_ptr<Sequence> _curves;
    ClosingSegment *_closing_seg;
    bool _closed;
}; // end class Path

Piecewise<D2<SBasis> > paths_to_pw(PathVector const &paths);

inline Coord nearest_time(Point const &p, Path const &c) { return c.nearestTime(p); }

} // end namespace Geom


#endif // LIB2GEOM_SEEN_PATH_H

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
