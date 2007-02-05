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

static double upper_bound(SBasis f, unsigned NbCuts=3){
  f.normalize();
  if (f.size()==0){return(0);}
  if (f.size()==1){return(max(f[0][0],f[0][1]));}
  //  if (f.size()==2){return(max_BB(f[0],f[1]));}

  double M=f[0][0];
  BezOrd sb10_hi[3*NbCuts];
  BezOrd sb10_lo[3*NbCuts];
  BezOrd sb01_hi[3*NbCuts];
  BezOrd sb01_lo[3*NbCuts];
  //linearize s(t)*(1-t)
  for (int i=0; i<NbCuts; i++){
    double a,b,t,fa,fb,ft,vt;
    a= i   /3./NbCuts;
    b=(i+1)/3./NbCuts;
    fa=a*(1-a)*(1-a);
    fb=b*(1-b)*(1-b);
    sb10_lo[i]=BezOrd(fa-(fb-fa)/(b-a)*a,fb+(fb-fa)/(b-a)*(1-b));
    t=(a+b)/2;
    ft=t*(1-t)*(1-t);
    vt=(1-t)*(1-3*t);
    sb10_hi[i]=BezOrd(ft-vt*t,ft+vt*(1-t));

    a+=1./3.;
    b+=1./3.;
    fa=a*(1-a)*(1-a);
    fb=b*(1-b)*(1-b);
    sb10_lo[i+NbCuts]=BezOrd(fa-(fb-fa)/(b-a)*a,fb+(fb-fa)/(b-a)*(1-b));
    t=(a+b)/2;
    ft=t*(1-t)*(1-t);
    vt=(1-t)*(1-3*t);
    sb10_hi[i+NbCuts]=BezOrd(ft-vt*t,ft+vt*(1-t));

    a+=1./3.;
    b+=1./3.;
    fa=a*(1-a)*(1-a);
    fb=b*(1-b)*(1-b);
    sb10_hi[i+2*NbCuts]=BezOrd(fa-(fb-fa)/(b-a)*a,fb+(fb-fa)/(b-a)*(1-b));
    t=(a+b)/2;
    ft=t*(1-t)*(1-t);
    vt=(1-t)*(1-3*t);
    sb10_lo[i+2*NbCuts]=BezOrd(ft-vt*t,ft+vt*(1-t));
  }

  //linearize s(t)*t...
  for (int i=0; i<3*NbCuts; i++){
    double x;
    sb01_lo[i]=sb10_lo[3*NbCuts-i-1];
    x=sb01_lo[i][0];sb01_lo[i][0]=sb01_lo[i][1];sb01_lo[i][1]=x;
    sb01_hi[i]=sb10_hi[3*NbCuts-i-1];
    x=sb01_hi[i][0];sb01_hi[i][0]=sb01_hi[i][1];sb01_hi[i][1]=x;
  }

  BezOrd tail[3*NbCuts];
  BezOrd new_tail[3*NbCuts];
  int deg_max=f.size()-1;
  for (int i=0; i<3*NbCuts; i++){
    tail[i]=f[deg_max];
  }
  for (int deg=deg_max;deg>0;deg--){
    for (int i=0; i<3*NbCuts; i++){
      if (tail[i][0]>0){
	new_tail[i]=f[deg-1]+tail[i][0]*sb10_hi[i];
      }else{
	new_tail[i]=f[deg-1]+tail[i][0]*sb10_lo[i];
      }
      if (tail[i][1]>0){
	new_tail[i]+=tail[i][1]*sb01_hi[i];
      }else{
	new_tail[i]+=tail[i][1]*sb01_lo[i];
      }
      tail[i]=new_tail[i];
    }
  }
  for (int i=0; i<3*NbCuts; i++){
    double a,b;
    a= i   /3./NbCuts;
    b=(i+1)/3./NbCuts;
    M=max(M,max(tail[i](a),tail[i](b)));
  }
  return(M);
}

static double cut_bounds(SBasis f,double &m, double &M, unsigned NbCuts=3){
  M= upper_bound( f,NbCuts);
  m=-upper_bound(-f,NbCuts);
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
	B.normalize();
	plot(cr,B,1);	
        cairo_set_source_rgba (cr, 0., 0., 0.8, 1);
        cairo_stroke(cr);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
    
	double m,M;
	*notify<<"Use handles to set the coefficients of the s-basis."<<std::endl;
	*notify<<"-Blue=basic bounds,"<<std::endl;
	*notify<<"-Red =slower but sharper."<<std::endl;

        clock_t end_t;
        unsigned iterations = 0;

        end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
	  bounds(B,m,M);
	  iterations++;
        }
        *notify << 1000*0.1/iterations <<" ms = bounds time"<< std::endl;

	//bounds(B,m,M);
	cairo_move_to(cr, Geom::Point(150,-m+300));
	cairo_line_to(cr, Geom::Point(450,-m+300));
        cairo_set_source_rgba (cr, 0., 0.5, 0.5, 1);
        cairo_stroke(cr);
	cairo_move_to(cr, Geom::Point(150,-M+300));
	cairo_line_to(cr, Geom::Point(450,-M+300));
        cairo_set_source_rgba (cr, 0., 0.75, 0.75, 1);
        cairo_stroke(cr);


        end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
	  cut_bounds(B,m,M);
	  iterations++;
        }
        *notify << 1000*0.1/iterations <<" ms = cut bounds time"<< std::endl;

	cairo_move_to(cr, Geom::Point(150,-M+300));
	cairo_line_to(cr, Geom::Point(450,-M+300));
	cairo_move_to(cr, Geom::Point(150,-m+300));
	cairo_line_to(cr, Geom::Point(450,-m+300));
        cairo_set_source_rgba (cr, 0, .75, 0, 1);
        cairo_stroke(cr);

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
// vim: filetype = cpp:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :

 	  	 
