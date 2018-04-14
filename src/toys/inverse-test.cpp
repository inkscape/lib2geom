#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#include <time.h>
using std::vector;
using namespace Geom;
using namespace std;

static void plot(cairo_t* cr, SBasis const &B,double vscale=1,double a=0,double b=1){
  D2<SBasis> plot;
  plot[0]=SBasis(Linear(150+a*300,150+b*300));
  plot[1]=B*-vscale;
  plot[1]+=450;
  cairo_d2_sb(cr, plot);
  cairo_stroke(cr);
}
static void plot(cairo_t* cr, Piecewise<SBasis> const &f,double vscale=1){
  for (unsigned i=0;i<f.size();i++){
      plot(cr,f.segs[i],vscale,f.cuts[i],f.cuts[i+1]);
      draw_cross(cr,Geom::Point(f.cuts[i]*300 + 150, f.segs[i][0][0]*(-vscale) + 450));
  }
}

static SBasis my_inverse(SBasis f, int order){
    double a0 = f[0][0];
    if(a0 != 0) {
        f -= a0;
    }
    double a1 = f[0][1];
    if(a1 == 0)
        THROW_NOTINVERTIBLE();
    //assert(a1 != 0);// not invertible.
    if(a1 != 1) {
        f /= a1;
    }

    SBasis g=SBasis(order, Linear());
    g[0] = Linear(0,1);
    double df0=derivative(f)(0);
    double df1=derivative(f)(1);
    SBasis r = Linear(0,1)-g(f);

    for(int i=1; i<order; i++){
        //std::cout<<"i: "<<i<<std::endl;
        r=Linear(0,1)-g(f);
        //std::cout<<"t-gof="<<r<<std::endl;
        r.normalize();
        if (r.size()==0) return(g);
        double a=r[i][0]/std::pow(df0,i);
        double b=r[i][1]/std::pow(df1,i);
        g[i] = Linear(a,b);
    }
    
    return(g);
}

static Piecewise<SBasis> pw_inverse(SBasis const &f, int order,double tol=.1,int depth=0){
    SBasis g=SBasis(Linear(0,1)),r;
    Piecewise<SBasis> res,res1,res2;
    
    //std::cout<<"depth: "<<depth<<std::endl;
    g=my_inverse(f,order);
    r=g(f);
    //std::cout<<"error: "<<g.tail_error(1)<<std::endl;
    if (g.tailError(1)<tol){
        res.segs.push_back(g);
        res.cuts.push_back(f[0][0]);
        res.cuts.push_back(f[0][1]);
    }else if (depth<200){
        SBasis ff;
        ff=f(Linear(0,.5));
        res=pw_inverse(ff,order,tol,depth+1);
        for (unsigned i=0;i<res.size();i++){
            res.segs[i]*=.5;
        }
        ff=f(Linear(.5,1));
        res1=pw_inverse(ff,order,tol,depth+1);
        for (unsigned i=0;i<res1.size();i++){
            res1.segs[i]*=.5;
            res1.segs[i]+=.5;
        }
        res.concat(res1);
    }
    return(res);
}



class InverseTester: public Toy {
  int size;
    PointSetHandle hand;

  void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
    
      for (int i=0;i<size;i++){
          hand.pts[i    ][0]=150+15*(i-size);
          hand.pts[i+size][0]=450+15*(i+1);
          cairo_move_to(cr, Geom::Point(hand.pts[i    ][0],150));
          cairo_line_to(cr, Geom::Point(hand.pts[i    ][0],450));
          cairo_move_to(cr, Geom::Point(hand.pts[i+size][0],150));
          cairo_line_to(cr, Geom::Point(hand.pts[i+size][0],450));
      }
      cairo_move_to(cr, Geom::Point(0,300));
      cairo_line_to(cr, Geom::Point(600,300));
      
      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 1);
      cairo_stroke(cr);
    
      SBasis f(size, Linear());//=SBasis(Linear(0,.5));
      for (int i=0;i<size;i++){
          f[i] = Linear(-(hand.pts[i     ][1]-300)*std::pow(4.,i)/150,
                        -(hand.pts[i+size][1]-300)*std::pow(4.,i)/150 );
      }
      plot(cr,Linear(0,1),300);	
      
      f.normalize();
      cairo_set_line_width (cr, 2);
      cairo_set_source_rgba (cr, 0., 0., 0.8, 1);
      plot(cr,f,300);	
      
      *notify<<"Use hand.pts to set the coefficients of the (blue) s-basis."<<std::endl;
      *notify<<" (keep it monotonic!)"<<std::endl;
      *notify<<"red=flipped inverse; should be the same as the blue one."<<std::endl;
      
      try {
          Piecewise<SBasis> g=pw_inverse(f,3);
          cairo_set_line_width (cr, 1);
          cairo_set_source_rgba (cr, 0.8, 0., 0., 1);
          plot(cr,g,300);	
          Piecewise<SBasis> h=compose(g,f); 
          cairo_set_line_width (cr, 1);
          cairo_set_source_rgba (cr, 0., 0.8, 0., 1);
          plot(cr,h,300);
          *notify<<g.size()<<" segments.";
      } catch(NotInvertible) {
          *notify << "function not invertible!" << std::endl;
      }

      Toy::draw(cr, notify, width, height, save,timer_stream);
  }        
  
public:
    InverseTester() {
    size=4;
    if(hand.pts.empty()) {
      for(int i = 0; i < 2*size; i++)
	hand.pts.push_back(Geom::Point(0,150+150+uniform()*300*0));
    }
    hand.pts[0][1]=300;
    hand.pts[size][1]=150;
    handles.push_back(&hand);
  }
};

int main(int argc, char **argv) {
    init(argc, argv, new InverseTester);
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
