/**
 * \file
 * \brief  Path - Series of continuous curves
 */ /*
  * Authors:
  *   MenTaLguY <mental@rydia.net>
  *   Marco Cecchetti <mrcekets at gmail.com>
  *
  * Copyright 2007-2008 Authors
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
#include <2geom/curve.h>
#include <2geom/bezier-curve.h>
#include <2geom/transforms.h>

namespace Geom {

class Path;

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

/*
 * Open and closed paths: all paths, whether open or closed, store a final
 * segment which connects the initial and final endpoints of the "real"
 * path data.  While similar to the "z" in an SVG path, it exists for
 * both open and closed paths, and is not considered part of the "normal"
 * path data, which is always covered by the range [begin(), end_open()).
 * Conversely, the range [begin(), end_closed()) always contains the "extra"
 * closing segment.
 *
 * The only difference between a closed and an open path is whether
 * end_default() returns end_closed() or end_open().  The idea behind this
 * is to let any path be stroked using [begin(), end_default()), and filled
 * using [begin(), end_closed()), without requiring a separate "filled" version
 * of the path to use for filling.
 *
 * \invariant : _curves always contains at least one segment. The last segment
 *              is always of type ClosingSegment. All constructors take care of this.
                (_curves.size() > 0) && dynamic_cast<ClosingSegment>(_curves.back())
 */
class Path
    : boost::equality_comparable1< Path
    , MultipliableNoncommutative< Path, Affine
    , MultipliableNoncommutative< Path, Translate
      > > >
{
  public:
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

    explicit Path(Point p = Point())
        : _curves(new Sequence())
        , _closing_seg(new ClosingSegment(p, p))
        , _closed(false)
    {
        _getCurves().push_back(_closing_seg);
    }

    Path(const_iterator const &first, const_iterator const &last, bool closed = false)
        : _curves(new Sequence(seq_iter(first), seq_iter(last)))
        , _closed(closed)
    {
        if (!_getCurves().empty()) {
            _closing_seg = new ClosingSegment(_curves->back().finalPoint(), _curves->front().initialPoint());
        } else {
            _closing_seg = new ClosingSegment();
        }
        _getCurves().push_back(_closing_seg);
    }

    virtual ~Path() {}

    // Path &operator=(Path const &other) - use default assignment operator

    /// \todo Add noexcept specifiers for C++11
    void swap(Path &other) {
        using std::swap;
        swap(other._curves, _curves);
        swap(other._closing_seg, _closing_seg);
        swap(other._closed, _closed);
    }
    friend inline void swap(Path &a, Path &b) { a.swap(b); }

    Curve const &operator[](unsigned i) const { return _getCurves()[i]; }
    Curve const &at_index(unsigned i) const { return _getCurves()[i]; }

    Curve const &front() const { return _getCurves()[0]; }
    Curve const &back() const { return back_open(); }
    Curve const &back_open() const {
        if (empty()) {
            THROW_RANGEERROR("Path contains not enough segments");
        }
        return (*_curves)[_curves->size() - 2];
    }
    Curve const &back_closed() const { return (*_curves)[_curves->size() - 1]; }
    Curve const &back_default() const { return (_closed ? back_closed() : back_open()); }

    const_iterator begin() const { return const_iterator(*this, 0); }
    const_iterator end() const { return const_iterator(*this, size()); }
    iterator begin() { return iterator(*this, 0); }
    iterator end() { return iterator(*this, size()); }

    const_iterator end_open() const { return const_iterator(*this, size()); }
    const_iterator end_closed() const { return const_iterator(*this, size() + 1); }
    const_iterator end_default() const { return (_closed ? end_closed() : end_open()); }

    size_type size_open() const { return _getCurves().size() - 1; }
    size_type size_closed() const { return _getCurves().size(); }
    size_type size_default() const { return (_closed ? size_closed() : size_open()); }
    size_type size() const { return size_open(); }

    size_type max_size() const { return _getCurves().max_size() - 1; }

    bool empty() const { return (_getCurves().size() == 1); }
    bool closed() const { return _closed; }
    void close(bool closed = true) { _closed = closed; }

    void clear();

    OptRect boundsFast() const;
    OptRect boundsExact() const;

    Piecewise<D2<SBasis> > toPwSb() const;

    bool operator==(Path const &other) const;

    Path &operator*=(Affine const &m);
    Path &operator*=(Translate const &m); // specialization over Affine, for faster computation

    Point pointAt(Coord t) const;
    Coord valueAt(Coord t, Dim2 d) const;

    Point operator()(Coord t) const { return pointAt(t); }

    std::vector<Coord> roots(double v, Dim2 d) const;

    std::vector<Coord> allNearestTimes(Point const &_point, double from, double to) const;

    std::vector<Coord> allNearestTimes(Point const &_point) const {
        unsigned int sz = size();
        if (closed())
            ++sz;
        return allNearestTimes(_point, 0, sz);
    }

    std::vector<Coord> nearestTimePerCurve(Point const &_point) const;

    Coord nearestTime(Point const &_point, double from, double to, double *distance_squared = NULL) const;

    Coord nearestTime(Point const &_point, double *distance_squared = NULL) const {
        unsigned int sz = size();
        if (closed())
            ++sz;
        return nearestTime(_point, 0, sz, distance_squared);
    }

    void appendPortionTo(Path &p, Coord f, Coord t) const;

    Path portion(Coord f, Coord t) const {
        Path ret;
        ret.close(false);
        appendPortionTo(ret, f, t);
        return ret;
    }
    Path portion(Interval i) const { return portion(i.min(), i.max()); }

    Path reversed() const;

    void insert(iterator const &pos, Curve const &curve, Stitching stitching = NO_STITCHING) {
        _unshare();
        Sequence::iterator seq_pos(seq_iter(pos));
        Sequence source;
        source.push_back(curve.duplicate());
        if (stitching)
            stitch(seq_pos, seq_pos, source);
        do_update(seq_pos, seq_pos, source.begin(), source.end(), source);
    }

    void insert(iterator const &pos, const_iterator const &first, const_iterator const &last,
                Stitching stitching = NO_STITCHING) {
        _unshare();
        Sequence::iterator seq_pos(seq_iter(pos));
        Sequence source(seq_iter(first), seq_iter(last));
        if (stitching)
            stitch(seq_pos, seq_pos, source);
        do_update(seq_pos, seq_pos, source.begin(), source.end(), source);
    }

    void erase(iterator const &pos, Stitching stitching = NO_STITCHING) {
        _unshare();
        Sequence::iterator seq_pos(seq_iter(pos));
        if (stitching) {
            Sequence stitched;
            stitch(seq_pos, seq_pos + 1, stitched);
            do_update(seq_pos, seq_pos + 1, stitched.begin(), stitched.end(), stitched);
        } else {
            do_update(seq_pos, seq_pos + 1, _getCurves().begin(), _getCurves().begin(), _getCurves());
        }
    }

    void erase(iterator const &first, iterator const &last, Stitching stitching = NO_STITCHING) {
        _unshare();
        Sequence::iterator seq_first = seq_iter(first);
        Sequence::iterator seq_last = seq_iter(last);
        if (stitching) {
            Sequence stitched;
            stitch(seq_first, seq_last, stitched);
            do_update(seq_first, seq_last, stitched.begin(), stitched.end(), stitched);
        } else {
            do_update(seq_first, seq_last, _getCurves().begin(), _getCurves().begin(), _getCurves());
        }
    }

    // erase last segment of path
    void erase_last() { erase(iterator(*this, size() - 1)); }

    void start(Point const &p);

    Point initialPoint() const { return (*_closing_seg)[1]; }
    Point finalPoint() const { return (*_closing_seg)[_closed ? 1 : 0]; }

    void setInitial(Point const &p) {
        if (empty())
            return;
        _unshare();
        std::auto_ptr<Curve> head(front().duplicate());
        head->setInitial(p);
        Sequence::iterator replaced = _getCurves().begin();
        Sequence source;
        source.push_back(head);
        do_update(replaced, replaced + 1, source.begin(), source.end(), source);
    }

    void setFinal(Point const &p) {
        if (empty())
            return;
        _unshare();
        std::auto_ptr<Curve> tail(back().duplicate());
        tail->setFinal(p);
        Sequence::iterator replaced = _getCurves().end() - 2;
        Sequence source;
        source.push_back(tail);
        do_update(replaced, replaced + 1, source.begin(), source.end(), source);
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

    void stitchTo(Point const &p) {
        if (!empty() && finalPoint() != p) {
            _unshare();
            do_append(new StitchSegment(finalPoint(), p));
        }
    }

    void replace(iterator const &replaced, Curve const &curve, Stitching stitching = NO_STITCHING) {
        _unshare();
        Sequence::iterator seq_replaced(seq_iter(replaced));
        Sequence source(1);
        source.push_back(curve.duplicate());
        if (stitching)
            stitch(seq_replaced, seq_replaced + 1, source);
        do_update(seq_replaced, seq_replaced + 1, source.begin(), source.end(), source);
    }

    void replace(iterator const &first_replaced, iterator const &last_replaced, Curve const &curve,
                 Stitching stitching = NO_STITCHING) {
        _unshare();
        Sequence::iterator seq_first_replaced(seq_iter(first_replaced));
        Sequence::iterator seq_last_replaced(seq_iter(last_replaced));
        Sequence source(1);
        source.push_back(curve.duplicate());
        if (stitching)
            stitch(seq_first_replaced, seq_last_replaced, source);
        do_update(seq_first_replaced, seq_last_replaced, source.begin(), source.end(), source);
    }

    void replace(iterator const &replaced, const_iterator const &first, const_iterator const &last,
                 Stitching stitching = NO_STITCHING) {
        _unshare();
        Sequence::iterator seq_replaced(seq_iter(replaced));
        Sequence source(seq_iter(first), seq_iter(last));
        if (stitching)
            stitch(seq_replaced, seq_replaced + 1, source);
        do_update(seq_replaced, seq_replaced + 1, source.begin(), source.end(), source);
    }

    void replace(iterator const &first_replaced, iterator const &last_replaced, const_iterator const &first,
                 const_iterator const &last, Stitching stitching = NO_STITCHING) {
        _unshare();
        Sequence::iterator seq_first_replaced(seq_iter(first_replaced));
        Sequence::iterator seq_last_replaced(seq_iter(last_replaced));
        Sequence source(seq_iter(first), seq_iter(last));
        if (stitching)
            stitch(seq_first_replaced, seq_last_replaced, source);
        do_update(seq_first_replaced, seq_last_replaced, source.begin(), source.end(), source);
    }

    /**
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

    void checkContinuity() const;

  private:
    static Sequence::iterator seq_iter(iterator const &iter) { return iter.path->_getCurves().begin() + iter.index; }
    static Sequence::const_iterator seq_iter(const_iterator const &iter) {
        return iter.path->_getCurves().begin() + iter.index;
    }

    Sequence &_getCurves() { return *_curves; }
    Sequence const &_getCurves() const { return *_curves; }

    void _unshare() {
        if (!_curves.unique()) {
            _curves.reset(new Sequence(*_curves));
            _closing_seg = static_cast<ClosingSegment*>(&_curves->back());
        }
    }

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

inline Coord nearest_point(Point const &p, Path const &c) { return c.nearestTime(p); }

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
