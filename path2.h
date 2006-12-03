#ifndef SEEN_GEOM_PATH_H
#define SEEN_GEOM_PATH_H

#include "point.h"
#include <iterator>
#include <algorithm>
#include <exception>
#include "multidim-sbasis.h"

namespace Geom {

namespace Path {

class Curve {
public:
  virtual ~Curve();

  virtual Point initialPoint() const = 0;
  virtual Point finalPoint() const = 0;

  virtual Curve *duplicate() const = 0;
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

private:
  Point c_[degree];
};

// Bezier<0> and Bezier<1> are meaningless; specialize them out
template <> class Bezier<0> { };
template <> class Bezier<1> { };

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
  
private:
  multidim_sbasis<2> coeffs_;
};

template <typename IteratorImpl>
class BaseIterator : public std::forward_iterator<Curve const &> {
public:
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

class Path {
public:
  typedef BaseIterator<std::vector<Curve *>::iterator> iterator;
  typedef BaseIterator<std::vector<Curve *>::const_iterator> const_iterator;
  typedef std::vector<Curve *>::size_type size_type;
  typedef std::vector<Curve *>::difference_type difference_type;

  Path() {
    curves_.push_back(new LineSegment());
  }

  Path(Path const &other)
  : closed_(other.closed_)
  {
    curves_.push_back(new LineSegment());
    insert(begin(), other.begin(), other.end_open());
  }

  ~Path() {
    std::vector<Curve *>::iterator iter;
    for ( iter = curves_.begin() ; iter != curves_.end() ; ++iter ) {
      delete *iter;
    }
  }

  iterator begin() { return curves_.begin(); }
  iterator end() { return curves_.end()-1; }

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

  void insert(iterator pos, Curve const &curve) {
    difference_type offset=pos.impl_-curves_.begin();
    curves_.insert(pos.impl_, curve.duplicate());
    stitch(offset, offset + 1);
  }

  template <InputIterator>
  void insert(iterator pos, InputIterator first, InputIterator last) {
    std::vector<Curve *> curves;
    for ( ; first != last ; ++first ) {
      curves.push_back(first->duplicate());
    }
    difference_type offset=pos.impl_-curves_.begin();
    curves_.insert(pos.impl_, curves.begin(), curves.end());
    stitch(offset, offset + curves.size());
  }

  void insert(iterator pos, size_type n, Curve const &curve) {
    if ( n > 0 ) {
      difference_type offset=pos.impl_-curves_.begin();
      curves_.reserve(curves_.size() + n);
      for ( size_type left=n ; left > 0 ; --left ) {
        curves_.push_back(curve->duplicate());
      }
      stitch(offset, offset + n);
    }
  }

  void clear() {
    if (!empty()) {
      curves_.erase(curves_.begin(), curves_.end()-1);
    }
  }

  void erase(iterator pos) {
    difference_type offset=pos.impl_-curves_.begin();
    delete *pos.impl_;
    curves_.erase(pos.impl_);
    stitch(offset, offset);
  }

  void erase(iterator first, iterator last) {
    difference_type offset=pos.impl_-curves_.begin();
    for ( iterator iter = first ; iter != last ; ++iter ) {
      delete *iter.impl_;
    }
    curves_.erase(first.impl_, last.impl_);
    stitch(offset, offset);
  }

  void replace(iterator pos, Curve const &curve) {
    difference_type offset=pos.impl_-curves_.begin();
    Curve *new_curve = curve->duplicate();
    delete *pos.impl_;
    *pos.impl_ = new_curve;
    stitch(offset, offset + 1);
  }

private:
  std::vector<Curve *> curves_;
  bool closed_;
};

class Set {
public:
private:
};

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

