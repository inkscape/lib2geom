/*
 * Nearest Points Toy 3
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


#include "d2.h"
#include "sbasis.h"
#include "path.h"
#include "bezier-to-sbasis.h"
#include "sbasis-geometric.h"
#include "piecewise.h"
#include "path-intersection.h"

#include "path-cairo.h"
#include "toy-framework-2.h"

#include <algorithm>


using namespace Geom;


class np_finder
{
public:
	np_finder(cairo_t* _cr, D2<SBasis> const& _c1, D2<SBasis> const& _c2)
		: cr(_cr), cc1(_c1), cc2(_c2), c1(_c1), c2(_c2)
	{
		
		dc1 = derivative(_c1);
		dc2 = derivative(_c2);
		cd1 = dot(_c1,dc1);
		cd2 = dot(_c2,dc2);
		dsq = 10e30;
		
		Piecewise< D2<SBasis> > uv1 = unitVector(dc1, EPSILON);
		Piecewise< D2<SBasis> > uv2 = unitVector(dc2, EPSILON);
		
		dcn1 = dot(Piecewise< D2<SBasis> >(dc1), uv1);
		dcn2 = dot(Piecewise< D2<SBasis> >(dc2), uv2);
		
		r_dcn1 = cross(derivative(uv1), uv1);
		r_dcn2 = cross(derivative(uv2), uv2);
				
		k1 = Geom::divide(r_dcn1, dcn1, EPSILON, 3);
		k2 = Geom::divide(r_dcn2, dcn2, EPSILON, 3);
		

		n1 = divide(rot90(uv1), k1, EPSILON, 3);
		n2 = divide(rot90(uv2), k2, EPSILON, 3);
		
		std::vector<double> cuts1, cuts2;

		// add cuts at points where the curvature is discontinuos
		for ( unsigned int i = 1; i < k1.size(); ++i )
		{
			if( !are_near(k1[i-1].at1(), k1[i].at0()) )
			{
				cuts1.push_back(k1.cuts[i]);
			}
		}
		for ( unsigned int i = 1; i < k2.size(); ++i )
		{
			if( !are_near(k2[i-1].at1(), k2[i].at0()) )
			{
				cuts2.push_back(k2.cuts[i]);
			}
		}
		
		c1 = partition(c1, cuts1);
		c2 = partition(c2, cuts2);
		
//		std::cerr << "# k1 discontinuitis" << std::endl;
//		for( unsigned int i = 0; i < cuts1.size(); ++i )
//		{
//			std::cerr << "[" << i << "]= " << cuts1[i] << std::endl;
//		}
//		std::cerr << "# k2 discontinuitis" << std::endl;
//		for( unsigned int i = 0; i < cuts2.size(); ++i )
//		{
//			std::cerr << "[" << i << "]= " << cuts2[i] << std::endl;
//		}		
		
		// add cuts at points were the curvature is zero
		std::vector<double> k1_roots = roots(k1);
		std::vector<double> k2_roots = roots(k2);
		std::sort(k1_roots.begin(), k1_roots.end());
		std::sort(k2_roots.begin(), k2_roots.end());
		c1 = partition(c1, k1_roots);
		c2 = partition(c2, k2_roots);
		
//		std::cerr << "# k1 zeros" << std::endl;
//		for( unsigned int i = 0; i < k1_roots.size(); ++i )
//		{
//			std::cerr << "[" << i << "]= " << k1_roots[i] << std::endl;
//		}
//		std::cerr << "# k2 zeros" << std::endl;
//		for( unsigned int i = 0; i < k2_roots.size(); ++i )
//		{
//			std::cerr << "[" << i << "]= " << k2_roots[i] << std::endl;
//		}
		
		
		cairo_set_line_width(cr, 0.2);
//		cairo_set_source_rgba(cr, 0.0, 0.5, 0.0, 1.0);
//		for( unsigned int i = 1; i < c1.size(); ++i )
//		{
//			draw_circ(cr, c1[i].at0() );
//		}
//		for( unsigned int i = 1; i < c2.size(); ++i )
//		{
//			draw_circ(cr, c2[i].at0() );
//		}
		
		
		// add cuts at nearest points to the other curve cuts points
		cuts1.clear();
		cuts1.reserve(c1.size()+1);
		for ( unsigned int i = 0; i < c1.size(); ++i )
		{
			cuts1.push_back( nearest_point(c1[i].at0(), _c2, dc2, cd2) );
		}
		cuts1.push_back( nearest_point(c1[c1.size()-1].at1(), _c2, dc2, cd2) );
		
//		for ( unsigned int i = 0; i < c1.size(); ++i )
//		{
//			cairo_move_to( cr, c1[i].at0() );
//			cairo_line_to(cr, c2(cuts1[i]) );
//		}
//		cairo_move_to( cr, c1[c1.size()-1].at1() );
//		cairo_line_to(cr, c2(cuts1[c1.size()]));
		
		std::sort(cuts1.begin(), cuts1.end());
		
		cuts2.clear();
		cuts2.reserve(c2.size()+1);
		for ( unsigned int i = 0; i < c2.size(); ++i )
		{
			cuts2.push_back( nearest_point(c2[i].at0(), _c1, dc1, cd1) );
		}
		cuts2.push_back( nearest_point(c2[c2.size()-1].at1(), _c1, dc1, cd1) );
		
//		for ( unsigned int i = 0; i < c2.size(); ++i )
//		{
//			cairo_move_to( cr, c2[i].at0() );
//			cairo_line_to(cr, c1(cuts2[i]) );
//		}
//		cairo_move_to( cr, c2[c2.size()-1].at1() );
//		cairo_line_to(cr, c1(cuts2[c2.size()]));
//		cairo_stroke(cr);
		
		std::sort(cuts2.begin(), cuts2.end());
		
		c1 = partition(c1, cuts2);
		c2 = partition(c2, cuts1);
		
		
		// copy curve to preserve cuts status  
		Piecewise< D2<SBasis> > pwc1 = c1;
		n1 = partition(n1, pwc1.cuts);
		pwc1 = partition(pwc1, n1.cuts);
		r_dcn1 = partition(r_dcn1, n1.cuts);
		Piecewise< D2<SBasis> > pwc2 = c2;
		n2 = partition(n2, pwc2.cuts);
		pwc2 = partition(pwc2, n2.cuts);
		
		assert( pwc1.size() == n1.size() );
		assert( pwc2.size() == n2.size() );
		assert( r_dcn1.size() == n1.size() );
		
		// add cuts at curvature max and min points
		Piecewise<SBasis> dk1 = derivative(k1);
		Piecewise<SBasis> dk2 = derivative(k2);
		std::vector<double> dk1_roots = roots(dk1);
		std::vector<double> dk2_roots = roots(dk2);
		std::sort(dk1_roots.begin(), dk1_roots.end());
		std::sort(dk2_roots.begin(), dk2_roots.end());
		
		c1 = partition(c1, dk1_roots);
		c2 = partition(c2, dk2_roots);

//		std::cerr << "# k1 min/max" << std::endl;
//		for( unsigned int i = 0; i < dk1_roots.size(); ++i )
//		{
//			std::cerr << "[" << i << "]= " << dk1_roots[i] << std::endl;
//		}
//		std::cerr << "# k2 min/max" << std::endl;
//		for( unsigned int i = 0; i < dk2_roots.size(); ++i )
//		{
//			std::cerr << "[" << i << "]= " << dk2_roots[i] << std::endl;
//		}
		
//		cairo_set_source_rgba(cr, 0.0, 0.0, 0.6, 1.0);
//		for( unsigned int i = 0; i < dk1_roots.size(); ++i )
//		{
//			draw_handle(cr, c1(dk1_roots[i]));
//		}
//		for( unsigned int i = 0; i < dk2_roots.size(); ++i )
//		{
//			draw_handle(cr, c2(dk2_roots[i]));
//		}
		
		
		// add cuts at nearest points to max and min curvature points 
		// of the other curve
		cuts1.clear();
		cuts1.reserve(dk2_roots.size());
		for ( unsigned int i = 0; i < dk2_roots.size(); ++i )
		{
			cuts1.push_back(nearest_point(_c2(dk2_roots[i]), _c1, dc1, cd1));
		}
		
//		for( unsigned int i = 0; i < dk2_roots.size(); ++i )
//		{
//			cairo_move_to(cr, c2(dk2_roots[i]));
//			cairo_line_to(cr, c1(cuts1[i]));
//		}
//		cairo_stroke(cr);

		std::sort(cuts1.begin(), cuts1.end());
		c1 = partition(c1, cuts1);
		
		
		// swap normal vector direction and fill the skip list
		skip_list.clear();
		skip_list.resize(c1.size(), false);
		double npt;
		Point p, nv;
		unsigned int si;
		for ( unsigned int i = 0; i < pwc1.size(); ++i )
		{
			p = pwc1[i](0.5);
			nv = n1[i](0.5);
			npt = nearest_point(p, _c2, dc2, cd2);
			if( dot( _c2(npt) - p, nv ) > 0 )
			{
				if ( dot( nv, n2(npt) ) > 0 )
				{
					n1[i] = -n1[i];
					r_dcn1[i] = -r_dcn1[i];
				}
				else
				{
					si = c1.segN( n1.mapToDomain(0.5, i) );
					skip_list[si] = true;
				}
			}
		}
		
		
		for ( unsigned int i = 0; i < pwc2.size(); ++i )
		{
			p = pwc2[i](0.5);
			nv = n2[i](0.5);
			npt = nearest_point(p, _c1, dc1, cd1);
			if( dot( _c1(npt) - p, nv ) > 0 )
			{
				if ( dot( nv, n1(npt) ) > 0 )
				{
					n2[i] = -n2[i];
				}
			}
		}
		
		
		evl1 = c1 + n1;
		evl2 = c2 + n2;

//		cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
//		for ( unsigned int i = 0; i < c1.size(); ++i )
//		{
//			double t = c1.mapToDomain(0.5, i);
//			cairo_move_to(cr, c1(t));
//			cairo_line_to(cr, c1(t) + 30*unit_vector(n1(t)));
//		}
//		
//		for ( unsigned int i = 0; i < c2.size(); ++i )
//		{
//			double t = c2.mapToDomain(0.5, i);
//			cairo_move_to(cr, c2(t));
//			cairo_line_to(cr, c2(t) + 30*unit_vector(n2(t)));
//		}
//		cairo_stroke(cr);
		
		std::cerr << "# skip list: ";
		for( unsigned int i = 0; i < c1.cuts.size(); ++i )
		{
			if ( skip_list[i] )
				std::cerr << i << "  ";
		}
		std::cerr << std::endl;
		
		cairo_set_line_width(cr, 0.4);
		cairo_set_source_rgba(cr, 0.6, 0.0, 0.0, 1.0);
		for( unsigned int i = 0; i < c1.size(); ++i )
		{
			if ( skip_list[i] )
			{
				cairo_move_to(cr, c1[i].at0());
				cairo_line_to(cr, c1[i].at1());
			}
		}
		cairo_stroke(cr);
		
		cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
		for( unsigned int i = 1; i < c1.size(); ++i )
		{
			draw_circ(cr, c1[i].at0() );
		}
		cairo_stroke(cr);
		
		std::cerr << "# c1 cuts: " << std::endl;
		for( unsigned int i = 0; i < c1.cuts.size(); ++i )
		{
			std::cerr << "c1.cuts[" << i << "]= " << c1.cuts[i] << std::endl;
		}
		
	}

	void operator() ()
	{
		nearest_points_impl();
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
	void nearest_points_impl()
	{		
		double t;
		for ( unsigned int i = 0; i < c1.size(); ++i )
		{
			if ( skip_list[i] ) continue;
			std::cerr << i << " ";
			t = c1.mapToDomain(0.5, i);
			std::pair<double, double> npc = loc_nearest_points(t, c1.cuts[i], c1.cuts[i+1]);
			if ( npc.second != -1 && dsq > L2sq(c1(npc.first) - c2(npc.second)) )
			{
				t1 = npc.first;
				t2 = npc.second;
				p1 = c1(t1);
				p2 = c2(t2);
				dsq = L2sq(p1 - p2);
			}
		}
	}
		
	std::pair<double, double> 
	loc_nearest_points( double t, double from = 0, double to = 1 )
	{
		std::cerr << "[" << from << "," << to << "] t: " << t << std::endl;
		unsigned int iter = 0, iter1 = 0, iter2 = 0;
		std::pair<double, double> np(-1,-1);
		std::pair<double, double> npf(from, -1);
		std::pair<double, double> npt(to, -1);
		double ct = t;
		double pt = -1;
		double s = nearest_point(c1(t), cc2, dc2, cd2);
		cairo_set_source_rgba(cr, 1/(t+1), t*t, t, 1.0);
		cairo_move_to(cr, c1(t));
		while( !are_near(ct, pt) && iter < 1000 )
		{
			pt = ct;
			double angle = angle_between( n1(ct), evl2(s) - evl1(ct) );
			assert( !IS_NAN(angle) );
			angle = (angle > 0) ? angle - M_PI : angle + M_PI;
			if ( std::fabs(angle) < M_PI/12 )
			{
				++iter2;
//				cairo_move_to(cr, c1(ct));
//				cairo_line_to(cr, evl1(ct));
//				cairo_line_to(cr, evl2(s));
				//std::cerr << "s: " << s << std::endl;
				//std::cerr << "t: " << ct << std::endl;

				ct = ct + angle / r_dcn1(ct);
				s = nearest_point(c1(ct), cc2, dc2, cd2);
//				angle = angle_between( n2(s), evl1(ct) - evl2(s) );
//				assert( !IS_NAN(angle) );
//				angle = (angle > 0) ? angle - M_PI : angle + M_PI;
//				s = s + angle / (dcn2(s) * k2(s));
			}
			else
			{
				++iter1;
				ct = nearest_point(c2(s), cc1, dc1, cd1, from, to);
				s = nearest_point(c1(ct), cc2, dc2, cd2);
			}
			iter = iter1 + iter2;
			//std::cerr << "s: " << s << std::endl;
			//std::cerr << "t: " << ct << std::endl;
	        //cairo_line_to(cr, c2(s));
			//cairo_line_to(cr, c1(ct));
			//std::cerr << "d(pt, ct) = " << std::fabs(ct - pt) << std::endl;
			if ( ct < from ) 
			{
				std::cerr << "break left" << std::endl;
				np = npf;
				break;
			}
			if ( ct > to ) 
			{
				std::cerr << "break right" << std::endl;
				np =npt;
				break;
			}
		}
		//std::cerr << "\n \n";
		std::cerr << "iterations: " << iter1 << " + " << iter2 << " = "<<  iter << std::endl;
		assert(iter < 3000);
		//cairo_move_to(cr, c1(ct));
		//cairo_line_to(cr, c2(s));
		cairo_stroke(cr);
		np.first = ct;
		np.second = s;
		return np;
	}
	
	double nearest_point( Point const& p, D2<SBasis> const&c, D2<SBasis> const& dc, SBasis const& cd, double from = 0, double to = 1 )
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
	
	Piecewise< D2<SBasis> >
	divide( Piecewise< D2<SBasis> > const& a, Piecewise<SBasis> const& b, double tol, unsigned int k, double zero=1.e-3)
	{
		D2< Piecewise<SBasis> > aa = make_cuts_independant(a);
		D2< Piecewise<SBasis> > q(Geom::divide(aa[0], b, tol, k, zero), Geom::divide(aa[1], b, tol, k, zero));
		return sectionize(q);
	}
		
	struct are_near_
	{
		bool operator() (double x, double y, double eps = Geom::EPSILON )
		{
			return are_near(x, y, eps);
		}
	};
	
private:
	static const Coord EPSILON = 1e-5;
	cairo_t* cr;
	D2<SBasis> const& cc1, cc2;
	Piecewise< D2<SBasis> > c1, c2;
	D2<SBasis> dc1, dc2;
	SBasis cd1, cd2;
	Piecewise< D2<SBasis> > n1, n2, evl1, evl2;
	Piecewise<SBasis> k1, k2, dcn1, dcn2, r_dcn1, r_dcn2;
	double t1, t2, d, dsq;
	Point p1, p2;
	std::vector<bool> skip_list;
};




class NearestPoints : public Toy
{
  private:
    void draw( cairo_t *cr,	std::ostringstream *notify, 
    		   int width, int height, bool save ) 
    {
    	cairo_set_line_width (cr, 0.3);
        D2<SBasis> A = pshA.asBezier();
        cairo_md_sb(cr, A);
        D2<SBasis> B = pshB.asBezier();
        cairo_md_sb(cr, B);
        cairo_stroke(cr);
        
        np_finder np(cr, A, B);
        Path AP, BP;
        AP.append(A); BP.append(B);
        Crossings ip_list = curve_sweep<SimpleCrosser>(AP, BP);
        if( ip_list.empty() )
        {
	        np();
	        cairo_set_line_width (cr, 0.4);
	        cairo_set_source_rgba(cr, 0.7, 0.0, 0.7, 1.0);
	        cairo_move_to(cr, np.firstPoint());
	        cairo_line_to(cr, np.secondPoint());
	        cairo_stroke(cr);
	        //std::cerr << "np: (" << np.firstValue() << "," << np.secondValue() << ")" << std::endl;
        }
    	Toy::draw(cr, notify, width, height, save);
    }
	
  public:
	NearestPoints(unsigned int _A_bez_ord, unsigned int _B_bez_ord)
		: A_bez_ord(_A_bez_ord), B_bez_ord(_B_bez_ord)
	{
	    handles.push_back(&pshA);
	    handles.push_back(&pshB);
		for ( unsigned int i = 0; i < A_bez_ord; ++i )
		    pshA.push_back(Geom::Point(uniform()*400, uniform()*400));
	    for ( unsigned int i = 0; i < B_bez_ord; ++i )
	        pshB.push_back(Geom::Point(uniform()*400, uniform()*400));

	}
	
  private:
    PointSetHandle pshA, pshB;
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
