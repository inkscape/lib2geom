/*
 * point-curve nearest point routines testing 
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

#include "d2.h"
#include "sbasis.h"
#include "path.h"
#include "angle.h"
#include "bezier-to-sbasis.h"
#include "sbasis-geometric.h"
#include "piecewise.h"


#include "path-cairo.h"
#include "toy-framework.h"


#include <algorithm>

using namespace Geom;



class NearestPoints : public Toy
{
  private:
    void draw( cairo_t *cr,	std::ostringstream *notify, 
    		   int width, int height, bool save ) 
    {
    	cairo_set_line_width (cr, 0.3);
        D2<SBasis> A = handles_to_sbasis(handles.begin(), A_bez_ord-1);
        cairo_md_sb(cr, A);
        D2<SBasis> B = handles_to_sbasis(handles.begin()+A_bez_ord, B_bez_ord-1);
        cairo_md_sb(cr, B);
        LineSegment seg(*(handles.end() - 5), *(handles.end() - 4));
        cairo_move_to(cr, *(handles.end() - 5));
        cairo_curve(cr, seg);
        SVGEllipticalArc earc;
        bool earc_constraints_satisfied = true;
        try
        {
        	earc.set(*(handles.end() - 3), 200, 150, 0, true, true, *(handles.end() - 2));
        }
        catch( RangeError e)
        {
        	earc_constraints_satisfied = false;
        }
        if ( earc_constraints_satisfied )
        {
        	cairo_md_sb(cr, earc.toSBasis());
        	cairo_stroke(cr);
        }
        
        double t;
        t= Geom::nearest_point(handles.back(), A);
        cairo_move_to(cr, handles.back());
        cairo_line_to(cr, A(t));
        Piecewise< D2<SBasis> > pwB(B);
        t = Geom::nearest_point(handles.back(), pwB);
        cairo_move_to(cr, handles.back());
        cairo_line_to(cr, pwB(t));
        t = seg.nearestPoint(handles.back());
        cairo_move_to(cr, handles.back());
        cairo_line_to(cr, seg.pointAt(t));
        if ( earc_constraints_satisfied )
        {
        	std::vector<double> times;
        	try
        	{
        		times = earc.allNearestPoints( handles.back() );
        	}
        	catch( Geom::Exception e )
        	{
        		std::cerr << e.what() << std::endl;
        	}
	        for ( unsigned int i = 0; i < times.size(); ++i )
	        {
	        	cairo_move_to(cr, handles.back());
	        	cairo_line_to( cr, earc.pointAt(times[i]) );
	        }
        }
//        std::vector<double> times = Geom::all_nearest_points(handles.back(), pwB);
//        for ( unsigned int i = 0; i < times.size(); ++i )
//        {
//        	cairo_move_to(cr, handles.back());
//        	cairo_line_to(cr, pwB(times[i]));
//        }

        cairo_stroke(cr);
    	Toy::draw(cr, notify, width, height, save);
    }
	
  public:
	NearestPoints(unsigned int _A_bez_ord, unsigned int _B_bez_ord)
		: A_bez_ord(_A_bez_ord), B_bez_ord(_B_bez_ord)
	{
		unsigned int total_handles = A_bez_ord + B_bez_ord + 5;
		for ( unsigned int i = 0; i < total_handles; ++i )
			handles.push_back(Geom::Point(uniform()*400, uniform()*400));
        *(handles.end() - 3) = Point(150, 150);
        *(handles.end() - 2) = Point(152, 300);
	}
	
  private:
	unsigned int A_bez_ord;
	unsigned int B_bez_ord;
};




int main(int argc, char **argv) 
{	
	unsigned int A_bez_ord=8;
	unsigned int B_bez_ord=5;
    if(argc > 2)
        sscanf(argv[2], "%d", &B_bez_ord);
    if(argc > 1)
        sscanf(argv[1], "%d", &A_bez_ord);
    init( argc, argv, new NearestPoints(A_bez_ord, B_bez_ord));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
