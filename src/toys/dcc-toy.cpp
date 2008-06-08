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

// this wrapper class is an helper to make up a curve portion and access it
// in an homogenous way 
template< typename Curve01T > 
class CurvePortion
{
  public:
    CurvePortion(const Curve & curve, double from, double to) 
        : m_curve_ptr(curve.portion(from, to))
    {
    }

    Curve01T & get_curve()
    {
        return *( static_cast<Curve01T*>(m_curve_ptr) );
    }

    ~CurvePortion()
    {
        if (m_curve_ptr != NULL)
            delete m_curve_ptr;
    }

  private:
    Curve* m_curve_ptr;
};

template<> 
class CurvePortion< D2<SBasis> >
{
  public:
    CurvePortion< D2<SBasis> >(const D2<SBasis> & curve, double from, double to) 
        : m_curve(portion(curve, from, to))
    {
    }

    D2<SBasis> & get_curve()
    {
        return m_curve;
    }

  private:
    D2<SBasis> m_curve;
};


template< typename Curve01T, typename CurveT >
class distance_impl
{
    typedef Curve01T curveA_type;
    typedef CurveT curveB_type;
    // determine how near a distance sample and the value computed through 
    // the interpolated function have to be
    double accuracy;
    // determine the recursion limit
    double adaptive_limit;
    // pieces of the initial subdivision  
    unsigned int piecees;
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
    // curve piece start t
    double portion_st;
    // curve piece end t
    double portion_et;

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
        N = piecees * samples_per_piece;
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
    
    bool check_accuracy( SBasis const& piece, 
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

    void init_power_matrix(NL::Matrix & power_matrix)
    {        
        double t = 0;
        double u0, u1, s;
        unsigned int half_rows = power_matrix.rows() / 2;
        unsigned int n = power_matrix.rows() - 1;
        for (unsigned int i0 = 0, i1 = n; i0 < half_rows; ++i0, --i1)
        {
            u0 = 1-t;
            u1 = t;
            s = u0 * u1;
            for (unsigned int j = 0; j < piece_size; j+=2)
            {
                power_matrix(i0, j) = u0;
                power_matrix(i0, j+1) = u1;
                power_matrix(i1, j) = u1;
                power_matrix(i1, j+1) = u0;
                u0 *= s;
                u1 *= s;
            }
            t += rec_step;
        }
        // t = 1/2
        assert( are_near(t, 0.5) );
        u1 = 1/2.0;
        s = 1/4.0;
        for (unsigned int j = 0; j < piece_size; j+=2)
        {
            power_matrix(half_rows, j) = u1;
            power_matrix(half_rows, j+1) = u1;
            u1 *= s;
        }
    }
    
    void interpolate( SBasis & piece, 
                      NL::Matrix & power_matrix,
                      NL::Vector & sample_distances,
                      double interpolation_si, double interpolation_samples, 
                      double _portion_st, double _portion_et )
    {
        piece.resize(2);
        NL::MatrixView m( power_matrix, 
                          interpolation_si, 0, 
                          interpolation_samples, piece_size );
        NL::VectorView v( sample_distances, 
                          interpolation_samples, 
                          interpolation_si );
        NL::LinearSystem ls(m, v);
        const NL::Vector & coeff = ls.SV_solve();
        for (unsigned int i = 0, k = 0; i < piece_size; i+=2, ++k)
        {
            piece[k][0] = coeff[i];
            piece[k][1] = coeff[i+1];
        }
        piece = portion(piece, _portion_st, _portion_et);
    }
    
    void evaluate_samples( curveA_type const& A, 
                           curveB_type const& B, 
                           NL::Vector & sample_distances,
                           double& t )
    {
        Point At;
        double nptime;
        //double u0, u1, s;
        for (unsigned int i = evaluation_si; i < evaluation_ei; ++i)
        {
            At = A(t);
            nptime = nearest_point(At, B);
            sample_distances[i] = distance(At, B(nptime));
            t += step;
        }
    }
    
    void evaluate_piece_rec( Piecewise<SBasis> & pwc, 
                             curveA_type const& A, 
                             curveB_type const& B,
                             NL::Matrix & power_matrix,
                             NL::Matrix & curr_matrix,
                             NL::Vector & curr_vector,
                             NL::Vector & sample_distances,
                             bool adaptive,
                             double _interpolation_si,
                             double _interpolation_ei,
                             double _interval_st,
                             double _interval_et,
                             double half_real_step )
    {
        SBasis piece;
        double _interpolation_samples = _interpolation_ei - _interpolation_si;
        interpolate( piece, curr_matrix, curr_vector,
                     _interpolation_si, _interpolation_samples,
                     _interval_st, _interval_et );       
        if (adaptive)
        {
            bool good 
                = check_accuracy( piece, sample_distances, rec_step );
            if (!good)
            {
                Piecewise<SBasis> spwc;
                CurvePortion<curveA_type> cp(A, _interval_st, _interval_et);
                evaluate_rec( spwc, 
                              cp.get_curve(), 
                              B, 
                              power_matrix,
                              sample_distances, 
                              half_real_step );
                append(pwc, spwc, _interval_st, rec_piece_step);
            }
            else
            {
                pwc.push(piece, _interval_et);
            }
        }
        else
        {
            pwc.push(piece, _interval_et);
        }
    }

    
    // recursive routine: if the interpolated piece is accurate enough 
    // it's returned in the out-parameter pwc, otherwise the computation of 
    // two new piecees is performed using half of the current step so the 
    // number of samples per piece is always the same, while the interpolation
    // of one piece is split into the computation of two new piecees when 
    // needed.
    void evaluate_rec( Piecewise<SBasis> & pwc, 
                       curveA_type const& A, 
                       curveB_type const& B,
                       NL::Matrix & power_matrix,
                       NL::Vector & sample_distances,
                       double real_step )
    {
        const double half_real_step = real_step / 2;
        const bool adaptive = !(real_step < adaptive_limit);
        static const unsigned int middle_sample_index = samples_per_piece + 1;
        
        pwc.clear();
        pwc.push_cut(0);
        // sample_distances used to check accuracy and for the interpolation 
        // of the two sub-pieces when needed
        NL::Vector sample_distances_1(rec_total_samples);
        NL::Vector sample_distances_2(rec_total_samples);

        // view of even indexes of sample_distances_1
        NL::VectorView 
        sd1_view_0(sample_distances_1, middle_sample_index, 0, 2);
        // view of even indexes of sample_distances_2
        NL::VectorView 
        sd2_view_0(sample_distances_2, middle_sample_index, 0, 2);
        // view of first half (+ 1) of sample_distances
        NL::VectorView 
        sd_view_1(sample_distances, middle_sample_index, 0);
        // view of second half of sample_distances
        NL::VectorView 
        sd_view_2(sample_distances, middle_sample_index, samples_per_piece);

        sd1_view_0 = sd_view_1;
        sd2_view_0 = sd_view_2;

        // if we have to check accuracy and go on with recursion
        // we need to compute the distance samples of middle points 
        // of all current samples, because the new step is half of 
        // the current one
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
        // make a copy of power_matrix
        curr_matrix = power_matrix;

        // first piece
        evaluate_piece_rec( pwc, A, B,
                            power_matrix,
                            curr_matrix,
                            sample_distances,
                            sample_distances_1,
                            adaptive,
                            0,                  // interpolation_si
                            shared_ei,          // interpolation_ei
                            0,                  // portion_st
                            rec_piece_step,     // portion_et
                            half_real_step );

        // copy back junction parts because 
        // the interpolate routine modifies them
        for ( unsigned int i = 0, j = samples_per_piece - 1; 
              i < samples_per_junction; 
              ++i, --j )
        {
            sd_view_1[j] = sd1_view_0[j];
            sd_view_2[i] = sd2_view_0[i];
        }

        // copy [shared_si, shared_si+samples_per_2junctions] rows of 
        // power_matrix to the same rows of curr_matrix, because 
        // the interpolate routine modifies them
        // middle sub-matrix view source
        NL::MatrixView mmvs( power_matrix, 
                             shared_si, 
                             0, 
                             samples_per_2junctions, 
                             piece_degree );
        // middle sub-matrix view destination
        NL::MatrixView mmvd( curr_matrix, 
                             shared_si, 
                             0, 
                             samples_per_2junctions, 
                             piece_degree );

        mmvd = mmvs;

        // last piece
        evaluate_piece_rec( pwc, A, B, 
                            power_matrix,
                            curr_matrix,
                            sample_distances,
                            sample_distances_2,
                            adaptive,
                            shared_si,          // interpolation_si
                            rec_total_samples,  // interpolation_ei
                            rec_piece_step,     // portion_st
                            rec_piece_2steps,   // portion_et
                            half_real_step );
    }

    
    void evaluate_piece( Piecewise<SBasis> & pwc, 
                         curveA_type const& A, 
                         curveB_type const& B,
                         NL::Matrix & power_matrix,
                         NL::MatrixView & power_matrix_view,
                         NL::Matrix & curr_matrix,
                         NL::Vector & curr_vector,
                         NL::Vector & sample_distances,		
                         NL::Vector & end_junction,
                         NL::VectorView & start_junction_view,
                         NL::VectorView & end_junction_view,
                         double & t )
    {
        //static size_t index = 0;
        //std::cerr << "index = " << index++ << std::endl;		
        bool good;
        SBasis piece;
        Piecewise<SBasis> spwc;
        interval_et += piece_step;
        //std::cerr << "interval: " << interval_st << ", " << interval_et << std::endl;
        //std::cerr << "interpolation range: " << interpolation_si << ", " << interpolation_ei << std::endl;
        //std::cerr << "interpolation samples = " <<  interpolation_samples << std::endl;
        evaluate_samples( A, B, curr_vector, t );
        //std::cerr << "current vector: " << curr_vector << std::endl;
        for ( unsigned int i = 0, k = interval_si; 
              i < sample_distances.size(); 
              i+=2, ++k )
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
        //std::cerr << "sample_distances: " << sample_distances << std::endl;
        end_junction = end_junction_view;
        curr_matrix = power_matrix_view;
        interpolate( piece, curr_matrix, curr_vector, 
                     interpolation_si, interpolation_samples, 
                     portion_st, portion_et );
        good = check_accuracy( piece, sample_distances, rec_step );
        //std::cerr << "good: " << good << std::endl;
        //good = false;
        if (!good)
        {
            CurvePortion<curveA_type> cp(A, interval_st, interval_et);
            evaluate_rec( spwc, 
                          cp.get_curve(), 
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
    }
	
  public:
    void evaluate( Piecewise<SBasis> & pwc, 
                   curveA_type const& A, 
                   curveB_type const& B, 
                   unsigned int _piecees )
    {
        piecees = _piecees;
        init();
        assert( !(piecees & 1) );
        assert( !(piece_size & 1) );
        assert( rec_total_samples & 1);
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
        NL::MatrixView power_matrix_view( power_matrix, 
                                          0, 
                                          0, 
                                          curr_matrix.rows(), 
                                          curr_matrix.columns() );
        double t = 0;

        // first piece
        evaluation_si = interval_si;
        evaluation_ei = samples_per_interpolation;
        interpolation_si = evaluation_si;
        interpolation_ei = evaluation_ei;
        interpolation_samples = interpolation_ei - interpolation_si;
        interval_st = 0;
        interval_et = 0;
        portion_st = 0.0;
        portion_et = (double)(samples_per_piece) / samples_per_interpolation;
        evaluate_piece( pwc, A, B, power_matrix,
                        power_matrix_view, curr_matrix, 
                        curr_vector, sample_distances, end_junction,
                        start_junction_view, end_junction_view,
                        t );

        // general case
        evaluation_si = interval_si + samples_per_junction;
        evaluation_ei = samples_per_interpolation;
        interpolation_si = 0;
        interpolation_ei = evaluation_ei;
        interpolation_samples = interpolation_ei - interpolation_si;
        portion_st = (double)(samples_per_junction) / samples_per_interpolation;
        portion_et = portion_st + portion_et;        
        for ( unsigned int piece_index = 1; 
              piece_index < piecees - 1; 
              ++piece_index )
        {
            evaluate_piece( pwc, A, B, power_matrix,
                            power_matrix_view, curr_matrix, 
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
                        power_matrix_view, curr_matrix,
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

template < typename Curve01T, typename CurveT >
inline
Piecewise<SBasis> 
distance( Curve01T const& A, 
          CurveT const& B, 
          unsigned int pieces = 40,
          double adaptive_limit = 1e-5,
          double accuracy = 1e-3 )
{

    detail::distance_impl<Curve01T, CurveT> dist;
    dist.set_accuracy(accuracy);
    dist.set_adaptive_limit(adaptive_limit);
    Piecewise<SBasis> pwc;
    dist.evaluate(pwc, A, B, pieces);
    return pwc;
}

template < typename CurveT >
inline
Piecewise<SBasis> 
distance( Piecewise< D2<SBasis> > const& A, 
          CurveT const& B,
          unsigned int pieces = 40,
          double adaptive_limit = 1e-5,
          double accuracy = 1e-3 )
{
    Piecewise<SBasis> result;
    Piecewise<SBasis> pwc;
    for (unsigned int i = 0; i < A.size(); ++i)
    {
        pwc = distance(A[i], B, pieces, adaptive_limit, accuracy);
        pwc.scaleDomain(A.cuts[i+1] - A.cuts[i]);
        pwc.offsetDomain(A.cuts[i]);
        result.concat(pwc);
    }
    return result;
}

template < typename CurveT >
inline
Piecewise<SBasis> 
distance( Path const& A, 
          CurveT const& B,
          unsigned int pieces = 40,
          double adaptive_limit = 1e-5,
          double accuracy = 1e-3 )
{
    Piecewise<SBasis> result;
    Piecewise<SBasis> pwc;
    unsigned int sz = A.size();
    if (A.closed()) ++sz;
    for (unsigned int i = 0; i < sz; ++i)
    {
        pwc = distance(A[i], B, pieces, adaptive_limit, accuracy);
        pwc.offsetDomain(i);
        result.concat(pwc);
    }
    return result;
}


template < typename Curve01T, typename CurveT >
unsigned int dist_test( Piecewise<SBasis> const& pwc, 
                        Curve01T const& A, 
                        CurveT const& B, 
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
        Point ulc(width - 300, height - 60 );
        toggles[0].bounds = Rect(ulc , ulc + Point(160,25) );
        draw_toggles(cr, toggles);
        
        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
        cairo_set_line_width (cr, 0.3);
        D2<SBasis> A = handles_to_sbasis(handles.begin(), A_order-1);
        //SBasisCurve A(A0);
        cairo_md_sb(cr, A);
        D2<SBasis> B0 = handles_to_sbasis(handles.begin() + A_order, B0_order-1);
        cairo_md_sb(cr, B0);
        D2<SBasis> B(B0);
        //D2<SBasis> B1 = handles_to_sbasis(handles.begin() + A_order + B0_order-1, B1_order-1);
        //cairo_md_sb(cr, B1);
//        CubicBezier B1( handles[A_order + B0_order-1], handles[A_order + B0_order], 
//                        handles[A_order + B0_order + 1], handles[A_order + B0_order + 2] );
//        cairo_md_sb(cr, B1.toSBasis());
//        EllipticalArc B2;
//        try
//        {
//            B2.set( handles[A_order + B0_order + B1_order-2],
//                    200, 100, 30, false, false,
//                    handles[A_order + B0_order + B1_order-1]);
//        }
//        catch(RangeError e)
//        {
//        }
//        cairo_md_sb(cr, B2.toSBasis());
        //Piecewise< D2<SBasis> > B;
        //B.push_cut(0); B.push(B0, 0.5); B.push(B1, 1);
//        Path B;
//        B.append(SBasisCurve(B0));
//        B.append(B1);
//        B.append(B2);
        double npt = nearest_point(handles.back(), A);
        handles.back() = A(npt);
        double nptB = nearest_point(handles.back(), B);
        cairo_move_to(cr, handles.back());
        cairo_line_to(cr, B(nptB));
        cairo_stroke(cr);
        Piecewise<SBasis> d;
        if (!toggles[0].on)
        {
             d = distance(A, B, 40);
        }
        else
        {
            d = distance(B, A, 40);
        }
        // uncomment following lines to view errors in computing the distance
//        unsigned int total_error = dist_test(d, A, B, 0.0004);
//        *notify << "total error: " << total_error;
        Piecewise< D2<SBasis> > pwc;
        pwc.cuts = d.cuts;
        pwc.segs.resize(d.size());
        D2<SBasis> piece;
        double domain_length = 800 / d.domain().extent();
        for ( unsigned int i = 0; i < d.size(); ++i )
        {
            piece[X] = SBasis(Linear(20,20) 
                              + domain_length * Linear(d.cuts[i], d.cuts[i+1]));
            piece[Y] = 3 * d.segs[i];
            pwc.segs[i] = piece;
        }
        cairo_set_source_rgb(cr, 0.7,0,0);
        cairo_pw_d2(cr, pwc);
        for (unsigned int i = 0; i < pwc.cuts.size(); ++i)
        {
            draw_handle(cr, pwc(pwc.cuts[i]));
        }
        *notify << "total cuts: " << pwc.cuts.size();
        
//        draw_handle(cr, pwc(0.0));
//        draw_handle(cr, pwc(0.25));
//        draw_handle(cr, pwc(0.5));
//        draw_handle(cr, pwc(0.75));
//        draw_handle(cr, pwc(1));
        draw_circ(cr, pwc(npt));
        cairo_stroke(cr);
        Toy::draw(cr, notify, width, height, save);
    }
    
    void mouse_pressed(GdkEventButton* e) 
    {
        toggle_events(toggles, e);
        Toy::mouse_pressed(e);
    }

  public:
    DCCToy()
    {
        A_order = 6;
        B0_order = 4;
        B1_order = 0;
        total_handles = A_order + B0_order + B1_order + 1;;
        for ( unsigned int i = 0; i < total_handles; ++i )
        {
            handles.push_back(Geom::Point(uniform()*400, uniform()*400));
        }
        
        toggles.push_back(Toggle("d(A,B) <-> d(B,A)", false));

    }
  private:
    unsigned int A_order, B0_order, B1_order, total_handles;
    std::vector<Toggle> toggles;
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
