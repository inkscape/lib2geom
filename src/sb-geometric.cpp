#include "sb-geometric.h"
#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "solver.h"
#include "s-basis-2d.h"
#include "sb-geometric.h"

/** Geometric operators on D2<SBasis> (1D->2D).
 * Copyright 2007 JF Barraud
 * Copyright 2007 N Hurst
 *
 * The functions defined in this header related to 2d geometric operations such as arc length,
 * unit_vector, curvature, and centroid.  Most are built on top of unit_vector, which takes an
 * arbitrary D2 and returns an D2 with unit length with the same direction.
 *
 * Todo/think about:
 *  arclength D2 -> sbasis (giving arclength function)
 *  does uniform_speed return natural parameterisation?
 *  integrate sb2d code from normal-bundle
 *  angle(md<2>) -> sbasis (gives angle from vector - discontinuous?)
 *  osculating circle center?
 *  
 **/

//namespace Geom{
using namespace Geom;
using namespace std;

//Useful? Using point_at(M,0) is not much slower!...
static 
Point point_at0(D2<SBasis> M){
    return Point((M[0].size()==0) ? 0 : M[0][0][0], 
                 (M[1].size()==0) ? 0 : M[1][0][0]);
}

static 
Point point_at1(D2<SBasis> M){
    return Point((M[0].size()==0) ? 0 : M[0][0][1], 
                 (M[1].size()==0) ? 0 : M[1][0][1]);
}

static 
Point deriv_at0(D2<SBasis> M){
    if (sbasisSize(M) == 0) {
        return Point(0, 0);
    }else{
        Coord x0=(M[0].size()<1)?0:M[0][0][0];
        Coord x1=(M[0].size()<1)?0:M[0][0][1];
        Coord xx=(M[0].size()<2)?0:M[0][1][0];
        Coord y0=(M[1].size()<1)?0:M[1][0][0];
        Coord y1=(M[1].size()<1)?0:M[1][0][1];
        Coord yy=(M[1].size()<2)?0:M[1][1][0];
        return Point(xx+(x1-x0),yy+(y1-y0));
    }
}

static 
Point deriv_at1(D2<SBasis> M){
    if (sbasisSize(M) == 0){
        return Point(0, 0);
    }else{
        Coord x0=(M[0].size()<1)?0:M[0][0][0];
        Coord x1=(M[0].size()<1)?0:M[0][0][1];
        Coord xx=(M[0].size()<2)?0:M[0][1][1];
        Coord y0=(M[1].size()<1)?0:M[1][0][0];
        Coord y1=(M[1].size()<1)?0:M[1][0][1];
        Coord yy=(M[1].size()<2)?0:M[1][1][1];
        return Point(-xx+(x1-x0),-yy+(y1-y0));
    }
}

std::vector<D2<SBasis> > Geom::unit_vector(D2<SBasis> const vect, std::vector<double> &cuts, double tol){
    std::vector<D2<SBasis> > uvect;
    SBasis alpha, c, s;
    D2<SBasis> V, dV;
    double alpha0, alpha1, dalpha0, dalpha1;

    V=vect;
    V[0].normalize();
    V[1].normalize();
    if (sbasisSize(V) == 0){
        return(uvect);
    }

    //--Compute angle jet at 0 and 1.
    bool notfound0=true, notfound1=true;
    Point V0,dV0,V1,dV1;
    while(sbasisSize(V) != 0 && (notfound0 || notfound1)){
        if(notfound0){
            V0 = point_at0(V);
            dV0 = deriv_at0(V);
            notfound0 = is_zero(point_at0(V));
        }
        if(notfound1){
            V1 = point_at1(V);
            dV1 = deriv_at1(V);
            notfound1 = is_zero(V1);
        }
        if(notfound0 || notfound1){
            V = derivative(V);
            V[0].normalize();
            V[1].normalize();
        }
    }
    alpha0 = atan(V0[1]/V0[0]);
    if (V0[0] < 0) alpha0+=M_PI;
    dalpha0 = -cross(V0, dV0) / L2sq(V0);//?!?! strange sign convention...
  
    alpha1 = atan(V1[1]/V1[0]);
    if (V1[0]<0) alpha1+=M_PI;
    dalpha1 = -cross(V1, dV1) / L2sq(V1);//?!?! strange sign convention...

    //--Choose the smallest angle jump, and define alpha(t).
    while(alpha0>alpha1+M_PI) alpha0-=2*M_PI;
    while(alpha0<alpha1-M_PI) alpha0+=2*M_PI;
    alpha.push_back(Linear(0, alpha1-alpha0));
    alpha.push_back(Linear(dalpha0-(alpha1-alpha0), -dalpha1+(alpha1-alpha0)));

    //--Compute sin and cos of alpha(t): (I am lazy here. Should define cos(SBasis) and sin(SBasis)...)
    if(fabs(alpha1-alpha0) > 0.01) {
        alpha *= 1./(alpha1-alpha0);
        c=compose(cos(Linear(0., alpha1-alpha0), 3), alpha);
        s=compose(sin(Linear(0., alpha1-alpha0), 3), alpha);
        c.truncate(3);
        s.truncate(3);

    }else{
        c = compose(cos(Linear(0., 1.), 3), alpha);
        c[0][1] = std::cos(alpha1-alpha0);
        s = compose(sin(Linear(0., 1.), 3), alpha);
        s[0][1] = std::sin(alpha1-alpha0);
        c.truncate(3);
        s.truncate(3);
    }
    //--Define what is supposed to be our unit vector:
    V[0] = c*std::cos(alpha0) - s*std::sin(alpha0);
    V[1] = c*std::sin(alpha0) + s*std::cos(alpha0);
    

    //--Check how good it is:
    //TODO1: if the curve is a "flat S", the half turns are not seen!!
    //TODO2: Find a good and fast "relative" tolerance...
    Interval bs = dot(vect, vect).boundsFast();
    double err = tol*std::sqrt(max(1., bs.min()));
    //double err=tol;

    SBasis area = SBasis(V[0]*vect[1] - V[1]*vect[0]);
    if(area.tailError(0) < err){
        //Check angle range: if too big, cos an sin are not accurate.
        // this can be solved either by using a range dependent degree for cos and sin,
        // or by adding one more cut here.
        //Notice that in the case of a flat half turn, subdivs wont reduce this range.
        if(fabs(alpha1-alpha0) < M_PI*0.8){
            uvect.push_back(V);
            cuts.push_back(1);
            return(uvect);
        }else{
            c=compose(cos(Linear(0., alpha1-alpha0), 3),compose(alpha, Linear(0,0.5)));
            s=compose(sin(Linear(0., alpha1-alpha0), 3),compose(alpha, Linear(0,0.5)));
            c.truncate(3);
            s.truncate(3);
            V[0]=c*std::cos(alpha0) - s*std::sin(alpha0)*s;
            V[1]=c*std::sin(alpha0) + s*std::cos(alpha0)*s;
            uvect.push_back(V);
            cuts.push_back(.5);

            c=compose(cos(Linear(0., alpha1-alpha0), 3), compose(alpha,Linear(0.5, 1)));
            s=compose(sin(Linear(0., alpha1-alpha0), 3), compose(alpha,Linear(0.5, 1)));
            c.truncate(3);
            s.truncate(3);
            V[0]=c*std::cos(alpha0) - s*std::sin(alpha0);
            V[1]=c*std::sin(alpha0) + s*std::cos(alpha0);
            uvect.push_back(V);
            cuts.push_back(1);
            return(uvect);      
        }
    }else{
        //TODO3: Use 'area' to find a better place to cut than 1/2?
        std::vector<D2<SBasis> > sub_uvect;
        std::vector<double> sub_cuts;
        int NbSubdiv=2;
        for (int i=0; i<NbSubdiv; i++){
            sub_uvect.clear();
            sub_cuts.clear();
            sub_uvect=unit_vector(compose(vect, Linear(i*1./NbSubdiv, (i+1)*1./NbSubdiv) ), sub_cuts, tol);
            for(int idx=0; idx<sub_cuts.size(); idx++){
                uvect.push_back(sub_uvect[idx]);
                cuts.push_back((i+sub_cuts[idx])*1./NbSubdiv);
            }
        }
        return(uvect);
    }
}

std::vector<D2<SBasis> > 
Geom::unit_vector(D2<SBasis> const vect,
                  double tol){
    std::vector<double> cuts;
    return(unit_vector(vect,cuts,tol));
}

std::vector<D2<SBasis> > 
Geom::uniform_speed(D2<SBasis> const M,
                    double tol){
    std::vector<D2<SBasis> > uspeed;
    std::vector<double> cuts;
    uspeed=unit_vector(derivative(M),cuts,tol);
    double t0=0.,t1;
    for (int i=0;i<cuts.size();i++){
        t1=cuts[i];
        uspeed[i]*=1/(t1-t0);
        t0=t1;
    }
    return(uspeed);
}

double 
Geom::arc_length(D2<SBasis> const M,
                 double tol){
    D2<SBasis> dM=derivative(M);
    std::vector<D2<SBasis> > uspeed;
    std::vector<double> cuts;
    uspeed=unit_vector(dM,cuts,tol);
    double t0=0.,t1, L=0.;
    for (int i=0;i<cuts.size();i++){
        t1=cuts[i];
        D2<SBasis> sub_dM=compose(dM,Linear(t0,t1));
        SBasis V=dot(uspeed[i],sub_dM);
//FIXME: if the curve is a flat S, this is wrong: the absolute value of V should be used.
        V=integral(V);
        L+=(V(1)-V(0))*fabs(t1-t0);
        t0=t1;
    }
    return(L);
}

/***
 * std::vector<SBasis > 
 * Geom::arc_length_sb(D2<SBasis> const M,
                    std::vector<double> &cuts,
                    double tol)
 * returns a piecewise sbasis function which maps the t parameter of M(t) to the arc length at that point.
 * In other words, it returns a function to computes the arc length at each point.
 *
 * Todo: replace the return value + cuts with a single class for handling piecewise sbasis
 * functions.
 **/

Geom::Piecewise<SBasis>
Geom::arc_length_sb(D2<SBasis> const M,
                    double tol) {
    Geom::Piecewise<SBasis> result;
    D2<SBasis> dM=derivative(M);
    std::vector<D2<SBasis> > uspeed;
    uspeed=unit_vector(dM,result.cuts,tol);
    double t0=0., t1, L=0.;
    for (int i=0;i<result.cuts.size();i++){
        t1=result.cuts[i];
        D2<SBasis> sub_dM=compose(dM,Linear(t0,t1));
        SBasis V=dot(uspeed[i],sub_dM);
//FIXME: if the curve is a flat S, this is wrong: the absolute value of V should be used.
        V=integral(V)*(t1-t0);
        V += L - V(0);
        result.segs.push_back(V); //  + Linear(L - V(0))
        L=V(1);
        t0=t1;
    }
    result.cuts.insert(result.cuts.begin(), 0); // start
    return result;
}

// incomplete.
Geom::Piecewise<SBasis>
Geom::curvature(D2<SBasis> const M,
                double tol) {
    D2<SBasis> dM=derivative(M);
    Piecewise<SBasis> result;
    std::vector<D2<SBasis> > cv = unit_vector(dM,result.cuts,tol);
    double t0=0.,t1;
    double base = 0;

    for (int i=0;i<cv.size();i++){
        t1=result.cuts[i];
	SBasis speed = dot(compose(dM,Linear(t0,t1)),cv[i]) * (t1-t0);
        D2<SBasis> dcv = derivative(cv[i]);
        dcv[0] = divide(dcv[0],speed,3);
        dcv[1] = divide(dcv[1],speed,3);
        result.segs.push_back(-cv[i][0]*dcv[1] + cv[i][1]*dcv[0]);// + Linear(base, base));
        t0=t1;
    }
    result.cuts.insert(result.cuts.begin(), 0);
    return(result);
}


Geom::D2<Piecewise<SBasis> >
Geom::arc_length_parametrization(D2<SBasis> const &M,
                           unsigned order,
                           double tol){
    D2<Piecewise<SBasis> > u;
    u[0].push_cut(0);
    u[1].push_cut(0);

    Piecewise<SBasis> s = arc_length_sb(M);
    for (int i=0; i < s.size();i++){
        double t0=s.cuts[i],t1=s.cuts[i+1];
        D2<SBasis> sub_M = compose(M,Linear(t0,t1));
        D2<SBasis> sub_u;
        for (int dim=0;dim<2;dim++){
            sub_u[dim]=compose_inverse(sub_M[dim],(s.segs[i]-Linear(s(t0))) / (s(t1)-s(t0)),order,tol);
            u[dim].push(sub_u[dim],s(t1));
        }
    }
    return(u);
}

Geom::D2<Piecewise<SBasis> >
Geom::arc_length_parametrization(D2<Piecewise<SBasis> > const &M,
                                 unsigned order,
                                 double tol){
    D2<Piecewise<SBasis> > result;

    Piecewise<D2<SBasis> > pieces = sectionize(M);
    result=arc_length_parametrization(pieces[0],order,tol);
    for (int i=1; i<M[0].size(); i++ ){
        D2<Piecewise<SBasis> > uniform_seg=arc_length_parametrization(pieces[i],order,tol);
        result[0].concat(uniform_seg[0]);
        result[1].concat(uniform_seg[1]);
    }
    return(result);
}

/** centroid using sbasis integration.
 * This approach uses green's theorem to compute the area and centroid using integrals.  For curved
 * shapes this is much faster than converting to polyline and using the above function.

 * Returned values: 
    0 for normal execution;
    2 if area is zero, meaning centroid is meaningless.

 * Copyright Nathan Hurst 2006
 */

int centroid(Piecewise<D2<SBasis> > const &p, Point& centroid, double &area) {
    Point centroid_tmp(0,0);
    double atmp = 0;
    for(int i = 0; i < p.size(); i++) {
        SBasis curl = dot(p[i], rot90(derivative(p[i])));
        SBasis A = integral(curl);
        D2<SBasis> C = integral(multiply(curl, p[i]));
        atmp += A.at1() - A.at0();
        //TODO: replace following with at0, at1
        centroid_tmp += C(1)- C(0); // first moment.
    }
// join ends
    centroid_tmp *= 2;
    //TODO: replace following with at0, at1
    Geom::Point final = p[p.size()](1), initial = p[0](0);
    const double ai = cross(final, initial);
    atmp += ai;
    centroid_tmp += ai*(final, initial); // first moment.
    
    area = atmp / 2;
    if (atmp != 0) {
        centroid = centroid_tmp / (3 * atmp);
        return 0;
    }
    return 2;
}

//}; // namespace


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

 	  	 

