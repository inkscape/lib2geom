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

class InverseTester: public Toy {
  int size;

  void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    
      SBasis f=SBasis(BezOrd(0,1));
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
          f.push_back(BezOrd(-(handles[i     ][1]-300)*pow(4.,i+1)/300,
                             -(handles[i+size][1]-300)*pow(4.,i+1)/300 ));
      }
      plot(cr,BezOrd(0,1),300);	
      
      f.normalize();
      cairo_set_line_width (cr, 2);
      cairo_set_source_rgba (cr, 0., 0., 0.8, 1);
      plot(cr,f,300);	
      
      *notify<<"Use handles to set the coefficients of the (blue) s-basis."<<std::endl;
      *notify<<" (keep it monotonic!)"<<std::endl;
      *notify<<"red=flipped inverse; should be the same as the blue one."<<std::endl;
      
      SBasis g=inverse(f,6); 
      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0.8, 0., 0., 1);
      plot_flip(cr,g,300);	
      

      Toy::draw(cr, notify, width, height, save);
  }        
  
public:
  InverseTester(){
    size=4;
    if(handles.empty()) {
      for(int i = 0; i < 2*size; i++)
	handles.push_back(Geom::Point(0,150+150+uniform()*300*0));
    }
    handles[0][1]+=10;
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
// vim: filetype = cpp:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :
