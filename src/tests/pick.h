/*
 * Routines for generating anything randomly
 *
 * Authors:
 *      Marco Cecchetti <mrcekets at gmail.com>
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

#ifndef _GEOM_SL_PICK_H_
#define _GEOM_SL_PICK_H_


#include <2geom/symbolic/multipoly.h>
#include <2geom/symbolic/matrix.h>

inline
size_t pick_uint(size_t max)
{
    return (std::rand() % (max+1));
}

inline
int pick_int(size_t max)
{
    int s = pick_uint(2);
    if (s == 0) s = -1;
    return s * (std::rand() % (max+1));
}

inline
Geom::SL::multi_index_type pick_multi_index(size_t N, size_t max)
{
    Geom::SL::multi_index_type I(N);
    for (size_t i = 0; i < I.size(); ++i)
        I[i] = pick_uint(max);
    return I;
}

template <size_t N>
inline
typename Geom::SL::mvpoly<N, double>::type
pick_polyN(size_t d, size_t m)
{
    typename Geom::SL::mvpoly<N, double>::type  p;
    size_t d0 = pick_uint(d);
    for (size_t i = 0; i <= d0; ++i)
    {
        p.coefficient(i, pick_polyN<N-1>(d, m));
    }
    return p;
}

template <>
inline
double pick_polyN<0>(size_t /*d*/, size_t m)
{
    return pick_int(m);
}


template <size_t N>
inline
typename Geom::SL::mvpoly<N, double>::type
pick_poly_max(size_t d, size_t m)
{
    typename Geom::SL::mvpoly<N, double>::type  p;
    for (size_t i = 0; i <= d; ++i)
    {
        p.coefficient(i, pick_poly_max<N-1>(d-i, m));
    }
    return p;
}

template <>
inline
double pick_poly_max<0>(size_t /*d*/, size_t m)
{
    return pick_int(m);
}


template <size_t N>
inline
Geom::SL::MultiPoly<N, double>
pick_multipoly(size_t d, size_t m)
{
    return Geom::SL::MultiPoly<N, double>(pick_polyN<N>(d, m));
}

template <size_t N>
inline
Geom::SL::MultiPoly<N, double>
pick_multipoly_max(size_t d, size_t m)
{
    return Geom::SL::MultiPoly<N, double>(pick_poly_max<N>(d, m));
}



inline
Geom::SL::Matrix< Geom::SL::MultiPoly<2, double> >
pick_matrix(size_t n, size_t d, size_t m)
{
    Geom::SL::Matrix< Geom::SL::MultiPoly<2, double> >  M(n, n);
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 0; j < n; ++j)
        {
            M(i,j) = pick_multipoly_max<2>(d, m);
        }
    }
    return M;
}


inline
Geom::SL::Matrix< Geom::SL::MultiPoly<2, double> >
pick_symmetric_matrix(size_t n, size_t d, size_t m)
{
    Geom::SL::Matrix< Geom::SL::MultiPoly<2, double> > M(n, n);
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 0; j < i; ++j)
        {
            M(i,j) = M(j,i) = pick_multipoly_max<2>(d, m);
        }
    }
    for (size_t i = 0; i < n; ++i)
    {
        M(i,i) = pick_multipoly_max<2>(d, m);
    }
    return M;
}


#endif // _GEOM_SL_PICK_H_


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
