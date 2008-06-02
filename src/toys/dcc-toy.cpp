/*
 * curve-curve distance 
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

#include "nearest-point.h"
#include "numeric/linear_system.h"

#include <algorithm>



namespace Geom
{
namespace detail
{

class distance_impl
{
	// determine how near a distance sample and the value computed through 
	// the interpoleted function have to be
	double accuracy;
	// determine the recursion limit
	double adaptive_limit;
	// picees of the initial subdivision  
	unsigned int pieces;
	// degree of the polynomial used to interpolate a piece
	unsigned int piece_degree;
	// number of coefficients = piece_degree + 1
	unsigned int piece_size;
	unsigned int samples_per_piece;
	// total initial samples 
	unsigned int N;
	// a junction is a part of the previous or of the next piece
	unsigned int samples_per_junction;
	unsigned int samples_per_2junctions;
	// number of distance samples used in the interpolation (in the general case)
	unsigned int samples_per_interpolation;
	// distance between two consecutive parameters at which samples are evaluated
	double step;
	double half_step;
	// length of the initial domain interval of a piece
	double piece_step;
	// length of the interval related to a junction
	double junction_step;
	// index of the first sample related to a piece
	unsigned int interval_si;
	// index of the last sample related to a piece
	unsigned int interval_ei;
	// index of the first sample to be evaluated for the current piece
	unsigned int evaluation_si;
	// index of the last sample to be evaluated for the current piece
	unsigned int evaluation_ei;
	// index of the first sample to be used for interpolating the current piece 
	unsigned int interpolation_si;
	// index of the last sample to be used for interpolating the current piece 
	unsigned int interpolation_ei;
	// number of total samples to be used for interpolating the current piece
	// this is equal to samples_per_interpolation except for the first and last 
	// piece
	unsigned int interpolation_samples;
	// parameter value for the first sample related to the current piece
	double interval_st;
	// interval_st + piece_step
	double interval_et;
	
	unsigned int rec_pieces;
	unsigned int rec_N;
	unsigned int shared_si;
	unsigned int shared_ei;
	double rec_step;
	double rec_half_step;
	double rec_piece_step;
	double rec_piece_2steps;
	unsigned int rec_total_samples;
	
	
	void init()
	{
		piece_degree = 3;
		piece_size = piece_degree + 1;
		samples_per_piece = 4;
		N = pieces * samples_per_piece;
		samples_per_junction = 2;
		samples_per_2junctions = 2*samples_per_junction;
		samples_per_interpolation 
			=  samples_per_piece + samples_per_2junctions;
		step = 1.0 / N;
		half_step = step / 2;
		piece_step = samples_per_piece * step;
		junction_step = samples_per_junction * step;
		interval_si = samples_per_junction;
		interval_ei = interval_si + samples_per_piece;
		
		// recursive routine parameters
		rec_pieces = 2;
		rec_N = rec_pieces * samples_per_piece;
		rec_total_samples = 2 * samples_per_piece + 1;
		shared_si = samples_per_piece - samples_per_junction;
		shared_ei = samples_per_piece + samples_per_junction;
		rec_step = 1.0 / rec_N;
		rec_half_step = rec_step / 2;
		rec_piece_step = samples_per_piece * rec_step;
		rec_piece_2steps = 2 * rec_piece_step;
	}
	
	void init_power_matrix(NL::Matrix & power_matrix)
	{
		double t = 0;
		double t_to_j;
		for (unsigned int i = 0; i < power_matrix.rows(); ++i)
		{
			t_to_j = 1;
			for (unsigned int j = 0; j < piece_size; ++j)
			{
				power_matrix(i,j) = t_to_j;
				t_to_j *= t;
			}
			t += rec_step;
		}

	}
	
	void interpolate( SBasis & piece, 
	                  D2<SBasis> const& A, D2<SBasis> const& B,
	                  NL::Matrix & power_matrix,
	                  NL::Vector & sample_distances,
	                  double interpolation_si, double interpolation_samples, 
	                  double interval_st, double interval_et)
	{
		piece = SBasis(0.0);
		NL::MatrixView m(power_matrix, interpolation_si, 0, interpolation_samples, piece_size);
		NL::VectorView v(sample_distances, interpolation_samples, interpolation_si);
		NL::LinearSystem ls(m, v);
		const NL::Vector & coeff = ls.SV_solve();
		const Linear t_interval(interval_st, interval_et);
		for (int i = piece_degree; i > 0; --i)
		{
			piece += SBasis(coeff[i]);
			piece *= t_interval;
		}
		piece += SBasis(coeff[0]);
	}
	
	bool check_accuracy( SBasis const& piece, 
			             D2<SBasis> const& A, D2<SBasis> const& B,
			             NL::Vector const& sample_distances,
			             double step )
	{
		double t = 0;
		for (unsigned int i = 0; i < sample_distances.size(); ++i)
		{
			if ( !are_near(piece(t), sample_distances[i], accuracy) )
			{
				return false;
			}
			t += step;
		}
		return true;
	}
	

	void append( Piecewise<SBasis> & pwc, 
			     Piecewise<SBasis> const& spwc,
			     double interval_st,
			     double interval_length )
	{
		double cut;
		for (unsigned int i = 0; i < spwc.size(); ++i)
		{
			cut = interval_st + spwc.cuts[i+1] * interval_length;
			pwc.push(spwc.segs[i], cut);
		}
	}
	
	void evaluate_samples( D2<SBasis> const& A, D2<SBasis> const& B, 
						   NL::Matrix & power_matrix,
						   NL::Vector & sample_distances,
						   double& t)
	{
		Point At;
		double nptime;
		double t_to_j;
		for (unsigned int i = evaluation_si; i < evaluation_ei; ++i)
		{
			At = A(t);
			nptime = nearest_point(At, B);
			sample_distances[i] = distance(At, B(nptime));
			t_to_j = 1;
			for (unsigned int j = 0; j < piece_size; ++j)
			{
				power_matrix(i,j) = t_to_j;
				t_to_j *= t;
			}
			t += step;
		}
	}
	
	// recursive routine: if the interpolated piece is enough accurate 
	// it's returned in the out-parameter pwc, else the computation of 
	// two new pieces is performed using the half of the current step
	// so the samples per piece is an invariant, while the interpolation 
	// of one piece is splitted in the computation of two new pieces when 
	// needed
	void evaluate_piece_rec( Piecewise<SBasis> & pwc, 
							 D2<SBasis> const& A, 
							 D2<SBasis> const& B,
							 NL::Matrix & power_matrix,
							 NL::Vector & sample_distances,
							 double real_step )
	{
		const double half_real_step = real_step / 2;
		const bool adaptive = !(real_step < adaptive_limit);
		static const unsigned int middle_sample_index = samples_per_piece + 1;
		bool good;
		SBasis piece;
		pwc.clear();
		pwc.push_cut(0);
		// sample_distances used to check accuracy and for the interpolation 
		// of the two sub-pieces when needed
		NL::Vector sample_distances_1(rec_total_samples);
		NL::Vector sample_distances_2(rec_total_samples);
		
		// view of even index of sample_distances_1
		NL::VectorView sd1_view_0(sample_distances_1, middle_sample_index, 0, 2);
		// view of even index of sample_distances_2
		NL::VectorView sd2_view_0(sample_distances_2, middle_sample_index, 0, 2);
		// view of first half (+ 1) of sample_distances
		NL::VectorView sd_view_1(sample_distances, middle_sample_index, 0);
		// view of second half of sample_distances
		NL::VectorView sd_view_2(sample_distances, middle_sample_index, samples_per_piece);
		sd1_view_0 = sd_view_1;
		sd2_view_0 = sd_view_2;
		
		if (adaptive)
		{
			Point At;
			double nptime;
			double t = rec_half_step;
			for (unsigned int i = 1; i < sample_distances.size(); i+=2)
			{
				At = A(t);
				nptime = nearest_point(At, B);
				sample_distances_1[i] = distance(At, B(nptime));
				At = A(t + rec_piece_step);
				nptime = nearest_point(At, B);
				sample_distances_2[i] = distance(At, B(nptime));
				t += rec_step;
			}
		}
		
		// current matrix
		NL::Matrix curr_matrix(power_matrix.rows(), power_matrix.columns());
		// middle sub-matrix view source
		NL::MatrixView mmvs(power_matrix, shared_si, 0, samples_per_2junctions, piece_degree);
		// middle sub-matrix view destination
		NL::MatrixView mmvd(curr_matrix, shared_si, 0, samples_per_2junctions, piece_degree);
		
		// first piece
		// make a copy of power_matrix
		curr_matrix = power_matrix;
		size_t interpolation_si = 0;
		size_t interpolation_ei = shared_ei;
		size_t interpolation_samples = interpolation_ei - interpolation_si;
		double interval_st = 0;
		double interval_et = rec_piece_step;
		interpolate( piece, A, B, curr_matrix, sample_distances, 
				     interpolation_si, interpolation_samples,
				     interval_st, interval_et );
		if (adaptive)
		{
			good = check_accuracy( piece, A, B, sample_distances_1, rec_step );
			if (!good)
			{
				Piecewise<SBasis> spwc;
				evaluate_piece_rec( spwc, 
						            portion(A, interval_st, interval_et), 
						            B, 
						            power_matrix, 
						            sample_distances_1, 
						            half_real_step );
				append(pwc, spwc, interval_st, rec_piece_step);
			}
			else
			{
				pwc.push(piece, interval_et);
			}
		}
		else
		{
			pwc.push(piece, interval_et);
		}
		// last piece
		// copy back junction parts
		for ( unsigned int i = 0, j = samples_per_piece - 1; 
		      i < samples_per_junction; 
		      ++i, --j )
		{
			sd_view_1[j] = sd1_view_0[j];
			sd_view_2[i] = sd2_view_0[i];
		}
		// copy middle sub-matrix of power_matrix to curr_matrix
		mmvd = mmvs;
		interpolation_si = shared_si;
		interpolation_ei = rec_total_samples;
		interpolation_samples = interpolation_ei - interpolation_si;
		interval_st = interval_et;
		interval_et = rec_piece_2steps;
		interpolate( piece, A, B, curr_matrix,  sample_distances,
				     interpolation_si, interpolation_samples,
				     interval_st, interval_et );
		if (adaptive)
		{
			good = check_accuracy( piece, A, B, sample_distances_2, rec_step );
			if (!good)
			{
				Piecewise<SBasis> spwc;
				evaluate_piece_rec( spwc, 
						            portion(A, interval_st, interval_et), 
						            B, 
						            power_matrix,
						            sample_distances_2, 
						            half_real_step );
				append(pwc, spwc, interval_st, rec_piece_step);
			}
			else
			{
				pwc.push(piece, interval_et);
			}
		}
		else
		{
			pwc.push(piece, interval_et);
		}
		
	}
	
	void evaluate_piece( Piecewise<SBasis> & pwc, 
			             D2<SBasis> const& A, D2<SBasis> const& B,
			             NL::Matrix & power_matrix,
			             NL::Matrix & next_matrix,
					     NL::Matrix & curr_matrix,
					     NL::Vector & curr_vector,
					     NL::Vector & sample_distances,
					     NL::Vector & end_junction,
					     NL::VectorView & start_junction_view,
					     NL::VectorView & end_junction_view,
					     double & t )
	{
//		static size_t index = 0;
//		std::cerr << "index = " << index++ << std::endl;		
		bool good;
		SBasis piece;
		Piecewise<SBasis> spwc;
		interval_et += piece_step;
//		std::cerr << "interval: " << interval_st << ", " << interval_et << std::endl;
//		std::cerr << "interpolation range: " << interpolation_si << ", " << interpolation_ei << std::endl;
//		std::cerr << "interpolation samples = " <<  interpolation_samples << std::endl;
		evaluate_samples( A, B, curr_matrix, curr_vector, t );
//		std::cerr << "current vector: " << curr_vector << std::endl;
		for (unsigned int i = 0, k = interval_si; i < sample_distances.size(); i+=2, ++k)
		{
			sample_distances[i] = curr_vector[k];
		}
		Point At;
		double nptime;
		double tt = interval_st + half_step;
		for (unsigned int i = 1; i < sample_distances.size(); i+=2)
		{
			At = A(tt);
			nptime = nearest_point(At, B);
			sample_distances[i] = distance(At, B(nptime));
			tt += step;
		}
//		std::cerr << "sample_distances: " << sample_distances << std::endl;
		end_junction = end_junction_view;
		for (unsigned int i = 0; i < samples_per_2junctions; ++i)
		{
			next_matrix.row_view(i) = curr_matrix.row_view(samples_per_piece + i);
		}
		interpolate( piece, A, B, curr_matrix, curr_vector, 
				     interpolation_si, interpolation_samples, 
				     interval_st, interval_et);
		good = check_accuracy( piece, A, B, sample_distances, rec_step);
		//std::cerr << "good: " << good << std::endl;
		if (!good)
		{
			evaluate_piece_rec( spwc, 
					            portion(A, interval_st, interval_et), 
					            B,
					            power_matrix,
					            sample_distances, 
					            half_step );
			append(pwc, spwc, interval_st, piece_step);
		}
		else
		{
			pwc.push(piece, interval_et);
		}
		interval_st = interval_et;
		for (unsigned int i = 0; i < samples_per_junction; ++i)
		{
			curr_vector[i] = start_junction_view[i];
			curr_vector[samples_per_junction + i] =  end_junction[i];
		}
		// inexpensive just pointers are swapped
		swap(curr_matrix, next_matrix);
	}
	
public:
	void evaluate( Piecewise<SBasis> & pwc, 
			       D2<SBasis> const& A, 
			       D2<SBasis> const& B, 
			       unsigned int _pieces )
	{
		pieces = _pieces;
		init();
		pwc.clear();
		pwc.push_cut(0);
		NL::Matrix power_matrix(rec_total_samples, piece_size);
		init_power_matrix(power_matrix);
		
		NL::Vector curr_vector(samples_per_interpolation);
		NL::Vector sample_distances(rec_total_samples);
		NL::Vector end_junction(samples_per_junction);
		NL::VectorView start_junction_view( 
							sample_distances, 
				            samples_per_junction, 
				            rec_total_samples - 1 - samples_per_2junctions, 
				            2 );
		NL::VectorView end_junction_view( 
							curr_vector, 
				            samples_per_junction, 
				            samples_per_junction + samples_per_piece );
		
		NL::Matrix curr_matrix(samples_per_interpolation, piece_size);
		NL::Matrix next_matrix(samples_per_interpolation, piece_size);
		double t = 0;
		
		// first piece
		evaluation_si = interval_si;
		evaluation_ei = samples_per_interpolation;
		interpolation_si = evaluation_si;
		interpolation_ei = evaluation_ei;
		interpolation_samples = interpolation_ei - interpolation_si;
		interval_st = 0;
		interval_et = 0;
		evaluate_piece( pwc, A, B, power_matrix,
				        curr_matrix, next_matrix, 
				        curr_vector, sample_distances, end_junction,
				        start_junction_view, end_junction_view,
				        t );

		// general case
		evaluation_si = interval_si + samples_per_junction;
		evaluation_ei = samples_per_interpolation;
		interpolation_si = 0;
		interpolation_ei = evaluation_ei;
		interpolation_samples = interpolation_ei - interpolation_si;
		for (unsigned int piece_index = 1; piece_index < pieces - 1; ++piece_index)
		{
			evaluate_piece( pwc, A, B, power_matrix,
					        curr_matrix, next_matrix, 
					        curr_vector, sample_distances, end_junction,
					        start_junction_view, end_junction_view,					
					        t );
		}
		
		// last piece
		evaluation_si = interval_si + samples_per_junction;
		evaluation_ei = interval_ei + 1;
		interpolation_si = 0;
		interpolation_ei = evaluation_ei;
		interpolation_samples = interpolation_ei -interpolation_si;
		evaluate_piece( pwc, A, B, power_matrix,
				        curr_matrix, next_matrix, 
				        curr_vector, sample_distances, end_junction,
				        start_junction_view, end_junction_view,			
				        t );
	}
	
	distance_impl()
		: accuracy(1e-3),
		  adaptive_limit(1e-5)
	{}
	
	void set_accuracy(double _accuracy)
	{
		accuracy = _accuracy;
	}
	
	void set_adaptive_limit(double _adaptive_limit)
	{
		adaptive_limit = _adaptive_limit;
	}
	
};  // end class distance_impl

}  // end namespace detail


Piecewise<SBasis> 
distance( D2<SBasis> const& A, D2<SBasis> const& B, 
	      unsigned int pieces = 40,
		  double adaptive_limit = 1e-5,
		  double accuracy = 1e-3 )
{
	detail::distance_impl dist;
	dist.set_accuracy(accuracy);
	dist.set_adaptive_limit(adaptive_limit);
	Piecewise<SBasis> pwc;
	dist.evaluate(pwc, A, B, pieces);
	return pwc;
}


unsigned int dist_test( Piecewise<SBasis> const& pwc, 
		                D2<SBasis> const& A, 
		                D2<SBasis> const& B, 
		                double step )
{
	std::cerr << "======= inside dist test =======" << std::endl;
	unsigned int total_checked_values = 0; 
	unsigned int total_error = 0;
	double nptime, sample_distance;
	Point At;
	for (double t = 0; t <= 1; t += step)
	{	
		At = A(t);
		nptime = nearest_point(At, B);
		sample_distance = distance(At, B(nptime));
		if ( !are_near(pwc(t), sample_distance, 0.001) )
		{
			++total_error;
			std::cerr << "error at t: " << t << std::endl;
		}
		++total_checked_values;
	}
	std::cerr << " total checked values : " << total_checked_values << std::endl;
	std::cerr << " total error : " << total_error << std::endl;
	return total_error;
}

}  // end namespace Geom


using namespace Geom;

class DCCToy : public Toy
{
private:
  void draw( cairo_t *cr, std::ostringstream *notify, 
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
	  Piecewise<SBasis> d = distance(A, B, 40);
	  // uncomment following lines to view errors in computing the distance
//	  unsigned int total_error = dist_test(d, A, B, 0.0004);
//	  *notify << "total error: " << total_error;
	  Piecewise< D2<SBasis> > pwc;
	  pwc.cuts = d.cuts;
	  pwc.segs.resize(d.size());
	  D2<SBasis> piece;
	  for ( unsigned int i = 0; i < d.size(); ++i )
	  {
		  piece[X] = SBasis(Linear(20,20) + 800 * Linear(d.cuts[i], d.cuts[i+1]));
		  piece[Y] = 3 * d.segs[i];
		  pwc.segs[i] = piece;
	  }
	  cairo_set_source_rgb(cr, 0.7,0,0);
	  cairo_pw_d2(cr, pwc);
	  draw_handle(cr, pwc(0.0));
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
		B_order = 6;
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
    init( argc, argv, new DCCToy(), 840, 600 );
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
