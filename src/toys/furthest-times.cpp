/*
 * Furthest Points Toy
 *
 * Authors:
 * 		Nathan Hurst    <njh at njhurst.com>
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

#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/path.h>
#include <2geom/bezier-to-sbasis.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework.h>


using namespace Geom;


class np_finder
{
public:
	np_finder(cairo_t* _cr, D2<SBasis> const& _c1, D2<SBasis> const& _c2)
		: cr(_cr), c1(_c1), c2(_c2)
	{
		dc1 = derivative(c1);
		dc2 = derivative(c2);
		cd1 = dot(c1,dc1);
		cd2 = dot(c2,dc2);
		dsq = 10e30;
	}

	void operator() ()
	{
		furthest_times_impl(0.5, 0, 1);
		d = sqrt(dsq);
	}
	
	Point firstPoint() const
	{
		return p1;
	}
	
	Point secondPoint() const
	{
		return p2;
	}
	
	double firstValue() const
	{
		return t1;
	}
	
	double secondValue() const
	{
		return t2;
	}
	
	double distance() const
	{
		return d;
	}
	
private:
	void furthest_times_impl( double t, double from = 0, double to = 1 )
	{
		//std::cerr << "[" << from << "," << to << "] t: " << t << std::endl;
		
		double st = t, et = t;
		std::pair<double, double> npc = loc_furthest_times(t, from, to);
		//std::cerr << "(" << npc.first << "," << npc.second << ")" << std::endl;
		if ( npc.second != -1 && dsq > L2sq(c1(npc.first) - c2(npc.second)) )
		{
			t1 = npc.first;
			t2 = npc.second;
			p1 = c1(t1);
			p2 = c2(t2);
			dsq = L2sq(p1 - p2);
		}
		if (npc.first < t)
			st = npc.first;
		else
			et = npc.first;
		//std::cerr << "[" << st << "," <<  et << "]"  << std::endl;
		double d1 = std::fabs(st - from);
		double d2 = std::fabs(to - et);
		if ( d1 > EPSILON )
			furthest_times_impl(from + d1/2, from, st);
		if ( d2 > EPSILON )
			furthest_times_impl(et + d2/2, et, to);
	}
	
	std::pair<double, double> 
	loc_furthest_times( double t, double from = 0, double to = 1 )
	{
		unsigned int i = 0;
		std::pair<double, double> np(-1,-1);
		std::pair<double, double> npf(from, -1);
		std::pair<double, double> npt(to, -1);
		double ct = t;
		double pt = -1;
		double s;
		//cairo_set_source_rgba(cr, 1/(t+1), t*t, t, 1.0);
		//cairo_move_to(cr, c1(t));
		while( !are_near(ct, pt) )
		{
			++i;
			pt = ct;
			s = furthest_time(c1(ct), c2, dc2, cd2);
			//std::cerr << "s: " << s << std::endl;
	        //cairo_line_to(cr, c2(s));
			ct = furthest_time(c2(s), c1, dc1, cd1, from, to);
			//std::cerr << "t: " << t1 << std::endl;
			//cairo_line_to(cr, c1(ct));
			if ( ct < from ) return npf;
			if ( ct > to ) return npt;
		}
		//std::cerr << "\n \n";
		//std::cerr << "iterations: " << i << std::endl;
		cairo_stroke(cr);
		np.first = ct;
		np.second = s;
		return np;
	}
	
	double furthest_time( Point const& p, D2<SBasis> const&c, D2<SBasis> const& dc, SBasis const& cd, double from = 0, double to = 1 )
	{
		D2<SBasis> sbc = c - p;
		SBasis dd = cd - dotp(p, dc);
		std::vector<double> zeros = roots(dd);
		double closest = from;
		double distsq = L2sq(sbc(from));
		for ( unsigned int i = 0; i < zeros.size(); ++i )
		{
			if ( distsq > L2sq(sbc(zeros[i])) )
			{
				closest = zeros[i];
				distsq = L2sq(sbc(closest));
			}
		}
		if ( distsq > L2sq(sbc(to)) )
			closest = to;
		return closest;
	}
	
	SBasis dotp(Point const& p, D2<SBasis> const& c)
	{
		SBasis d;
		d.resize(c[X].size());
		for ( unsigned int i = 0; i < c[0].size(); ++i )
		{
			for( unsigned int j = 0; j < 2; ++j )
				d[i][j] = p[X] * c[X][i][j] + p[Y] * c[Y][i][j];                                         
		}
		return d;
	}

private:
	static const Coord EPSILON = 10e-3;
	cairo_t* cr;
	D2<SBasis> const& c1, c2;
	D2<SBasis> dc1, dc2;
	SBasis cd1, cd2;
	double t1, t2, d, dsq;
	Point p1, p2;
};




class FurthestPoints : public Toy
{
  private:
    void draw( cairo_t *cr,	std::ostringstream *notify, 
    		   int width, int height, bool save, std::ostringstream *timer_stream) 
    {
    	cairo_set_line_width (cr, 0.2);
        D2<SBasis> A = handles_to_sbasis(handles.begin(), A_bez_ord-1);
        cairo_d2_sb(cr, A);
        D2<SBasis> B = handles_to_sbasis(handles.begin()+A_bez_ord, B_bez_ord-1);
        cairo_d2_sb(cr, B);
        
        
        np_finder np(cr, A, B);
        np();
        cairo_move_to(cr, np.firstPoint());
        cairo_line_to(cr, np.secondPoint());
        cairo_stroke(cr);
        //std::cerr << "np: (" << np.firstValue() << "," << np.secondValue() << ")" << std::endl;
        
    	Toy::draw(cr, notify, width, height, save,timer_stream);
    }
	
  public:
	FurthestPoints(unsigned int _A_bez_ord, unsigned int _B_bez_ord)
		: A_bez_ord(_A_bez_ord), B_bez_ord(_B_bez_ord)
	{
		unsigned int total_handles = A_bez_ord + B_bez_ord;
		for ( unsigned int i = 0; i < total_handles; ++i )
			handles.push_back(Geom::Point(uniform()*400, uniform()*400));
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

    init( argc, argv, new FurthestPoints(A_bez_ord, B_bez_ord));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
