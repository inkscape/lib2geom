#ifndef SEEN_GEOM_PATH_H
#define SEEN_GEOM_PATH_H

#include "point.h"
#include <iterator>

namespace Geom {

namespace Path {

class Curve {
public:
  Curve(Point p, Curve *next) : initial_point_(p), next_(next) {}
  virtual ~Curve();

  Curve *next() const { return next_; }
  Point initialPoint() const { return initial_point_; }
  void setInitialPoint(Point p) { initial_point_ = p; }
  Point finalPoint() const { return next_->initialPoint(); }

private:
  void setNext(Curve *next) { next_ = next; }

  Point initial_point_;
  Curve *next_;

  friend class Path;
};

class LineSegment : public Curve {
public:
  LineSegment(Point p, Curve *next)
  : Curve(p, next) {}
};

class QuadraticBezier : public Curve {
public:
  QuadraticBezier(Point p0, Point p1, Curve *next)
  : Curve(p0, next), p1_(p1) {}
private:
  Point p1_;
};

class CubicBezier : public Curve {
public:
  CubicBezier(Point p0, Point p1, Point p2, Curve *next)
  : Curve(p0, next), p1_(p1), p2_(p2) {}

private:
  Point p1_;
  Point p2_;
};

class SVGEllipticalArc : public Curve {
public:
  SVGEllipticalArc(Point p, double rx, double ry,
                   double x_axis_rotation, bool large_arc,
                   bool sweep, Curve *next)
  : Curve(p, next), rx_(rx), ry_(ry), x_axis_rotation_(x_axis_rotation),
    large_arc_(large_arc), sweep_(sweep)
  {}

private:
  double rx_;
  double ry_;
  double x_axis_rotation_;
  bool large_arc_;
  bool sweep_;
};

/*
class SBasis : public Curve {
private:
};
*/

class Iterator : public std::forward_iterator<Curve const> {
public:
  Iterator() : current_(NULL), offset_(0) {}

  Curve const &operator*() const { return *current_; }
  Curve const *operator->() const { return current_; }

  Iterator &operator++(int) {
    current = current_->next();
    ++offset_;
    return *this;
  }

  Iterator const operator++() {
    Iterator saved=*this;
    ++(*this);
    return saved;
  }

private:
  Iterator(Curve const *element, unsigned offset)
  : current_(element), offset_(offset) {}

  Curve const *current_;
  unsigned offset_;

  friend class Path;
};

class Path {
public:
  typedef Curve const value_type;
  typedef Iterator iterator;
  typedef Iterator const_iterator;
  typedef Curve const &reference;
  typedef Curve const *pointer;
  typedef Iterator::difference_type difference_type;
  typedef unsigned size_type;

  Path(Point p) : size_(0), is_closed_(false) {
    first_ = last_ = terminator_ = new LineSegment(p, NULL);
  }

  ~Path() {
    for ( Curve *iter = first_, Curve *next = first_->next ;
          iter != terminator_ ;
          iter = next )
    {
      delete iter;
    }
    delete terminator_;
  }

  Iterator begin() const {
    return Iterator(first_, 0);
  }
  Iterator end_open() const {
    return Iterator(terminator_, size_);
  }
  Iterator end() const {
    if (is_closed_) {
      end_closed();
    } else {
      end_open();
    }
  }
  Iterator end_closed() const {
    return Iterator(first_, size_ + 1);
  }
  
  template <typename CurveType>
  void append(Point p) {
    do_append(new CurveType(last_->finalPoint(), last_->next()), p);
  }

  template <typename CurveType, typename A>
  void append(A a, Point p) {
    do_append(new CurveType(last_->finalPoint(), a, last_->next()), p);
  }

  template <typename CurveType, typename A, typename b>
  void append(A a, B b, Point p) {
    do_append(new CurveType(last_->finalPoint(), a, b, last_->next()), p);
  }

  template <typename CurveType, typename A, typename B, typename C>
  void append(A a, B b, C c, Point p) {
    do_append(new CurveType(last_->finalPoint(), a, b, c, last_->next()), p);
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D>
  void append(A a, B b, C c, D d, Point p) {
    do_append(new CurveType(last_->finalPoint(), a, b, c, d, last_->next()), p);
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E>
  void append(A a, B b, C c, D d, E e, Point p) {
    do_append(new CurveType(last_->finalPoint(), a, b, c, d, e, last_->next()), p);
  }

  template <typename CurveType, typename A, typename B, typename C,
                                typename D, typename E, typename F>
  void append(A a, B b, C c, D d, E e, F f, Point p) {
    do_append(new CurveType(last_->finalPoint(), a, b, c, d, e, f, last_->next()), p);
  }

  void close(bool close=true) {
    is_closed_ = close;
  }

  void is_closed() const {
    return terminator_->next();
  }

private:
  void do_append(Curve *curve, Point p) {
    terminator_->setInitialPoint(p);
    if ( first_ == terminator_ ) {
      terminator_->setNext(first_);
      first_ = curve;
    }
    last_ = curve;
    ++size_;
  }

  Curve *first_;
  Curve *last_;
  LineSegment *terminator_;
  unsigned size_;
  bool is_closed_;
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

