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
#include <2geom/utils.h>
#include <iostream>
#include <iterator>

namespace Geom {

struct PathIntersectionGraph::IntersectionVertexLess {
    bool operator()(IntersectionVertex const &a, IntersectionVertex const &b) const {
        return a.pos < b.pos;
    }
};

/** @class PathIntersectionGraph
 * @brief Intermediate data for computing Boolean operations on paths.
 *
 * This class implements the Greiner-Hormann clipping algorithm,
 * with improvements inspired by Foster and Overfelt as well as some
 * original contributions.
 *
 * @ingroup Paths
 */

PathIntersectionGraph::PathIntersectionGraph(PathVector const &a, PathVector const &b, Coord precision)
{
    if (a.empty() || b.empty()) return;

    _pv[0] = a;
    _pv[1] = b;

    // all paths must be closed, otherwise we will miss some intersections
    for (int w = 0; w < 2; ++w) {
        for (std::size_t i = 0; i < _pv[w].size(); ++i) {
            _pv[w][i].close();
        }
    }

    std::vector<PVIntersection> pxs = _pv[0].intersect(_pv[1], precision);
    // NOTE: this early return means that the path data structures will not be created
    // if there are no intersections at all!
    if (pxs.empty()) return;

    // prepare intersection lists for each path component
    for (unsigned w = 0; w < 2; ++w) {
        for (std::size_t i = 0; i < _pv[w].size(); ++i) {
            _components[w].push_back(new PathData(w, i));
        }
    }

    for (std::size_t i = 0; i < pxs.size(); ++i) {
        IntersectionVertex *xa, *xb;
        xa = new IntersectionVertex();
        xb = new IntersectionVertex();
        //xa->processed = xb->processed = false;
        xa->which = 0; xb->which = 1;
        xa->pos = pxs[i].first;
        xb->pos = pxs[i].second;
        xa->p = xb->p = pxs[i].point();
        xa->neighbor = xb;
        xb->neighbor = xa;
        xa->next = xa->previous = xb->next = xb->previous = OUTSIDE;
        _xs.push_back(xa);
        _xs.push_back(xb);
        _components[0][xa->pos.path_index].xlist.push_back(*xa);
        _components[1][xb->pos.path_index].xlist.push_back(*xb);
    }

    // sort components according to time value of intersections
    for (unsigned w = 0; w < 2; ++w) {
        for (std::size_t i = 0; i < _components[w].size(); ++i) {
            _components[w][i].xlist.sort(IntersectionVertexLess());
        }
    }

    typedef IntersectionList::iterator Iter;

    // determine the winding numbers of path portions between intersections
    for (unsigned w = 0; w < 2; ++w) {
        unsigned ow = (w+1) % 2;

        for (unsigned li = 0; li < _components[w].size(); ++li) {
            IntersectionList &xl = _components[w][li].xlist;
            for (Iter i = xl.begin(); i != xl.end(); ++i) {
                //if (i->next == BOTH) continue;
                Iter n = cyclic_next(i, xl);
                std::size_t pi = i->pos.path_index;

                PathInterval ival = forward_interval(i->pos, n->pos, _pv[w][pi].size());
                PathTime mid = ival.inside(precision);

                // TODO check for degenerate cases
                int wdg = _pv[ow].winding(_pv[w][pi].pointAt(mid));
                if (wdg % 2) {
                    i->next = INSIDE;
                    n->previous = INSIDE;
                } else {
                    i->next = OUTSIDE;
                    n->previous = OUTSIDE;
                }
            }
        }
    }

    // correct disagreements
    for (unsigned w = 0; w < 2; ++w) {
        unsigned ow = (w+1) % 2;
        for (unsigned li = 0; li < _components[w].size(); ++li) {
            IntersectionList &xl = _components[w][li].xlist;
            for (Iter i = xl.begin(); i != xl.end(); ++i) {
                if (i->next == i->previous) {
                    IntersectionList &oxl = _components[ow][i->neighbor->pos.path_index].xlist;
                    InOutFlag x = BOTH;
                    if (i->next == OUTSIDE) {
                        x = INSIDE;
                    } else if (i->next == INSIDE) {
                        x = OUTSIDE;
                    }
                    Iter niter = oxl.iterator_to(*i->neighbor);
                    niter->next = niter->previous = x;
                    cyclic_prior(niter, oxl)->next = x;
                    cyclic_next(niter, oxl)->previous = x;
                }
            }
        }
    }

    // If a path has only degenerate intersections, assign its status now.
    // This protects against later accidentaly picking a point for winding
    // determination that is exactly at a removed intersection.
    for (unsigned w = 0; w < 2; ++w) {
        for (unsigned li = 0; li < _components[w].size(); ++li) {
            IntersectionList &xl = _components[w][li].xlist;
            bool has_in = false;
            bool has_out = false;
            for (Iter i = xl.begin(); i != xl.end(); ++i) {
                has_in |= (i->next == INSIDE);
                has_out |= (i->next == OUTSIDE);
            }
            if (has_in && !has_out) {
                _components[w][li].status = INSIDE;
            }
            if (!has_in && has_out) {
                _components[w][li].status = OUTSIDE;
            }
        }
    }

    // Assign entry / exit flags.
    // TODO: overlapping edges.
    for (unsigned w = 0; w < 2; ++w) {
        unsigned ow = (w+1) % 2;
        for (unsigned li = 0; li < _components[w].size(); ++li) {
            IntersectionList &xl = _components[w][li].xlist;
            // remove intersections that don't change between in/out
            // and assign exit / entry flags
            for (Iter i = xl.begin(); i != xl.end();) {
                if (i->previous == i->next) {
                    std::size_t oli = i->neighbor->pos.path_index;
                    IntersectionList &oxl = _components[ow][oli].xlist;
                    oxl.erase(oxl.iterator_to(*i->neighbor));
                    xl.erase(i++);
                } else {
                    i->entry = ((i->next == INSIDE) && (i->previous == OUTSIDE));
                    ++i;
                }
            }
        }
    }
}

PathVector PathIntersectionGraph::getUnion()
{
    PathVector result = _getResult(false, false);
    _handleNonintersectingPaths(result, 0, false);
    _handleNonintersectingPaths(result, 1, false);
    return result;
}

PathVector PathIntersectionGraph::getIntersection()
{
    PathVector result = _getResult(true, true);
    _handleNonintersectingPaths(result, 0, true);
    _handleNonintersectingPaths(result, 1, true);
    return result;
}

PathVector PathIntersectionGraph::getAminusB()
{
    PathVector result = _getResult(false, true);
    _handleNonintersectingPaths(result, 0, false);
    _handleNonintersectingPaths(result, 1, true);
    return result;
}

PathVector PathIntersectionGraph::getBminusA()
{
    PathVector result = _getResult(true, false);
    _handleNonintersectingPaths(result, 1, false);
    _handleNonintersectingPaths(result, 0, true);
    return result;
}

PathVector PathIntersectionGraph::getXOR()
{
    PathVector r1 = getAminusB();
    PathVector r2 = getBminusA();
    std::copy(r2.begin(), r2.end(), std::back_inserter(r1));
    return r1;
}

std::size_t PathIntersectionGraph::size() const
{
    std::size_t result = 0;
    for (std::size_t i = 0; i < _components[0].size(); ++i) {
        result += _components[0][i].xlist.size();
    }
    return result;
}

std::vector<Point> PathIntersectionGraph::intersectionPoints() const
{
    std::vector<Point> result;

    typedef IntersectionList::const_iterator Iter;
    for (std::size_t i = 0; i < _components[0].size(); ++i) {
        for (Iter j = _components[0][i].xlist.begin(); j != _components[0][i].xlist.end(); ++j) {
            result.push_back(j->p);
        }
    }
    return result;
}

PathVector PathIntersectionGraph::_getResult(bool enter_a, bool enter_b)
{
    typedef IntersectionList::iterator Iter;
    typedef boost::ptr_vector<PathData>::iterator PIter;
    PathVector result;
    if (_xs.empty()) return result;

    // reset processed status
    _ulist.clear();
    for (unsigned w = 0; w < 2; ++w) {
        for (PIter li = _components[w].begin(); li != _components[w].end(); ++li) {
            for (Iter k = li->xlist.begin(); k != li->xlist.end(); ++k) {
                _ulist.push_back(*k);
            }
        }
    }

    unsigned n_processed = 0;

    while (true) {
        // get unprocessed intersection
        if (_ulist.empty()) break;
        IntersectionVertex &iv = _ulist.front();
        unsigned w = iv.which;
        Iter i = _components[w][iv.pos.path_index].xlist.iterator_to(iv);

        result.push_back(Path(i->p));
        result.back().setStitching(true);

        while (i->_proc_hook.is_linked()) {
            Iter prev = i;
            std::size_t pi = i->pos.path_index;
            // determine which direction to go
            // union: always go outside
            // intersection: always go inside
            // a minus b: go inside in b, outside in a
            // b minus a: go inside in a, outside in b
            bool reverse = false;
            if (w == 0) {
                reverse = i->entry ^ enter_a;
            } else {
                reverse = i->entry ^ enter_b;
            }

            // get next intersection
            if (reverse) {
                i = cyclic_prior(i, _components[w][pi].xlist);
            } else {
                i = cyclic_next(i, _components[w][pi].xlist);
            }

            // append portion of path
            PathInterval ival = PathInterval::from_direction(
                prev->pos.asPathTime(), i->pos.asPathTime(),
                reverse, _pv[w][pi].size());

            _pv[w][pi].appendPortionTo(result.back(), ival, prev->p, i->p);

            // mark both vertices as processed
            //prev->processed = true;
            //i->processed = true;
            n_processed += 2;
            if (prev->_proc_hook.is_linked()) {
                _ulist.erase(_ulist.iterator_to(*prev));
            }
            if (i->_proc_hook.is_linked()) {
                _ulist.erase(_ulist.iterator_to(*i));
            }

            // switch to the other path
            w = (w+1) % 2;
            i = _components[w][i->neighbor->pos.path_index].xlist.iterator_to(*i->neighbor);
        }
        result.back().close(true);

        assert(!result.back().empty());
    }

    assert(n_processed == size() * 2);

    return result;
}

void PathIntersectionGraph::_handleNonintersectingPaths(PathVector &result, unsigned which, bool inside)
{
    /* Every component that has any intersections will be processed by _getResult.
     * Here we take care of paths that don't have any intersections. They are either
     * completely inside or completely outside the other pathvector. We test this by
     * evaluating the winding rule at the initial point. If inside is true and
     * the path is inside, we add it to the result.
     */
    unsigned w = which;
    unsigned ow = (w+1) % 2;

    for (std::size_t i = 0; i < _pv[w].size(); ++i) {
        // the path data vector might have been left empty if there were no intersections at all
        bool has_path_data = !_components[w].empty();
        // Skip if the path has intersections
        if (has_path_data && !_components[w][i].xlist.empty()) continue;
        bool path_inside = false;

        // Use the in/out determination from constructor, if available
        if (has_path_data && _components[w][i].status == INSIDE) {
            path_inside = true;
        } else if (has_path_data && _components[w][i].status == OUTSIDE) {
            path_inside = false;
        } else {
            int wdg = _pv[ow].winding(_pv[w][i].initialPoint());
            path_inside = wdg % 2 != 0;
        }

        if (path_inside == inside) {
            result.push_back(_pv[w][i]);
        }
    }
}

std::ostream &operator<<(std::ostream &os, PathIntersectionGraph const &pig)
{
    os << "Intersection graph:\n"
       << pig._xs.size()/2 << " total intersections\n"
       << pig.size() << " considered intersections\n";
    for (std::size_t i = 0; i < pig._components[0].size(); ++i) {
        typedef PathIntersectionGraph::IntersectionList::const_iterator Iter;
        for (Iter j = pig._components[0][i].xlist.begin(); j != pig._components[0][i].xlist.end(); ++j) {
            os << j->pos << " - " << j->neighbor->pos << " @ " << j->p << "\n";
        }
    }
    return os;
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
