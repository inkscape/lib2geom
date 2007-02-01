#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis.h"

#include "path-cairo.h"

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
}

//---- Original version: ----
static void original_bounds(SBasis const & s,
            double &lo, double &hi) {
    double ss = 0.25;
    double mid = s(0.5);
    lo = hi = mid;
    for(unsigned i = 1; i < s.size(); i++) {
        for(unsigned dim = 0; dim < 2; dim++) {
            double b = s[i][dim]*ss;
            if(b < 0)
                lo += b;
            if(b > 0)
                hi += b;
        }
        ss *= 0.25;
    }
    lo = std::min(std::min(lo, s[0][1]),s[0][0]);
    hi = std::max(std::max(hi, s[0][0]), s[0][1]);
}

//---- A (slightly) more accurate version: ----
//  ...
//  (moved to 's-basis-roots.cpp')
//  ...

//---- A second layer: slower but sharper ----
/* (moved to 's-basis-roots.cpp')
static void slow_bounds(cairo_t *cr, //DEBUG
			SBasis const & f_in,
			double &m, 
			double &M,
			int order=0,
			double tol=.01
			) {

  assert (tol>0);
  order=std::max(0,order);
  SBasis f;
  for (int i=order;i<f_in.size();i++){
    f.push_back(f_in[i]);
  }

  double Mf,mf,Mdf,mdf,Md2f,md2f,t,h,err;
  double step,step_min;
  SBasis df=derivative(f);
  SBasis d2f=derivative(df);
  bounds(df,mdf,Mdf);
  bounds(d2f,md2f,Md2f);
  if (f.size()<=1||mdf>=0||Mdf<=0){
    bounds(f,m,M);
  }else{
    bounds(f,mf,Mf);
    err=tol*std::max(1.,Mf-mf);//Fix me: 
    //1)Mf-mf should not be 0...
    //2)tolerance relative to inaccurate bounds (but using
    // the current value of M and m can be speed consuming).
    double err_M=0,err_m=0;
    step_min=err/std::max(Mdf,-mdf);
    M=std::max(f[0][0],f[0][1]);
    m=std::min(f[0][0],f[0][1]);
    for (t=0;t<=1;){
      double ft=f(t);
      double dft=df(t);
      M=std::max(M,ft);
      m=std::min(m,ft);
      if (M<ft){
	M=ft;
	err_M=dft*step_min;
      }
      if (m>ft){
	m=ft;
	err_m=-dft*step_min;
      }
      if (dft>0){
	step=std::max(-dft/md2f,step_min);
      }else if (dft<0){
	step=std::max(-dft/Md2f,step_min);
      }else{
	step=step_min;
      }
      step=std::max(step,std::min((M-ft)/Mdf,(m-ft)/mdf));
      draw_handle(cr, Geom::Point(150+t*300,300));//DEBUG      
      t+=step;
    }
    M+=err_M;
    m-=err_m;
    M*=pow(.25,order);
    m*=pow(.25,order);
  }
}
*/

//---- Another try: ----
//seems to be slower than brutal evaluation anyway.
void cut_bounds(SBasis const & f,
		   double &m, double &M, int order=0) {
  bounds(f,m,M,1);
  double a=f[0][0];
  double b=f[0][1];
  double t,t_max,t_min;

  if (M>0){t=((b-a)+M)/2/M;}
  if (M<=0||t<0||t>1){
    M=std::max(a,b);
    t_max=(a>b)?0:1;
  }else{
    M=a*(1-t)+b*t+M*t*(1-t);	  
    t_max=t;
  }
  
  if (m<0){t=((b-a)+m)/2/m;}
  if (m>=0||t<0||t>1){
    m=std::min(a,b);
    t_min=(a<b)?0:1;
  }else{
    m=a*(1-t)+b*t+m*t*(1-t);	  
    t_min=t;
  }
  if (f(t_max)-f(t_min)<0.99*(M-m)){
    double M0,m0,M1,m1;
    cut_bounds(compose(f,BezOrd(0,.5)),m0,M0,0);
    cut_bounds(compose(f,BezOrd(.5,1)),m1,M1,0);
    M=std::max(M0,M1);
    m=std::min(m0,m1);
  }
}


class BoundsTester: public Toy {

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    
      int deg=4;
	SBasis B;
	for (int i=0;i<deg;i++){
	  handles[i    ][0]=150+20*(i-deg);
	  handles[i+deg][0]=450+20*(i+1);
	  cairo_move_to(cr, Geom::Point(handles[i    ][0],150));
	  cairo_line_to(cr, Geom::Point(handles[i    ][0],450));
	  cairo_move_to(cr, Geom::Point(handles[i+deg][0],150));
	  cairo_line_to(cr, Geom::Point(handles[i+deg][0],450));
	}
	cairo_move_to(cr, Geom::Point(0,300));
	cairo_line_to(cr, Geom::Point(600,300));
	
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 1);
        cairo_stroke(cr);

	for (int i=0;i<deg;i++){
	  B.push_back(BezOrd(-(handles[i][1]-300)*pow(4.,i),-(handles[i+deg][1]-300)*pow(4.,i)));
	}
	plot(cr,B,1);	
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0., 0., 0.8, 1);
        cairo_stroke(cr);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
    
	double m,M;
	clock_t chrono;
	int nbIter=1;

	std::cout<<std::endl;
	std::cout<<"--times (for "<<nbIter<<" iterations): --"<<std::endl;

	chrono=clock();
	for(int i=0;i<nbIter;i++){bounds(B,m,M);}
	chrono-=clock();
	std::cout<<"bounds: "<<-chrono<<std::endl;

	cairo_move_to(cr, Geom::Point(150,-m+300));
	cairo_line_to(cr, Geom::Point(450,-m+300));
	cairo_move_to(cr, Geom::Point(150,-M+300));
	cairo_line_to(cr, Geom::Point(450,-M+300));
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0., 0.5, 0.5, 1);
        cairo_stroke(cr);

	/*
	chrono=clock();
	for(int i=0;i<nbIter;i++){original_bounds(B,m,M);}
	chrono-=clock();
	std::cout<<"original: "<<-chrono<<std::endl;

	cairo_move_to(cr, Geom::Point(150,-m+300));
	cairo_line_to(cr, Geom::Point(450,-m+300));
	cairo_move_to(cr, Geom::Point(150,-M+300));
	cairo_line_to(cr, Geom::Point(450,-M+300));
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0.5, 0., 0., 1);
        cairo_stroke(cr);
	*/

	chrono=clock();
	for(int i=0;i<nbIter;i++){slow_bounds(B,m,M);}
	chrono-=clock();
	std::cout<<"eval: "<<-chrono<<std::endl;

	cairo_move_to(cr, Geom::Point(150,-M+300));
	cairo_line_to(cr, Geom::Point(450,-M+300));
	cairo_move_to(cr, Geom::Point(150,-m+300));
	cairo_line_to(cr, Geom::Point(450,-m+300));
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, .75, 0, 0, 1);
        cairo_stroke(cr);

	/*
	chrono=clock();
	for(int i=0;i<nbIter;i++){cut_bounds(B,m,M);}
	chrono-=clock();
	std::cout<<"cuts: "<<-chrono<<std::endl;

	cairo_move_to(cr, Geom::Point(150,-M+300));
	cairo_line_to(cr, Geom::Point(450,-M+300));
	cairo_move_to(cr, Geom::Point(150,-m+300));
	cairo_line_to(cr, Geom::Point(450,-m+300));
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 1, .5, 0, 1);
        cairo_stroke(cr);
	*/

	*notify<<"Use handles to set the coefficients of the s-basis."<<std::endl;
	*notify<<"-Blue=basic bounds,"<<std::endl;
	*notify<<"-Red =slower but sharper."<<std::endl;

        Toy::draw(cr, notify, width, height, save);
    }        
  
public:
    BoundsTester(){
        if(handles.empty()) {
            for(int i = 0; i < 8; i++)
	      handles.push_back(Geom::Point(0,300));
        }
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
//vim:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :

 	  	 
