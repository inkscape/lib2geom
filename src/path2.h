#ifndef SEEN_GEOM_PATH_H
#define SEEN_GEOM_PATH_H

#include "point.h"
#include "rect.h"
#include <iterator>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include "multidim-sbasis.h"
#include "bezier-to-sbasis.h"
#include "md-pw-sb.h"

namespace Geom {

namespace Path2 {

class Path;

class Curve {
public:
  virtual ~Curve() {}

  virtual Point initialPoint() const = 0;
  virtual Point finalPoint() const = 0;

  virtual Curve *duplicate() const = 0;

  virtual Rect boundsFast() const = 0;
  virtual Rect boundsExact() const = 0;

  virtual Path &subdivide(Coord t, Path &out) const {
    //TODO: require
    return out;
  }

  Point pointAt(Coord t) const { return pointAndDerivativesAt(t, 0, NULL); }
  virtual Point pointAndDerivativesAt(Coord t, unsigned n, Point *ds) const = 0;
  virtual MultidimSBasis<2> sbasis() const = 0;
};

struct BezierHelpers {
protected:
  static Rect bounds(unsigned degree, Point const *points);
  static Point point_and_derivatives_at(Coord t,
                                        unsigned degree, Point const *points,
                                        unsigned n_derivs, Point *derivs);
  static Point subdivideArr(Coord t, unsigned degree, Point const *V,
                         Point *Left, Point *Right);

};

template <unsigned bezier_degree>
class Bezier : public Curve, private BezierHelpers {
public:
  template <unsigned required_degree>
  static void assert_degree(Bezier<required_degree> const *) {}

  Bezier() {}

  // default copy
  // default assign

  Bezier(Point c0, Point c1) {
    assert_degree<1>(this);
    c_[0] = c0;
    c_[1] = c1;
  }

  Bezier(Point c0, Point c1, Point c2) {
    assert_degree<2>(this);
    c_[0] = c0;
    c_[1] = c1;
    c_[2] = c2;
  }

  Bezier(Point c0, Point c1, Point c2, Point c3) {
    assert_degree<3>(this);
    c_[0] = c0;
    c_[1] = c1;
    c_[2] = c2;
    c_[3] = c3;
  }

  unsigned degree() const { return bezier_degree; }

  Curve *duplicate() const { return new Bezier(*this); }

  Point initialPoint() const { return c_[0]; }
  Point finalPoint() const { return c_[bezier_degree]; }

  Point &operator[](int index) { return c_[index]; }
  Point const &operator[](int index) const { return c_[index]; }

  Rect boundsFast() const { return bounds(bezier_degree, c_); }
  Rect boundsExact() const { return bounds(bezier_degree, c_); }

  Path &subdivide(Coord t, Path &out) const;
  
  Point pointAndDerivativesAt(Coord t, unsigned n_derivs, Point *derivs)
  const
  {
    return point_and_derivatives_at(t, bezier_degree, c_, n_derivs, derivs);
  }

  MultidimSBasis<2> sbasis() const {
    return bezier_to_sbasis<2, bezier_degree, Geom::Point const *>(c_);
  }

protected:
  Bezier(Point c[]) {
    std::copy(c, c+bezier_degree+1, c_);
  }

private:
  Point c_[bezier_degree+1];
};

// Bezier<0> is meaningless; specialize it out
template <> class Bezier<0> { Bezier(); };

typedef Bezier<1> LineSegment;
typedef Bezier<2> QuadraticBezier;
typedef Bezier<3> CubicBezier;

class SVGEllipticalArc : public Curve {
public:
  SVGEllipticalArc() {}

  SVGEllipticalArc(Point initial, double rx, double ry,
                   double x_axis_rotation, bool large_arc,
                   bool sweep, Point final)
  : initial_(initial), rx_(rx), ry_(ry), x_axis_rotation_(x_axis_rotation),
    large_arc_(large_arc), sweep_(sweep), final_(final)
  {}

  Curve *duplicate() const { return new SVGEllipticalArc(*this); }

  Point initialPoint() const { return initial_; }
  Point finalPoint() const { return final_; }

  Rect boundsFast() const;
  Rect boundsExact() const;

  Path &subdivide(Coord t, Path &out) const;
  
  Point pointAndDerivativesAt(Coord t, unsigned n_derivs, Point *derivs) const;

  MultidimSBasis<2> sbasis() const;

private:
  Point initial_;
  double rx_;
  double ry_;
  double x_axis_rotation_;
  bool large_arc_;
  bool sweep_;
  Point final_;
};

class SBasis : public Curve {
public:
  SBasis() {}

  explicit SBasis(MultidimSBasis<2> const &coeffs)
  : coeffs_(coeffs) {}

  Point initialPoint() const {
    return Point(coeffs_[X][0][0], coeffs_[Y][0][0]);
  }
  Point finalPoint() const {
    return Point(coeffs_[X][0][1], coeffs_[Y][0][1]);
  }

  Curve *duplicate() const { return new SBasis(*this); }

  Rect boundsFast() const;
  Rect boundsExact() const;

  Point pointAndDerivativesAt(Coord t, unsigned n_derivs, Point *derivs) const;

  MultidimSBasis<2> sbasis() const { return coeffs_; }
  
private:
  MultidimSBasis<2> coeffs_;
};

template <typename IteratorImpl>
class BaseIterator
: public std::iterator<std::forward_iterator_tag, Curve const>
{
public:
  BaseIterator() {}

  // default construct
  // default copy

  bool operator==(BaseIterator const &other) {
    return other.impl_ == impl_;
  }
  bool operator!=(BaseIterator const &other) {
    return other.impl_ != impl_;
  }

  Curve const &operator*() const { return **impl_; }
  Curve const *operator->() const { return *impl_; }

  BaseIterator &operator++() {
    ++impl_;
    return *this;
  }
  BaseIterator operator++(int) {
    BaseIterator old=*this;
    ++(*this);
    return old;
  }

private:
  BaseIterator(IteratorImpl const &pos) : impl_(pos) {}

  IteratorImpl impl_;
  friend class Path;
};

template <typename Iterator>
class DuplicatingIterator
: public std::iterator<std::input_iterator_tag, Curve *>
{
public:
  DuplicatingIterator() {}
  DuplicatingIterator(Iterator const &iter) : impl_(iter) {}

  bool operator==(DuplicatingIterator const &other) {
    return other.impl_ == impl_;
  }
  bool operator!=(DuplicatingIterator const &other) {
    return other.impl_ != impl_;
  }

  Curve *operator*() const { return (*impl_)->duplicate(); }

  DuplicatingIterator &operator++() {
    ++impl_;
    return *this;
  }
  DuplicatingIterator operator++(int) {
    DuplicatingIterator old=*this;
    ++(*this);
    return old;
  }

private:
  Iterator impl_;
};

class ContinuityError : public std::runtime_error {
public:
  ContinuityError() : runtime_error("non-contiguous path") {}
  ContinuityError(std::string const &message) : runtime_error(message) {}
};

class Path {
private:
  typedef std::vector<Curve *> Sequence;

public:
  typedef BaseIterator<Sequence::iterator> iterator;
  typedef BaseIterator<Sequence::const_iterator> const_iterator;
  typedef Sequence::size_type size_type;
  typedef Sequence::difference_type difference_type;

  Path()
  : closed_(false)
  {
    curves_.push_back(&final_);
  }

  Path(Path const &other)
  : closed_(other.closed_)
  {
    curves_.push_back(&final_);
    insert(begin(), other.begin(), other.end());
  }

  explicit Path(Point p)
  : closed_(false)
  {
    curves_.push_back(&final_);
    final_[0] = final_[1] = p;
  }

  template <typename Impl>
  Path(BaseIterator<Impl> first, BaseIterator<Impl> last, bool closed=false)
  : closed_(closed)
  {
    curves_.push_back(&final_);
    insert(begin(), first, last);
  }

  ~Path();

  void swap(Path &other);

  iterator begin() { return curves_.begin(); }
  iterator end() { return curves_.end()-1; }

  Curve const &front() const { return *curves_[0]; }
  Curve const &back() const { return *curves_[curves_.size()-2]; }

  const_iterator begin() const { return curves_.begin(); }
  const_iterator end() const { return curves_.end()-1; }

  const_iterator end_open() const { return curves_.end()-1; }
  const_iterator end_closed() const { return curves_.end(); }
  const_iterator end_default() const {
    return ( closed_ ? end_closed() : end_open() );
  }

  size_type size() const { return curves_.size()-1; }
  size_type max_size() const { return curves_.max_size()-1; }

  bool empty() const { return curves_.size() == 1; }
  bool closed() const { return closed_; }
  void close(bool closed=true) { closed_ = closed; }

  Rect boundsFast() const;
  Rect boundsExact() const;

  md_pw_sb<2> toMdSb() const {
    int i = 0;
    pw_sb x, y;
    for(const_iterator it = begin(); it != end(); ++it, i++) {
      MultidimSBasis<2> foo = (*it).sbasis();
      x.cuts.push_back(i);
      x.segs.push_back(foo[0]);
      y.cuts.push_back(i);
      y.segs.push_back(foo[1]);
    }
    x.cuts.push_back(i);
    y.cuts.push_back(i);
    md_pw_sb<2> ret;
    ret[0] = x; ret[1] = y;
    return ret;
  }

  void insert(iterator pos, Curve const &curve) {
    Sequence source(1, curve.duplicate());
    try {
      do_update(pos.impl_, pos.impl_, source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  template <typename Impl>
  void insert(iterator pos, BaseIterator<Impl> first, BaseIterator<Impl> last)
  {
    Sequence source(DuplicatingIterator<Impl>(first.impl_),
                    DuplicatingIterator<Impl>(last.impl_));
    try {
      do_update(pos.impl_, pos.impl_, source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  void clear() {
    do_update(curves_.begin(), curves_.end()-1,
              curves_.begin(), curves_.begin());
  }

  void erase(iterator pos) {
    do_update(pos.impl_, pos.impl_+1, curves_.begin(), curves_.begin());
  }

  void erase(iterator first, iterator last) {
    do_update(first.impl_, last.impl_, curves_.begin(), curves_.begin());
  }

  void replace(iterator replaced, Curve const &curve) {
    Sequence source(1, curve.duplicate());
    try {
      do_update(replaced.impl_, replaced.impl_+1, source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  void replace(iterator first_replaced, iterator last_replaced,
               Curve const &curve)
  {
    Sequence source(1, curve.duplicate());
    try {
      do_update(first_replaced.impl_, last_replaced.impl_,
                source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  template <typename Impl>
  void replace(iterator replaced,
               BaseIterator<Impl> first, BaseIterator<Impl> last)
  {
    Sequence source(DuplicatingIterator<Impl>(first.impl_),
                    DuplicatingIterator<Impl>(last.impl_));
    try {
      do_update(replaced.impl_, replaced.impl_+1, source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  template <typename Impl>
  void replace(iterator first_replaced, iterator last_replaced,
               BaseIterator<Impl> first, BaseIterator<Impl> last)
  {
    Sequence source(first.impl_, last.impl_);
    try {
      do_update(first_replaced.impl_, last_replaced.impl_,
                source.begin(), source.end());
    } catch (...) {
      delete_range(source.begin(), source.end());
      throw;
    }
  }

  void start(Point p) {
    clear();
    final_[0] = final_[1] = p;
  }

  Point initialPoint() const { return final_[1]; }
  Point finalPoint() const { return final_[0]; }

  void append(Curve const &curve);

  void append(MultidimSBasis<2> const &curve);

  template <typename CurveType, typename A>
  void appendNew(A a) {
    do_append(new CurveType(final_[0], a));
  }

  template <typename CurveType, typename A, typename B>
  void appendNew(A a, B b) {
    do_append(new CurveType(final_[0], a, b));
  }

  template <typename CurveType, typename A, typename B, typename C>
  void appendNew(A a, B b, C c) {
    do_append(new CurveType(final_[0], a, b, c));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D>
  void appendNew(A a, B b, C c, D d) {
    do_append(new CurveType(final_[0], a, b, c, d));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E>
  void appendNew(A a, B b, C c, D d, E e) {
    do_append(new CurveType(final_[0], a, b, c, d, e));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F>
  void appendNew(A a, B b, C c, D d, E e, F f) {
    do_append(new CurveType(final_[0], a, b, c, d, e, f));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F,
                                typename G>
  void appendNew(A a, B b, C c, D d, E e, F f, G g) {
    do_append(new CurveType(final_[0], a, b, c, d, e, f, g));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F,
                                typename G, typename H>
  void appendNew(A a, B b, C c, D d, E e, F f, G g, H h) {
    do_append(new CurveType(final_[0], a, b, c, d, e, f, g, h));
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F,
                                typename G, typename H, typename I>
  void appendNew(A a, B b, C c, D d, E e, F f, G g, H h, I i) {
    do_append(new CurveType(final_[0], a, b, c, d, e, f, g, h, i));
  }

private:
  void do_update(Sequence::iterator first_replaced,
                 Sequence::iterator last_replaced,
                 Sequence::iterator first,
                 Sequence::iterator last);

  void do_append(Curve *curve);

  void delete_range(Sequence::iterator first, Sequence::iterator last);

  void check_continuity(Sequence::iterator first_replaced,
                        Sequence::iterator last_replaced,
                        Sequence::iterator first,
                        Sequence::iterator last);

  Sequence curves_;
  LineSegment final_;
  bool closed_;
};

template <unsigned bezier_degree>
inline Path &Bezier<bezier_degree>::subdivide(Coord t, Path &out) const {
  Bezier a, b;
  subdivideArr(t, bezier_degree, c_, a.c_, b.c_);
  out.clear();
  out.close(false);
  out.append(a);
  out.append(b);
  return out;
}

inline Path &SVGEllipticalArc::subdivide(Coord t, Path &out) const {
  SVGEllipticalArc a;
  SVGEllipticalArc b;
  a.rx_ = b.rx_ = rx_;
  a.ry_ = b.ry_ = ry_;
  a.x_axis_rotation_ = b.x_axis_rotation_ = x_axis_rotation_;
  a.initial_ = initial_;
  a.final_ = b.initial_ = pointAt(t);
  b.final_ = final_;
  out.clear();
  out.close(false);
  out.append(a);
  out.append(b);
  return out;
}

class Set {
public:
private:
};

}

}

namespace std {

template <>
inline void swap<Geom::Path2::Path>(Geom::Path2::Path &a, Geom::Path2::Path &b)
{
  a.swap(b);
}

}

#endif // SEEN_GEOM_PATH_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

