/*
 * Elliptical Arc Fitting Toy
 *
 * Authors:
 * 		Marco Cecchetti <mrcekets at gmail.com>
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

#include "numeric/linear_system.h"

#include "d2.h"
#include "sbasis.h"
#include "path.h"
#include "bezier-to-sbasis.h"

#include "path-cairo.h"
#include "toy-framework.h"


using namespace Geom;


struct ellipse_equation
{
	ellipse_equation(double a, double b, double c, double d, double e, double f)
		: A(a), B(b), C(c), D(d), E(e), F(f)
	{
	}
	
	double operator()(double x, double y)
	{
		return A* x * x + B * x * y + C * y * y + D * x + E * y + F;
	}
	
	double operator()(Point const& p)
	{
		return (*this)(p[X], p[Y]);
	}

	double A, B, C, D, E, F;
};

bool sbasis_to_svg_elliptical_arc( D2<SBasis> const& curve, 
		                           SVGEllipticalArc& ea, 
		                           unsigned int N, 
		                           double tolerance  )
{
	// TODO: is curve degenerate ?
	
	// calculating best coefficients with the least squares method
	double partitions = N - 1;
	Point p[N];
	double x1, x2, x3;
	double y1, y2, y3, y4;
	double x1y1, x2y1, x1y2, x2y2, x3y1, x1y3;
	
	NL::Matrix m(N, N, 0);
	NL::Vector col(N, 0.0);
	
	for ( unsigned int k = 0; k < N; ++k )
	{
		p[k] = curve( k / partitions );
		x1 = p[k][X];  x2 = x1 * x1;  x3 = x2 * x1;
		y1 = p[k][Y];  y2 = y1 * y1;  y3 = y2 * y1; y4 = y3 * y1;
		x1y1 = x1 * y1; 
		x2y1 = x2 * y1; x1y2 = x1 * y2; x2y2 = x2*y2;
		x3y1 = x3 * y1; x1y3 = x1 * y3;
		
		m(0,0) += y4;
		m(0,1) += x1y3;
		m(0,2) += x1y2;
		m(0,3) += y3;
		m(0,4) += y2;
		
		m(1,0) += x1y3; 
		m(1,1) += x2y2; 
		m(1,2) += x2y1;	
		m(1,3) += x1y2;	
		m(1,4) += x1y1;
		
		m(2,0) += x1y2;
		m(2,1) += x2y1;
		m(2,2) += x2;
		m(2,3) += x1y1;
		m(2,4) += x1;
		
		m(3,0) += y3;
		m(3,1) += x1y2;
		m(3,2) += x1y1;
		m(3,3) += y2;
		m(3,4) += y1;
		
		m(4,0) += y2;
		m(4,1) += x1y1;
		m(4,2) += x1;
		m(4,3) += y1;
		m(4,4) += 1;
		
		col[0] -= x2y2;
		col[1] -= x3y1;
		col[2] -= x3;
		col[3] -= x2y1;
		col[4] -= x2;
	}
	
	NL::LinearSystem ls(m, col);
	
	NL::Vector x = ls.SV_solve();
	
	const double & C = x[0];
	const double & B = x[1];
	const double & D = x[2];
	const double & E = x[3];
	const double & F = x[4];
	
	
	// check error magnitude
	ellipse_equation eval_error(1, B, C, D, E, F);
	
	double e1x = (2 + B) * tolerance;
	double e1y = (B + 2*C) * tolerance;
	double e2 = ((D + E)  + (1 + B + C) * tolerance) * tolerance;
//	std::cerr << "e1x = " << e1x << std::endl;
//	std::cerr << "e1y = " << e1y << std::endl;
//	std::cerr << "e2 = " << e2 << std::endl;
	double err, bound;
	for ( unsigned int k = 0; k < N; ++k )
	{
		err = std::fabs(eval_error(p[k]));
		bound = std::fabs( e1x * p[k][X] + e1y * p[k][Y] + e2 );
		if ( err > bound )
		{
			
			std::cerr 
				<< "tolerance error" << std::endl
				<< "at point: " << k << std::endl
				<< "error value: "<< err << std::endl
			    << "bound: " << bound << std::endl;	
			return false;
		}
	}

	// evaluate ellipse centre
	Point centre;
	double a, b, c;
	
	double den = 4*C - B*B;
	assert ( den != 0 );
	centre[Y] = (B*D - 2*E) / den;
	centre[X] = (B*E - 2*C*D) / den;
	den = centre[X] * centre[X] + B * centre[X] * centre[Y] + C * centre[Y] * centre[Y] - F;
	assert(den != 0);
	a = 1 / den;
	b = a * B;
	c = a * C;
	
//	std::cerr << "a = " << a << std::endl;
//	std::cerr << "B = " << B << std::endl;
//	std::cerr << "C = " << C << std::endl;
//	std::cerr << "D = " << D << std::endl;
//	std::cerr << "E = " << E << std::endl;
//	std::cerr << "F = " << F << std::endl;
	
	//evaluate ellipse rotation angle
	double rot = std::atan2( -B, -(1 - C) )/2;
//	std::cerr << "rot = " << rot << std::endl;
	bool swap_axes = false;
	if ( are_near(rot, 0) ) rot = 0;
	if ( are_near(rot, M_PI/2)  || rot < 0 )
	{
		swap_axes = true;
	}
	
	
	
	// evaluate ellipse ray lengths
	double cosrot = std::cos(rot);
	double sinrot = std::sin(rot);
	double cos2 = cosrot * cosrot;
	double sin2 = sinrot * sinrot;
	double cossin = cosrot * sinrot;
	
	den = a * cos2 + b * cossin + c * sin2;
	//assert(den > 0);
	if ( den <= 0 )
	{
		std::cerr 
			<< "!(den > 0) error" << std::endl
			<< "evaluating rx" << std::endl;
		return false;

	}
	double rx = std::sqrt( 1/den );
	
	den = c * cos2 - b * cossin + a * sin2;
	//assert(den > 0);
	if ( den <= 0 )
	{
		std::cerr 
			<< "!(den > 0) error" << std::endl
			<< "evaluating ry" << std::endl;
		return false;

	}
	double ry = std::sqrt(1/den);
	
	// we choose always the ellipse with a rotation angle between 0 and PI/2 
	if ( swap_axes ) std::swap(rx, ry);
	if( are_near(rot,  M_PI/2) || are_near(rot, -M_PI/2) || are_near(rx, ry) )
	{
		rot = 0;
	}
	else if ( rot < 0 )
	{
		rot += M_PI/2;
	}
	
//	std::cerr << "swap axes: " << swap_axes << std::endl;
//	std::cerr << "rx = " << rx << " ry = " << ry << std::endl;
//	std::cerr << "rot = " << rad_to_deg(rot) << std::endl;
//	std::cerr << "centre: " << centre << std::endl;
	
	// find out how we should set the large_arc_flag and sweep_flag
	bool large_arc_flag = true;
	bool sweep_flag = true;
	
	Point initial_point = curve.at0();
	Point final_point = curve.at1();
	Point inner_point = curve(0.5);
	
	double angle1 = angle_between(initial_point - centre, final_point - centre);
	double angle2 = angle_between(initial_point - centre, inner_point - centre);
	double angle3 = angle_between(inner_point - centre, final_point - centre);
	
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
	
	// finally creating the elliptical arc !
	ea = SVGEllipticalArc(initial_point, rx, ry, rot, large_arc_flag, sweep_flag, final_point);
	
	return true;
}



class EAFittingToy : public Toy
{
  private:
    void draw( cairo_t *cr,	std::ostringstream *notify, 
  	      	   int width, int height, bool save ) 
    {
    	cairo_set_line_width (cr, 0.2);
    	D2<SBasis> SB = handles_to_sbasis(handles.begin(), total_handles - 1);
    	cairo_md_sb(cr, SB);
    	cairo_stroke(cr);
    	
    	cairo_set_line_width (cr, 0.4);
    	cairo_set_source_rgba(cr, 0.0, 0.0, 0.7, 1.0);
    	try
    	{
    		SVGEllipticalArc EA;
    		bool exit_status = sbasis_to_svg_elliptical_arc(SB, EA, 10, tolerance);
    		if ( !exit_status ) 
    		{
    			Toy::draw(cr, notify, width, height, save);
    			return;
    		}
    		D2<SBasis> easb = EA.toSBasis();
        	cairo_md_sb(cr, easb);
        	cairo_stroke(cr);
    	}
        catch( RangeError e )
        {
        	std::cerr << e.what() << std::endl;
        	Toy::draw(cr, notify, width, height, save);
        	return;
        } 	

    	Toy::draw(cr, notify, width, height, save);
    }

  public:
	EAFittingToy( double _tolerance )
		: tolerance(_tolerance)
	{
		total_handles = 6;
		for ( unsigned int i = 0; i < total_handles; ++i )
		{
			handles.push_back(Point(uniform()*400, uniform()*400));
		}
	}
	
	unsigned int total_handles;
	double tolerance;
};



int main(int argc, char **argv) 
{	
	double tolerance = 8;
	if(argc > 1)
	        sscanf(argv[1], "%lg", &tolerance);
    init( argc, argv, new EAFittingToy(tolerance) );
    return 0;
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
