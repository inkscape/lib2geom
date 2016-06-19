/** @file
 * @brief Furthest time routines for D2<SBasis> and Piecewise<D2<SBasis>>
 *//*
 * Authors:
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Jabier Arraiza <jabier.arraiza@marker.es>
 *
 * Copyright 2007-2008  authors
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


#include <2geom/furthest-time.h>
#include <algorithm>

namespace Geom
{

Coord furthest_time(Point const &p, D2<Bezier> const &input, Coord from, Coord to)
{
    Interval domain(from, to);
    bool partial = false;

    if (domain.min() < 0 || domain.max() > 1) {
        THROW_RANGEERROR("[from,to] interval out of bounds");
    }

    if (input.isConstant(0)) return from;

    D2<Bezier> bez;
    if (domain.min() != 0 || domain.max() != 1) {
        bez = portion(input, domain) - p;
        partial = true;
    } else {
        bez = input - p;
    }

    // find extrema of the function x(t)^2 + y(t)^2
    // use the fact that (f^2)' = 2 f f'
    // this reduces the order of the distance function by 1
    D2<Bezier> deriv = derivative(bez);
    std::vector<Coord> ts = (multiply(bez[X], deriv[X]) + multiply(bez[Y], deriv[Y])).roots();

    Coord t = -1, maxd = 0;
    for (unsigned i = 0; i < ts.size(); ++i) {
        Coord droot = L2sq(bez.valueAt(ts[i]));
        if (droot > maxd) {
            maxd = droot;
            t = ts[i];
        }
    }

    // also check endpoints
    Coord dinitial = L2sq(bez.at0());
    Coord dfinal = L2sq(bez.at1());

    if (dinitial > maxd) {
        maxd = dinitial;
        t = 0;
    }
    if (dfinal > maxd) {
        //maxd = dfinal;
        t = 1;
    }

    if (partial) {
        t = domain.valueAt(t);
    }
    return t;
}

////////////////////////////////////////////////////////////////////////////////
// D2<SBasis> versions

/*
 * Return the parameter t of the furthest time value on the portion of the curve "c",
 * related to the interval [from, to], to the point "p".
 * The needed curve derivative "dc" is passed as parameter.
 * The function return the first furthest time value to "p" that is found.
 */

double furthest_time(Point const& p,
                    D2<SBasis> const& c,
                    D2<SBasis> const& dc,
                    double from, double to )
{
    if ( from > to ) std::swap(from, to);
    if ( from < 0 || to > 1 )
    {
        THROW_RANGEERROR("[from,to] interval out of bounds");
    }
    if (c.isConstant()) return from;
    SBasis dd = dot(c - p, dc);
    //std::cout << dd << std::endl;
    std::vector<double> zeros = Geom::roots(dd);

    double closest = from;
    double max_dist_sq = L2sq(c(from) - p);
    for ( size_t i = 0; i < zeros.size(); ++i )
    {
        double distsq = L2sq(c(zeros[i]) - p);
        if ( max_dist_sq < L2sq(c(zeros[i]) - p) )
        {
            closest = zeros[i];
            max_dist_sq = distsq;
        }
    }
    if ( max_dist_sq < L2sq( c(to) - p ) )
        closest = to;
    return closest;

}

/*
 * Return the parameters t of all the furthest points on the portion of
 * the curve "c", related to the interval [from, to], to the point "p".
 * The needed curve derivative "dc" is passed as parameter.
 */

std::vector<double>
all_furthest_times(Point const &p,
                  D2<SBasis> const &c,
                  D2<SBasis> const &dc,
                  double from, double to)
{
    if (from > to) {
        std::swap(from, to);
    }
    if (from < 0 || to > 1) {
        THROW_RANGEERROR("[from,to] interval out of bounds");
    }

    std::vector<double> result;
    if (c.isConstant()) {
        result.push_back(from);
        return result;
    }
    SBasis dd = dot(c - p, dc);

    std::vector<double> zeros = Geom::roots(dd);
    std::vector<double> candidates;
    candidates.push_back(from);
    candidates.insert(candidates.end(), zeros.begin(), zeros.end());
    candidates.push_back(to);
    std::vector<double> distsq;
    distsq.reserve(candidates.size());
    for (unsigned i = 0; i < candidates.size(); ++i) {
        distsq.push_back(L2sq(c(candidates[i]) - p));
    }
    unsigned closest = 0;
    double dsq = distsq[0];
    for (unsigned i = 1; i < candidates.size(); ++i) {
        if (dsq < distsq[i]) {
            closest = i;
            dsq = distsq[i];
        }
    }
    for (unsigned i = 0; i < candidates.size(); ++i) {
        if (distsq[closest] == distsq[i]) {
            result.push_back(candidates[i]);
        }
    }
    return result;
}


////////////////////////////////////////////////////////////////////////////////
// Piecewise< D2<SBasis> > versions


double furthest_time(Point const &p,
                    Piecewise< D2<SBasis> > const &c,
                    double from, double to)
{
    if (from > to) std::swap(from, to);
    if (from < c.cuts[0] || to > c.cuts[c.size()]) {
        THROW_RANGEERROR("[from,to] interval out of bounds");
    }

    unsigned si = c.segN(from);
    unsigned ei = c.segN(to);
    if (si == ei) {
        double furthest =
            furthest_time(p, c[si], c.segT(from, si), c.segT(to, si));
        return c.mapToDomain(furthest, si);
    }

    double t;
    double furthest = furthest_time(p, c[si], c.segT(from, si));
    unsigned int ni = si;
    double dsq;
    double maxdistsq = distanceSq(p, c[si](furthest));
    Rect bb;
    for (unsigned i = si + 1; i < ei; ++i) {
        bb = *bounds_fast(c[i]);
        dsq = distanceSq(p, bb);
        if ( maxdistsq > dsq ) continue;

        t = furthest_time(p, c[i]);
        dsq = distanceSq(p, c[i](t));
        if (maxdistsq < dsq) {
            furthest = t;
            ni = i;
            maxdistsq = dsq;
        }
    }
    bb = *bounds_fast(c[ei]);
    dsq = distanceSq(p, bb);
    if (maxdistsq < dsq) {
        t = furthest_time(p, c[ei], 0, c.segT(to, ei));
        dsq = distanceSq(p, c[ei](t));
        if (maxdistsq < dsq) {
            furthest = t;
            ni = ei;
        }
    }
    return c.mapToDomain(furthest, ni);
}

std::vector<double>
all_furthest_times(Point const &p,
                  Piecewise< D2<SBasis> > const &c,
                  double from, double to)
{
    if (from > to) {
        std::swap(from, to);
    }
    if (from < c.cuts[0] || to > c.cuts[c.size()]) {
        THROW_RANGEERROR("[from,to] interval out of bounds");
    }

    unsigned si = c.segN(from);
    unsigned ei = c.segN(to);
    if ( si == ei )
    {
        std::vector<double>	all_furthest =
            all_furthest_times(p, c[si], c.segT(from, si), c.segT(to, si));
        for ( unsigned int i = 0; i < all_furthest.size(); ++i )
        {
            all_furthest[i] = c.mapToDomain(all_furthest[i], si);
        }
        return all_furthest;
    }
    std::vector<double> all_t;
    std::vector< std::vector<double> > all_np;
    all_np.push_back( all_furthest_times(p, c[si], c.segT(from, si)) );
    std::vector<unsigned> ni;
    ni.push_back(si);
    double dsq;
    double maxdistsq = distanceSq( p, c[si](all_np.front().front()) );
    Rect bb;

    for (unsigned i = si + 1; i < ei; ++i) {
        bb = *bounds_fast(c[i]);
        dsq = distanceSq(p, bb);
        if ( maxdistsq > dsq ) continue;
        all_t = all_furthest_times(p, c[i]);
        dsq = distanceSq( p, c[i](all_t.front()) );
        if ( maxdistsq < dsq )
        {
            all_np.clear();
            all_np.push_back(all_t);
            ni.clear();
            ni.push_back(i);
            maxdistsq = dsq;
        }
        else if ( maxdistsq == dsq )
        {
            all_np.push_back(all_t);
            ni.push_back(i);
        }
    }
    bb = *bounds_fast(c[ei]);
    dsq = distanceSq(p, bb);
    if (maxdistsq < dsq) {
        all_t = all_furthest_times(p, c[ei], 0, c.segT(to, ei));
        dsq = distanceSq( p, c[ei](all_t.front()) );
        if (maxdistsq < dsq) {
            for (unsigned int i = 0; i < all_t.size(); ++i) {
                all_t[i] = c.mapToDomain(all_t[i], ei);
            }
            return all_t;
        } else if (maxdistsq == dsq) {
            all_np.push_back(all_t);
            ni.push_back(ei);
        }
    }
    std::vector<double> all_furthest;
    for (unsigned i = 0; i < all_np.size(); ++i) {
        for (unsigned int j = 0; j < all_np[i].size(); ++j) {
            all_furthest.push_back( c.mapToDomain(all_np[i][j], ni[i]) );
        }
    }
    all_furthest.erase(std::unique(all_furthest.begin(), all_furthest.end()),
                      all_furthest.end());
    return all_furthest;
}

} // end namespace Geom


