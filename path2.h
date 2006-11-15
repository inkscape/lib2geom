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
  void setFinalPoint(Point p) { next_->setInitialPoint(p); }

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

template <typename T>
class BaseIterator : public std::forward_iterator<T> {
public:
  BaseIterator() : current_(NULL), offset_(0) {}

  reference operator*() const { return *current_; }
  pointer operator->() const { return current_; }

  BaseIterator &operator++(int) {
    current = current_->next();
    ++offset_;
    return *this;
  }

  BaseIterator const operator++() {
    BaseIterator saved=*this;
    ++(*this);
    return saved;
  }

protected:
  BaseIterator(pointer element, unsigned offset)
  : current_(element), offset_(offset) {}

  pointer current_;
  unsigned offset_;
};

class Iterator : public BaseIterator<Curve> {
public:
  Iterator() : BaseIterator() {}

private:
  Iterator(pointer element, unsigned offset)
  : BaseIterator(element, offset) {}

  friend class Path;
};

class ConstIterator : public BaseIterator<Curve const> {
public:
  ConstIterator() : BaseIterator() {}

private:
  ConstIterator(pointer element, unsigned offset)
  : BaseIterator(element, offset) {}

  friend class Path;
};


class Path {
public:
  typedef Curve const value_type;
  typedef Iterator iterator;
  typedef ConstIterator const_iterator;
  typedef Iterator::reference reference;
  typedef Iterator::pointer pointer;
  typedef Iterator::difference_type difference_type;
  typedef unsigned size_type;

  Path(Point p) : is_closed_(false) {
    init_elements(p);
  }
  Path(Path const &other) : is_closed_(false) {
    copy_elements(other);
  }

  ~Path() {
    free_elements();
  }

  Path &operator=(Path const &other) {
    free_elements();
    is_closed_ = other.is_closed_
    copy_elements(other);
  }

  size_type size() const {
    return size_;
  }
  size_type max_size() const {
    return ~0;
  }
  bool empty() const {
    return !size_;
  }

  void swap(Path &other) {
    Curve *temp_first = other.first_;
    Curve *temp_last = other.last_;
    LineSegment *temp_terminator = other.terminator_;
    size_type temp_size = other.size_;
    bool temp_is_closed = other.is_closed_;

    other.first_ = first_;
    other.last_ = last_;
    other.terminator_ = terminator_;
    other.size_ = size;
    other.is_closed_ = is_closed_;

    first_ = temp_first;
    last_ = temp_last;
    terminator_ = temp_terminator;
    size_ = temp_size;
    is_closed_ = temp_is_closed;
  }

  iterator begin() {
    return iterator(first_, 0);
  }
  const_iterator begin() const {
    return const_iterator(first_, 0);
  }
  iterator end_open() {
    return iterator(terminator_, size_);
  }
  const_iterator end_open() const {
    return const_iterator(terminator_, size_);
  }
  iterator end() {
    if (is_closed_) {
      return end_closed();
    } else {
      return end_open();
    }
  }
  const_iterator end() const {
    if (is_closed_) {
      return end_closed();
    } else {
      return end_open();
    }
  }
  iterator end_closed() {
    return iterator(first_, size_ + 1);
  }
  const_iterator end_closed() const {
    return const_iterator(first_, size_ + 1);
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

  void insert(iterator pos) {
    // FIXME
  }

  template <typename InputIterator>
  void insert(iterator pos, InputIterator f, InputIterator l) {
    for ( InputIterator iter = f ; iter != l ; ++iter, ++pos ) {
      insert(pos, *iter);
    }
  }

  void erase(iterator pos) {
    // FIXME
  }

  void erase(iterator first, iterator last) {
    // FIXME
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

  void free_elements() {
    for ( Curve *iter = first_, Curve *next = first_->next ;
          iter != terminator_ ;
          iter = next )
    {
      delete iter;
    }
    delete terminator_;
  }

  void init_elements(Point p) {
    size_ = 0;
    first_ = last_ = terminator_ = new LineSegment(p, NULL);
  }

  void copy_elements(Path const &other) {
    init_elements(other.first_->initialPoint());
    insert(begin(), other.begin(), other.end_open());
  }

  Curve *first_;
  Curve *last_;
  LineSegment *terminator_;
  size_type size_;
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

