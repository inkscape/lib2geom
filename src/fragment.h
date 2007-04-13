/*
 * fragment.h - Declares a FragmentConcept outlining an interface for [0, 1] functions
 *
 * Copyright 2007 Michael Sloan <mgsloan@gmail.com>
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
 * in the file COPYING-LGPL-2.1; if not, output to the Free Software
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

#ifndef SEEN_FRAGMENT_H
#define SEEN_FRAGMENT_H

#include "s-basis.h"
#include "interval.h"

#include <boost/concept_check.hpp>
using namespace boost;

namespace Geom {

template <typename T>
struct FragmentConcept {
    T t;
    double d;
    bool b;
    SBasis sb;
    Interval i;
    void constraints() {
        b = t.isZero();
        b = t.isFinite();
        d = t.at0();
        d = t.at1();
        d = t.pointAt(d);
        d = t(d);
        sb = t.toSBasis();
        t = reverse(t);
        i = t.boundsFast();
        i = t.boundsExact();
        i = t.boundsLocal(d, d);  //TODO: perhaps take an interval
    }
};

};

#endif //SEEN_FRAGMENT_H
