/**
 * \file
 * \brief Circle Curve
 *
 * Authors:
 *      Nathan Hurst <njh@njhurst.com
 *
 * Copyright 2009  authors
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


#include <2geom/conicsec.h>
#include <2geom/numeric/fitting-tool.h>
#include <2geom/numeric/fitting-model.h>


// File: convert.h
#include <sstream>
#include <stdexcept>

// GPL taint?
#include <2geom/numeric/linear_system.h>

namespace Geom
{

LineSegment intersection(Line l, Rect r) {
    Point p0, p1;
    double a,b,c;
    std::vector<double> ifc = l.implicit_form_coefficients();
    a = ifc[0];
    b = ifc[1];
    c = ifc[2];
    if (fabs(b) > fabs(a)) {
        p0 = Point(r[0][0], (-c - a*r[0][0])/b);
        if (p0[1] < r[1][0])
            p0 = Point((-c - b*r[1][0])/a, r[1][0]);
        if (p0[1] > r[1][1])
            p0 = Point((-c - b*r[1][1])/a, r[1][1]);
        p1 = Point(r[0][1], (-c - a*r[0][1])/b);
        if (p1[1] < r[1][0])
            p1 = Point((-c - b*r[1][0])/a, r[1][0]);
        if (p1[1] > r[1][1])
            p1 = Point((-c - b*r[1][1])/a, r[1][1]);
    } else {
        p0 = Point((-c - b*r[1][0])/a, r[1][0]);
        if (p0[0] < r[0][0])
            p0 = Point(r[0][0], (-c - a*r[0][0])/b);
        if (p0[0] > r[0][1])
            p0 = Point(r[0][1], (-c - a*r[0][1])/b);
        p1 = Point((-c - b*r[1][1])/a, r[1][1]);
        if (p1[0] < r[0][0])
            p1 = Point(r[0][0], (-c - a*r[0][0])/b);
        if (p1[0] > r[0][1])
            p1 = Point(r[0][1], (-c - a*r[0][1])/b);
    }
    return LineSegment(p0, p1);
}

static double det(Point a, Point b) {
    return a[0]*b[1] - a[1]*b[0];
}

template <typename T>
static T det(T a, T b, T c, T d) {
    return a*d - b*c;
}

template <typename T>
static T det(T M[2][2]) {
    return M[0][0]*M[1][1] - M[1][0]*M[0][1];
}

template <typename T>
static T det3(T M[3][3]) {
    return ( M[0][0] * det(M[1][1], M[1][2],
                           M[2][1], M[2][2])
             -M[1][0] * det(M[0][1], M[0][2],
                            M[2][1], M[2][2])
             +M[2][0] * det(M[0][1], M[0][2],
                            M[1][1], M[1][2]));
}

static double boxprod(Point a, Point b, Point c) {
    return det(a,b) - det(a,c) + det(b,c);
}


/**
 * Find the roots of (q2x + q1)x+q0 = 0
 * Tries to be numerically robust.
 */
template <typename T>
static std::vector<T> quadratic_roots(T q0, T q1, T q2) {
    std::vector<double> r;
    if(q2 == 0) {
        if(q1 == 0) { // zero or infinite roots
            return r;
        }
        r.push_back(-q0/q1);
    } else {
        double desc = q1*q1 - 4*q2*q0;
        /*cout << q2 << ", "
          << q1 << ", "
          << q0 << "; "
          << desc << "\n";*/
        if (desc < 0)
            return r;
        else if (desc == 0)
            r.push_back(-q1/(2*q2));
        else {
            desc = std::sqrt(desc);
            double t = -0.5*(q1+sgn(q1)*desc);
            r.push_back(t/q2);
            r.push_back(q0/t);
        }
    }
    return r;
}



class BadConversion : public std::runtime_error {
public:
    BadConversion(const std::string& s)
        : std::runtime_error(s)
    { }
};

template <typename T>
inline std::string stringify(T x)
{
    std::ostringstream o;
    if (!(o << x))
        throw BadConversion("stringify(T)");
    return o.str();
}

  /* A G4 continuous cubic parametric approximation for rational quadratics.
     See
  An analysis of cubic approximation schemes for conic sections
            Michael Floater
            SINTEF

     This is less accurate overall than some of his other schemes, but
     produces very smooth joins and is still optimally h^-6
     convergent.
  */

double RatQuad::lambda() const {
  return 2*(6*w*w +1 -std::sqrt(3*w*w+1))/(12*w*w+3);
}

RatQuad RatQuad::fromPointsTangents(Point P0, Point dP0,
                                    Point P,
                                    Point P2, Point dP2) {
  Line Line0 = Line::fromPointDirection(P0, dP0);
  Line Line2 = Line::fromPointDirection(P2, dP2);
  try {
    OptCrossing oc = intersection(Line0, Line2);
    if(!oc) // what to do?
        return RatQuad(Point(), Point(), Point(), 0); // need opt really
    //assert(0);
    Point P1 = Line0.pointAt((*oc).ta);
    double triarea = boxprod(P0, P1, P2);
    if(triarea == 0)
        return RatQuad(P0, 0.5*(P0+P2), P2, 0); // line
    double tau0 = boxprod(P, P1, P2)/triarea;
    double tau1 = boxprod(P0, P, P2)/triarea;
    double tau2 = boxprod(P0, P1, P)/triarea;
    double w = tau1/(2*std::sqrt(tau0*tau2));
    return  RatQuad(P0, P1, P2, w);
  } catch(Geom::InfiniteSolutions) {
    return RatQuad(P0, 0.5*(P0+P2), P2, 1);
  }
  return RatQuad(Point(), Point(), Point(), 0); // need opt really
}

RatQuad RatQuad::circularArc(Point P0, Point P1, Point P2) {
    Line Line0 = Line::fromPointDirection(P0, P1 - P0);
    Line Line2 = Line::fromPointDirection(P2, P1 - P2);
    return RatQuad(P0, P1, P2, dot(unit_vector(P0 - P1), unit_vector(P0 - P2)));
}


CubicBezier RatQuad::toCubic() const {
    return toCubic(lambda());
}

CubicBezier RatQuad::toCubic(double lamb) const {
  return CubicBezier(P[0],
		     (1-lamb)*P[0] + lamb*P[1],
		     (1-lamb)*P[2] + lamb*P[1],
		     P[2]);
}

Point RatQuad::pointAt(double t) const {
  Bezier xt(P[0][0], P[1][0]*w, P[2][0]);
  Bezier yt(P[0][1], P[1][1]*w, P[2][1]);
  double wt = Bezier(1, w, 1).valueAt(t);
  return Point(xt.valueAt(t)/wt,
	       yt.valueAt(t)/wt);
}

void RatQuad::split(RatQuad &a, RatQuad &b) const {
  a.P[0] = P[0];
  b.P[2] = P[2];
  a.P[1] = (P[0]+w*P[1])/(1+w);
  b.P[1] = (w*P[1]+P[2])/(1+w);
  a.w = b.w = std::sqrt((1+w)/2);
  a.P[2] = b.P[0] = (0.5*a.P[1]+0.5*b.P[1]);
}


D2<SBasis> RatQuad::hermite() const {
  SBasis t = Linear(0, 1);
  SBasis omt = Linear(1, 0);

  D2<SBasis> out(omt*omt*P[0][0]+2*omt*t*P[1][0]*w+t*t*P[2][0],
		 omt*omt*P[0][1]+2*omt*t*P[1][1]*w+t*t*P[2][1]);
  for(int dim = 0; dim < 2; dim++) {
    out[dim] = divide(out[dim], (omt*omt+2*omt*t*w+t*t), 2);
  }
  return out;
}

  std::vector<SBasis> RatQuad::homogenous() const {
    std::vector<SBasis> res(3, SBasis());
  Bezier xt(P[0][0], P[1][0]*w, P[2][0]);
  bezier_to_sbasis(res[0],xt);
  Bezier yt(P[0][1], P[1][1]*w, P[2][1]);
  bezier_to_sbasis(res[1],yt);
  Bezier wt(1, w, 1);
  bezier_to_sbasis(res[2],wt);
  return res;
}

  std::string xAx::categorise() const {
  double M[3][3] = {{c[0], c[1], c[3]},
		    {c[1], c[2], c[4]},
		    {c[3], c[4], c[5]}};
  double D = det3(M);
  if (c[0] == 0 && c[1] == 0 && c[2] == 0)
    return "line";
  std::string res = stringify(D);
  double descr = c[1]*c[1] - c[0]*c[2];
  if (descr < 0) {
    if (c[0] == c[2] && c[1] == 0)
      return res + "circle";
    return res + "ellipse";
  } else if (descr == 0) {
    return res + "parabola";
  } else if (descr > 0) {
    if (c[0] + c[2] == 0) {
      if (D == 0)
	return res + "two lines";
      return res + "rectangular hyperbola";
    }
    return res + "hyperbola";

  }
  return "no idea!";
}

std::vector<Point> decompose_degenerate(xAx const & C1, xAx const & C2, xAx const & xC0) {
    std::vector<Point> res;
    double A[2][2] = {{2*xC0.c[0], xC0.c[1]},
                      {xC0.c[1], 2*xC0.c[2]}};
//Point B0 = xC0.bottom();
    double const determ = det(A);
    //std::cout << determ << "\n";
    if (fabs(determ) >= 1e-20) { // hopeful, I know
        Geom::Coord const ideterm = 1.0 / determ;

        double b[2] = {-xC0.c[3], -xC0.c[4]};
        Point B0((A[1][1]*b[0]  -A[0][1]*b[1]),
                 (-A[1][0]*b[0] +  A[0][0]*b[1]));
        B0 *= ideterm;
        Point n0, n1;
        // Are these just the eigenvectors of A11?
        if(xC0.c[0] == xC0.c[2]) {
            double b = 0.5*xC0.c[1]/xC0.c[0];
            double c = xC0.c[2]/xC0.c[0];
            //assert(fabs(b*b-c) > 1e-10);
            double d =  std::sqrt(b*b-c);
            //assert(fabs(b-d) > 1e-10);
            n0 = Point(1, b+d);
            n1 = Point(1, b-d);
        } else if(fabs(xC0.c[0]) > fabs(xC0.c[2])) {
            double b = 0.5*xC0.c[1]/xC0.c[0];
            double c = xC0.c[2]/xC0.c[0];
            //assert(fabs(b*b-c) > 1e-10);
            double d =  std::sqrt(b*b-c);
            //assert(fabs(b-d) > 1e-10);
            n0 = Point(1, b+d);
            n1 = Point(1, b-d);
        } else {
            double b = 0.5*xC0.c[1]/xC0.c[2];
            double c = xC0.c[0]/xC0.c[2];
            //assert(fabs(b*b-c) > 1e-10);
            double d =  std::sqrt(b*b-c);
            //assert(fabs(b-d) > 1e-10);
            n0 = Point(b+d, 1);
            n1 = Point(b-d, 1);
        }

        Line L0 = Line::fromPointDirection(B0, rot90(n0));
        Line L1 = Line::fromPointDirection(B0, rot90(n1));

        std::vector<double> rts = C1.roots(L0);
        for(unsigned i = 0; i < rts.size(); i++) {
            Point P = L0.pointAt(rts[i]);
            res.push_back(P);
        }
        rts = C1.roots(L1);
        for(unsigned i = 0; i < rts.size(); i++) {
            Point P = L1.pointAt(rts[i]);
            res.push_back(P);
        }
    } else {
        // single or double line
        Point trial_pt(0,0);
        Point g = xC0.gradient(trial_pt);
        if(L2sq(g) == 0) {
            trial_pt[0] += 1;
            g = xC0.gradient(trial_pt);
            if(L2sq(g) == 0) {
                trial_pt[1] += 1;
                g = xC0.gradient(trial_pt);
                if(L2sq(g) == 0) {
                    trial_pt[1] += 1;
                    g = xC0.gradient(trial_pt);
                }
            }
        }
        //std::cout << trial_pt << ", " << g << "\n";
        /**
         * At this point we have tried up to 4 points: 0,0, 1,0, 1,1, 1,2
         *
         * I'm pretty sure that no degenerate conic can pass through these points, so we can assume
         * that we've found a perpendicular to the double line.  Prove. (6 points in the general
         * case, but what are they?)
         *
         * alternatively, there may be a way to determine this directly from xC0
         */
        assert(L2sq(g) != 0);

        Line Lx = Line::fromPointDirection(trial_pt, g); // a line along the gradient
        double A[2][2] = {{2*xC0.c[0], xC0.c[1]},
                          {xC0.c[1], 2*xC0.c[2]}};
        double const determ = det(A);
        std::vector<double> rts = xC0.roots(Lx);
        for(unsigned i = 0; i < rts.size(); i++) {
            Point P0 = Lx.pointAt(rts[i]);
            //std::cout << P0 << "\n";
            Line L = Line::fromPointDirection(P0, rot90(g));
            std::vector<double> cnrts;
            // It's very likely that at least one of the conics is degenerate, this will hopefully pick the more generate of the two.
            if(fabs(C1.hessian().det()) > fabs(C2.hessian().det()))
                cnrts = C1.roots(L);
            else
                cnrts = C2.roots(L);
            for(unsigned j = 0; j < cnrts.size(); j++) {
                Point P = L.pointAt(cnrts[j]);
                res.push_back(P);
            }
        }
    }
    return res;
}

double xAx_descr(xAx const & C) {
    double mC[3][3] = {{C.c[0], (C.c[1])/2, (C.c[3])/2},
                       {(C.c[1])/2, C.c[2], (C.c[4])/2},
                       {(C.c[3])/2, (C.c[4])/2, C.c[5]}};

    return det3(mC);
}


std::vector<Point> intersect(xAx const & C1, xAx const & C2) {
    // You know, if either of the inputs are degenerate we should use them first!
    if(xAx_descr(C1) == 0) {
        return decompose_degenerate(C1, C2, C1);
    }
    if(xAx_descr(C2) == 0) {
        return decompose_degenerate(C1, C2, C2);
    }
    std::vector<Point> res;
    SBasis T(Linear(-1,1));
    SBasis S(Linear(1,1));
    SBasis C[3][3] = {{T*C1.c[0]+S*C2.c[0], (T*C1.c[1]+S*C2.c[1])/2, (T*C1.c[3]+S*C2.c[3])/2},
                      {(T*C1.c[1]+S*C2.c[1])/2, T*C1.c[2]+S*C2.c[2], (T*C1.c[4]+S*C2.c[4])/2},
                      {(T*C1.c[3]+S*C2.c[3])/2, (T*C1.c[4]+S*C2.c[4])/2, T*C1.c[5]+S*C2.c[5]}};

    SBasis D = det3(C);
    std::vector<double> rts = Geom::roots(D);
    if(rts.empty()) {
        T = Linear(1,1);
        S = Linear(-1,1);
        SBasis C[3][3] = {{T*C1.c[0]+S*C2.c[0], (T*C1.c[1]+S*C2.c[1])/2, (T*C1.c[3]+S*C2.c[3])/2},
                          {(T*C1.c[1]+S*C2.c[1])/2, T*C1.c[2]+S*C2.c[2], (T*C1.c[4]+S*C2.c[4])/2},
                          {(T*C1.c[3]+S*C2.c[3])/2, (T*C1.c[4]+S*C2.c[4])/2, T*C1.c[5]+S*C2.c[5]}};

        D = det3(C);
        rts = Geom::roots(D);
    }
    // at this point we have a T and S and perhaps some roots that represent our degenerate conic
    // Let's just pick one randomly (can we do better?)
    //for(unsigned i = 0; i < rts.size(); i++) {
    if(!rts.empty()) {
        unsigned i = 0;
        double t = T.valueAt(rts[i]);
        double s = S.valueAt(rts[i]);
        xAx xC0 = C1*t + C2*s;
        //::draw(cr, xC0, screen_rect); // degen

        return decompose_degenerate(C1, C2, xC0);


    } else {
        std::cout << "What?\n";
        ;//std::cout << D << "\n";
    }
    return res;
}


xAx xAx::fromPoint(Point p) {
  return xAx(1., 0, 1., -2*p[0], -2*p[1], dot(p,p));
}

xAx xAx::fromDistPoint(Point /*p*/, double /*d*/) {
    return xAx();//1., 0, 1., -2*(1+d)*p[0], -2*(1+d)*p[1], dot(p,p)+d*d);
}

xAx xAx::fromLine(Point n, double d) {
  return xAx(n[0]*n[0], 2*n[0]*n[1], n[1]*n[1], 2*d*n[0], 2*d*n[1], d*d);
}

xAx xAx::fromLine(Line l) {
  double dist;
  Point norm = l.normalAndDist(dist);

  return fromLine(norm, dist);
}

xAx xAx::fromPoints(std::vector<Geom::Point> const &pt) {
    Geom::NL::Vector V(pt.size(), -1.0);
    Geom::NL::Matrix M(pt.size(), 5);
    for(unsigned i = 0; i < pt.size(); i++) {
        Geom::Point P = pt[i];
        Geom::NL::VectorView vv = M.row_view(i);
        vv[0] = P[0]*P[0];
        vv[1] = P[0]*P[1];
        vv[2] = P[1]*P[1];
        vv[3] = P[0];
        vv[4] = P[1];
    }

    Geom::NL::LinearSystem ls(M, V);

    Geom::NL::Vector x = ls.SV_solve();
    return Geom::xAx(x[0], x[1], x[2], x[3], x[4], 1);

}



double xAx::valueAt(Point P) const {
  return evaluate_at(P[0], P[1]);
}

xAx xAx::scale(double sx, double sy) const {
  return xAx(c[0]*sx*sx, c[1]*sx*sy, c[2]*sy*sy,
	     c[3]*sx, c[4]*sy, c[5]);
}

Point xAx::gradient(Point p)  const{
  double x = p[0];
  double y = p[1];
  return Point(2*c[0]*x + c[1]*y + c[3],
	       c[1]*x + 2*c[2]*y + c[4]);
}

xAx xAx::operator-(xAx const &b) const {
  xAx res;
  for(int i = 0; i < 6; i++) {
    res.c[i] = c[i] - b.c[i];
  }
  return res;
}
xAx xAx::operator+(xAx const &b) const {
  xAx res;
  for(int i = 0; i < 6; i++) {
    res.c[i] = c[i] + b.c[i];
  }
  return res;
}
xAx xAx::operator+(double const &b) const {
  xAx res;
  for(int i = 0; i < 5; i++) {
    res.c[i] = c[i];
  }
  res.c[5] = c[5] + b;
  return res;
}

xAx xAx::operator*(double const &b) const {
  xAx res;
  for(int i = 0; i < 6; i++) {
    res.c[i] = c[i] * b;
  }
  return res;
}

  std::vector<Point> xAx::crossings(Rect r) const {
    std::vector<Point> res;
  for(int ei = 0; ei < 4; ei++) {
    Geom::LineSegment ls(r.corner(ei), r.corner(ei+1));
    D2<SBasis> lssb = ls.toSBasis();
    SBasis edge_curve = evaluate_at(lssb[0], lssb[1]);
    std::vector<double> rts = Geom::roots(edge_curve);
    for(unsigned eci = 0; eci < rts.size(); eci++) {
      res.push_back(lssb.valueAt(rts[eci]));
    }
  }
  return res;
}

  boost::optional<RatQuad> xAx::toCurve(Rect const & bnd) const {
  std::vector<Point> crs = crossings(bnd);
  if(crs.size() == 1) {
      Point A = crs[0];
      Point dA = rot90(gradient(A));
      if(L2sq(dA) <= 1e-10) { // perhaps a single point?
          return boost::optional<RatQuad> ();
      }
      LineSegment ls = intersection(Line::fromPointDirection(A, dA), bnd);
      return RatQuad::fromPointsTangents(A, dA, ls.pointAt(0.5), ls[1], dA);
  }
  else if(crs.size() >= 2 and crs.size() < 4) {
    Point A = crs[0];
    Point C = crs[1];
    if(crs.size() == 3) {
        if(distance(A, crs[2]) > distance(A, C))
            C = crs[2];
        else if(distance(C, crs[2]) > distance(A, C))
            A = crs[2];
    }
    Line bisector = make_bisector_line(LineSegment(A, C));
    std::vector<double> bisect_rts = this->roots(bisector);
    if(bisect_rts.size() > 0) {
      int besti = -1;
      for(unsigned i =0; i < bisect_rts.size(); i++) {
	Point p = bisector.pointAt(bisect_rts[i]);
	if(bnd.contains(p)) {
	  besti = i;
	}
      }
      if(besti >= 0) {
	Point B = bisector.pointAt(bisect_rts[besti]);

        Point dA = gradient(A);
        Point dC = gradient(C);
        if(L2sq(dA) <= 1e-10 or L2sq(dC) <= 1e-10) {
            return RatQuad::fromPointsTangents(A, C-A, B, C, A-C);
        }

	RatQuad rq = RatQuad::fromPointsTangents(A, rot90(dA),
						 B, C, rot90(dC));
	return rq;
	//std::vector<SBasis> hrq = rq.homogenous();
	/*SBasis vertex_poly = evaluate_at(hrq[0], hrq[1], hrq[2]);
	  std::vector<double> rts = roots(vertex_poly);
	  for(unsigned i = 0; i < rts.size(); i++) {
	  //draw_circ(cr, Point(rq.pointAt(rts[i])));
	  }*/
      }
    }
  }
  return boost::optional<RatQuad>();
}

  std::vector<double> xAx::roots(Point d, Point o) const {
  // Find the roots on line l
  // form the quadratic Q(t) = 0 by composing l with xAx
  double q2 = c[0]*d[0]*d[0] + c[1]*d[0]*d[1] + c[2]*d[1]*d[1];
  double q1 = (2*c[0]*d[0]*o[0] +
	       c[1]*(d[0]*o[1]+d[1]*o[0]) +
	       2*c[2]*d[1]*o[1] +
	       c[3]*d[0] + c[4]*d[1]);
  double q0 = c[0]*o[0]*o[0] + c[1]*o[0]*o[1] + c[2]*o[1]*o[1] + c[3]*o[0] + c[4]*o[1] + c[5];
  std::vector<double> r;
  if(q2 == 0) {
    if(q1 == 0) {
      return r;
    }
    r.push_back(-q0/q1);
  } else {
    double desc = q1*q1 - 4*q2*q0;
    /*std::cout << q2 << ", "
      << q1 << ", "
      << q0 << "; "
      << desc << "\n";*/
    if (desc < 0)
      return r;
    else if (desc == 0)
      r.push_back(-q1/(2*q2));
    else {
      desc = std::sqrt(desc);
      double t = -0.5*(q1+sgn(q1)*desc);
      r.push_back(t/q2);
      r.push_back(q0/t);
    }
  }
  return r;
}

std::vector<double> xAx::roots(Line const &l) const {
  return roots(l.versor(), l.origin());
}

Interval xAx::quad_ex(double a, double b, double c, Interval ivl) {
  double cx = -b*0.5/a;
  Interval bnds((a*ivl[0]+b)*ivl[0]+c, (a*ivl[1]+b)*ivl[1]+c);
  if(ivl.contains(cx))
    bnds.extendTo((a*cx+b)*cx+c);
  return bnds;
}

Geom::Matrix xAx::hessian() const {
  return Matrix(2*c[0], c[1],
		c[1], 2*c[2],
		0, 0);
}


boost::optional<Point> solve(double A[2][2], double b[2]) {
    double const determ = det(A);
    if (determ !=  0.0) { // hopeful, I know
        Geom::Coord const ideterm = 1.0 / determ;

        return Point ((A[1][1]*b[0]  -A[0][1]*b[1]),
                      (-A[1][0]*b[0] +  A[0][0]*b[1]))* ideterm;
    } else {
        return boost::optional<Point>();
    }
}

boost::optional<Point> xAx::bottom() const {
    double A[2][2] = {{2*c[0], c[1]},
                      {c[1], 2*c[2]}};
    double b[2] = {-c[3], -c[4]};
    return solve(A, b);
    //return Point(-c[3], -c[4])*hessian().inverse();
}

Interval xAx::extrema(Rect r) const {
  if (c[0] == 0 and c[1] == 0 and c[2] == 0) {
    Interval ext(valueAt(r.corner(0)));
    for(int i = 1; i < 4; i++)
      ext |= Interval(valueAt(r.corner(i)));
    return ext;
  }
  double k = r[0][0];
  Interval ext = quad_ex(c[2], c[1]*k+c[4],  (c[0]*k + c[3])*k + c[5], r[1]);
  k = r[0][1];
  ext |= quad_ex(c[2], c[1]*k+c[4],  (c[0]*k + c[3])*k + c[5], r[1]);
  k = r[1][0];
  ext |= quad_ex(c[0], c[1]*k+c[3],  (c[2]*k + c[4])*k + c[5], r[0]);
  k = r[1][1];
  ext |= quad_ex(c[0], c[1]*k+c[3],  (c[2]*k + c[4])*k + c[5], r[0]);
  boost::optional<Point> B0 = bottom();
  if (B0 and r.contains(*B0))
    ext.extendTo(0);
  return ext;
}

bool xAx::isDegenerate() const {
    assert(0);
    return false; // XXX:
}








/*
 *  helper functions
 */

bool at_infinity (Point const& p)
{
    if (p[X] == infinity() || p[X] == -infinity()
        || p[Y] == infinity() || p[Y] == -infinity())
    {
        return true;
    }
    return false;
}




/*
 *  Define a conic section by computing the one that fits better with
 *  N points.
 *
 *  points: points to fit
 *
 *  precondition: there must be at least 5 non-overlapping points
 */
void xAx::set(std::vector<Point> const& points)
{
    size_t sz = points.size();
    if (sz < 5)
    {
        THROW_RANGEERROR("fitting error: too few points passed");
    }
    NL::LFMConicSection model;
    NL::least_squeares_fitter<NL::LFMConicSection> fitter(model, sz);

    for (size_t i = 0; i < sz; ++i)
    {
        fitter.append(points[i]);
    }
    fitter.update();

    NL::Vector z(sz, 0.0);
    model.instance(*this, fitter.result(z));
}

/*
 *  Define a section conic by providing the coordinates of one of its vertex,
 *  the major axis inclination angle and the coordinates of its foci
 *  with respect to the unidimensional system defined by the major axis with
 *  origin set at the provided vertex.
 *
 *  _vertex :   section conic vertex V
 *  _angle :    section conic major axis angle
 *  _dist1:     +/-distance btw V and nearest focus
 *  _dist2:     +/-distance btw V and farest focus
 *
 *  prerequisite: _dist1 <= _dist2
 */
void xAx::set (const Point& _vertex, double _angle, double _dist1, double _dist2)
{
    if (_dist2 == infinity() || _dist2 == -infinity())  // parabola
    {
        if (_dist1 == infinity()) // degenerate to a line
        {
            Line l(_vertex, _angle);
            std::vector<double> lcoeff = l.implicit_form_coefficients();
            coeff(3) = lcoeff[0];
            coeff(4) = lcoeff[1];
            coeff(5) = lcoeff[2];
            return;
        }

        // y^2 - 4px == 0
        double cD = -4 * _dist1;

        double cosa = std::cos (_angle);
        double sina = std::sin (_angle);
        double cca = cosa * cosa;
        double ssa = sina * sina;
        double csa = cosa * sina;

        coeff(0) = ssa;
        coeff(1) = -2 * csa;
        coeff(2) = cca;
        coeff(3) = cD * cosa;
        coeff(4) = cD * sina;

        double VxVx = _vertex[X] * _vertex[X];
        double VxVy = _vertex[X] * _vertex[Y];
        double VyVy = _vertex[Y] * _vertex[Y];

        coeff(5) = coeff(0) * VxVx + coeff(1) * VxVy + coeff(2) * VyVy
               - coeff(3) * _vertex[X] - coeff(4) * _vertex[Y];
        coeff(3) -= (2 * coeff(0) * _vertex[X] + coeff(1) * _vertex[Y]);
        coeff(4) -= (2 * coeff(2) * _vertex[Y] + coeff(1) * _vertex[X]);

        return;
    }

    if (std::fabs(_dist1) > std::fabs(_dist2))
    {
        std::swap (_dist1, _dist2);
    }
    if (_dist1 < 0)
    {
        _angle -= M_PI;
        _dist1 = -_dist1;
        _dist2 = -_dist2;
    }

    // ellipse and hyperbola
    double lin_ecc = (_dist2 - _dist1) / 2;
    double rx = (_dist2 + _dist1) / 2;

    double cA = rx * rx - lin_ecc * lin_ecc;
    double cC = rx * rx;
    double cF = - cA * cC;
//    std::cout << "cA: " << cA << std::endl;
//    std::cout << "cC: " << cC << std::endl;
//    std::cout << "cF: " << cF << std::endl;

    double cosa = std::cos (_angle);
    double sina = std::sin (_angle);
    double cca = cosa * cosa;
    double ssa = sina * sina;
    double csa = cosa * sina;

    coeff(0) = cca * cA + ssa * cC;
    coeff(2) = ssa * cA + cca * cC;
    coeff(1) = 2 * csa * (cA - cC);

    Point C (rx * cosa + _vertex[X], rx * sina + _vertex[Y]);
    double CxCx = C[X] * C[X];
    double CxCy = C[X] * C[Y];
    double CyCy = C[Y] * C[Y];

    coeff(3) = -2 * coeff(0) * C[X] - coeff(1) * C[Y];
    coeff(4) = -2 * coeff(2) * C[Y] - coeff(1) * C[X];
    coeff(5) = cF + coeff(0) * CxCx + coeff(1) * CxCy + coeff(2) * CyCy;
}

/*
 *  Define a conic section by providing one of its vertex and its foci.
 *
 *  _vertex: section conic vertex
 *  _focus1: section conic focus
 *  _focus2: section conic focus
 */
void xAx::set (const Point& _vertex, const Point& _focus1, const Point& _focus2)
{
    if (at_infinity(_vertex))
    {
        THROW_RANGEERROR("case not handled: vertex at infinity");
    }
    if (at_infinity(_focus2))
    {
        if (at_infinity(_focus1))
        {
            THROW_RANGEERROR("case not handled: both focus at infinity");
        }
        Point VF = _focus1 - _vertex;
        double dist1 = L2(VF);
        double angle = atan2(VF);
        set(_vertex, angle, dist1, infinity());
        return;
    }
    else if (at_infinity(_focus1))
    {
        Point VF = _focus2 - _vertex;
        double dist1 = L2(VF);
        double angle = atan2(VF);
        set(_vertex, angle, dist1, infinity());
        return;
    }
    assert (are_collinear (_vertex, _focus1, _focus2));
    if (!are_near(_vertex, _focus1))
    {
        Point VF = _focus1 - _vertex;
        Line axis(_vertex, _focus1);
        double angle = atan2(VF);
        double dist1 = L2(VF);
        double dist2 = distance (_vertex, _focus2);
        double t = axis.timeAt(_focus2);
        if (t < 0)  dist2 = -dist2;
//        std::cout << "t = " << t << std::endl;
//        std::cout << "dist2 = " << dist2 << std::endl;
        set (_vertex, angle, dist1, dist2);
    }
    else if (!are_near(_vertex, _focus2))
    {
        Point VF = _focus2 - _vertex;
        double angle = atan2(VF);
        double dist1 = 0;
        double dist2 = L2(VF);
        set (_vertex, angle, dist1, dist2);
    }
    else
    {
        coeff(0) = coeff(2) = 1;
        coeff(1) = coeff(3) = coeff(4) = coeff(5) = 0;
    }
}


/*
 *  Compute the solutions of the conic section algebraic equation with respect to
 *  one coordinate after substituting to the other coordinate the passed value
 *
 *  sol: the computed solutions
 *  v:   the provided value
 *  d:   the index of the coordinate the passed value have to be substituted to
 */
void xAx::roots (std::vector<double>& sol, Coord v, Dim2 d) const
{
    sol.clear();
    if (d < 0 || d > Y)
    {
        THROW_RANGEERROR("dimension parameter out of range");
    }

    // p*t^2 + q*t + r = 0;
    double p, q, r;

    if (d == X)
    {
        p = coeff(2);
        q = coeff(4) + coeff(1) * v;
        r = coeff(5) + (coeff(0) * v + coeff(3)) * v;
    }
    else
    {
        p = coeff(0);
        q = coeff(3) + coeff(1) * v;
        r = coeff(5) + (coeff(2) * v + coeff(4)) * v;
    }

    if (are_near(p, 0, 1e-8))
    {
        if (are_near(q, 0, 1e-8))  return;
        double t = -r/q;
        sol.push_back(t);
        return;
    }
    double delta = q*q - 4*p*r;
    //std::cout << "delta = " << delta << std::endl;
    if (delta < 0)  return;
    double p2 = 2 * p;
    if (are_near(delta, 0, 1e-8))
    {
        double t = -q / p2;
        sol.push_back(t);
        return;
    }
    // else
    double srd = std::sqrt(delta);
    double t1 = -(q + srd) / p2;
    double t2 = (-q + srd) / p2;
    sol.push_back(t1);
    sol.push_back(t2);
}

} // end namespace Geom




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
