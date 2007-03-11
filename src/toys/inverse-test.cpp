#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis.h"

#include "path.h"
#include "path-cairo.h"
#include "path-builder.h"

#include "toy-framework.h"

#include <time.h>

using std::vector;
using namespace Geom;
using namespace std;

static void plot(cairo_t* cr, SBasis const &B,double vscale=1,double a=0,double b=1){
  MultidimSBasis<2> plot;
  plot[0]=SBasis(BezOrd(150+a*300,150+b*300));
  plot[1]=-vscale*B;
  plot[1]+=450;
  cairo_md_sb(cr, plot);
  cairo_stroke(cr);
}
static void plot_flip(cairo_t* cr, SBasis const &B,double vscale=1,double a=0,double b=1){
  MultidimSBasis<2> plot;
  plot[1]=SBasis(BezOrd(450-a*300,450-b*300));
  plot[0]=150+vscale*B;
  cairo_md_sb(cr, plot);
  cairo_stroke(cr);
}
static void plot(cairo_t* cr, pw_sb const &f,double vscale=1){
  for (int i=0;i<f.size();i++){
      plot(cr,f.segs[i],vscale,f.cuts[i],f.cuts[i+1]);
      draw_cross(cr,Geom::Point(150+300*f.cuts[i],450-vscale*f.segs[i][0][0]));
  }
}
static void plot_flip(cairo_t* cr, pw_sb const &f,double vscale=1){
  for (int i=0;i<f.size();i++){
      plot_flip(cr,f.segs[i],vscale,f.cuts[i],f.cuts[i+1]);
      draw_cross(cr,Geom::Point(150+vscale*f.segs[i][0][0],450-300*f.cuts[i]));
  }
}

static SBasis my_inverse(SBasis f, int order){
    double a0 = f[0][0];
    if(a0 != 0) {
        f -= a0;
    }
    double a1 = f[0][1];
    assert(a1 != 0);// not invertable.
    if(a1 != 1) {
        f /= a1;
    }

    SBasis g=SBasis(BezOrd(0,1)),r;
    double df0=derivative(f)(0);
    double df1=derivative(f)(1);
    r=BezOrd(0,1)-g(f);

    for(int i=1; i<order; i++){
        //std::cout<<"i: "<<i<<std::endl;
        r=BezOrd(0,1)-g(f);
        //std::cout<<"t-gof="<<r<<std::endl;
        r.normalize();
        if (r.size()==0) return(g);
        double a=r[i][0]/pow(df0,i);
        double b=r[i][1]/pow(df1,i);
        g.push_back(BezOrd(a,b));
    }
    
    return(g);
}

static pw_sb pw_inverse(SBasis const &f, int order,double tol=.1,int depth=0){
    SBasis g=SBasis(BezOrd(0,1)),r;
    pw_sb res,res1,res2;
    
    //std::cout<<"depth: "<<depth<<std::endl;
    g=my_inverse(f,order);
    r=g(f);
    //std::cout<<"error: "<<g.tail_error(1)<<std::endl;
    if (g.tail_error(1)<tol){
        res.segs.push_back(g);
        res.cuts.push_back(f[0][0]);
        res.cuts.push_back(f[0][1]);
    }else if (depth<200){
        SBasis ff;
        ff=f(BezOrd(0,.5));
        res=pw_inverse(ff,order,tol,depth+1);
        for (int i=0;i<res.size();i++){
            res.segs[i]*=.5;
        }
        ff=f(BezOrd(.5,1));
        res1=pw_inverse(ff,order,tol,depth+1);
        for (int i=0;i<res1.size();i++){
            res1.segs[i]*=.5;
            res1.segs[i]+=.5;
        }
        res.concat(res1);
    }
    return(res);
}



class InverseTester: public Toy {
  int size;

  void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    
      SBasis f;//=SBasis(BezOrd(0,.5));
      for (int i=0;i<size;i++){
          handles[i    ][0]=150+15*(i-size);
          handles[i+size][0]=450+15*(i+1);
          cairo_move_to(cr, Geom::Point(handles[i    ][0],150));
          cairo_line_to(cr, Geom::Point(handles[i    ][0],450));
          cairo_move_to(cr, Geom::Point(handles[i+size][0],150));
          cairo_line_to(cr, Geom::Point(handles[i+size][0],450));
      }
      cairo_move_to(cr, Geom::Point(0,300));
      cairo_line_to(cr, Geom::Point(600,300));
      
      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 1);
      cairo_stroke(cr);
    
      for (int i=0;i<size;i++){
          f.push_back(BezOrd(-(handles[i     ][1]-300)*pow(4.,i)/150,
                             -(handles[i+size][1]-300)*pow(4.,i)/150 ));
      }
      plot(cr,BezOrd(0,1),300);	
      
      f.normalize();
      cairo_set_line_width (cr, 2);
      cairo_set_source_rgba (cr, 0., 0., 0.8, 1);
      plot(cr,f,300);	
      
      *notify<<"Use handles to set the coefficients of the (blue) s-basis."<<std::endl;
      *notify<<" (keep it monotonic!)"<<std::endl;
      *notify<<"red=flipped inverse; should be the same as the blue one."<<std::endl;
      
      pw_sb g=pw_inverse(f,3);
      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0.8, 0., 0., 1);
      plot(cr,g,300);	
      pw_sb h=compose(g,f); 
      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0., 0.8, 0., 1);
      plot(cr,h,300);

      *notify<<g.size()<<" segments.";

      Toy::draw(cr, notify, width, height, save);
  }        
  
public:
  InverseTester(){
    size=4;
    if(handles.empty()) {
      for(int i = 0; i < 2*size; i++)
	handles.push_back(Geom::Point(0,150+150+uniform()*300*0));
    }
    handles[0][1]=300;
    handles[size][1]=150;
  }
};

int main(int argc, char **argv) {
    init(argc, argv, "bounds-test", new InverseTester);
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
//vim:filetype = cpp:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :
