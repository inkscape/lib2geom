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

  // virtual Rect bounds() const = 0;
  // virtual Point at_t(Coord t) const = 0;
  // virtual multidim_sbasis<2> sbasis() const = 0;
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

  ~Path() {
    delete_sequence(curves_.begin(), curves_.end()-1);
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

  void insert(iterator pos, Curve const &curve) {
    insert(pos, iterator(pos.impl_+1), &curve, &curve+1);
  }

  template <InputIterator>
  void insert(iterator pos, InputIterator first, InputIterator last) {
    Sequence source(first, last);
    check_insert_continuity(pos.impl_, pos.impl_, source.begin(), source.end());
    duplicate_in_place(source.begin(), source.end());
    curves_.insert(pos.impl_, source.begin(), source.end());
    update_final();
  }

  void insert(iterator pos, unsigned n, Curve const &curve) {
    check_insert_continuity(pos.impl_, pos.impl_, &curve, &curve+1);
    if ( n > 1 && curve.initialPoint() != curve.finalPoint() ) {
      throw ContinuityError();
    }
    Sequence source(n, &curve);
    duplicate_in_place(source.begin(), source.end());
    curves_.insert(pos.impl_, source.begin(), source.end());
    update_final();
  }

  void clear() {
    delete_sequence(curves_.begin(), curves_.end()-1);
    curves_.erase(curves_.begin(), curves_.end()-1);
    // no need to update final when path is empty
  }

  void erase(iterator pos) {
    erase(pos, iterator(pos.impl_+1));
  }

  void erase(iterator first, iterator last) {
    check_erase_continuity(first.impl_, last.impl_);
    delete_sequence(first.impl_, last.impl_);
    curves_.erase(first.impl_, last.impl_);
    update_final();
  }

  void replace(iterator replaced, Curve const &curve) {
    replace(replaced, iterator(replaced.impl_+1), &curve, &curve+1);
  }

  void replace(iterator first_replaced, iterator last_replaced,
               Curve const &curve)
  {
    replace(first_replaced, last_replaced, &curve, &curve+1);
  }

  template <typename InputIterator>
  void replace(iterator replaced, InputIterator first, InputIterator last) {
    replace(replaced, iterator(replaced.impl_+1), first, last);
  }

  template <typename InputIterator>
  void replace(iterator first_replaced, iterator last_replaced,
               InputIterator first, InputIterator last)
  {
    Sequence source(first, last);
    check_replace_continuity(first_replaced.impl_, last_replaced.impl_,
                             source.begin(), source.end());
    duplicate_in_place(source.begin(), source.end());
    delete_sequence(first_replaced.impl_, last_replaced.impl_);

    // need to use a different method if Sequence is not a std::vector
    if ( source.size() == ( last_replaced.impl_ - first_replaced.impl_ ) ) {
      std::copy(source.begin(), source.end(), &*first_replaced.impl_);
    } else {
      curves_.erase(first_replaced.impl_, last_replaced.impl_);
      curves_.insert(first_replaced.impl_, source.begin(), source.end());
    }

    update_final();
  }

private:
  void duplicate_in_place(Sequence::iterator first, Sequence::iterator last) {
    for ( Sequence::iterator iter=first ; iter != last ; ++iter ) {
      *iter = (*iter)->duplicate();
    }
  }

  template <typename InputIterator>
  void delete_sequence(InputIterator first, InputIterator last)
  {
    for ( Sequence::iterator iter=first ; iter != last ; ++iter ) {
      delete *iter;
    }
  }

  template <typename InputIterator>
  void check_insert_continuity(Sequence::iterator pos,
                               InputIterator first, InputIterator last)
  {
  }

  void check_erase_continuity(Sequence::iterator first,
                              Sequence::iterator last)
  {
  }

  template <typename InputIterator>
  void check_replace_continuity(Sequence::iterator first_replaced,
                                Sequence::iterator last_replaced,
                                InputIterator first, InputIterator last)
  {
  }

  void update_final() {
    if ( curves_.front() != &final_ ) {
      final_[0] = back().finalPoint();
      final_[1] = front().initialPoint();
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

