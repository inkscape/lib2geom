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
  plot[1]+=300;
  cairo_md_sb(cr, plot);
  cairo_stroke(cr);
}
static void plot_bar(cairo_t* cr, double height, double vscale=1,double a=0,double b=1){
  cairo_move_to(cr, Geom::Point(150+300*a,-height*vscale+300));
  cairo_line_to(cr, Geom::Point(150+300*b,-height*vscale+300));
  cairo_stroke(cr);
}
static void plot_box(cairo_t* cr, double a, double b, double min, double max){
  cairo_rectangle (cr,150+a*300,-max+300,300*(b-a),max-min);
  cairo_fill (cr);
}

static void local_bounds(SBasis const & s,
		  double t0,double t1,
		  double &lo, double &hi) {
  int imax=s.size()-1;
  lo=0;
  hi=0;
  
  for(int i = imax; i >=0; i--) {
    double a=s[i][0];
    double b=s[i][1];
    double t;
    if (hi>0){t=((b-a)+hi)/2/hi;}
    if (hi<=0||t<t0||t>t1){
      hi=std::max(a*(1-t0)+b*t0+hi*t0*(1-t0),a*(1-t1)+b*t1+hi*t1*(1-t1));
    }else{
      hi=a*(1-t)+b*t+hi*t*(1-t);	  
    }
    if (lo<0){t=((b-a)+lo)/2/lo;}
    if (lo>=0||t<t0||t>t1){
      lo=std::min(a*(1-t0)+b*t0+lo*t0*(1-t0),a*(1-t1)+b*t1+lo*t1*(1-t1));
    }else{
      lo=a*(1-t)+b*t+lo*t*(1-t);	  
    }
  }
}

static void my_roots_internal(SBasis const &f,
			      SBasis const &df,
			      vector<double> &roots,
			      double tol,
			      double a,
			      double fa,
			      double b,
			      double fb){

  if (f.size()==0){
    roots.push_back(a);
    roots.push_back(b);
    return;
  }
  if (f.size()==1){
    if(f[0][0]==0&&f[0][1]==0){
      roots.push_back(a);
      roots.push_back(b);
    }else{
      if (fa*fb<=0) roots.push_back(a-fa/(fb-fa)*(b-a));
    }
    return;
  }
  
  if ((b-a)<tol){
    //TODO: use different tol for t and f !
    if(fa*fb<=0||fabs(fa)<tol||fabs(fb)<tol){
      roots.push_back((a+b)/2);
    }
    return;
  }

  double m,M;
  local_bounds(df,a,b,m,M);
  if (m>=0&&(fa> tol||fb<-tol)) return;
  if (M<=0&&(fa<-tol||fb>tol)) return;
  
  double t0,t1,t,ft;
  t0=(fa<0)?a-fa/M:a-fa/m;
  t1=(fb>0)?b-fb/M:b-fb/m;
  if (t0>t1) return;
  t=(t0+t1)/2;
  ft=f(t);
  my_roots_internal(f,df,roots,tol,t0,f(t0),t ,ft   );
  my_roots_internal(f,df,roots,tol,t ,ft   ,t1,f(t1));
}

static void new_roots(SBasis const &f,
		      vector<double> &roots,
		      double tol=1e-7,
		      double a=0,
		      double b=1){
  SBasis df=derivative(f);
  my_roots_internal(f,df,roots,tol,a,f(a),b,f(b));
}

class BoundsTester: public Toy {
  int size;

  void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    
    SBasis B;
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
      B.push_back(BezOrd(-(handles[i     ][1]-300)*pow(4.,i),
			 -(handles[i+size][1]-300)*pow(4.,i) ));
    }
    B.normalize();
    plot(cr,B,1);	
    cairo_set_source_rgba (cr, 0., 0., 0.8, 1);
    cairo_stroke(cr);
    cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
    
    *notify<<"Use handles to set the coefficients of the s-basis."<<std::endl;
    //*notify<<"-Blue=basic bounds,"<<std::endl;
    //*notify<<"-Red =slower but sharper?"<<std::endl;
    
    double m,M;
    vector<double>my_roots;

    cairo_set_source_rgba (cr, 0.9, 0., 0.8, 1);
    my_roots=roots(B);
    for(int i=0;i<my_roots.size();i++){
      draw_handle(cr,Point(150+300*my_roots[i],300));
    }
    cairo_set_source_rgba (cr, 0.9, 0., 0.8, 1);
    my_roots.clear();
    new_roots(B,my_roots,1e-7,0,1);
    for(int i=0;i<my_roots.size();i++){
      draw_cross(cr,Point(150+300*my_roots[i],300));
    }
    //cout<<"nb roots: "<<my_roots.size()<<endl;
    
    clock_t end_t;
    unsigned iterations = 0;

    my_roots.clear();
    end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
    iterations = 0;
    while(end_t > clock()) {
      my_roots.clear();
      my_roots=roots(B);
      iterations++;
    }
    *notify << 1000*0.1/iterations <<" ms = roots time"<< std::endl;
    
    end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
    iterations = 0;
    while(end_t > clock()) {
      my_roots.clear();
      new_roots(B,my_roots);
      iterations++;
    }
    *notify << 1000*0.1/iterations <<" ms = new_roots time"<< std::endl;
    
    Toy::draw(cr, notify, width, height, save);
  }        
  
public:
  BoundsTester(){
    size=8;
    if(handles.empty()) {
      for(int i = 0; i < 2*size; i++)
	handles.push_back(Geom::Point(0,150+150+uniform()*300*0));
    }
    handles[0][1]+=10;
  }
};

int main(int argc, char **argv) {
    init(argc, argv, "bounds-test", new BoundsTester);
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
