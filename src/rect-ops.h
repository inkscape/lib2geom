#ifndef SEEN_Geom_RECT_OPS_H
#define SEEN_Geom_RECT_OPS_H

/*
 * Rect operators
 *
 * Copyright 2004  MenTaLguY <mental@rydia.net>,
 *   bulia byak <buliabyak@users.sf.net>
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

namespace Geom {

inline Rect expand(Rect const &r, double by) {
    Geom::Point const p(by, by);
    return Rect(r.min() + p, r.max() - p);
}

inline Rect expand(Rect const &r, Geom::Point by) {
    return Rect(r.min() + by, r.max() - by);
}

#if 0
inline ConvexHull operator*(Rect const &r, Matrix const &m) {
    /* FIXME: no mention of m.  Should probably be made non-inline. */
    ConvexHull points(r.corner(0));
    for ( unsigned i = 1 ; i < 4 ; i++ ) {
        points.add(r.corner(i));
    }
    return points;
}
#endif

} /* namespace Geom */


#endif /* !SEEN_Geom_RECT_OPS_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
