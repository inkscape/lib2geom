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
#include "numeric/linear_system.h"

#include "path-cairo.h"
#include "toy-framework.h"


#include <algorithm>


using namespace Geom;




SBasis 
interpolate ( D2<SBasis> const& A, D2<SBasis> const& B, 
				std::vector<double> const& sample_distances,
				unsigned int interpolation_si, unsigned int interpolation_ei,
				double interpolation_st, double interval_st, double interval_et, 
				double step )
{
	SBasis piece(0);
	const unsigned int piece_degree = 3;
	const unsigned int samples_per_piece = interpolation_ei - interpolation_si;
	if (samples_per_piece == 2)
	{
//		std::cerr << "samples_per_piece == 1" << std::endl;
//		std::cerr 
//			<< "st: " << interval_st
//			<< "  et: " << interval_et << std::endl;		
		piece = SBasis(Linear(sample_distances[interpolation_si], sample_distances[interpolation_si+1]));
		return piece;
	}
	const unsigned int columns = std::min(samples_per_piece, piece_degree + 1);
	NL::Matrix m(samples_per_piece, columns);
	NL::Vector v(samples_per_piece);
	double t = interpolation_st;
	double t_to_j;
	for (unsigned int i = 0; i < samples_per_piece; ++i)
	{
		t_to_j = 1;
		for (unsigned int j = 0; j < columns; ++j)
		{
			m(i,j) = t_to_j;
			t_to_j *= t;
		}
		v[i] = sample_distances[interpolation_si + i];
		t += step;
	}
	NL::LinearSystem ls(m, v);
	NL::Vector coeff = ls.SV_solve();
	const Linear t_interval(interval_st, interval_et);
	for (int i = columns - 1; i > 0; --i)
	{
		piece += SBasis(coeff[i]);
		piece *= t_interval;
	}
	piece += SBasis(coeff[0]);
	return piece;
}


Piecewise<SBasis> 
distance (cairo_t* cr, D2<SBasis> const& A, D2<SBasis> const& B, 
		    unsigned int pieces, unsigned int samples_per_piece)
{
	Piecewise<SBasis> pwc;
	pwc.push_cut(0);
	SBasis piece;
	const unsigned int N = pieces * samples_per_piece;
	const unsigned int samples_per_junction = 2;
	const unsigned int samples_per_2junctions = 2*samples_per_junction;
	const unsigned int samples_per_interpolation 
		=  samples_per_piece + samples_per_2junctions;
	std::vector<double> sample_distances(samples_per_interpolation);
	const unsigned int interval_si = samples_per_junction;
	const double step = 1.0 / N;
	const double piece_step = samples_per_piece * step;
	const double junction_step = samples_per_junction * step;

	double t = 0;
	bool corner = false;
	unsigned int evaluation_si = samples_per_junction + 1;
	unsigned int interpolation_si = samples_per_junction;
	unsigned int interpolation_ei = samples_per_interpolation;
	double interval_st = 0;
	double interval_et = interval_st + piece_step;
	Point At = A(t);
	double old_nptime = nearest_point(At, B);
	double nptime;
	sample_distances[interpolation_si] = distance(At, B(old_nptime));
	cairo_move_to(cr, 1600*t, 4*sample_distances[interval_si]);
	double interpolation_st = 0;	
	while (interval_et + piece_step <= 1)
	{
		corner = false;
		interpolation_ei = samples_per_interpolation;
		interval_et = interval_st + piece_step;
		for (unsigned int i = evaluation_si; i < samples_per_interpolation; ++i)
		{
			t += step;
			At = A(t);
			nptime = nearest_point(At, B);
			sample_distances[i] = distance(At, B(nptime));
			cairo_line_to(cr, 800*t, 2*sample_distances[i]);
			corner = !are_near(old_nptime, nptime, 0.1)
						|| are_near(sample_distances[i], 0, 1);
			old_nptime = nptime;
			//corner = false;
			if (corner)
			{
//				std::cerr << "corner at: " << t << std::endl;
//				std::cerr << "d[i-1] = " << sample_distances[i-1] << std::endl;
//				std::cerr << "d[i] = " << sample_distances[i] << std::endl;
//				std::cerr 
//					<< "d[i] - d[i-1] = " 
//					<< sample_distances[i] - sample_distances[i-1] 
//					<< std::endl;
				interpolation_ei = i+1;
				interval_et = t;
				break;
			}
		}
		
		piece = interpolate(A, B, sample_distances, 
				              interpolation_si, interpolation_ei, 
				              interpolation_st, interval_st, interval_et, step);
		pwc.push(piece, interval_et);
		
//		if (pwc.segs.size() >= 2)
//		{
//			const SBasis & seg1 = *(pwc.segs.end() - 2);
//			const SBasis & seg2 = *(pwc.segs.end() - 1);
//			if ( !are_near(seg1(1), seg2(0), 1) )
//			{
//				std::cerr 
//					<< "difference: " << seg1(1) - seg2(0) 
//					<< " at t = " <<  interval_st << std::endl
//					<< "seg(1) = " << seg1(1) << std::endl
//					<< "seg(0) = " << seg2(0) << std::endl
//					<< "interpolation_si = " << interpolation_si << std::endl
//					<< "interpolation_ei = " << interpolation_ei << std::endl
//					<< "interpolation_st = " << interpolation_st << std::endl
//					<< "evaluation_si = " << evaluation_si << std::endl;
//				for(unsigned int i = 0; i < evaluation_si; ++i)
//				{
//					std::cerr 
//						<< "sample_distances[" << i << "] = " 
//						<< sample_distances[i] << std::endl;
//				}
//			}
//		}

		interval_st = interval_et;
		if (corner)
		{
			sample_distances[interval_si] = sample_distances[interpolation_ei-1];
			interpolation_si = samples_per_junction;
			evaluation_si = samples_per_junction + 1;
			interpolation_st = interval_st;
		}
		else
		{
			interpolation_si = 0;
			evaluation_si = samples_per_2junctions;
			interpolation_st = interval_st - junction_step;
			for(unsigned int i = 0; i < evaluation_si; ++i)
			{
				sample_distances[i] = sample_distances[samples_per_piece + i];
			}
		}
	}
	while (interval_et < 1)
	{
		corner = false;
		interpolation_ei = samples_per_interpolation;
		for (unsigned int i = evaluation_si; i < samples_per_interpolation; ++i)
		{
			t += step;
			if ( t > 1 )
			{
				interpolation_ei = i;
				interval_et = 1;
				break;
			}
			At = A(t);
			nptime = nearest_point(At, B);
			sample_distances[i] = distance(At, B(nptime));
			cairo_line_to(cr, 800*t, 2*sample_distances[i]);
			corner = !are_near(old_nptime, nptime, 0.1)
						|| are_near(sample_distances[i], 0, 1);
			old_nptime = nptime;
			//corner = false;
			if (corner)
			{
				interpolation_ei = i;
				interval_et = t;
				break;
			}
		}
		
		piece = interpolate(A, B, sample_distances, 
				              interpolation_si, interpolation_ei, 
				              interpolation_st, interval_st, interval_et, step);
		pwc.push(piece, interval_et);
		interval_st = interval_et;
		if (corner)
		{
			sample_distances[interval_si] = sample_distances[interpolation_ei];
			interpolation_si = samples_per_junction;
			evaluation_si = samples_per_junction + 1;
			interpolation_st = interval_st;
		}
		else
		{
			interpolation_si = 0;
			evaluation_si = samples_per_2junctions;
			interpolation_st = interval_st - junction_step;
			for(unsigned int i = 0; i < evaluation_si; ++i)
			{
				sample_distances[i] = sample_distances[samples_per_piece + i];
			}
		}
	}

	cairo_stroke(cr);
	return pwc;
}




class DCCToy : public Toy
{
private:
  void draw( cairo_t *cr,	std::ostringstream *notify, 
  		     int width, int height, bool save ) 
  {
	  cairo_set_line_width (cr, 0.3);
	  D2<SBasis> A = handles_to_sbasis(handles.begin(), A_order-1);
	  cairo_md_sb(cr, A);
	  D2<SBasis> B = handles_to_sbasis(handles.begin() + A_order, B_order-1);
	  cairo_md_sb(cr, B);
	  double npt = nearest_point(handles.back(), A);
	  handles.back() = A(npt);
	  double nptB = nearest_point(handles.back(), B);
	  cairo_move_to(cr, handles.back());
	  cairo_line_to(cr, B(nptB));
	  cairo_stroke(cr);
	  Piecewise<SBasis> d = distance(cr, A, B, 100, 4);
	  Piecewise< D2<SBasis> > pwc;
	  pwc.cuts = d.cuts;
	  pwc.segs.resize(d.size());
	  D2<SBasis> piece;
	  for ( unsigned int i = 0; i < d.size(); ++i )
	  {
		  piece[X] = SBasis(800 * Linear(d.cuts[i], d.cuts[i+1]));
		  piece[Y] = 2 * d.segs[i];
		  pwc.segs[i] = piece;
	  }
	  cairo_set_source_rgb(cr, 0.7,0,0);
	  cairo_pw_d2(cr, pwc);
	  draw_handle(cr, pwc(0));
	  draw_handle(cr, pwc(0.25));
	  draw_handle(cr, pwc(0.5));
	  draw_handle(cr, pwc(0.75));
	  draw_handle(cr, pwc(1));
	  draw_circ(cr, pwc(npt));
	  cairo_stroke(cr);
	  Toy::draw(cr, notify, width, height, save);
  }

public:
	DCCToy()
	{
		A_order = 6;
		B_order = 4;
		total_handles = A_order + B_order + 1;;
    	for ( unsigned int i = 0; i < total_handles; ++i )
    	{
    		handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    	}

	}
private:
	unsigned int A_order, B_order, total_handles;
};




int main(int argc, char **argv) 
{	
    init( argc, argv, new DCCToy() );
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
