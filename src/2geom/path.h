/*
 * Path - Series of continuous curves
 *
 * Authors:
 * 		MenTaLguY <mental@rydia.net>
 * 		Marco Cecchetti <mrcekets at gmail.com>
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




#ifndef SEEN_GEOM_PATH_H
#define SEEN_GEOM_PATH_H


#include <boost/shared_ptr.hpp>
#include <2geom/curves.h>

#include <iterator>
#include <algorithm>


namespace Geom
{

class Path;

namespace PathInternal {

typedef std::vector<boost::shared_ptr<Curve const> > Sequence;

// Conditional expression for types. If true, first, if false, second.
template<bool _Cond, typename _Iftrue, typename _Iffalse>
  struct __conditional_type
  { typedef _Iftrue __type; };

template<typename _Iftrue, typename _Iffalse>
  struct __conditional_type<false, _Iftrue, _Iffalse>
  { typedef _Iffalse __type; };


template <typename IteratorImpl>
class BaseIterator
: public std::iterator<std::forward_iterator_tag, Curve const>
{
public:
  BaseIterator() {}

  // default construct
  // default copy

  // Allow Sequence::iterator to Sequence::const_iterator conversion
  // unfortunately I do not know how to imitate the way __normal_iterator 
  // does it, because I don't see a way to get the typename of the container 
  // IteratorImpl is pointing at...
  BaseIterator (  typename __conditional_type<
                    (std::__are_same<IteratorImpl, Sequence::const_iterator >::__value),  // check if this instantiation is of const_iterator type
                    const BaseIterator< Sequence::iterator >,     // if true:  accept iterator in const_iterator instantiation
                    const BaseIterator<IteratorImpl> > ::__type   // if false: default to standard copy constructor
                  & __other)
    : impl_(__other.impl_) { }
  friend class BaseIterator< Sequence::const_iterator >;

  bool operator==(BaseIterator const &other) {
    return other.impl_ == impl_;
  }
  bool operator!=(BaseIterator const &other) {
    return other.impl_ != impl_;
  }

  Curve const &operator*() const { return **impl_; }
  Curve const *operator->() const { return boost::get_pointer(*impl_); }

  BaseIterator &operator++() {
    ++impl_;
    return *this;
  }

  BaseIterator operator++(int) {
    BaseIterator old=*this;
    ++(*this);
    return old;
  }

  BaseIterator &operator--() {
    --impl_;
    return *this;
  }

  BaseIterator operator--(int) {
    BaseIterator old=*this;
    --(*this);
    return old;
  }

private:
  BaseIterator(IteratorImpl const &pos) : impl_(pos) {}

  IteratorImpl impl_;
  friend class ::Geom::Path;
};

}

/*
 * Open and closed paths: all paths, whether open or closed, store a final
 * segment which connects the initial and final endpoints of the "real"
 * path data.  While similar to the "z" in an SVG path, it exists for
 * both open and closed paths, and is not considered part of the "normal"
 * path data, which is always covered by the range [begin(), end_open()).
 * Conversely, the range [begin(), end_closed()) always contains the "extra"
 * closing segment.
 *
 * The only difference between a closed and an open path is whether end()
 * returns end_closed() or end_open().  The idea behind this is to let
 * any path be stroked using [begin(), end_default()), and filled using
 * [begin(), end_closed()), without requiring a separate "filled" version
 * of the path to use for filling.
 */
class Path {
public:
  typedef PathInternal::Sequence Sequence;
  typedef PathInternal::BaseIterator<Sequence::iterator> iterator;
  typedef PathInternal::BaseIterator<Sequence::const_iterator> const_iterator;
  typedef Sequence::size_type size_type;
  typedef Sequence::difference_type difference_type;

  class ClosingSegment : public LineSegment {
  public:
    ClosingSegment() : LineSegment() {}
    ClosingSegment(Point const &p1, Point const &p2) : LineSegment(p1, p2) {}
    virtual Curve *duplicate() const { return new ClosingSegment(*this); }
    virtual Curve *reverse() const { return new ClosingSegment((*this)[1], (*this)[0]); }
  };

  enum Stitching {
    NO_STITCHING=0,
    STITCH_DISCONTINUOUS
  };

  class StitchSegment : public LineSegment {
  public:
    StitchSegment() : LineSegment() {}
    StitchSegment(Point const &p1, Point const &p2) : LineSegment(p1, p2) {}
    virtual Curve *duplicate() const { return new StitchSegment(*this); }
    virtual Curve *reverse() const { return new StitchSegment((*this)[1], (*this)[0]); }
  };

  // Path(Path const &other) - use default copy constructor

  explicit Path(Point p=Point())
  : curves_(boost::shared_ptr<Sequence>(new Sequence(1, boost::shared_ptr<Curve>()))),
    final_(new ClosingSegment(p, p)),
    closed_(false)
  {
    get_curves_().back() = boost::shared_ptr<Curve>(final_);
  }

  template <typename Impl>
  Path(PathInternal::BaseIterator<Impl> first,
       PathInternal::BaseIterator<Impl> last,
       bool closed=false)
  : curves_(boost::shared_ptr<Sequence>(new Sequence(first.impl_, last.impl_))),
    closed_(closed)
  {
    ClosingSegment *final;
    if (!get_curves_().empty()) {
      final_ = new ClosingSegment(get_curves_().back()->finalPoint(),
                                  get_curves_().front()->initialPoint());
    } else {
      final_ = new ClosingSegment();
    }
    get_curves_().push_back(boost::shared_ptr<Curve>(final_));
  }

  virtual ~Path() {}

  
  // Path &operator=(Path const &other) - use default assignment operator

  void swap(Path &other) {
    std::swap(other.curves_, curves_);
    std::swap(other.final_, final_);
    std::swap(other.closed_, closed_);
  }

  Curve const &operator[](unsigned i) const { return *get_curves_()[i]; }

  Curve const &front() const { return *get_curves_()[0]; }
  Curve const &back() const { return *get_curves_()[get_curves_().size()-2]; }
  Curve const &back_open() const { return *get_curves_()[get_curves_().size()-2]; }
  Curve const &back_closed() const { return *get_curves_()[get_curves_().size()-1]; }
  Curve const &back_default() const {
    return ( closed_ ? back_closed() : back_open() );
  }

  const_iterator begin() const { return get_curves_().begin(); }
  const_iterator end() const { return get_curves_().end()-1; }
  iterator begin() { return get_curves_().begin(); }
  iterator end() { return get_curves_().end()-1; }

  const_iterator end_open() const { return get_curves_().end()-1; }
  const_iterator end_closed() const { return get_curves_().end(); }
  const_iterator end_default() const {
    return ( closed_ ? end_closed() : end_open() );
  }

  size_type size() const { return get_curves_().size()-1; }
  size_type max_size() const { return get_curves_().max_size()-1; }

  bool empty() const { return get_curves_().size() == 1; }
  bool closed() const { return closed_; }
  void close(bool closed=true) { closed_ = closed; }

  Rect boundsFast() const;
  Rect boundsExact() const;

  Piecewise<D2<SBasis> > toPwSb() const {
    Piecewise<D2<SBasis> > ret;
    ret.push_cut(0);
    unsigned i = 1;
    // pw<d2<>> is always open. so if path is closed, add closing segment as well to pwd2.
    for(const_iterator it = begin(); it != end_default(); ++it) {
      if (!it->isDegenerate()) {
        ret.push(it->toSBasis(), i++);
      }
    }
    return ret;
  }

  bool operator==(Path const &m) const {
      if (size() != m.size() || closed() != m.closed())
          return false;
      const_iterator it2 = m.get_curves_().begin();
    for(const_iterator it = get_curves_().begin(); it != get_curves_().end(); ++it) {
        const Curve& a = (*it);
        const Curve& b = (*it2);
        if(!(a == b))
            return false;
        ++it2;
    }
    return true;
  }

  Path operator*(Matrix const &m) const {
    Path ret(*this);
    ret *= m;
    return ret;
  }

  Path &operator*=(Matrix const &m) {
    Sequence::iterator it;
    Sequence::iterator last;
    Point prev;
    last = get_curves_().end() - 1;
    unshare_curves();
    for (it = get_curves_().begin() ; it != last ; ++it) {
      *it = boost::shared_ptr<Curve>((*it)->transformed(m));
      if ( it != get_curves_().begin() && (*it)->initialPoint() != prev ) {
        THROW_CONTINUITYERROR();
      }
      prev = (*it)->finalPoint();
    }
    unshare_final();
    for ( int i = 0 ; i < 2 ; ++i ) {
      final_->setPoint(i, (*final_)[i] * m);
    }
    if (get_curves_().size() > 1) {
      if ( front().initialPoint() != initialPoint() || back().finalPoint() != finalPoint() ) {
        THROW_CONTINUITYERROR();
      }
    }
    return *this;
  }
  
  Point pointAt(double t) const 
  {
	  unsigned int sz = size();
	  if ( closed() ) ++sz;
	  if ( t < 0 || t > sz  )
	  {
		  THROW_RANGEERROR("parameter t out of bounds");
	  }
	  if ( empty() ) return Point(0,0);
	  double k, lt = modf(t, &k);
	  unsigned int i = static_cast<unsigned int>(k);
	  if ( i == sz ) 
	  { 
		  --i;
		  lt = 1;
	  }
	  return (*this)[i].pointAt(lt);
  }

  double valueAt(double t, Dim2 d) const 
  {
	  unsigned int sz = size();
	  if ( closed() ) ++sz;
	  if ( t < 0 || t > sz  )
	  {
		  THROW_RANGEERROR("parameter t out of bounds");
	  }
	  if ( empty() ) return 0;
	  double k, lt = modf(t, &k);
	  unsigned int i = static_cast<unsigned int>(k);
	  if ( i == sz ) 
	  { 
		  --i;
		  lt = 1;
	  }
	  return (*this)[i].valueAt(lt, d);
  }

  
  Point operator() (double t) const
  {
	  return pointAt(t);
  }
  
  std::vector<double> roots(double v, Dim2 d) const {
    std::vector<double> res;
    for(unsigned i = 0; i <= size(); i++) {
      std::vector<double> temp = (*this)[i].roots(v, d);
      for(unsigned j = 0; j < temp.size(); j++)
        res.push_back(temp[j] + i);
    }
    return res;
  }
  
  std::vector<double> 
  allNearestPoints(Point const& _point, double from, double to) const;
  
  std::vector<double>
  allNearestPoints(Point const& _point) const
  {
	  unsigned int sz = size();
	  if ( closed() ) ++sz;
	  return allNearestPoints(_point, 0, sz);
  }
  
  
  double nearestPoint(Point const& _point, double from, double to) const;
  
  double nearestPoint(Point const& _point) const
  {
	  unsigned int sz = size();
	  if ( closed() ) ++sz;
	  return nearestPoint(_point, 0, sz);
  }
   
  void appendPortionTo(Path &p, double f, double t) const;

  Path portion(double f, double t) const {
    Path ret;
    ret.close(false);
    appendPortionTo(ret, f, t);
    return ret;
  }
  Path portion(Interval i) const { return portion(i.min(), i.max()); }

  Path reverse() const {
    Path ret(*this);
    ret.unshare_curves();
    for ( Sequence::iterator iter = ret.get_curves_().begin() ;
          iter != ret.get_curves_().end()-1 ; ++iter )
    {
      *iter = boost::shared_ptr<Curve>((*iter)->reverse());
    }
    std::reverse(ret.get_curves_().begin(), ret.get_curves_().end()-1);
    ret.unshare_final();
    ret.final_ = static_cast<ClosingSegment *>(ret.final_->reverse());
    ret.get_curves_().back() = boost::shared_ptr<Curve>(ret.final_);
    return ret;
  }

  void insert(iterator pos, Curve const &curve, Stitching stitching=NO_STITCHING) {
    Sequence source(1, boost::shared_ptr<Curve>(curve.duplicate()));
    if (stitching) stitch(pos.impl_, pos.impl_, source);
    do_update(pos.impl_, pos.impl_, source.begin(), source.end());
  }

  template <typename Impl>
  void insert(iterator pos,
              PathInternal::BaseIterator<Impl> first,
              PathInternal::BaseIterator<Impl> last,
              Stitching stitching=NO_STITCHING)
  {
    Sequence source(first.impl_, last.impl_);
    if (stitching) stitch(pos.impl_, pos.impl_, source);
    do_update(pos.impl_, pos.impl_, source.begin(), source.end());
  }

  void clear() {
    do_update(get_curves_().begin(), get_curves_().end()-1,
              get_curves_().begin(), get_curves_().begin());
  }

  void erase(iterator pos, Stitching stitching=NO_STITCHING) {
    if (stitching) {
      Sequence stitched;
      stitch(pos.impl_, pos.impl_+1, stitched);
      do_update(pos.impl_, pos.impl_+1, stitched.begin(), stitched.end());
    } else {
      do_update(pos.impl_, pos.impl_+1, get_curves_().begin(), get_curves_().begin());
    }
  }

  void erase(iterator first, iterator last, Stitching stitching=NO_STITCHING) {
    if (stitching) {
      Sequence stitched;
      stitch(first.impl_, last.impl_, stitched);
      do_update(first.impl_, last.impl_, stitched.begin(), stitched.end());
    } else {
      do_update(first.impl_, last.impl_, get_curves_().begin(), get_curves_().begin());
    }
  }

  // erase last segment of path
  void erase_last() {
    erase(get_curves_().end()-2);
  }

  void replace(iterator replaced, Curve const &curve, Stitching stitching=NO_STITCHING) {
    Sequence source(1, boost::shared_ptr<Curve>(curve.duplicate()));
    if (stitching) stitch(replaced.impl_, replaced.impl_+1, source);
    do_update(replaced.impl_, replaced.impl_+1, source.begin(), source.end());
  }

  void replace(iterator first_replaced, iterator last_replaced,
               Curve const &curve, Stitching stitching=NO_STITCHING)
  {
    Sequence source(1, boost::shared_ptr<Curve>(curve.duplicate()));
    if (stitching) stitch(first_replaced.impl_, last_replaced.impl_, source);
    do_update(first_replaced.impl_, last_replaced.impl_,
              source.begin(), source.end());
  }

  template <typename Impl>
  void replace(iterator replaced,
               PathInternal::BaseIterator<Impl> first,
               PathInternal::BaseIterator<Impl> last,
               Stitching stitching=NO_STITCHING)
  {
    Sequence source(first.impl_, last.impl_);
    if (stitching) stitch(replaced.impl_, replaced.impl_+1, source);
    do_update(replaced.impl_, replaced.impl_+1, source.begin(), source.end());
  }

  template <typename Impl>
  void replace(iterator first_replaced, iterator last_replaced,
               PathInternal::BaseIterator<Impl> first,
               PathInternal::BaseIterator<Impl> last,
               Stitching stitching=NO_STITCHING)
  {
    Sequence source(first.impl_, last.impl_);
    if (stitching) stitch(first_replaced.impl_, last_replaced.impl_, source);
    do_update(first_replaced.impl_, last_replaced.impl_,
              source.begin(), source.end());
  }

  void start(Point p) {
    clear();
    unshare_final();
    final_->setPoint(0, p);
    final_->setPoint(1, p);
  }

  Point initialPoint() const { return (*final_)[1]; }
  Point finalPoint() const { return (*final_)[0]; }

  void setInitial(Point const& p)
  {
	  if ( empty() ) return;
	  boost::shared_ptr<Curve> head(front().duplicate());
	  head->setInitial(p);
	  Sequence::iterator replaced = get_curves_().begin();
	  Sequence source(1, head);
	  do_update(replaced, replaced + 1, source.begin(), source.end());
  }

  void setFinal(Point const& p)
  {
	  if ( empty() ) return;
	  boost::shared_ptr<Curve> tail(back().duplicate());
	  tail->setFinal(p);
	  Sequence::iterator replaced = get_curves_().end() - 2;
	  Sequence source(1, tail);
	  do_update(replaced, replaced + 1, source.begin(), source.end());
  }

  void append(Curve const &curve, Stitching stitching=NO_STITCHING) {
    if (stitching) stitchTo(curve.initialPoint());
    do_append(curve.duplicate());
  }
  void append(D2<SBasis> const &curve, Stitching stitching=NO_STITCHING) {
    if (stitching) stitchTo(Point(curve[X][0][0], curve[Y][0][0]));
    do_append(new SBasisCurve(curve));
  }
  void append(Path const &other, Stitching stitching=NO_STITCHING) {
    insert(end(), other.begin(), other.end(), stitching);
  }

  void stitchTo(Point const &p) {
    if (!empty() && finalPoint() != p) {
      do_append(new StitchSegment(finalPoint(), p));
    }
  }

  template <typename CurveType, typename A>
  void appendNew(A a) {
    do_append(new CurveType(finalPoint(), a));
  }

  template <typename CurveType, typename A, typename B>
  void appendNew(A a, B b) {
    do_append(new CurveType(finalPoint(), a, b));
  }

  template <typename CurveType, typename A, typename B, typename C>
  void appendNew(A a, B b, C c) {
    do_append(new CurveType(finalPoint(), a, b, c));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D>
  void appendNew(A a, B b, C c, D d) {
    do_append(new CurveType(finalPoint(), a, b, c, d));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E>
  void appendNew(A a, B b, C c, D d, E e) {
    do_append(new CurveType(finalPoint(), a, b, c, d, e));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F>
  void appendNew(A a, B b, C c, D d, E e, F f) {
    do_append(new CurveType(finalPoint(), a, b, c, d, e, f));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F,
                                typename G>
  void appendNew(A a, B b, C c, D d, E e, F f, G g) {
    do_append(new CurveType(finalPoint(), a, b, c, d, e, f, g));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F,
                                typename G, typename H>
  void appendNew(A a, B b, C c, D d, E e, F f, G g, H h) {
    do_append(new CurveType(finalPoint(), a, b, c, d, e, f, g, h));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F,
                                typename G, typename H, typename I>
  void appendNew(A a, B b, C c, D d, E e, F f, G g, H h, I i) {
    do_append(new CurveType(finalPoint(), a, b, c, d, e, f, g, h, i));
  }

private:
  Sequence &get_curves_() { return *curves_; }
  Sequence const &get_curves_() const { return *curves_; }

  void unshare_curves() {
    if (!curves_.unique()) {
      curves_ = boost::shared_ptr<Sequence>(new Sequence(*curves_));
    }
  }

  void unshare_final() {
    if (!get_curves_().back().unique()) {
      unshare_curves();
      final_ = static_cast<ClosingSegment *>(final_->duplicate());
      get_curves_().back() = boost::shared_ptr<Curve>(final_);
    }
  }

  void stitch(Sequence::iterator first_replaced,
              Sequence::iterator last_replaced,
              Sequence &sequence);

  void do_update(Sequence::iterator first_replaced,
                 Sequence::iterator last_replaced,
                 Sequence::iterator first,
                 Sequence::iterator last);

  // n.b. takes ownership of curve object
  void do_append(Curve *curve);

  void check_continuity(Sequence::iterator first_replaced,
                        Sequence::iterator last_replaced,
                        Sequence::iterator first,
                        Sequence::iterator last);

  boost::shared_ptr<Sequence> curves_;
  ClosingSegment *final_;
  bool closed_;
};  // end class Path

inline static Piecewise<D2<SBasis> > paths_to_pw(std::vector<Path> paths) {
    Piecewise<D2<SBasis> > ret = paths[0].toPwSb();
    for(unsigned i = 1; i < paths.size(); i++) {
        ret.concat(paths[i].toPwSb());
    }
    return ret;
}

inline
Coord nearest_point(Point const& p, Path const& c)
{
	return c.nearestPoint(p);
}

}  // end namespace Geom

namespace std {

template <>
inline void swap<Geom::Path>(Geom::Path &a, Geom::Path &b)
{
  a.swap(b);
}

}  // end namespace std


#endif // SEEN_GEOM_PATH_H




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
