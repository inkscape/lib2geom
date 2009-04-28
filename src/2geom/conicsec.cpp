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


#include <2geom/line.h>
#include <2geom/conicsec.h>
#include <string>

// File: convert.h
#include <sstream>
#include <stdexcept>
 

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

static double det(double a, double b, double c, double d) {
  return a*d - b*c;
}

static double det3(double M[3][3]) {
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
      LineSegment ls = intersection(Line::fromPointDirection(A, dA), bnd);
      if(L2sq(dA) <= 1e-10) { // perhaps a single point?
          return boost::optional<RatQuad> ();
      }
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
    
Point xAx::bottom() const {
  return Point(-c[3], -c[4])*hessian().inverse();
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
  if (r.contains(bottom()))
    ext.extendTo(0);
  return ext;
}

bool xAx::isDegenerate() const {
    assert(0);
    return false; // XXX:
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
