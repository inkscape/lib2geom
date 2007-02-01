/*
 * bezier-to-sbasis.h
 *
 * Copyright 2006 Nathan Hurst <njh@mail.csse.monash.edu.au>
 * Copyright 2006 Michael G. Sloan <mgsloan@gmail.com>
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
 *
 */

#ifndef _BEZIER_TO_SBASIS
#define _BEZIER_TO_SBASIS

#include "multidim-sbasis.h"

namespace Geom{

template <typename T, unsigned D, unsigned order>
struct bezier_to_sbasis_impl {
    static inline MultidimSBasis<D> compute(T const &handles) {
        return multiply(BezOrd(1, 0), bezier_to_sbasis_impl<T, D, order-1>::compute(handles)) +
            multiply(BezOrd(0, 1), bezier_to_sbasis_impl<T, D, order-1>::compute(handles+1));
    }
};

template <typename T, unsigned D>
struct bezier_to_sbasis_impl<T, D, 1> {
    static inline MultidimSBasis<D> compute(T const &handles) {
        MultidimSBasis<D> mdsb;
        for(unsigned d = 0 ; d < D; d++) {
            mdsb[d] = BezOrd(handles[0][d], handles[1][d]);
        }
        return mdsb;
    }
};

template <typename T, unsigned D>
struct bezier_to_sbasis_impl<T, D, 0> {
    static inline MultidimSBasis<D> compute(T const &handles) {
        MultidimSBasis<D> mdsb;
        for(unsigned d = 0 ; d < D; d++) {
            mdsb[d] = BezOrd(handles[0][d], handles[0][d]);
        }
        return mdsb;
    }
};

template <unsigned D, unsigned order, typename T>
inline MultidimSBasis<D>
bezier_to_sbasis(T const &handles) {
    return bezier_to_sbasis_impl<T, D, order>::compute(handles);
}

};
#endif
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
