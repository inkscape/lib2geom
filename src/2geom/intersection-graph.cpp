/**
 * \file
 * \brief Intersection graph for Boolean operations
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2015 Authors
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

#include <2geom/intersection-graph.h>
#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <iostream>

namespace Geom {

struct IntersectionVertexLess {
    bool operator()(IntersectionVertex const &a, IntersectionVertex const &b) const {
        return a.pos < b.pos;
    }
};

/** @class PathIntersectionGraph
 * @brief Intermediate data for computing Boolean operations on paths.
 *
 * This class implements the Greiner-Hormann clipping algorithm,
 * with improvements by Foster and Overfelt.
 *
 * @ingroup Paths
 */

PathIntersectionGraph::PathIntersectionGraph(Path const &a, Path const &b, Coord precision)
    : _a(a)
    , _b(b)
{
    assert(a.closed());
    assert(b.closed());

    std::vector<PathIntersection> pxs = a.intersect(b, precision);
    if (pxs.empty()) return;

    if (pxs.size() % 2) return;

    for (std::size_t i = 0; i < pxs.size(); ++i) {
        IntersectionVertex *xa, *xb;
        xa = new IntersectionVertex();
        xb = new IntersectionVertex();
        xa->processed = xb->processed = false;
        xa->pos = pxs[i].first;
        xb->pos = pxs[i].second;
        xa->p = xb->p = pxs[i].point();
        xa->neighbor = xb;
        xb->neighbor = xa;
        _xs.push_back(xa);
        _xs.push_back(xb);
        _xalist.push_back(*xa);
        _xblist.push_back(*xb);
    }

    _xalist.sort(IntersectionVertexLess());
    _xblist.sort(IntersectionVertexLess());

    typedef IntersectionList::iterator Iter;

    // determine in/out/on flags using winding
    for (unsigned nlist = 0; nlist < 2; ++nlist) {
        IntersectionList &l = nlist ? _xblist : _xalist;
        Path const &path = nlist ? b : a;
        Path const &other = nlist ? a : b;

        for (Iter i = l.begin(); i != l.end(); ++i) {
            Iter n = boost::next(i);
            if (n == l.end()) {
                n = l.begin();
            }

            PathInterval ival = forward_interval(i->pos, n->pos, path.size());
            PathPosition mid = ival.inside(precision);

            // TODO check for degenerate cases
            // requires changes in the winding routine
            int w = other.winding(path.pointAt(mid));
            if (w % 2) {
                i->next = POINT_INSIDE;
                n->previous = POINT_INSIDE;
            } else {
                i->next = POINT_OUTSIDE;
                n->previous = POINT_OUTSIDE;
            }
        }

        for (Iter i = l.begin(); i != l.end(); ++i) {
            i->entry = ((i->next == POINT_INSIDE) && (i->previous == POINT_OUTSIDE));
        }
    }
}

PathVector PathIntersectionGraph::getUnion() const
{
    PathVector result;
    // handle the case of no intersections
    if (_xs.empty()) {
        bool b_in_a = _a.winding(_b.initialPoint()) % 2;
        bool a_in_b = _b.winding(_a.initialPoint()) % 2;

        assert(!(b_in_a && a_in_b));

        if (a_in_b) {
            result.push_back(_b);
            return result;
        }
        if (b_in_a) {
            result.push_back(_a);
            return result;
        }
        result.push_back(_a);
        result.push_back(_b);
        return result;
    }

    result = _getResult(false, false);
    return result;
}

PathVector PathIntersectionGraph::getIntersection() const
{
    PathVector result;
    if (_xs.empty()) {
        bool b_in_a = _a.winding(_b.initialPoint()) % 2;
        bool a_in_b = _b.winding(_a.initialPoint()) % 2;

        assert(!(b_in_a && a_in_b));

        if (a_in_b) {
            result.push_back(_a);
            return result;
        }
        if (b_in_a) {
            result.push_back(_b);
            return result;
        }
        return result;
    }

    result = _getResult(true, true);
    return result;
}

PathVector PathIntersectionGraph::_getResult(bool enter_a, bool enter_b) const
{
    typedef IntersectionList::const_iterator Iter;
    PathVector result;

    // reset processed status
    for (unsigned nlist = 0; nlist < 2; ++nlist) {
        IntersectionList const &l = nlist ? _xblist : _xalist;
        for (Iter k = l.begin(); k != l.end(); ++k) {
            k->processed = false;
        }
    }

    unsigned n_processed = 0;

    while (true) {
        Path const *cur = &_a, *other = &_b;
        IntersectionList const *lcur = &_xalist, *lother = &_xblist;

        // find unprocessed intersection
        Iter i = _xalist.begin();
        for (; i != _xalist.end(); ++i) {
            if (!i->processed) break;
        }
        if (i == _xalist.end()) break;

        result.push_back(Path(i->p));
        result.back().setStitching(true);

        while (!i->processed) {
            Iter prev = i;
            // determine which direction to go
            // union: always go outside
            // intersection: always go inside
            // a minus b: go inside in b, outside in a
            // b minus a: go inside in a, outside in b
            bool reverse = false;
            if (cur == &_a) {
                reverse = i->entry ^ enter_a;
            } else {
                reverse = i->entry ^ enter_b;
            }

            // get next intersection
            if (reverse) {
                if (i == lcur->begin()) {
                    i = lcur->end();
                }
                --i;
            } else {
                ++i;
                if (i == lcur->end()) {
                    i = lcur->begin();
                }
            }

            // append portion of path
            PathInterval ival = PathInterval::from_direction(prev->pos, i->pos, reverse, cur->size());
            cur->appendPortionTo(result.back(), ival, prev->p, i->p);

            // mark both vertices as processed
            prev->processed = true;
            i->processed = true;
            n_processed += 2;

            // switch to the other path
            i = lother->iterator_to(*i->neighbor);
            std::swap(lcur, lother);
            std::swap(cur, other);
        }

        assert(!result.back().empty());
    }

    assert(n_processed == _xalist.size() + _xblist.size());

    return result;
}

} // namespace Geom

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
