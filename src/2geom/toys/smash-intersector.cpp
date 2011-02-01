/*
 * Diffeomorphism-based intersector: given two curves
 *  M(t)=(x(t),y(t)) and N(u)=(X(u),Y(u))
 * and supposing M is a graph over the x-axis, we compute y(x) and solve
 *  Y(u) - y(X(u)) = 0
 * to get the intersections of the two curves...
 *
 * Notice the result can be far from intuitive because of the choice we have
 * to make to consider a curve as a graph over x or y. For instance the two
 * branches of xy=eps are never close from this point of view (!)...
 *
 * Authors:
 * 		J.-F. Barraud    <jfbarraud at gmail.com>
 * Copyright 2010  authors
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
#include <2geom/sbasis-geometric.h>
#include <2geom/intersection-by-smashing.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <cstdlib>
#include <cstdio>
#include <set>
#include <vector>
#include <algorithm>


using namespace Geom;

#define VERBOSE 0

static double exp_rescale(double x){ return pow(10, x);}
std::string exp_formatter(double x){ return default_formatter(exp_rescale(x));}



#if 0
//useless here;
Piecewise<D2<SBasis> > linearizeCusps( D2<SBasis> f, double tol){
	D2<SBasis> df = derivative( f );
	std::vector<Interval> xdoms = level_set( df[X], 0., tol);
	std::vector<Interval> ydoms = level_set( df[Y], 0., tol);
	std::vector<Interval> doms;
	//TODO: use order!!
	for ( unsigned i=0; i<xdoms.size(); i++ ){
		OptInterval inter = xdoms[i];
		for ( unsigned j=0; j<ydoms.size(); j++ ){
			inter &= ydoms[j];
		}
		if (inter) {
			doms.push_back( *inter );
		}
	}
	Piecewise<D2<SBasis> > result;
	if (doms.size() == 0 ) return Piecewise<D2<SBasis> >(f);
	if (doms[0].min() > 0 ){
		result.cuts.push_back( 0 );
		result.cuts.push_back( doms[0].min() );
		result.segs.push_back( portion( f, Interval( 0, doms[0].min() ) ) );
	}
	for ( unsigned i=0; i<doms.size(); i++ ){
		Point a = result.segs.back().at1();
		Point b = f.valueAt( doms[i].middle() );
		Point c = f.valueAt( doms[i].max() );
		result.cuts.push_back( doms[i].middle() );
		result.segs.push_back( D2<SBasis>( Linear( a[X], b[X] ), Linear( a[Y], b[Y] ) ) );
		result.cuts.push_back( doms[i].max() );
		result.segs.push_back( D2<SBasis>( Linear( b[X], c[X] ), Linear( b[Y], c[Y] ) ) );
		double t = ( i+1 == doms.size() )? 1 : doms[i+1].min();
		result.cuts.push_back( t );
		result.segs.push_back( portion( f, Interval( doms[i].max(), t ) ) );
	}
	return result;
}
#endif

/* Returns the intervals over which the curve keeps its slope
 * in one of the 8 sectors delimited by x=0, y=0, y=x, y=-x.
 */
std::vector<Interval> monotonicSplit(D2<SBasis> const &p){
	std::vector<Interval> result;

	D2<SBasis> v = derivative(p);

	std::vector<double> someroots;
	std::vector<double> cuts (2,0.);
	cuts[1] = 1.;

	someroots = roots(v[X]);
	cuts.insert( cuts.end(), someroots.begin(), someroots.end() );

	someroots = roots(v[Y]);
	cuts.insert( cuts.end(), someroots.begin(), someroots.end() );

	someroots = roots(v[X]-v[Y]);
	cuts.insert( cuts.end(), someroots.begin(), someroots.end() );

	someroots = roots(v[X]+v[Y]);
	cuts.insert( cuts.end(), someroots.begin(), someroots.end() );

	sort(cuts.begin(),cuts.end());
	unique(cuts.begin(), cuts.end() );

	for (unsigned i=1; i<cuts.size(); i++){
		result.push_back( Interval( cuts[i-1], cuts[i] ) );
	}
	return result;
}

#if 0
/* Computes the intersection of two sets given as (ordered) union intervals.
 */
std::vector<Interval> intersect( std::vector<Interval> const &a, std::vector<Interval> const &b){
	std::vector<Interval> result;
	//TODO: use order!
	for (unsigned i=0; i < a.size(); i++){
		for (unsigned j=0; j < b.size(); j++){
			OptInterval c( a[i] );
			c &= b[j];
			if ( c ) {
				result.push_back( *c );
			}
		}
	}
	return result;
}

/* Computes the top and bottom boundaries of the L_\infty neighborhood
 * of a curve. The curve is supposed to be a graph over the x-axis.
 */
void computeLinfinityNeighborhood( D2<SBasis > const &f, double tol, D2<Piecewise<SBasis> > &topside, D2<Piecewise<SBasis> > &botside ){
	double signx = ( f[X].at0() > f[X].at1() )? -1 : 1;
	double signy = ( f[Y].at0() > f[Y].at1() )? -1 : 1;

	Piecewise<D2<SBasis> > top, bot;
	top = Piecewise<D2<SBasis> > (f);
	top.cuts.insert( top.cuts.end(), 2);
	top.segs.insert( top.segs.end(), D2<SBasis>(Linear( f[X].at1(), f[X].at1()+2*tol*signx),
			                                    Linear( f[Y].at1() )) );
	bot = Piecewise<D2<SBasis> >(f);
	bot.cuts.insert( bot.cuts.begin(), - 1 );
	bot.segs.insert( bot.segs.begin(), D2<SBasis>(Linear( f[X].at0()-2*tol*signx, f[X].at0()),
												  Linear( f[Y].at0() )) );
	top += Point(-tol*signx,  tol);
	bot += Point( tol*signx, -tol);

	if ( signy < 0 ){
		std::swap( top, bot );
		top += Point( 0,  2*tol);
		bot += Point( 0, -2*tol);
	}
	topside = make_cuts_independent(top);
	botside = make_cuts_independent(bot);
}


/*Compute top and bottom boundaries of the L^infty nbhd of the graph of a *monotonic* function f.
 * if f is increasing, it is given by [f(t-tol)-tol, f(t+tol)+tol].
 * if not, it is [f(t+tol)-tol, f(t-tol)+tol].
 */
void computeLinfinityNeighborhood( Piecewise<SBasis> const &f, double tol, Piecewise<SBasis> &top, Piecewise<SBasis> &bot){
	top = f + tol;
	top.offsetDomain( - tol );
	top.cuts.insert( top.cuts.end(), f.domain().max() + tol);
	top.segs.insert( top.segs.end(), SBasis(Linear( f.lastValue() + tol )) );

	bot = f - tol;
	bot.offsetDomain( tol );
	bot.cuts.insert( bot.cuts.begin(), f.domain().min() - tol);
	bot.segs.insert( bot.segs.begin(), SBasis(Linear( f.firstValue() - tol )) );

	if ( f.firstValue() > f.lastValue() ){
	std::swap( top, bot );
	top += 2*tol;
	bot -= 2*tol;
	}
}

std::vector<Interval> level_set( D2<SBasis> const &f, Rect region){
	std::vector<Interval> x_in_reg = level_set( f[X], region[X] );
	std::vector<Interval> y_in_reg = level_set( f[Y], region[Y] );
	std::vector<Interval> result = intersect ( x_in_reg, y_in_reg );
	return result;
}

void prolongateByConstants( Piecewise<SBasis> &f, double paddle_width ){
	if ( f.size() == 0 ) return; //do we have a covention about the domain of empty pwsb?
	f.cuts.insert( f.cuts.begin(), f.cuts.front() - paddle_width );
	f.segs.insert( f.segs.begin(), SBasis( f.segs.front().at0() ) );
	f.cuts.insert( f.cuts.end(), f.cuts.back() + paddle_width );
	f.segs.insert( f.segs.end(), SBasis( f.segs.back().at1() ) );
}



/* Returns the intervals over which the curve keeps its slope
 * in one of the 8 sectors delimited by x=0, y=0, y=x, y=-x.
 * WARNING: both curves are supposed to be a graphs over x or y axis,
 *     and the smaller the slopes the better (typically <=45°).
 */
std::vector<std::pair<Interval, Interval> > smash_intersect( D2<SBasis> const &a, D2<SBasis> const &b,
			double tol, cairo_t *cr , bool draw_more_stuff=false ){

	std::vector<std::pair<Interval, Interval> > res;

	// a and b or X and Y may have to be exchanged, so make local copies.
	D2<SBasis> aa = a;
	D2<SBasis> bb = b;
	bool swapresult = false;
	bool swapcoord = false;//debug only!

	if ( draw_more_stuff ){
		cairo_set_line_width (cr, 3);
		cairo_set_source_rgba(cr, .5, .9, .7, 1 );
		cairo_d2_sb(cr, aa);
		cairo_d2_sb(cr, bb);
		cairo_stroke(cr);
	}

#if 1
	//if the (enlarged) bounding boxes don't intersect, stop.
	if ( !draw_more_stuff ){
		OptRect abounds = bounds_fast( a );
		OptRect bbounds = bounds_fast( b );
		if ( !abounds || !bbounds ) return res;
		abounds->expandBy(tol);
		if ( !(abounds->intersects(*bbounds))){
			return res;
		}
	}
#endif

	//Choose the best curve to be re-parametrized by x or y values.
	OptRect dabounds = bounds_exact(derivative(a));
	OptRect dbbounds = bounds_exact(derivative(b));
	if	( dbbounds->min().length() > dabounds->min().length() ){
		aa=b;
		bb=a;
		std::swap( dabounds, dbbounds );
		swapresult = true;
	}

	//Choose the best coordinate to use as new parameter
	double dxmin = std::min( abs((*dabounds)[X].max()), abs((*dabounds)[X].min()) );
	double dymin = std::min( abs((*dabounds)[Y].max()), abs((*dabounds)[Y].min()) );
	if ( (*dabounds)[X].max()*(*dabounds)[X].min() < 0 ) dxmin=0;
	if ( (*dabounds)[Y].max()*(*dabounds)[Y].min() < 0 ) dymin=0;
	assert (dxmin>=0 && dymin>=0);

	if (dxmin < dymin) {
		aa = D2<SBasis>( aa[Y], aa[X] );
		bb = D2<SBasis>( bb[Y], bb[X] );
		swapcoord = true;
	}

	//re-parametrize aa by the value of x.
	Interval x_range_strict( aa[X].at0(), aa[X].at1() );
	Piecewise<SBasis> y_of_x = pw_compose_inverse(aa[Y],aa[X], 2, 1e-5);

	//Compute top and bottom boundaries of the L^infty nbhd of aa.
	Piecewise<SBasis> top_ay, bot_ay;
	computeLinfinityNeighborhood( y_of_x, tol, top_ay, bot_ay);

	Interval ax_range = top_ay.domain();//i.e. aa[X] domain ewpanded by tol.

	if ( draw_more_stuff ){
		Piecewise<SBasis> dbg_x( SBasis( Linear( top_ay.domain().min(), top_ay.domain().max() ) ) );
		dbg_x.setDomain( top_ay.domain() );
		D2<Piecewise<SBasis> > dbg_side ( Piecewise<SBasis>( SBasis( Linear(   0    ) ) ),
				                          Piecewise<SBasis>( SBasis( Linear( 0, 2*tol) ) ) );

		D2<Piecewise<SBasis> > dbg_rgn;
		unsigned h = ( swapcoord ) ? Y : X;
		dbg_rgn[h].concat ( dbg_x );
		dbg_rgn[h].concat ( dbg_side[X] + dbg_x.lastValue() );
		dbg_rgn[h].concat ( reverse(dbg_x) );
		dbg_rgn[h].concat ( dbg_side[X] + dbg_x.firstValue() );

		dbg_rgn[1-h].concat ( bot_ay );
		dbg_rgn[1-h].concat ( dbg_side[Y] + bot_ay.lastValue() );
		dbg_rgn[1-h].concat ( reverse(top_ay) );
		dbg_rgn[1-h].concat ( reverse( dbg_side[Y] ) + bot_ay.firstValue() );

		cairo_set_line_width (cr, 1.);
		cairo_set_source_rgba(cr, 0., 1., 0., .75 );
		cairo_d2_pw_sb(cr, dbg_rgn );
		cairo_stroke(cr);

		D2<SBasis> bbb = bb;
		if ( swapcoord ) std::swap( bbb[X], bbb[Y] );
		//Piecewise<D2<SBasis> > dbg_rgnB = neighborhood( bbb, tol );
		D2<Piecewise<SBasis> > dbg_topB, dbg_botB;
		computeLinfinityNeighborhood( bbb, tol, dbg_topB, dbg_botB );
		cairo_set_line_width (cr, 1.);
		cairo_set_source_rgba(cr, .2, 8., .2, .4 );
//		cairo_pw_d2_sb(cr, dbg_rgnB );
		cairo_d2_pw_sb(cr, dbg_topB );
		cairo_d2_pw_sb(cr, dbg_botB );
		cairo_stroke(cr);
	}

	std::vector<Interval> bx_in_ax_range = level_set(bb[X], ax_range );

	// find times when bb is in the neighborhood of aa.
	std::vector<Interval> tbs;
	for (unsigned i=0; i<bx_in_ax_range.size(); i++){
		D2<Piecewise<SBasis> > bb_in;
		bb_in[X] = Piecewise<SBasis> ( portion( bb[X], bx_in_ax_range[i] ) );
		bb_in[Y] = Piecewise<SBasis> ( portion( bb[Y], bx_in_ax_range[i]) );
		bb_in[X].setDomain( bx_in_ax_range[i] );
		bb_in[Y].setDomain( bx_in_ax_range[i] );

		Piecewise<SBasis> h;
		Interval level;
		h = bb_in[Y] - compose( top_ay, bb_in[X] );
		level = Interval( -infinity(), 0 );
		std::vector<Interval> rts_lo = level_set( h, level);
		h = bb_in[Y] - compose( bot_ay, bb_in[X] );
		level = Interval( 0, infinity());
		std::vector<Interval> rts_hi = level_set( h, level);

		std::vector<Interval> rts = intersect( rts_lo, rts_hi );
		tbs.insert(tbs.end(), rts.begin(),  rts.end()  );
	}

	std::vector<std::pair<Interval, Interval> > result(tbs.size(),std::pair<Interval,Interval>());

	/* for each solution I, find times when aa is in the neighborhood of bb(I).
	 * (Note: the preimage of bb[X](I) by aa[X], enlarged by tol, is a good approximation of this:
	 * it would give points in the 2*tol neighborhood of bb (if the slope of aa is never more than 1).
	 *  + faster computation.
	 *  - implies little jumps depending on the subdivision of the input curve into monotonic pieces
	 *  and on the choice of prefered axis. If noticable, these jumps would feel random to the user :-(
	 */
	for (unsigned j=0; j<tbs.size(); j++){
		result[j].second = tbs[j];
		std::vector<Interval> tas;
		Piecewise<SBasis> fat_y_of_x = y_of_x;
		prolongateByConstants( fat_y_of_x, 100*(1+tol) );

		D2<Piecewise<SBasis> > top_b, bot_b;
		D2<SBasis> bbj = portion( bb, tbs[j] );
		computeLinfinityNeighborhood( bbj, tol, top_b, bot_b );

		Piecewise<SBasis> h;
		Interval level;
		h = top_b[Y] - compose( fat_y_of_x, top_b[X] );
		level = Interval( +infinity(), 0 );
		std::vector<Interval> rts_top = level_set( h, level);
		for (unsigned idx=0; idx < rts_top.size(); idx++){
			rts_top[idx] = Interval( top_b[X].valueAt( rts_top[idx].min() ),
									 top_b[X].valueAt( rts_top[idx].max() ) );
		}
		assert( rts_top.size() == 1 );

		h = bot_b[Y] - compose( fat_y_of_x, bot_b[X] );
		level = Interval( 0, -infinity());
		std::vector<Interval> rts_bot = level_set( h, level);
		for (unsigned idx=0; idx < rts_bot.size(); idx++){
			rts_bot[idx] = Interval( bot_b[X].valueAt( rts_bot[idx].min() ),
									 bot_b[X].valueAt( rts_bot[idx].max() ) );
		}
		assert( rts_bot.size() == 1 );

#if VERBOSE
		printf("range(aa[X]) = [%f, %f];\n", y_of_x.domain().min(), y_of_x.domain().max());
		printf("range(bbj[X]) = [%f, %f]; tol= %f\n", bbj[X].at0(), bbj[X].at1(), tol);

		printf("rts_top = ");
		for (unsigned dbgi=0; dbgi<rts_top.size(); dbgi++){
			printf("[%f,%f]U", rts_top[dbgi].min(), rts_top[dbgi].max() );
		}
		printf("\n");
		printf("rts_bot = ");
		for (unsigned dbgi=0; dbgi<rts_bot.size(); dbgi++){
			printf("[%f,%f]U", rts_bot[dbgi].min(), rts_bot[dbgi].max() );
		}
		printf("\n");
#endif
		rts_top = intersect( rts_top, rts_bot );
#if VERBOSE
		printf("intersection = ");
		for (unsigned dbgi=0; dbgi<rts_top.size(); dbgi++){
			printf("[%f,%f]U", rts_top[dbgi].min(), rts_top[dbgi].max() );
		}
		printf("\n\n");

		if (rts_top.size() != 1){
			printf("!!!!!!!!!!!!!!!!!!!!!!\n!!!!!!!!!!!!!!!!!!!!!!\n");
			rts_top[0].unionWith( rts_top[1] );
			assert( false );
		}
#endif
		assert (rts_top.size() == 1);
		Interval x_dom = rts_top[0];

		if ( x_dom.max() <= x_range_strict.min() ){
			tas.push_back( Interval ( ( aa[X].at0() < aa[X].at1() ) ? 0 : 1 ) );
		}else if ( x_dom.min() >= x_range_strict.max() ){
			tas.push_back( Interval ( ( aa[X].at0() < aa[X].at1() ) ? 1 : 0 ) );
		}else{
			tas = level_set(aa[X], x_dom );
		}

#if VERBOSE
		if ( tas.size() != 1 ){
			printf("Error: preimage of [%f, %f] by x:[0,1]->[%f, %f] is ",
					x_dom.min(), x_dom.max(), x_range_strict.min(), x_range_strict.max());
			if ( tas.size() == 0 ){
				printf( "empty.\n");
			}else{
				printf("\n   [%f,%f]", tas[0].min(), tas[0].max() );
				for (unsigned toto=1; toto<tas.size(); toto++){
					printf(" U [%f,%f]", tas[toto].min(), tas[toto].max() );
				}
			}
		}
#endif
		assert( tas.size()==1 );
		result[j].first = tas.front();
	}

	if (swapresult) {
		for ( unsigned i=0; i<result.size(); i++){
			Interval temp = result[i].first;
			result[i].first = result[i].second;
			result[i].second = temp;
		}
	}
	return result;
}

#endif

class Intersector : public Toy
{
  private:
	void draw( cairo_t *cr,	std::ostringstream *notify,
    		   int width, int height, bool save, std::ostringstream *timer_stream) 
    {
        double tol = exp_rescale(slider.value());
    	D2<SBasis> A = handles_to_sbasis(psh.pts.begin(), A_bez_ord-1);
        D2<SBasis> B = handles_to_sbasis(psh.pts.begin()+A_bez_ord, B_bez_ord-1);
    	cairo_set_line_width (cr, .8);
    	cairo_set_source_rgba(cr,0.,0.,0.,.6);
        cairo_d2_sb(cr, A);
        cairo_d2_sb(cr, B);
    	cairo_stroke(cr);

    	Rect tolbytol( anchor.pos, anchor.pos );
    	tolbytol.expandBy( tol );
        cairo_rectangle(cr, tolbytol);
    	cairo_stroke(cr);
/*
		Piecewise<D2<SBasis> > smthA = linearizeCusps(A+Point(0,10), tol);
        cairo_set_line_width (cr, 1.);
        cairo_set_source_rgba(cr, 1., 0., 1., 1. );
        cairo_pw_d2_sb(cr, smthA);
    	cairo_stroke(cr);
*/

        std::vector<Interval> Acuts = monotonicSplit(A);
        std::vector<Interval> Bcuts = monotonicSplit(B);

#if 0
        for (unsigned i=0; i<Acuts.size(); i++){
        	D2<SBasis> Ai = portion( A, Acuts[i]);
            cairo_set_line_width (cr, .2);
            cairo_set_source_rgba(cr, 0., 0., 0., 1. );
            draw_cross(cr, Ai.at0());
        	cairo_stroke(cr);
        	for (unsigned j=0; j<Bcuts.size(); j++){
            	std::vector<std::pair<Interval, Interval> > my_intersections;
            	D2<SBasis> Bj = portion( B, Bcuts[j]);
                cairo_set_line_width (cr, .2);
                cairo_set_source_rgba(cr, 0., 0., 0., 1. );
                draw_cross(cr, Bj.at0());
            	cairo_stroke(cr);
        	}
        }
#endif

		std::vector<Intersection> my_intersections;
     	my_intersections = smash_intersect( A, B, tol );

     	for (unsigned k=0; k<my_intersections.size(); k++){
     		cairo_set_line_width (cr, 2.5);
            cairo_set_source_rgba(cr, 1., 0., 0., .8 );
            cairo_d2_sb(cr, portion( A, my_intersections[k].times[X]));
        	cairo_stroke(cr);
            cairo_set_line_width (cr, 2.5);
            cairo_set_source_rgba(cr, 0., 0., 1., .8 );
            cairo_d2_sb(cr, portion( B, my_intersections[k].times[Y]));
        	cairo_stroke(cr);
     	}
#if 0

        unsigned apiece( slidera.value()/100. * Acuts.size() );
        unsigned bpiece( sliderb.value()/100. * Bcuts.size() );


        for (unsigned i=0; i<Acuts.size(); i++){
        	D2<SBasis> Ai = portion( A, Acuts[i]);
        	for (unsigned j=0; j<Bcuts.size(); j++){
            	if ( toggle.on &&  (i != apiece || j != bpiece) ) continue;

        		std::vector<Intersection> my_intersections;
            	D2<SBasis> Bj = portion( B, Bcuts[j]);
            	bool draw_more = toggle.on &&  i == apiece && j == bpiece;
//             	my_intersections = smash_intersect( Ai, Bj, tol, cr, draw_more );
             	my_intersections = monotonic_smash_intersect( Ai, Bj, tol );

             	for (unsigned k=0; k<my_intersections.size(); k++){
             		cairo_set_line_width (cr, 2.5);
                    cairo_set_source_rgba(cr, 1., 0., 0., .8 );
                    cairo_d2_sb(cr, portion( Ai, my_intersections[k].times[X]));
                	cairo_stroke(cr);
                    cairo_set_line_width (cr, 2.5);
                    cairo_set_source_rgba(cr, 0., 0., 1., .8 );
                    cairo_d2_sb(cr, portion( Bj, my_intersections[k].times[Y]));
                	cairo_stroke(cr);
             	}
            }
        }
#endif
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
	
  public:
    Intersector(unsigned int _A_bez_ord, unsigned int _B_bez_ord)
		: A_bez_ord(_A_bez_ord), B_bez_ord(_B_bez_ord)
	{
    	unsigned int total_handles = A_bez_ord + B_bez_ord;
		for ( unsigned int i = 0; i < total_handles; ++i )
			psh.push_back(Geom::Point(uniform()*400, uniform()*400));
		handles.push_back(&psh);
		slider = Slider(-4, 2, 0, 1.2, "tolerance");
		slider.geometry(Point(30, 20), 250);
		slider.formatter(&exp_formatter);
		handles.push_back(&slider);
		slidera = Slider(0, 100, 1, 0., "piece on A");
		slidera.geometry(Point(300, 50), 250);
		handles.push_back(&slidera);
		sliderb = Slider(0, 100, 1, 0., "piece on B");
		sliderb.geometry(Point(300, 80), 250);
		handles.push_back(&sliderb);
		toggle = Toggle( Rect(Point(300,10), Point(440,30)), "Piece by piece", false );
		handles.push_back(&toggle);
		anchor = PointHandle ( Point(100, 100 ) );
		handles.push_back(&anchor);
	}
	
  private:
	unsigned int A_bez_ord;
	unsigned int B_bez_ord;
	PointSetHandle psh;
	PointHandle anchor;
	Slider slider,slidera,sliderb;
	Toggle toggle;
};


int main(int argc, char **argv) 
{	
	unsigned int A_bez_ord=4;
	unsigned int B_bez_ord=4;
    if(argc > 2)
        sscanf(argv[2], "%d", &B_bez_ord);
    if(argc > 1)
        sscanf(argv[1], "%d", &A_bez_ord);

    init( argc, argv, new Intersector(A_bez_ord, B_bez_ord));
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
