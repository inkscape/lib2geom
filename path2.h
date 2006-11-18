#ifndef SEEN_GEOM_PATH_H
#define SEEN_GEOM_PATH_H

#include "point.h"
#include <iterator>
#include <algorithm>
#include "multidim-sbasis.h"

namespace Geom {

namespace Path {

class Curve {
public:
  virtual ~Curve();

  virtual Point initialPoint() const = 0;
  virtual Point finalPoint() const = 0;
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

  Point initialPoint() const { return c_[0]; }
  Point finalPoint() const { return c_[degree-1]; }

  Point &operator[](int index) { return c_[index]; }
  Point const &operator[](int index) const { return c_[index]; }

private:
  Point c_[degree];
};

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
  explicit SBasis(multidim_sbasis<2> const &coeffs)
  : coeffs_(coeffs) {}

  Point initialPoint() const {
    return Point(coeffs_[X][0][0], coeffs_[Y][0][0]);
  }
  Point finalPoint() const {
    return Point(coeffs_[X][0][1], coeffs_[Y][0][1]);
  }

  double &operator[](int index) { return coeffs_[index]; }
  double const &operator[](int index) const { return coeffs_[index]; }
  
private:
  multidim_sbasis<2> coeffs_;
};

class Iterator {
public:
private:
};

class Path {
public:
private:
};

class Location {
public:
private:
  Iterator pos;
  double t;
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

