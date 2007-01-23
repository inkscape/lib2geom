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

// TODO: 
// use path2
// replace Ray stuff with path2 line segments.

//-----------------------------------------------

class CurvatureTester: public Toy {

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    
        MultidimSBasis<2> B = bezier_to_sbasis<2, 3>(handles.begin());

        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
        cairo_md_sb(cr, B);
        cairo_stroke(cr);

	handles[4][1]=400;
	handles[4][0]=std::max(150.,handles[4][0]);
	handles[4][0]=std::min(450.,handles[4][0]);
	cairo_move_to(cr, Geom::Point(150,400));
	cairo_line_to(cr, Geom::Point(450,400));
	cairo_set_source_rgba (cr, 0., 0., 0.5, 0.8);
	cairo_stroke(cr);

	double t=std::max(0.,std::min(1.,(handles[4][0]-150)/300.));
	vector<double> cuts;
        vector<SBasis> K=curvature(B,cuts);
        
        for(int ix = 0; ix < K.size(); ix++) {
            MultidimSBasis<2> Kxy;
            Kxy[1] = 300*K[ix] + BezOrd(300);
            Kxy[0] = BezOrd(300*cuts[ix], 300*cuts[ix+1]);
            cairo_md_sb(cr, Kxy);
            cairo_stroke(cr);
        }
        
	int i=0;
	double t0=0,t1=cuts[0];
	while (t1<t){t0=t1;t1=cuts[++i];}
	double radius=K[i]((t-t0)/(t1-t0));
	*notify<<"K="<<radius<<std::endl;
	if (fabs(radius)>1e-4){
	  radius=1./radius;
	  Geom::Point normal=unit_vector(point_at(derivative(B),t));
	  normal=rot90(normal);
	  Geom::Point center=point_at(B,t)-radius*normal;
	  
	  cairo_arc(cr, center[0], center[1],fabs(radius), 0, M_PI*2);
	  draw_handle(cr, center);
	  draw_handle(cr, point_at(B,t));
	}else{
	  Geom::Point p=point_at(B,t);
	  Geom::Point v=point_at(derivative(B),t);
	  draw_handle(cr, p);
	  cairo_move_to(cr, p-100*v);
	  cairo_line_to(cr, p+100*v);
	}
	cairo_set_source_rgba (cr, 0.5, 0.2, 0., 0.8);
	cairo_stroke(cr);
        Toy::draw(cr, notify, width, height, save);
    }        
  
public:
    CurvatureTester(){
        if(handles.empty()) {
            for(int i = 0; i < 4; i++)
	      handles.push_back(Geom::Point(150+uniform()*300,150+uniform()*300));
	      //handles.push_back(Geom::Point(200+50*i,300));
                handles.push_back(Geom::Point(250,300));
        }
    }
};

int main(int argc, char **argv) {
    std::cout << "testing unit_normal(multidim_sbasis) based offset." << std::endl;
    init(argc, argv, "offset-test", new CurvatureTester);
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

 	  	 
