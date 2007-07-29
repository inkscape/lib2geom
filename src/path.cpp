/*
 * Path - Series of continuous curves
 *   
 * Copyright 2007  MenTaLguY <mental@rydia.net>
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

#include "path.h"

#include <iostream>

namespace Geom {

namespace {

enum Cmp {
  LESS_THAN=-1,
  GREATER_THAN=1,
  EQUAL_TO=0
};

inline Cmp operator-(Cmp x) {
  switch(x) {
  case LESS_THAN:
    return GREATER_THAN;
  case GREATER_THAN:
    return LESS_THAN;
  case EQUAL_TO:
    return EQUAL_TO;
  }
}

template <typename T1, typename T2>
inline Cmp cmp(T1 const &a, T2 const &b) {
  if ( a < b ) {
    return LESS_THAN;
  } else if ( b < a ) {
    return GREATER_THAN;
  } else {
    return EQUAL_TO;
  }
}

}

int CurveHelpers::sbasis_winding(D2<SBasis> const &sb, Point p) {
    SBasis fy = sb[Y];
    fy -= p[Y];

    std::vector<double> ts = roots(fy);
    if(ts.empty()) return 0;

    // winding determined by crossings at roots
    int wind=0;
    // previous time
    double pt = 0;
    for ( std::vector<double>::iterator ti = ts.begin()
        ; ti != ts.end()
        ; ++ti )
    {
        double t = *ti;
        if ( t <= 0. || t >= 1. ) continue; //skip endpoint roots 
        if ( sb[X](t) > p[X] ) { // root is ray intersection
            // Get t of next:
            std::vector<double>::iterator next = ti;
            next++;
            double nt;
            if(next == ts.end()) nt = 1; else nt = *next;
            
            // Check before in time and after in time for positions
            // Currently we're using the average times between next and previous segs
            Cmp after_to_ray = cmp(sb[Y]((t + nt) / 2), p[Y]);
            Cmp before_to_ray = cmp(sb[Y]((t + pt) / 2 ), p[Y]);
            // if y is included, these will have opposite values, giving order.
            Cmp c = cmp(after_to_ray, before_to_ray);
            if(c != EQUAL_TO) //Should always be true, but yah never know..
                wind += c;
            pt = t;
        }
    }
    
    return wind;
}

void Path::swap(Path &other) {
  std::swap(curves_, other.curves_);
  std::swap(closed_, other.closed_);
  std::swap(*final_, *other.final_);
  curves_[curves_.size()-1] = final_;
  other.curves_[other.curves_.size()-1] = other.final_;
}

Rect Path::boundsFast() const {
  Rect bounds=front().boundsFast();
  for ( const_iterator iter=++begin(); iter != end() ; ++iter ) {
    bounds.unionWith(iter->boundsFast());
  }
  return bounds;
}

Rect Path::boundsExact() const {
  Rect bounds=front().boundsExact();
  for ( const_iterator iter=++begin(); iter != end() ; ++iter ) {
    bounds.unionWith(iter->boundsExact());
  }
  return bounds;
}

int Path::winding(Point p) const {
  //start on a segment which is not a horizontal line with y = p[y]
  const_iterator start;
  for(const_iterator iter = begin(); ; ++iter) {
    if(iter == end_closed()) { return 0; }
    if(iter->initialPoint()[Y]!=p[Y])  { start = iter; break; }
    if(iter->finalPoint()[Y]!=p[Y])    { start = iter; break; }
    if(iter->boundsFast().height()!=0.){ start = iter; break; }
  }
  int wind = 0;
  const_iterator iter = start;
  bool temp = true;
  for (; iter != start || temp
       ; ++iter, iter = (iter == end_closed()) ? begin() : iter )
  {
    temp = false;
    Rect bounds = iter->boundsFast();
    Coord x = p[X], y = p[Y];
    if(x > bounds.right() || !bounds[Y].contains(y)) continue;
    Point final = iter->finalPoint();
    Point initial = iter->initialPoint();
    Cmp final_to_ray = cmp(final[Y], y);
    Cmp initial_to_ray = cmp(initial[Y], y);
    // if y is included, these will have opposite values, giving order.
    Cmp c = cmp(final_to_ray, initial_to_ray); 
    if(x < bounds.left()) {    //ray goes through bbox
        // winding determined by position of endpoints
        if(final_to_ray != EQUAL_TO) {
            wind += int(c); // GT = counter-clockwise = 1; LT = clockwise = -1; EQ = not-included = 0
            std::cout << int(c) << "\n";
            goto cont;
        }
    } else {
        //inside bbox, use custom per-curve winding thingie
        int delt = iter->winding(p);
        wind += delt;
        std::cout << "n " << delt << "\n";
    }
    //Handling the special case of an endpoint on the ray:
    if(final[Y] == y) {
        //Traverse segments until it breaks away from y
        //99.9% of the time this will happen the first go
        const_iterator next = iter;
        next++;
        for(; next != iter; next++) {
            if(next == end_closed()) next = begin();
            Rect bnds = next->boundsFast();
            //TODO: X considerations
            if(bnds.height() > 0) {
                //It has diverged
                if(bnds.contains(p)) {
                    const double fudge = 0.01;
                    if(cmp(y, next->valueAt(fudge)[Y]) == initial_to_ray) {
                        wind += int(c);
                        std::cout << "!!!!!" << " " << int(c) << "\n";
                    }
                    iter = next; // No increment, as the rest of the thing hasn't been counted.
                } else {
                    Coord ny = next->initialPoint()[Y];
                    if(cmp(y, ny) == initial_to_ray) {
                        //Is a continuation through the ray, so counts windingwise
                        wind += int(c);
                        std::cout << "!!!!!" << " " << int(c) << "\n";
                    }
                    iter = ++next;
                }
                
                goto cont;
            }
            if(next==start) return wind;
        }
        //Looks like it looped, which means everything's flat
        return wind;
    }
    
    cont:(void)0;
  }
  return wind;
  
    /* OLD CODE never checked in
    
        Point initial = iter->initialPoint();
        Point final = iter->finalPoint();
    Cmp c = cmp(final[Y], initial[Y]);
    if(initial[Y] < p[Y] && p[Y] <= final[Y] ||
       final[Y] < p[Y] && p[Y] <= initial[Y]) {
        
            //At this point we know it intersects our vertical ray an odd number of times, yielding a wind of 1 or -1
            //c now represents the overall direction of the curve, LT is ccw, GT is cw.
            if(c != EQUAL_TO) {
                if(final[Y] != p[Y]) {
                    if(c == LESS_THAN) wind++; else wind--;
                    goto skip;
                } else {
                    //Looks like we'll have to take into account the next segment
                    const_iterator next = iter+1;
                    if(next == end_closed()) next = begin();
                    Rect bnds = iter->boundsFast();
                    if(bnds.bottom() > p[Y]) {
                        Cmp c2 = cmp(next->finalPoint(), p[X])
                        //If the next segment continues the trend, count it
                        if(c2 == c)
                            if(c == LESS_THAN) wind++ else wind--;
                        goto skip;
                    }
                    //Looks like the next seg isn't such an elementary case.
                }
            }
        }
    } else {
        //No way it can affect winding at this point (unless it breaks continuity)
        if(bounds.bottom() > p[Y]) goto skip;
    }*/
}

void Path::append(Curve const &curve) {
  if ( curves_.front() != final_ && curve.initialPoint() != (*final_)[0] ) {
    throw ContinuityError();
  }
  do_append(curve.duplicate());
}

void Path::append(D2<SBasis> const &curve) {
  if ( curves_.front() != final_ ) {
    for ( int i = 0 ; i < 2 ; ++i ) {
      if ( curve[i][0][0] != (*final_)[0][i] ) {
        throw ContinuityError();
      }
    }
  }
  do_append(new SBasisCurve(curve));
}

void Path::do_update(Sequence::iterator first_replaced,
                     Sequence::iterator last_replaced,
                     Sequence::iterator first,
                    Sequence::iterator last)
{
  // note: modifies the contents of [first,last)

  check_continuity(first_replaced, last_replaced, first, last);
  delete_range(first_replaced, last_replaced);
  if ( ( last - first ) == ( last_replaced - first_replaced ) ) {
    std::copy(first, last, first_replaced);
  } else {
    // this approach depends on std::vector's behavior WRT iterator stability
    curves_.erase(first_replaced, last_replaced);
    curves_.insert(first_replaced, first, last);
  }

  if ( curves_.front() != final_ ) {
    final_->setPoint(0, back().finalPoint());
    final_->setPoint(1, front().initialPoint());
  }
}

void Path::do_append(Curve *curve) {
  if ( curves_.front() == final_ ) {
    final_->setPoint(1, curve->initialPoint());
  }
  curves_.insert(curves_.end()-1, curve);
  final_->setPoint(0, curve->finalPoint());
}

void Path::delete_range(Sequence::iterator first, Sequence::iterator last) {
  for ( Sequence::iterator iter=first ; iter != last ; ++iter ) {
    delete *iter;
  }
}

void Path::check_continuity(Sequence::iterator first_replaced,
                            Sequence::iterator last_replaced,
                            Sequence::iterator first,
                            Sequence::iterator last)
{
  if ( first != last ) {
    if ( first_replaced != curves_.begin() ) {
      if ( (*first_replaced)->initialPoint() != (*first)->initialPoint() ) {
        throw ContinuityError();
      }
    }
    if ( last_replaced != (curves_.end()-1) ) {
      if ( (*(last_replaced-1))->finalPoint() != (*(last-1))->finalPoint() ) {
        throw ContinuityError();
      }
    }
  } else if ( first_replaced != last_replaced && first_replaced != curves_.begin() && last_replaced != curves_.end()-1) {
    if ( (*first_replaced)->initialPoint() !=
         (*(last_replaced-1))->finalPoint() )
    {
      throw ContinuityError();
    }
  }
}

Rect SVGEllipticalArc::boundsFast() const {
    throw NotImplemented();
}
Rect SVGEllipticalArc::boundsExact() const {
    throw NotImplemented();
}

std::vector<Point> SVGEllipticalArc::valueAndDerivatives(Coord t, unsigned n) const {
    throw NotImplemented();
}

D2<SBasis> SVGEllipticalArc::toSBasis() const {
    throw NotImplemented();
}

}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=2:tabstop=8:softtabstop=2 :
*/
