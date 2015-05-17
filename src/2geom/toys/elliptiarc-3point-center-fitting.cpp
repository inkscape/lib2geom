/*
 * make up an elliptical arc knowing 3 points lying on the arc
 * and the ellipse centre
 *
 * Authors:
 *      Marco Cecchetti <mrcekets at gmail.com>
 *
 * Copyright 2008  authors
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

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <2geom/elliptical-arc.h>
#include <2geom/numeric/linear_system.h>

namespace Geom
{

bool make_elliptical_arc( EllipticalArc & ea,
						  Point const& centre,
		                  Point const& initial,
		                  Point const& final,
		                  Point const& inner )
{

	Point p[3] = { initial, inner, final };
	double x1, x2, x3, x4;
	double y1, y2, y3, y4;
	double x1y1, x2y2, x3y1, x1y3;
	NL::Matrix m(3,3);
	NL::Vector v(3);
	NL::LinearSystem ls(m, v);

	m.set_all(0);
	v.set_all(0);
	for (unsigned int k = 0; k < 3; ++k)
	{
		// init_x_y
		x1 = p[k][X] - centre[X]; x2 = x1 * x1; x3 = x2 * x1; x4 = x3 * x1;
		y1 = p[k][Y] - centre[Y]; y2 = y1 * y1; y3 = y2 * y1; y4 = y3 * y1;
		x1y1 = x1 * y1;
		x2y2 = x2 * y2;
		x3y1 = x3 * y1; x1y3 = x1 * y3;

		// init linear system
		m(0,0) += x4;
		m(0,1) += x3y1;
		m(0,2) += x2y2;

		m(1,0) += x3y1;
		m(1,1) += x2y2;
		m(1,2) += x1y3;

		m(2,0) += x2y2;
		m(2,1) += x1y3;
		m(2,2) += y4;

		v[0] += x2;
		v[1] += x1y1;
		v[2] += y2;
	}

	ls.SV_solve();

	double A = ls.solution()[0];
	double B = ls.solution()[1];
	double C = ls.solution()[2];


	//evaluate ellipse rotation angle
	double rot = std::atan2( -B, -(A - C) )/2;
	std::cerr << "rot = " << rot << std::endl;
	bool swap_axes = false;
	if ( are_near(rot, 0) ) rot = 0;
	if ( are_near(rot, M_PI/2)  || rot < 0 )
	{
		swap_axes = true;
	}

	// evaluate the length of the ellipse rays
	double cosrot = std::cos(rot);
	double sinrot = std::sin(rot);
	double cos2 = cosrot * cosrot;
	double sin2 = sinrot * sinrot;
	double cossin = cosrot * sinrot;

	double den = A * cos2 + B * cossin + C * sin2;
	if ( den <= 0 )
	{
		std::cerr << "!(den > 0) error" << std::endl;
		std::cerr << "evaluating rx" << std::endl;
		return false;
	}
	double rx = std::sqrt(1/den);

	den = C * cos2 - B * cossin + A * sin2;
	if ( den <= 0 )
	{
		std::cerr << "!(den > 0) error" << std::endl;
		std::cerr << "evaluating ry" << std::endl;
		return false;
	}
	double ry = std::sqrt(1/den);


	// the solution is not unique so we choose always the ellipse
	// with a rotation angle between 0 and PI/2
	if ( swap_axes ) std::swap(rx, ry);
	if (    are_near(rot,  M_PI/2)
		 || are_near(rot, -M_PI/2)
		 || are_near(rx, ry)       )
	{
		rot = 0;
	}
	else if ( rot < 0 )
	{
		rot += M_PI/2;
	}

	std::cerr << "swap axes: " << swap_axes << std::endl;
	std::cerr << "rx = " << rx << " ry = " << ry << std::endl;
	std::cerr << "rot = " << rad_to_deg(rot) << std::endl;
	std::cerr << "centre: " << centre << std::endl;


	// find out how we should set the large_arc_flag and sweep_flag
	bool large_arc_flag = true;
	bool sweep_flag = true;

	Point sp_cp = initial - centre;
	Point ep_cp = final - centre;
	Point ip_cp = inner - centre;

	double angle1 = angle_between(sp_cp, ep_cp);
	double angle2 = angle_between(sp_cp, ip_cp);
	double angle3 = angle_between(ip_cp, ep_cp);

	if ( angle1 > 0 )
	{
		if ( angle2 > 0 && angle3 > 0 )
		{
			large_arc_flag = false;
			sweep_flag = true;
		}
		else
		{
			large_arc_flag = true;
			sweep_flag = false;
		}
	}
	else
	{
		if ( angle2 < 0 && angle3 < 0 )
		{
			large_arc_flag = false;
			sweep_flag = false;
		}
		else
		{
			large_arc_flag = true;
			sweep_flag = true;
		}
	}

	// finally we're going to create the elliptical arc!
	try
	{
		ea.set( initial, rx, ry, rot,
				large_arc_flag, sweep_flag, final );
	}
    catch( RangeError e )
    {
    	std::cerr << e.what() << std::endl;
    	return false;
    }

	return true;
}


}



using namespace Geom;

class ElliptiArcMaker : public Toy
{
  private:
	void draw( cairo_t *cr, std::ostringstream *notify,
  		       int width, int height, bool save, std::ostringstream *timer_stream)
	{
		cairo_set_line_width (cr, 0.3);
		cairo_set_source_rgb(cr, 0,0,0.3);
		draw_text(cr, O.pos, "centre");
		draw_text(cr, A.pos, "initial");
		draw_text(cr, B.pos, "final");
		draw_text(cr, C.pos, "inner");
		cairo_stroke(cr);
		cairo_set_source_rgb(cr, 0.7,0,0);
		bool status
			= make_elliptical_arc(ea, O.pos, A.pos, B.pos, C.pos);
		if (status)
		{
			D2<Geom::SBasis> easb = ea.toSBasis();
			cairo_d2_sb(cr, easb);
		}
		cairo_stroke(cr);
		Toy::draw(cr, notify, width, height, save,timer_stream);
	}

  public:
	ElliptiArcMaker()
	    : O(443, 441),
	      A(516, 275),
	      B(222, 455),
	      C(190, 234)
	{
		handles.push_back(&O);
		handles.push_back(&A);
		handles.push_back(&B);
		handles.push_back(&C);
	}

  private:
    PointHandle O, A, B, C;
	EllipticalArc ea;
};








int main(int argc, char **argv)
{
    init( argc, argv, new ElliptiArcMaker() );
    return 0;
}

