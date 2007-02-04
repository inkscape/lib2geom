#include "sb-geometric.h"
#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "solver.h"
#include "multidim-sbasis.h"
#include "s-basis-2d.h"
#include "sb-geometric.h"

/** Geometric operators on multidim_sbasis<2> (1D->2D).
 * Copyright 2007 JF Barraud
 * Copyright 2007 N Hurst
 *
 * The functions defined in this header related to 2d geometric operations such as arc length,
 * unit_vector and curvature.  Most are built on top of unit_vector, which takes an arbitrary md<2>
 * and returns an md<2> with unit length with the same direction.
 *
 * Todo/think about:
 *  arclength md<2> -> sbasis (giving arclength function)
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
Point point_at0(MultidimSBasis<2> M){
    if (M.size()==0)
        return Point(0,0);
    else{
        Coord x=(M[0].size()==0)?0:M[0][0][0];
        Coord y=(M[1].size()==0)?0:M[1][0][0];
        return Point(x,y);
    }
}

static 
Point point_at1(MultidimSBasis<2> M){
    if (M.size()==0)
        return Point(0,0);
    else{
        Coord x=(M[0].size()==0)?0:M[0][0][1];
        Coord y=(M[1].size()==0)?0:M[1][0][1];
        return Point(x,y);
    }
}

static 
Point deriv_at0(MultidimSBasis<2> M){
    if (M.size()==0){
        return Point(0,0);
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
Point deriv_at1(MultidimSBasis<2> M){
    if (M.size()==0){
        return Point(0,0);
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

std::vector<MultidimSBasis<2> > Geom::unit_vector(MultidimSBasis<2> const vect, std::vector<double> &cuts,double tol){
    std::vector<MultidimSBasis<2> > uvect;
    SBasis alpha,c,s;
    MultidimSBasis<2> V,dV;
    double alpha0,alpha1,dalpha0,dalpha1;

    V=vect;
    V[0].normalize();
    V[1].normalize();
    if (V.size()==0){
        return(uvect);
    }

    //--Compute angle jet at 0 and 1.
    bool notfound0=true,notfound1=true;
    Point V0,dV0,V1,dV1;
    while (V.size()!=0&&(notfound0||notfound1)){
        if (notfound0){
            V0=point_at0(V);
            dV0=deriv_at0(V);
            notfound0=is_zero(point_at0(V));
        }
        if (notfound1){
            V1=point_at1(V);
            dV1=deriv_at1(V);
            notfound1=is_zero(V1);
        }
        if (notfound0||notfound1){
            V=derivative(V);
            V[0].normalize();
            V[1].normalize();
        }
    }
    alpha0=atan(V0[1]/V0[0]);
    if (V0[0]<0){alpha0+=M_PI;}
    dalpha0= -cross(V0,dV0)/L2sq(V0);//?!?! strange sign convention...
  
    alpha1=atan(V1[1]/V1[0]);
    if (V1[0]<0){alpha1+=M_PI;}
    dalpha1= -cross(V1,dV1)/L2sq(V1);//?!?! strange sign convention...

    //--Choose the smallest angle jump, and define alpha(t).
    while(alpha0>alpha1+M_PI)alpha0-=2*M_PI;
    while(alpha0<alpha1-M_PI)alpha0+=2*M_PI;
    alpha.push_back(BezOrd(0,alpha1-alpha0));
    alpha.push_back(BezOrd(dalpha0-(alpha1-alpha0),-dalpha1+(alpha1-alpha0)));

    //--Compute sin and cos of alpha(t): (I am lazy here. Should define cos(SBasis) and sin(SBasis)...)
    if(fabs(alpha1-alpha0)>0.01) {
        alpha*=1./(alpha1-alpha0);
        c=compose(cos(BezOrd(0.,alpha1-alpha0),3),alpha);
        s=compose(sin(BezOrd(0.,alpha1-alpha0),3),alpha);
        c.truncate(3);
        s.truncate(3);

    }else{
        c=compose(cos(BezOrd(0.,1.),3),alpha);
        c[0][1]=std::cos(alpha1-alpha0);
        s=compose(sin(BezOrd(0.,1.),3),alpha);
        s[0][1]=std::sin(alpha1-alpha0);
        c.truncate(3);
        s.truncate(3);
    }
    //--Define what is supposed to be our unit vector:
    V[0]=std::cos(alpha0)*c-std::sin(alpha0)*s;
    V[1]=std::sin(alpha0)*c+std::cos(alpha0)*s;

    //--Check how good it is:
    //TODO1: if the curve is a "flat S", the half turns are not seen!!
    //TODO2: Find a good and fast "relative" tolerance...
    double m,M;
    bounds(dot(vect,vect),m,M);
    double err=tol*std::sqrt(max(1.,m));
    //double err=tol;

    SBasis area=SBasis(V[0]*vect[1]-V[1]*vect[0]);
    if (area.tail_error(0)<err){
        //Check angle range: if too big, cos an sin are not accurate.
        // this can be solved either by using a range dependent degree for cos and sin,
        // or by adding one more cut here.
        //Notice that in the case of a flat half turn, subdivs wont reduce this range.
        if (fabs(alpha1-alpha0)<M_PI*0.8){
            uvect.push_back(V);
            cuts.push_back(1);
            return(uvect);
        }else{
            c=compose(cos(BezOrd(0.,alpha1-alpha0),3),compose(alpha,BezOrd(0,0.5)));
            s=compose(sin(BezOrd(0.,alpha1-alpha0),3),compose(alpha,BezOrd(0,0.5)));
            c.truncate(3);
            s.truncate(3);
            V[0]=std::cos(alpha0)*c-std::sin(alpha0)*s;
            V[1]=std::sin(alpha0)*c+std::cos(alpha0)*s;
            uvect.push_back(V);
            cuts.push_back(.5);

            c=compose(cos(BezOrd(0.,alpha1-alpha0),3),compose(alpha,BezOrd(0.5,1)));
            s=compose(sin(BezOrd(0.,alpha1-alpha0),3),compose(alpha,BezOrd(0.5,1)));
            c.truncate(3);
            s.truncate(3);
            V[0]=std::cos(alpha0)*c-std::sin(alpha0)*s;
            V[1]=std::sin(alpha0)*c+std::cos(alpha0)*s;
            uvect.push_back(V);
            cuts.push_back(1);
            return(uvect);      
        }
    }else{
        //TODO3: Use 'area' to find a better place to cut than 1/2?
        std::vector<MultidimSBasis<2> > sub_uvect;
        std::vector<double> sub_cuts;
        int NbSubdiv=2;
        for (int i=0;i<NbSubdiv;i++){
            sub_uvect.clear();
            sub_cuts.clear();
            sub_uvect=unit_vector(compose(vect,BezOrd(i*1./NbSubdiv,(i+1)*1./NbSubdiv)), sub_cuts,tol);
            for(int idx=0;idx<sub_cuts.size();idx++){
                uvect.push_back(sub_uvect[idx]);
                cuts.push_back((i+sub_cuts[idx])*1./NbSubdiv);
            }
        }
        return(uvect);
    }
}

std::vector<MultidimSBasis<2> > 
Geom::unit_vector(MultidimSBasis<2> const vect,
                  double tol){
    std::vector<double> cuts;
    return(unit_vector(vect,cuts,tol));
}

std::vector<MultidimSBasis<2> > 
Geom::uniform_speed(MultidimSBasis<2> const M,
                    double tol){
    std::vector<MultidimSBasis<2> > uspeed;
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
Geom::arc_length(MultidimSBasis<2> const M,
                 double tol){
    MultidimSBasis<2> dM=derivative(M);
    std::vector<MultidimSBasis<2> > uspeed;
    std::vector<double> cuts;
    uspeed=unit_vector(dM,cuts,tol);
    double t0=0.,t1, L=0.;
    for (int i=0;i<cuts.size();i++){
        t1=cuts[i];
        MultidimSBasis<2> sub_dM=compose(dM,BezOrd(t0,t1));
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
 * Geom::arc_length_sb(MultidimSBasis<2> const M,
                    std::vector<double> &cuts,
                    double tol)
 * returns a piecewise sbasis function which maps the t parameter of M(t) to the acr length at that point.
 * In other words, it returns a function to compute the arc length at each point.
 *
 * Todo: replace the return value + cuts with a single class for handling piecewise sbasis
 * functions.
 **/

std::vector<SBasis > 
Geom::arc_length_sb(MultidimSBasis<2> const M,
                    std::vector<double> &cuts,
                    double tol){
    std::vector<SBasis > al;
    MultidimSBasis<2> dM=derivative(M);
    std::vector<MultidimSBasis<2> > uspeed;
    uspeed=unit_vector(dM,cuts,tol);
    double t0=0.,t1, L=0.;
    for (int i=0;i<cuts.size();i++){
        t1=cuts[i];
        MultidimSBasis<2> sub_dM=compose(dM,BezOrd(t0,t1));
        SBasis V=dot(uspeed[i],sub_dM);
//FIXME: if the curve is a flat S, this is wrong: the absolute value of V should be used.
        V=(t1-t0)*integral(V);
        V += L - V(0);
        al.push_back(V); //  + BezOrd(L - V(0))
        L=V(1);
        t0=t1;
    }
    return(al);
}

// incomplete.
std::vector<SBasis > 
Geom::curvature(MultidimSBasis<2> const M,
                std::vector<double> &cuts,
                double tol){
    std::vector<MultidimSBasis<2> > cv;
    MultidimSBasis<2> dM=derivative(M);
    std::vector<SBasis > res;
    cv=unit_vector(dM,cuts,tol);
    double t0=0.,t1;
    double base = 0;

    for (int i=0;i<cv.size();i++){
        t1=cuts[i];
	SBasis speed=(t1-t0)*dot(compose(dM,BezOrd(t0,t1)),cv[i]);
        MultidimSBasis<2> dcv =derivative(cv[i]);
        dcv[0]=divide(dcv[0],speed,3);
        dcv[1]=divide(dcv[1],speed,3);
        res.push_back(-cv[i][0]*dcv[1] + cv[i][1]*dcv[0]);// + BezOrd(base, base));
        //base = res.back()[0][1] - base;
        t0=t1;
    }
    return(res);
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

 	  	 

