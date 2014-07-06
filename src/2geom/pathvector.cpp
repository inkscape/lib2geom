/*
 * PathVector - std::vector containing Geom::Path
 * This file provides a set of operations that can be performed on PathVector,
 * e.g. an affine transform.
 *
 * Authors:
 *  Johan Engelen <goejendaagh@zonnet.nl>
 * 
 * Copyright 2008  authors
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

#ifndef SEEN_GEOM_PATHVECTOR_CPP
#define SEEN_GEOM_PATHVECTOR_CPP

#include <2geom/pathvector.h>

#include <2geom/path.h>
#include <2geom/affine.h>

namespace Geom {

//PathVector &PathVector::operator+=(PathVector const &other);

void PathVector::reverse(bool reverse_paths)
{
    if (reverse_paths) {
        std::reverse(begin(), end());
    }
    for (iterator i = begin(); i != end(); ++i) {
        *i = i->reversed();
    }
}

PathVector PathVector::reversed(bool reverse_paths)
{
    PathVector ret;
    for (iterator i = begin(); i != end(); ++i) {
        ret.push_back(i->reversed());
    }
    if (reverse_paths) {
        std::reverse(ret.begin(), ret.end());
    }
    return ret;
}

OptRect PathVector::boundsFast() const
{
    OptRect bound;
    if (empty()) return bound;

    bound = front().boundsFast();
    for (const_iterator it = ++begin(); it != end(); ++it) {
        bound.unionWith(it->boundsFast());
    }
    return bound;
}

OptRect PathVector::boundsExact() const
{
    OptRect bound;
    if (empty()) return bound;

    bound = front().boundsExact();
    for (const_iterator it = ++begin(); it != end(); ++it) {
        bound.unionWith(it->boundsExact());
    }
    return bound;
}

Coord PathVector::valueAt(Coord t, Dim2 d) const
{
    size_type path_index = 0, curve_index = 0;
    Coord f = _getIndices(t, path_index, curve_index);
    return at(path_index)[curve_index].valueAt(f, d);
}
Point PathVector::pointAt(Coord t) const
{
    size_type path_index = 0, curve_index = 0;
    Coord f = _getIndices(t, path_index, curve_index);
    return at(path_index)[curve_index].pointAt(f);
}

Coord PathVector::_getIndices(Coord t, size_type &path_index, size_type &curve_index) const
{
    Coord path = 0;
    Coord f = modf(t, &path);
    for (; path_index < size(); ++path_index) {
        unsigned s = _data.at(path_index).size();
        if (s > curve_index) break;
        curve_index -= s;
    }
    return f;
}

boost::optional<PathVectorPosition> PathVector::nearestPosition(Point const& _point, double *distance_squared) const
{
    boost::optional<PathVectorPosition> retval;

    double mindsq = infinity();
    unsigned int i = 0;
    for (const_iterator pit = begin(); pit != end(); ++pit) {
        double dsq;
        double t = pit->nearestTime(_point, &dsq);
        //std::cout << t << "," << dsq << std::endl;
        if (dsq < mindsq) {
            mindsq = dsq;
            retval = PathVectorPosition(i, t);
        }

        ++i;
    }

    if (distance_squared) {
        *distance_squared = mindsq;
    }
    return retval;
}

std::vector<PathVectorPosition> PathVector::allNearestPositions(Point const& _point, double *distance_squared) const
{
    std::vector<PathVectorPosition> retval;

    double mindsq = infinity();
    unsigned int i = 0;
    for (const_iterator pit = begin(); pit != end(); ++pit) {
        double dsq;
        double t = pit->nearestTime(_point, &dsq);
        if (dsq < mindsq) {
            mindsq = dsq;
            retval.clear();
        }
        if (dsq <= mindsq) {
            retval.push_back(PathVectorPosition(i, t));
        }

        ++i;
    }

    if (distance_squared) {
        *distance_squared = mindsq;
    }
    return retval;
}

} // namespace Geom

#endif // SEEN_GEOM_PATHVECTOR_CPP

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
