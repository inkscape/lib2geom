#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "solver.h"
#include "multidim-sbasis.h"
#include "s-basis-2d.h"

#include "path.h"
#include "path-cairo.h"
#include "path-builder.h"

#include "toy-framework.h"

#include <time.h>

using std::vector;
using namespace Geom;
using namespace std;

//-----------------------------------------------

void plot_offset(cairo_t* cr, multidim_sbasis<2> const &M,Coord offset=10,int NbPts=10){
  multidim_sbasis<2> dM=derivative(M);
  for (int i=0;i<NbPts;i++){
    double t=i*1./NbPts;
    Geom::Point V=point_at(dM,t);
    V=offset*rot90(unit_vector(V));
    draw_handle(cr, point_at(M,t)+V);
  }
}

//Usefull? Using point_at(M,0) is not much slower!...
Point point_at0(multidim_sbasis<2> M){
  if (M.size()==0)
    return Point(0,0);
  else{
    Coord x=(M[0].size()==0)?0:M[0][0][0];
    Coord y=(M[1].size()==0)?0:M[1][0][0];
    return Point(x,y);
  }
}
Point point_at1(multidim_sbasis<2> M){
  if (M.size()==0)
    return Point(0,0);
  else{
    Coord x=(M[0].size()==0)?0:M[0][0][1];
    Coord y=(M[1].size()==0)?0:M[1][0][1];
    return Point(x,y);
  }
}

vector<multidim_sbasis<2> > unit_vector(multidim_sbasis<2> const vect, vector<double> &cuts,double tol=.1){
  vector<multidim_sbasis<2> > uvect;
  SBasis alpha,c,s;
  multidim_sbasis<2> V,dV;
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
    dV=derivative(V);
    if (notfound0){
      V0=point_at0(V);
      dV0=point_at0(dV);
      notfound0=is_zero(point_at0(V));
    }
    if (notfound1){
      V1=point_at1(V);
      dV1=point_at1(dV);
      notfound1=is_zero(V1);
    }
    V=dV;
    V[0].normalize();
    V[1].normalize();
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
     c[0][1]=cos(alpha1-alpha0);
     s=compose(sin(BezOrd(0.,1.),3),alpha);
     s[0][1]=sin(alpha1-alpha0);
     c.truncate(3);
     s.truncate(3);
  }
  //--Define what is supposed to be our unit vector:
  V[0]=cos(alpha0)*c-sin(alpha0)*s;
  V[1]=sin(alpha0)*c+cos(alpha0)*s;

  //--Check how good it is:
  //TODO1: if the curve is a "flat S", the half turns are not seen!!
  //TODO2: Find a good and fast "relative" tolerance...
  double m,M;
  bounds(dot(vect,vect),m,M);
  double err=tol*sqrt(max(1.,m));
  //double err=tol;

  SBasis area=SBasis(V[0]*vect[1]-V[1]*vect[0]);
  if (area.tail_error(0)<err){
    uvect.push_back(V);
    cuts.push_back(1);
    return(uvect);
  }else{
    //TODO3: Use 'area' to find a better place to cut than 1/2?
    vector<multidim_sbasis<2> > sub_uvect;
    vector<double> sub_cuts;
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

vector<multidim_sbasis<2> > unit_vector(multidim_sbasis<2> const vect,double tol=.1){
  vector<double> cuts;
  return(unit_vector(vect,cuts,tol));
  }

vector<multidim_sbasis<2> > uniform_speed(multidim_sbasis<2> const M,double tol=.1){
  vector<multidim_sbasis<2> > uspeed;
  vector<double> cuts;
  uspeed=unit_vector(derivative(M),cuts,tol);
  double t0=0.,t1;
  for (int i=0;i<cuts.size();i++){
    t1=cuts[i];
    uspeed[i]*=1/(t1-t0);
    t0=t1;
  }
  return(uspeed);
}

double arc_length(multidim_sbasis<2> const M,double tol=.1){
  multidim_sbasis<2> dM=derivative(M);
  vector<multidim_sbasis<2> > uspeed;
  vector<double> cuts;
  uspeed=unit_vector(dM,cuts,tol);
  double t0=0.,t1, L=0.;
  for (int i=0;i<cuts.size();i++){
    t1=cuts[i];
    multidim_sbasis<2> sub_dM=compose(dM,BezOrd(t0,t1));
    SBasis V=dot(uspeed[i],sub_dM);
//FIXME: if the curve is a flat S, this is wrong: the absolute value of V should be used.
    V=integral(V);
    L+=(V(1)-V(0))*fabs(t1-t0);
    t0=t1;
  }
  return(L);
}


class OffsetTester: public Toy {

  void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    
    multidim_sbasis<2> B = bezier_to_sbasis<2, 3>(handles.begin());

    cairo_set_line_width (cr, 1);
    cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
    cairo_md_sb(cr, B);
    cairo_stroke(cr);

    Coord offset=-100;
    plot_offset(cr,B,offset,11);
    cairo_set_source_rgba (cr, 0, 0, 1, 1);
    cairo_stroke(cr);
    *notify<<"Curve offset:"<<endl;
    *notify<<" -blue: pointwise plotted offset,"<<endl;
    *notify<<" -red:  sbasis approximation,"<<endl;
    *notify<<"Rays are drawn where the curve has been splitted"<<endl;


    cairo_set_source_rgba (cr, 0.5, 0.2, 0., 0.8);
    vector<multidim_sbasis<2> > V;
    vector<double> cuts;
    multidim_sbasis<2> subB,N,Ray;
    double t0=0,t1;

    V=unit_vector(derivative(B),cuts);
    for(int i=0; i<V.size();i++){
      t1=cuts[i];
      subB=compose(B,BezOrd(t0,t1));
      N=offset*rot90(V[i])+subB;
      cairo_md_sb(cr,N);
      cairo_set_source_rgba (cr, 1, 0, 0, 1);
      cairo_stroke(cr);

      Ray[0]=SBasis(BezOrd(point_at0(subB)[0],point_at0(N)[0]));
      Ray[1]=SBasis(BezOrd(point_at0(subB)[1],point_at0(N)[1]));
      cairo_md_sb(cr,Ray);
      cairo_set_source_rgba (cr, 1, 0, 0, 0.2);
      cairo_stroke(cr);
      t0=t1;
    }
    Ray[0]=SBasis(BezOrd(point_at1(subB)[0],point_at1(N)[0]));
    Ray[1]=SBasis(BezOrd(point_at1(subB)[1],point_at1(N)[1]));
    cairo_md_sb(cr,Ray);
    cairo_set_source_rgba (cr, 1, 0, 0, 0.2);
    cairo_stroke(cr);
    *notify<<"Total length: "<<arc_length(B)<<endl;
    *notify<<"(nb of cuts: "<<V.size()-1<<")"<<endl;
    
    Toy::draw(cr, notify, width, height, save);
}        
  
public:
    OffsetTester(){
        if(handles.empty()) {
            for(int i = 0; i < 4; i++)
                handles.push_back(Geom::Point(200+50*i,400));
        }
    }
};

int main(int argc, char **argv) {
  std::cout<<"testing sqrt..."<<std::endl;
    init(argc, argv, "normal-bundle", new OffsetTester);
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

 	  	 
