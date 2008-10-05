/**
 * \file
 * \brief  \todo brief description
 *
 * Authors:
 *      ? <?@?.?>
 * 
 * Copyright ?-?  authors
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

#ifndef LIB2GEOM_STURM_HEADER
#define LIB2GEOM_STURM_HEADER

#include <2geom/poly.h>
#include <2geom/utils.h>

namespace Geom {

class sturm : public std::vector<Poly>{
public:
    sturm(Poly const &X) {
        push_back(X);
        push_back(derivative(X));
        Poly Xi = back();
        Poly Xim1 = X;
        std::cout << "sturm:\n" << Xim1 << std::endl;
        std::cout << Xi << std::endl;
        while(Xi.size() > 1) {
            Poly r;
            divide(Xim1, Xi, r);
            std::cout << r << std::endl;
            assert(r.size() < Xi.size());
            Xim1 = Xi;
            Xi = -r;
            assert(Xim1.size() > Xi.size());
            push_back(Xi);
        }
    }
    
    unsigned count_signs(double t) {
        unsigned n_signs = 0;/*  Number of sign-changes */
        const double big = 1e20; // a number such that practical polys would overflow on evaluation
        if(t >= big) {
            int old_sign = sgn((*this)[0].back());
            for (unsigned i = 1; i < size(); i++) {
                int sign = sgn((*this)[i].back());
                if (sign != old_sign)
                    n_signs++;
                old_sign = sign;
            }
        } else {
            int old_sign = sgn((*this)[0].eval(t));
            for (unsigned i = 1; i < size(); i++) {
                int sign = sgn((*this)[i].eval(t));
                if (sign != old_sign)
                    n_signs++;
                old_sign = sign;
            }
        }
        return n_signs;
    }
    
    unsigned n_roots_between(double l, double r) {
        return count_signs(l) - count_signs(r);
    }
};

} //namespace Geom

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
