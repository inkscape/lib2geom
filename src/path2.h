#ifndef SEEN_GEOM_PATH_H
#define SEEN_GEOM_PATH_H

#include "point.h"
#include "rect.h"
#include <iterator>
#include <algorithm>
#include <exception>
#include "multidim-sbasis.h"
#include "bezier-to-sbasis.h"

namespace Geom {

namespace Path {

class Curve {
public:
  virtual ~Curve();

  virtual Point initialPoint() const = 0;
  virtual Point finalPoint() const = 0;

  virtual Curve *duplicate() const = 0;

  virtual Rect boundsFast() const = 0;
  virtual Rect boundsExact() const = 0;

  Point pointAt(Coord t) { return pointAndDerivativesAt(t, 0, NULL); }
  virtual Point pointAndDerivativesAt(Coord t, unsigned n, Point *ds) const = 0;
  virtual multidim_sbasis<2> sbasis() const = 0;
};

template <unsigned degree>
class Bezier : public Curve {
public:
  template <unsigned other_degree>
  static void assert_degree(Bezier<other_degree> const *) {}

  Bezier() {}

  // default copy
  // default assign

  Bezier(Point c0, Point c1) {
    assert_degree<2>(this);
    c_[0] = c0;
    c_[1] = c1;
  }

  Bezier(Point c0, Point c1, Point c2) {
    assert_degree<3>(this);
    c_[0] = c0;
    c_[1] = c1;
    c_[2] = c2;
  }

  Bezier(Point c0, Point c1, Point c2, Point c3) {
    assert_degree<4>(this);
    c_[0] = c0;
    c_[1] = c1;
    c_[2] = c2;
    c_[3] = c3;
  }

  Curve *duplicate() const { return new Bezier(*this); }

  Point initialPoint() const { return c_[0]; }
  Point finalPoint() const { return c_[degree-1]; }

  Point &operator[](int index) { return c_[index]; }
  Point const &operator[](int index) const { return c_[index]; }

  Rect boundsFast() const { return bounds(); }
  Rect boundsExact() const { return bounds(); }

  multidim_sbasis<2> sbasis() const {
    return bezier_to_sbasis<2, degree>(c_);
  }

private:
  Rect bounds() const {
    Point min=c_[0];
    Point max=c_[0];
    for ( unsigned i = 1 ; i < degree ; ++i ) {
      for ( unsigned axis = 0 ; axis < 2 ; ++axis ) {
        min[axis] = std::min(min[axis], c_[i][axis]);
        max[axis] = std::max(max[axis], c_[i][axis]);
      }
    }
    return Rect(min, max);
  }

  Point c_[degree];
};

// Bezier<0> and Bezier<1> are meaningless; specialize them out
template <> class Bezier<0> { Bezier(); };
template <> class Bezier<1> { Bezier(); };

typedef Bezier<2> LineSegment;
typedef Bezier<3> QuadraticBezier;
typedef Bezier<4> CubicBezier;

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

  explicit SBasis(multidim_sbasis<2> const &coeffs)
  : coeffs_(coeffs) {}

  Point initialPoint() const {
    return Point(coeffs_[X][0][0], coeffs_[Y][0][0]);
  }
  Point finalPoint() const {
    return Point(coeffs_[X][0][1], coeffs_[Y][0][1]);
  }

  Curve *duplicate() const { return new SBasis(*this); }

  double &operator[](int index) { return coeffs_[index]; }
  double const &operator[](int index) const { return coeffs_[index]; }

  multidim_sbasis<2> sbasis() const { return coeffs_; }
  
private:
  multidim_sbasis<2> coeffs_;
};

template <typename IteratorImpl>
class BaseIterator : public std::forward_iterator<Curve const &> {
public:
  typedef std::ptrdiff_t difference_type;
  typedef std::size_t size_type;

  BaseIterator() {}

  // default construct
  // default copy

  bool operator==(BaseIterator const &other) {
    return other.impl_ == impl_;
  }
  bool operator!=(BaseIterator const &other) {
    return other.impl_ != pos;
  }

  Curve const &operator*() const { return **impl_; }
  Curve const *operator->() const { return *impl_; }

  BaseIterator &operator++(int) {
    ++_pos;
    return *this;
  }
  BaseIterator operator++() {
    BaseIterator old=*this;
    ++(*this);
    return old;
  }

private:
  explicit BaseIterator(IteratorImpl const &pos)
  : impl_(pos) {}

  IteratorImpl impl_;
  friend class Path;
};

class ContinuityError : public std::exception {
  ContinuityError() : exception("non-contiguous path") {}
  ContinuityError(char const *message) : exception(message) {}
  ContinuityError(std::string const &message) : exception(message) {}
};

class Path {
private:
  typedef std::vector<Curve *> Sequence;

public:
  typedef BaseIterator<Sequence::iterator> iterator;
  typedef BaseIterator<Sequence::const_iterator> const_iterator;
  typedef Sequence::size_type size_type;
  typedef Sequence::difference_type difference_type;

  Path() {
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

  ~Path() {
    delete_range(curves_.begin(), curves_.end()-1);
  }

  void swap(Path &other) {
    std::swap(curves_, other.curves_);
    std::swap(closed_, other.closed_);
    std::swap(final_, other.final_);
    curves_[curves_.size()-1] = &final_;
    other.curves_[other.curves_.size()-1] = &other.final_;
  }

  iterator begin() { return curves_.begin(); }
  iterator end() { return curves_.end()-1; }

  Curve const &front() const { return curves_[0]; }
  Curve const &back() const { return curves_[curves.size()-2]; }

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

  Rect boundsFast() const {
    Rect bounds=front().boundsFast();
    const_iterator iter=begin();
    for ( ++iter ; iter != end() ; ++iter ) {
      bounds.expandTo(iter->boundsFast());
    }
    return bounds;
  }

  Rect boundsExact() const {
    Rect bounds=front().boundsFast();
    const_iterator iter=begin();
    for ( ++iter ; iter != end() ; ++iter ) {
      bounds.expandTo(iter->boundsExact());
    }
    return bounds;
  }

  void insert(iterator pos, Curve const &curve) {
    Sequence source(1, &curve);
    do_update(pos.impl_, pos.impl_, source.begin(), source.end());
  }

  template <typename Impl>
  void insert(iterator pos, BaseIterator<Impl> first, BaseIterator<Impl> last)
  {
    Sequence source(first.impl_, last.impl_);
    do_update(pos.impl_, pos.impl_, source.begin(), source.end());
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
    Sequence source(1, &curve);
    do_update(replaced.impl_, replaced.impl_+1, source.begin(), source.end());
  }

  void replace(iterator first_replaced, iterator last_replaced,
               Curve const &curve)
  {
    Sequence source(1, &curve);
    do_update(first.impl_, last.impl_, source.begin(), source.end());
  }

  template <typename Impl>
  void replace(iterator replaced,
               BaseIterator<Impl> first, BaseIterator<Impl> last)
  {
    Sequence source(first.impl_, last.impl_);
    do_update(replaced.impl_, replaced.impl_+1, source.begin(), source.end());
  }

  template <typename Impl>
  void replace(iterator first_replaced, iterator last_replaced,
               BaseIterator<Impl> first, BaseIterator<Impl> last)
  {
    Sequence source(first.impl_, last.impl_);
    do_update(first_replaced.impl_, last_replaced.impl_,
           source.begin(), source.end());
  }

  void start(Point p) {
    clear();
    final_[0] = final_[1] = p;
  }

  void append(Curve const &curve) {
    if ( curves_.front() != &final_ && curve.initialPoint() != final_[0] ) {
      throw ContinuityError();
    }
    do_append(curve.duplicate());
  }

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
                 Sequence::iterator last)
  {
    // note: modifies the contents of [first,last)

    check_continuity(first_replaced, last_replaced, first, last);
    duplicate_in_place(first, last);
    delete_range(first_replaced, last_replaced);

    if ( ( last - first ) == ( last_replaced - first_replaced ) ) {
      std::copy(first, last, first_replaced);
    } else {
      // this approach depends on std::vector's behavior WRT iterator stability
      curves_.erase(first_replaced, last_replaced);
      curves_.insert(first_replaced, first, last);
    }

    if ( curves_.front() != &final_ ) {
      final_[0] = back().finalPoint();
      final_[1] = front().initialPoint();
    }
  }

  void do_append(Curve *curve) {
    if ( curves_.front() == &final_ ) {
      final_[1] = curve->initialPoint();
    }
    curves_.insert(curves_.end()-1, curve);
    final_[0] = curve->finalPoint();
  }

  void duplicate_in_place(Sequence::iterator first, Sequence::iterator last) {
    for ( Sequence::iterator iter=first ; iter != last ; ++iter ) {
      *iter = (*iter)->duplicate();
    }
  }

  void delete_range(Sequence::iterator first, Sequence::iterator last) {
    for ( Sequence::iterator iter=first ; iter != last ; ++iter ) {
      delete *iter;
    }
  }

  void check_continuity(Sequence::iterator first_replaced,
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
    } else if ( first_replaced != last_replaced ) {
      if ( (*first_replaced)->initialPoint() !=
           (*(last_replaced-1))->finalPoint() )
      {
        throw ContinuityError();
      }
    }
  }

  Sequence curves_;
  LineSegment final_;
  bool closed_;
};

class Set {
public:
private:
};

}

}

template <>
void std::swap<Path>(Path &a, Path &b) {
  a.swap(b);
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

