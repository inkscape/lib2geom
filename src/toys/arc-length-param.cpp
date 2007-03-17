#include "s-basis.h"
#include "sb-geometric.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis.h"
#include "s-basis-2d.h"
#include "D2.h"

#include "path-cairo.h"

#include <map>
#include <iterator>
#include "translate.h"

#include "toy-framework.h"

using std::vector;
using namespace Geom;

static void dot_plot(cairo_t *cr, D2<pw_sb> const &M,double space=10){
    double t=M[0].cuts.front();
    while (t<M[0].cuts.back()){
        D2<double> p=M(t);
        draw_handle(cr,Geom::Point(p[0],p[1]));
        t+=space;
    }
    for (int i=0;i<M[0].size();i++){
        MultidimSBasis<2> mdsbM;
        mdsbM[0]=M[0][i];
        mdsbM[1]=M[1][i];
        cairo_md_sb(cr, mdsbM);
    }
    cairo_stroke(cr);
}

static int valuation(SBasis const &f, double tol=1e-1){
    int val=0;
    while(val<f.size()&& fabs(f[val][0])<tol && fabs(f[val][1])<tol) val++;
    return val;
}


static bool compose_inverse(SBasis const &g,SBasis const & x, SBasis &f, int order){
    double tol=1e-4;
    SBasis r=g;
    int val=0;
    SBasis Pk=BezOrd(1,0),Qk=BezOrd(0,1);
    
    for (int k=0; k<=order; k++){
        SBasis Pk_x=Pk(x),Qk_x=Qk(x);
        int v=std::min(valuation(Pk_x),valuation(Qk_x));
        
        if (v<valuation(r)){
            f.push_back(BezOrd(0));
        }else if (v==valuation(r)){
            double p10=Pk_x[v][0];
            double p01=Pk_x[v][1];
            double q10=Qk_x[v][0];
            double q01=Qk_x[v][1];
            double r10=r[v][0];
            double r01=r[v][1];
            if ( fabs(p10*q01-p01*q10)<tol && (fabs(r10)>tol || fabs(r01)>tol )){
                return(false);
            }
            double a=( q01*r10-q10*r01)/(p10*q01-p01*q10);
            double b=(-p01*r10+p10*r01)/(p10*q01-p01*q10);
            f.push_back(BezOrd(a,b));
            r=g-f(x);
        }
        if (valuation(r)==r.size()) return true;
        Pk=shift(Pk,1);
        Qk=shift(Qk,1);
    }
    return(true);
}

static D2<pw_sb> arc_length_parametrization(MultidimSBasis<2> const &M){
    D2<pw_sb> u;
    u[0].push_cut(0);
    u[1].push_cut(0);

    pw_sb s = arc_length_sb(M);
    std::cout<<"nb pieces:"<<s.size()<<std::endl;
    for (int i=0; i<s.size();i++){
        double t0=s.cuts[i],t1=s.cuts[i+1];
        MultidimSBasis<2> sub_M=compose(M,BezOrd(t0,t1));
        MultidimSBasis<2> sub_u;
        bool ok;
        ok=      compose_inverse(sub_M[0],1/(s(t1)-s(t0))*(s.segs[i]-BezOrd(s(t0))),sub_u[0],3);
        ok=ok && compose_inverse(sub_M[1],1/(s(t1)-s(t0))*(s.segs[i]-BezOrd(s(t0))),sub_u[1],3);
        if (!ok) std::cout<<"IMPOSSIBLE!!!!!!"<<std::endl;
        u[0].push(sub_u[0],s(t1));
        u[1].push(sub_u[1],s(t1));
    }
    return(u);
}

#define SIZE 5

class LengthTester: public Toy {

    void draw(cairo_t *cr,
	      std::ostringstream *notify,
	      int width, int height, bool save) {
    
      MultidimSBasis<2> B = bezier_to_sbasis<2, SIZE-1>(handles.begin());
      D2<pw_sb>d2B;
      d2B[0]=pw_sb(B[0]);
      d2B[1]=pw_sb(B[1]);

      cairo_set_line_width (cr, .5);
      cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
      cairo_md_sb(cr, B);
      cairo_stroke(cr);

      D2<pw_sb> U = arc_length_parametrization(B);
      cairo_set_source_rgba (cr, 0., 0., 0.9, 1);
      dot_plot(cr,U);
      cairo_stroke(cr);
      

      Toy::draw(cr, notify, width, height, save);
    }        
  
public:
    LengthTester(){
        if(handles.empty()) {
            for(int i = 0; i < SIZE; i++)
                handles.push_back(Geom::Point(150+uniform()*300,150+uniform()*300));
        }
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "bounds-test", new LengthTester);
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
// vim: filetype = cpp:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :

 	  	 
