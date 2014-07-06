/**
 * \file
 * \brief PathVector - a sequence of contiguous subpaths.
 *//*
 * Authors:
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2008-2014 authors
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

#ifndef LIB2GEOM_SEEN_PATHVECTOR_H
#define LIB2GEOM_SEEN_PATHVECTOR_H

#include <boost/concept/requires.hpp>
#include <boost/shared_ptr.hpp>
#include <2geom/forward.h>
#include <2geom/path.h>
#include <2geom/transforms.h>

namespace Geom {

struct PathVectorPosition {
    unsigned index;
    double t;
    PathVectorPosition() : index(0), t(0) {}
    PathVectorPosition(unsigned _i, double _t) : index(_i), t(_t) {}
};

/** @brief A sequence of contiguous subpaths.
 *
 * This class corresponds to the SVG notion of a path:
 * a sequence of any number of open or closed contiguous subpaths.
 */
class PathVector
    : MultipliableNoncommutative< PathVector, Affine
    , MultipliableNoncommutative< PathVector, Translate
    , MultipliableNoncommutative< PathVector, Scale
    , MultipliableNoncommutative< PathVector, Rotate
    , MultipliableNoncommutative< PathVector, HShear
    , MultipliableNoncommutative< PathVector, VShear
    , MultipliableNoncommutative< PathVector, Zoom
    , boost::addable< PathVector
      > > > > > > > >
{
    typedef std::vector<Path> Sequence;
public:
    typedef PathVectorPosition Position;
    typedef Sequence::iterator iterator;
    typedef Sequence::const_iterator const_iterator;
    typedef Sequence::size_type size_type;
    typedef Path value_type;
    typedef Path &reference;
    typedef Path const &const_reference;
    typedef Path *pointer;
    typedef std::ptrdiff_t difference_type;

    PathVector() {}
    PathVector(Path const &p)
        : _data(1, p)
    {}
    template <typename InputIter>
    PathVector(InputIter first, InputIter last)
        : _data(first, last)
    {}

    bool empty() const {
        return _data.empty();
    }
    size_type size() const { return _data.size(); }
    size_type curveCount() const;

    iterator begin() { return _data.begin(); }
    iterator end() { return _data.end(); }
    const_iterator begin() const { return _data.begin(); }
    const_iterator end() const { return _data.end(); }
    Path &operator[](size_type index) {
        return _data[index];
    }
    Path const &operator[](size_type index) const {
        return _data[index];
    }
    Path &at(size_type index) {
        return _data.at(index);
    }
    Path const &at(size_type index) const {
        return _data.at(index);
    }
    Path &front() { return _data.front(); }
    Path const &front() const { return _data.front(); }
    Path &back() { return _data.back(); }
    Path const &back() const { return _data.back(); }
    void push_back(Path const &path) {
        _data.push_back(path);
    }
    void pop_back() {
        _data.pop_back();
    }
    iterator insert(iterator pos, Path const &p) {
        return _data.insert(pos, p);
    }
    template <typename InputIter>
    void insert(iterator out, InputIter first, InputIter last) {
        _data.insert(out, first, last);
    }
    iterator erase(iterator i) {
        return _data.erase(i);
    }
    iterator erase(iterator first, iterator last) {
        return _data.erase(first, last);
    }
    void clear() { _data.clear(); }
    void resize(size_type n) { _data.resize(n); }

    void reverse(bool reverse_paths = true);
    PathVector reversed(bool reverse_paths = true);

    Interval timeRange() const;
    Point initialPoint() const {
        return _data.front().initialPoint();
    }
    Point finalPoint() const {
        return _data.back().finalPoint();
    }
    OptRect boundsFast() const;
    OptRect boundsExact() const;

    template <typename T>
    BOOST_CONCEPT_REQUIRES(((TransformConcept<T>)), (PathVector &))
    operator*=(T const &t) {
        if (empty()) return *this;
        for (iterator i = begin(); i != end(); ++i) {
            *i *= t;
        }
        return *this;
    }

    Coord valueAt(Position const &pos, Dim2 d) const { return at(pos.index).valueAt(pos.t, d); }
    Coord valueAt(Coord t, Dim2 d) const;
    Point pointAt(Position const &pos) const { return at(pos.index).pointAt(pos.t); }
    Point pointAt(Coord t) const;
    /*Curve const &curveAt(Position const &pos, Coord *rest = NULL) const;
    Curve const &curveAt(Coord t, Coord *rest = NULL) const;
    Path const &pathAt(Position const &pos, Coord *rest = NULL) const;
    Path const &pathAt(Coord t, Coord *rest = NULL) const;*/

    Coord nearestTime(Point const &p) const;
    boost::optional<Position> nearestPosition(Point const &p, double *distance_squared = NULL) const;
    std::vector<Position> allNearestPositions(Point const &p, double *distance_squared = NULL) const;

private:
    Coord _getIndices(Coord t, size_type &path_index, size_type &curve_index) const;

    Sequence _data;
};

inline OptRect bounds_fast(PathVector const &pv) { return pv.boundsFast(); }
inline OptRect bounds_exact(PathVector const &pv) { return pv.boundsExact(); }

} // end namespace Geom

#endif // LIB2GEOM_SEEN_PATHVECTOR_H

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
