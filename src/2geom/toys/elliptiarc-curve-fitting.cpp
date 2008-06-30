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

#include <gsl/gsl_linalg.h>

#include "d2.h"
#include "sbasis.h"
#include "path.h"
#include "bezier-to-sbasis.h"
#include "sbasis-geometric.h"
#include "numeric/linear_system.h"

#include "numeric/fitting-tool.h"
#include "numeric/fitting-model.h"
#include "ellipse.h"

#include "path-cairo.h"
#include "toy-framework-2.h"



using namespace Geom;


struct ellipse_equation
{
	ellipse_equation(double a, double b, double c, double d, double e, double f)
		: A(a), B(b), C(c), D(d), E(e), F(f)
	{
	}

	double operator()(double x, double y) const
	{
		return A * x * x + B * x * y + C * y * y + D * x + E * y + F;
	}

	double operator()(Point const& p) const
	{
		return (*this)(p[X], p[Y]);
	}

	Point normal(double x, double y) const
	{
		Point n( 2 * A * x + B * y + D, 2 * C * y + B * x + E );
		return unit_vector(n);
	}

	Point normal(Point const& p) const
	{
		return normal(p[X], p[Y]);
	}

	double A, B, C, D, E, F;
};


class elliptiarc_converter
{
  public:
    typedef D2<SBasis> curve_type;

    elliptiarc_converter( SVGEllipticalArc& _ea,
                          curve_type const& _curve,
                          unsigned int _N,
                          double _tolerance )
        : ea(_ea), curve(_curve),
          dcurve( unitVector(derivative(curve)) ),
          model(), fitter(model, _N),
          tolerance(_tolerance), tol_at_extr(tolerance/2),
          tol_at_center(0.1), angle_tol(0.1),
          initial_point(curve.at0()), final_point(curve.at1()),
          N(_N), last(N-1), partitions(N-1), p(N)
    {
    }

  private:
    bool bound_exceeded( unsigned int k, ellipse_equation const & ee,
                         double e1x, double e1y, double e2 )
    {
        dist_err = std::fabs( ee(p[k]) );
        dist_bound = std::fabs( e1x * p[k][X] + e1y * p[k][Y] + e2 );
        angle_err = std::fabs( dot( dcurve(k/partitions), ee.normal(p[k]) ) );
        //angle_err *= angle_err;
        return ( dist_err  > dist_bound || angle_err > angle_tol );
    }

    bool check_bound(double A, double B, double C, double D, double E, double F)
    {
        // check error magnitude
        ellipse_equation ee(A, B, C, D, E, F);

        double e1x = (2*A + B) * tol_at_extr;
        double e1y = (B + 2*C) * tol_at_extr;
        double e2 = ((D + E)  + (A + B + C) * tol_at_extr) * tol_at_extr;
        if ( bound_exceeded(0, ee, e1x, e1y, e2) )
        {
            print_bound_error(0);
            return false;
        }
        if ( bound_exceeded(0, ee, e1x, e1y, e2) )
        {
            print_bound_error(last);
            return false;
        }

        e1x = (2*A + B) * tolerance;
        e1y = (B + 2*C) * tolerance;
        e2 = ((D + E)  + (A + B + C) * tolerance) * tolerance;
    //  std::cerr << "e1x = " << e1x << std::endl;
    //  std::cerr << "e1y = " << e1y << std::endl;
    //  std::cerr << "e2 = " << e2 << std::endl;

        for ( unsigned int k = 1; k < last; ++k )
        {
            if ( bound_exceeded(k, ee, e1x, e1y, e2) )
            {
                print_bound_error(k);
                return false;
            }
        }

        return true;
    }

    void fit()
    {
        for (unsigned int k = 0; k < N; ++k)
        {
            p[k] = curve( k / partitions );
            fitter.append(p[k]);
        }
        fitter.update();

        NL::Vector z(N, 0.0);
        fitter.result(z);
    }

    bool make_elliptiarc()
    {
        const NL::Vector & coeff = fitter.result();
        Ellipse e;
        try
        {
            e.set(1, coeff[0], coeff[1], coeff[2], coeff[3], coeff[4]);
        }
        catch(LogicalError exc)
        {
            return false;
        }
        Point inner_point = curve(0.5);
//        try
//        {
//            ea = e.arc(initial_point, inner_point, final_point);
//        }
//        catch(RangeError exc)
//        {
//            return false;
//        }

        ea = e.arc(initial_point, inner_point, final_point);

        if ( !are_near( e.center(),
                        ea.center(),
                        tol_at_center * std::min(e.ray(X),e.ray(Y))
                      )
           )
        {
            return false;
        }
        return true;
    }

    void print_bound_error(unsigned int k)
    {
        std::cerr
            << "tolerance error" << std::endl
            << "at point: " << k << std::endl
            << "error value: "<< dist_err << std::endl
            << "bound: " << dist_bound << std::endl
            << "angle error: " << angle_err
            << " (" << angle_tol << ")" << std::endl;
    }

  public:
    bool operator()()
    {
        const NL::Vector & coeff = fitter.result();
        fit();
        if ( !check_bound(1, coeff[0], coeff[1], coeff[2], coeff[3], coeff[4]) )
            return false;
        if ( !(make_elliptiarc()) ) return false;
        return true;
    }

  private:
      SVGEllipticalArc& ea;
      const curve_type & curve;
      Piecewise<D2<SBasis> > dcurve;
      NL::LFMEllipse model;
      NL::least_squeares_fitter<NL::LFMEllipse> fitter;
      double tolerance, tol_at_extr, tol_at_center, angle_tol;
      Point initial_point, final_point;
      unsigned int N;
      unsigned int last; // N-1
      double partitions; // N-1
      std::vector<Point> p; // sample points
      double dist_err, dist_bound, angle_err;
};




class EAFittingToy : public Toy
{
  private:
    void draw( cairo_t *cr,	std::ostringstream *notify,
  	      	   int width, int height, bool save )
    {
    	cairo_set_line_width (cr, 0.2);
    	//D2<SBasis> SB = handles_to_sbasis(handles.begin(), total_handles - 1);
    	D2<SBasis> SB = psh.asBezier();
    	cairo_md_sb(cr, SB);
    	cairo_stroke(cr);

    	cairo_set_line_width (cr, 0.4);
    	cairo_set_source_rgba(cr, 0.0, 0.0, 0.7, 1.0);
    	try
    	{
    		SVGEllipticalArc EA;
    		elliptiarc_converter convert(EA, SB, 10, tolerance);
    		if ( !convert() )
    		{
//    			*notify << "distance error: " << convert.get_error()
//    			        << " ( " << convert.get_bound() << " )" << std::endl
//    			        << "angle error: " << convert.get_angle_error()
//    			        << " ( " << convert.get_angle_tolerance() << " )";
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
	    handles.push_back(&psh);
		total_handles = 6;
		for ( unsigned int i = 0; i < total_handles; ++i )
		{
			psh.push_back(uniform()*400, uniform()*400);
		}
	}

	PointSetHandle psh;
	unsigned int total_handles;
	double tolerance;
};



int main(int argc, char **argv)
{
	double tolerance = 8;
	if(argc > 1)
	        sscanf(argv[1], "%lf", &tolerance);
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
