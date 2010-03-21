/*
 * Diffeomorphism-based intersector: given two curves
 *  M(t)=(x(t),y(t)) and N(u)=(x'(u),y'(u))
 * and supposing M is a graph over the x-axis, we compute y(x) and solve
 *  y'(u) - y(x'(u)) = 0
 * to get the intersections of the two curves...
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

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <cstdlib>
#include <cstdio>
#include <set>
#include <vector>
#include <algorithm>


using namespace Geom;

static double exp_rescale(double x){ return pow(10, x);}
std::string exp_formatter(double x){ return default_formatter(exp_rescale(x));}



std::vector<Interval> level_set(Piecewise<SBasis> const &f, Interval const &level, double tol=1e-3){

    std::vector<Interval> result;
    for (unsigned i=0; i<f.size(); i++){
        std::vector<Interval> resulti = level_set( f[i], level, 0., 1., tol);
        for (unsigned j=0; j<resulti.size(); j++){
        	Interval domj( f.mapToDomain(resulti[j].min(), i ), f.mapToDomain(resulti[j].max(), i ) );
        	if ( j==0 && result.size() > 0 && result.back().intersects(domj) ){
        		result.back().unionWith(domj);
        	}else{
        		result.push_back(domj);
        	}
        }
    }
    return result;
}
std::vector<Interval> level_set(Piecewise<SBasis> const &f, double v, double vtol, double tol=1e-3){
	Interval level ( v-vtol, v+vtol );
	return level_set( f, level, tol);
}


//returns the intervals on which either x or y is a graph over t.
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

	//we could split in the middle to avoid computing roots again...
	someroots = roots(v[X]-v[Y]);
	cuts.insert( cuts.end(), someroots.begin(), someroots.end() );

	someroots = roots(v[X]+v[Y]);
	cuts.insert( cuts.end(), someroots.begin(), someroots.end() );

	sort(cuts.begin(),cuts.end());
	unique(cuts.begin(), cuts.end() );

	for (unsigned i=1; i<cuts.size(); i++){
//		double middle = (cuts[i-1] + cuts[i])/2;
//		result.push_back( Interval( cuts[i-1], middle  ) );
//		result.push_back( Interval( middle   , cuts[i] ) );
		result.push_back( Interval( cuts[i-1], cuts[i] ) );
	}
	return result;
}

#if 0
std::vector<std::pair<double, double> > intersect( D2<SBasis> const &a, D2<SBasis> const &b ){

	std::vector<std::pair<double, double> > res;

	D2<SBasis> aa = a;
	D2<SBasis> bb = b;
	bool swapresult = false;

	OptRect dabounds = bounds_exact(derivative(a));
	OptRect dbbounds = bounds_exact(derivative(b));
	if	( dbbounds->min().length() > dabounds->min().length() ){
		aa=b;
		bb=a;
		std::swap( dabounds, dbbounds );
		swapresult = true;
	}

	double dxmin = std::min( abs((*dabounds)[X].max()), abs((*dabounds)[X].min()) );
	double dymin = std::min( abs((*dabounds)[Y].max()), abs((*dabounds)[Y].min()) );
	if ( (*dabounds)[X].max()*(*dabounds)[X].min() < 0 ) dxmin=0;
	if ( (*dabounds)[Y].max()*(*dabounds)[Y].min() < 0 ) dymin=0;
	assert (dxmin>=0 && dymin>=0);

	if (dxmin < dymin) {
		aa = D2<SBasis>( aa[Y], aa[X] );
		bb = D2<SBasis>( bb[Y], bb[X] );
	}

	Piecewise<SBasis> y_of_x = pw_compose_inverse(aa[Y],aa[X], 2, 1e-3);

	Interval x_range( aa[X].at0(), aa[X].at1() );
	std::vector<double> left = roots(bb[X] - aa[X].at0() );
	std::vector<double> right = roots(bb[X] - aa[X].at1() );
	std::vector<double> limits(2,0.);
	limits[1]=1.;
	limits.insert(limits.end(), left.begin(),  left.end()  );
	limits.insert(limits.end(), right.begin(), right.end() );
	std::sort(limits.begin(), limits.end());
	std::unique(limits.begin(), limits.end());

	std::vector<double> tbs;

	for (unsigned i=1; i<limits.size(); i++){
		Interval dom(limits[i-1],limits[i]);
		double t = dom.middle();
		if ( !x_range.contains( bb[X].valueAt(t) ) ) continue;
		D2<Piecewise<SBasis> > bb_in;
		bb_in[X] = Piecewise<SBasis> ( portion( bb[X], dom ) );
		bb_in[Y] = Piecewise<SBasis> ( portion( bb[Y], dom ) );
		bb_in[X].setDomain( dom );
		bb_in[Y].setDomain( dom );
		Piecewise<SBasis> h = bb_in[Y] - compose( y_of_x, bb_in[X] );

		std::vector<double> rts = roots(h);
		tbs.insert(tbs.end(), rts.begin(),  rts.end()  );
	}

	std::vector<std::pair<double, double> > result(tbs.size(),std::pair<double,double>());
	for (unsigned j=0; j<tbs.size(); j++){
		result[j].second = tbs[j];
		std::vector<double> tas = roots(aa[X]-bb[X].valueAt(tbs[j]));
		assert( tas.size()==1 );
		result[j].first = tas.front();
	}

	if (swapresult) {
		for ( unsigned i=0; i<result.size(); i++){
			double temp = result[i].first;
			result[i].first = result[i].second;
			result[i].second = temp;
		}
	}
	return result;
}
#endif

std::vector<std::pair<Interval, Interval> > smash_intersect(cairo_t *cr, D2<SBasis> const &a, D2<SBasis> const &b, double tol=1e-3 ){

	std::vector<std::pair<Interval, Interval> > res;

	D2<SBasis> aa = a;
	D2<SBasis> bb = b;
	bool swapresult = false;

	OptRect dabounds = bounds_exact(derivative(a));
	OptRect dbbounds = bounds_exact(derivative(b));
	if	( dbbounds->min().length() > dabounds->min().length() ){
		aa=b;
		bb=a;
		std::swap( dabounds, dbbounds );
		swapresult = true;
	}

	double dxmin = std::min( abs((*dabounds)[X].max()), abs((*dabounds)[X].min()) );
	double dymin = std::min( abs((*dabounds)[Y].max()), abs((*dabounds)[Y].min()) );
	if ( (*dabounds)[X].max()*(*dabounds)[X].min() < 0 ) dxmin=0;
	if ( (*dabounds)[Y].max()*(*dabounds)[Y].min() < 0 ) dymin=0;
	assert (dxmin>=0 && dymin>=0);

	if (dxmin < dymin) {
		aa = D2<SBasis>( aa[Y], aa[X] );
		bb = D2<SBasis>( bb[Y], bb[X] );
	}

	Piecewise<SBasis> y_of_x = pw_compose_inverse(aa[Y],aa[X], 2, 1e-5);

	Interval x_range( aa[X].at0(), aa[X].at1() );

	std::vector<Interval> domains = level_set(bb[X], x_range );

	std::vector<Interval> tbs;

	for (unsigned i=0; i<domains.size(); i++){
		D2<Piecewise<SBasis> > bb_in;
		bb_in[X] = Piecewise<SBasis> ( portion( bb[X], domains[i] ) );
		bb_in[Y] = Piecewise<SBasis> ( portion( bb[Y], domains[i]) );
		bb_in[X].setDomain( domains[i] );
		bb_in[Y].setDomain( domains[i] );
		Piecewise<SBasis> h = bb_in[Y] - compose( y_of_x, bb_in[X] );

		std::vector<Interval> rts = level_set( h, 0., tol);
		tbs.insert(tbs.end(), rts.begin(),  rts.end()  );
	}

	std::vector<std::pair<Interval, Interval> > result(tbs.size(),std::pair<Interval,Interval>());
	for (unsigned j=0; j<tbs.size(); j++){
		result[j].second = tbs[j];
		Interval x_dom = Interval( bb[X].valueAt( tbs[j].min() ),  bb[X].valueAt( tbs[j].max() ));

		if ( x_dom.extent() < tol ) x_dom +=Interval( -tol, tol );
		std::vector<Interval> tas = level_set(aa[X], x_dom );

		if ( tas.size() != 1 ){
			printf("Error: preimage of [%f, %f] by x:[0,1]->[%f, %f] is ",
					x_dom.min(), x_dom.max(), x_range.min(), x_range.max());
			if ( tas.size() == 0 ){
				printf( "empty.\n");
			}else{
				printf("\n   [%f,%f]", tas[0].min(), tas[0].max() );
				for (unsigned toto=1; toto<tas.size(); toto++){
					printf(" U [%f,%f]", tas[toto].min(), tas[toto].max() );
				}
			}
		}
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


class Intersector : public Toy
{
  private:
    void draw( cairo_t *cr,	std::ostringstream *notify, 
    		   int width, int height, bool save, std::ostringstream *timer_stream) 
    {
        double tol = exp_rescale(slider.value());
    	D2<SBasis> A = handles_to_sbasis(psh.pts.begin(), A_bez_ord-1);
        D2<SBasis> B = handles_to_sbasis(psh.pts.begin()+A_bez_ord, B_bez_ord-1);

        std::vector<Interval> Acuts = monotonicSplit(A);
        std::vector<Interval> Bcuts = monotonicSplit(B);
        for (unsigned i=0; i<Acuts.size(); i++){
            for (unsigned j=0; j<Bcuts.size(); j++){
            	std::vector<std::pair<Interval, Interval> > my_intersections;
            	D2<SBasis> Ai = portion( A, Acuts[i]);
            	D2<SBasis> Bj = portion( B, Bcuts[j]);
             	my_intersections = smash_intersect( cr, Ai, Bj, tol);
             	for (unsigned k=0; k<my_intersections.size(); k++){
             		Point p = Ai.valueAt(my_intersections[k].first.middle());

                    cairo_set_line_width (cr, 2.);
                    cairo_set_source_rgba(cr, 1., 0., 0., 1. );
                    cairo_d2_sb(cr, portion( Ai, my_intersections[k].first));
                	cairo_stroke(cr);
                    cairo_set_line_width (cr, 4.);
                    cairo_set_source_rgba(cr, 0., 0., 1., 1. );
                    cairo_d2_sb(cr, portion( Bj, my_intersections[k].second));
                	cairo_stroke(cr);
             	}
            }
        }
    	cairo_set_line_width (cr, 1.);
    	cairo_set_source_rgba(cr,0.,0.,0.,.6);
        cairo_d2_sb(cr, A);
        cairo_d2_sb(cr, B);
    	cairo_stroke(cr);
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
		slider = Slider(-4, 2, 0, 0., "tolerance");
		slider.geometry(Point(50, 20), 250);
		slider.formatter(&exp_formatter);
		handles.push_back(&slider);
	}
	
  private:
	unsigned int A_bez_ord;
	unsigned int B_bez_ord;
	PointSetHandle psh;
	Slider slider;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
