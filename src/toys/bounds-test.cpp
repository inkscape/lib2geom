#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "solver.h"
#include "multidim-sbasis.h"
#include "s-basis-2d.h"
#include "sb-geometric.h"

#include "path.h"
#include "path-cairo.h"
#include "path-builder.h"

#include "toy-framework.h"

#include <time.h>

using std::vector;
using namespace Geom;
using namespace std;

static void plot(cairo_t* cr, SBasis const &B,double vscale=1){
      MultidimSBasis<2> plot;
      plot[0]=SBasis(BezOrd(150,450));
      plot[1]=-vscale*B;
      plot[1]+=300;
      cairo_md_sb(cr, plot);
}

static void old_bounds(SBasis const & s,
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


class BoundsTester: public Toy {

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    
	SBasis B;
	for (int i=0;i<4;i++){
	  handles[i][0]=50+20*i;
	  cairo_move_to(cr, Geom::Point(50+20*i,150));
	  cairo_line_to(cr, Geom::Point(50+20*i,450));
	}
	for (int i=4;i<8;i++){
	  handles[i][0]=410+20*i;
	  cairo_move_to(cr, Geom::Point(410+20*i,150));
	  cairo_line_to(cr, Geom::Point(410+20*i,450));
	}
	cairo_move_to(cr, Geom::Point(0,300));
	cairo_line_to(cr, Geom::Point(600,300));
	

// 	handles[0][0]=50;
// 	handles[1][0]=70;
// 	handles[2][0]=90;
// 	handles[3][0]=510;
// 	handles[4][0]=530;
// 	handles[5][0]=550;

// 	cairo_move_to(cr, Geom::Point(0,300));
// 	cairo_line_to(cr, Geom::Point(600,300));
// 	cairo_move_to(cr, Geom::Point(50,150));
// 	cairo_line_to(cr, Geom::Point(50,450));
// 	cairo_move_to(cr, Geom::Point(70,150));
// 	cairo_line_to(cr, Geom::Point(70,450));
// 	cairo_move_to(cr, Geom::Point(90,150));
// 	cairo_line_to(cr, Geom::Point(90,450));
// 	cairo_move_to(cr, Geom::Point(510,150));
// 	cairo_line_to(cr, Geom::Point(510,450));
// 	cairo_move_to(cr, Geom::Point(530,150));
// 	cairo_line_to(cr, Geom::Point(530,450));
// 	cairo_move_to(cr, Geom::Point(550,150));
// 	cairo_line_to(cr, Geom::Point(550,450));
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 1);
        cairo_stroke(cr);


	B.push_back(BezOrd(handles[0][1]-300,handles[4][1]-300));
	B.push_back(BezOrd(handles[1][1]-300,handles[5][1]-300));
	B.push_back(BezOrd(handles[2][1]-300,handles[6][1]-300));
	B.push_back(BezOrd(handles[3][1]-300,handles[7][1]-300));

	plot(cr,B,-1);	
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
        cairo_stroke(cr);
    
	double m,M;
	bounds(B,m,M);

	cairo_move_to(cr, Geom::Point(150,m+300));
	cairo_line_to(cr, Geom::Point(450,m+300));
	cairo_move_to(cr, Geom::Point(150,M+300));
	cairo_line_to(cr, Geom::Point(450,M+300));
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0., 0.5, 0.5, 1);
        cairo_stroke(cr);

	old_bounds(B,m,M);

	cairo_move_to(cr, Geom::Point(150,m+300));
	cairo_line_to(cr, Geom::Point(450,m+300));
	cairo_move_to(cr, Geom::Point(150,M+300));
	cairo_line_to(cr, Geom::Point(450,M+300));
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0.5, 0., 0., 1);
        cairo_stroke(cr);

	*notify<<"Use handles to set the coefficients of the s-basis."<<std::endl;

        Toy::draw(cr, notify, width, height, save);
    }        
  
public:
    BoundsTester(){
        if(handles.empty()) {
            for(int i = 0; i < 8; i++)
	      handles.push_back(Geom::Point(150+uniform()*300,300));
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

 	  	 
