/*
 * interval.h - Simple closed interval class
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
#ifndef SEEN_INTERVAL_H
#define SEEN_INTERVAL_H

#include "coord.h"
#include "maybe.h"

namespace Geom {

class Interval {
private:
    Coord _min, _max;
public:
    //TODO: remove; required by maybe
    Interval(): _min(0.0), _max(0.0) {}
    Interval(Coord u): _min(u), _max(u) {}
    Interval(Coord u, Coord v) {
        if(u < v) {
            _min = u; _max = v;
        } else {
            _min = v; _max = u;
        }
    }

    inline Coord min() const { return _min; }
    inline Coord max() const { return _max; }
    inline Coord size() const { return _max - _min; }
    inline Coord average() const { return (_min + _max) / 2; }

    bool contains(Coord val) const { return _min <= val && val <= _max; }
    bool contains(const Interval & val) const { return _min <= val._min && _max <= val._max; }

//MUTATOR PRISON
    //TODO: NaN handleage for the next two?
    //TODO: Evaluate if wrap behaviour is proper.
    //If val > max, then rather than becoming a min==max range, it 'wraps' over
    inline void setMin(Coord val) {
        if(val > _max) {
            _min = _max;
            _max = val;
        } else {
            _min = val;
        }
    }

    //If val < min, then rather than becoming a min==max range, it 'wraps' over
    inline void setMax(Coord val) {
        if(val < _min) {
            _max = _min;
            _min = val;
        } else {
            _max = val;
        }
    }

    inline void extendTo(Coord val) {
       if(val < _min) _min = val;
       if(val > _max) _max = val;  //no else, as we want to handle NaN
    }

    inline void operator+(Coord amnt) {
        _min += amnt;
        _max += amnt;
    }

    static Interval fromArray(const Coord* c, int n) {
        assert(n > 0);
        Interval result(c[0]);
        for(int i = 0; i < n; i++) result.extendTo(c[i]);
        return result;
    }

    //TODO: evil, but temporary 
    //friend class MaybeStorage<Rect>;
};

// 'union' conflicts with C keyword
inline Interval interval_union(const Interval & a, const Interval & b) {
    return Interval(std::min(a.min(), b.min()),
                    std::max(a.max(), b.max()));
}

// named for consistancy with above
inline Maybe<Interval> interval_intersect(const Interval & a, const Interval & b) {
    Coord u = std::max(a.min(), b.min()),
          v = std::min(a.max(), b.max());
    if(u > v) return Nothing();            //TODO: presumes that 0 size disallowed
    return Interval(u, v);
}

}
#endif //SEEN_INTERVAL_H
