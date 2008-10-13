#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-geometric.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <time.h>
using std::vector;
using namespace Geom;
using namespace std;

//-----------------------------------------------

class CurvatureTester: public Toy {
    PointSetHandle curve_handle;
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    
        D2<SBasis> B = curve_handle.asBezier();

        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);

	for(int i = 0; i < 2; i++) {
            Geom::Point center=curve_handle.pts[1+i];
            Geom::Point normal=center- curve_handle.pts[3*i];
            double radius = Geom::L2(normal);
            *notify<<"K="<<radius<<std::endl;
            if (fabs(radius)>1e-4){
	    
                cairo_arc(cr, center[0], center[1],fabs(radius), 0, M_PI*2);
                draw_handle(cr, center);
            }else{
            }
            cairo_set_source_rgba (cr, 0.5, 0.2, 0., 0.8);
            cairo_stroke(cr);
	}
	if (1) {
            Geom::Point A = curve_handle.pts[0];
            Geom::Point B = curve_handle.pts[3];
            Geom::Point dA = curve_handle.pts[1]-A;
            Geom::Point dB = B-curve_handle.pts[2];
            std::vector<D2<SBasis> > candidates = 
                cubics_fitting_curvature(curve_handle.pts[0],curve_handle.pts[3],
                                         rot90(dA),
                                         rot90(dB),
                                         -L2sq(dA), L2sq(dB));
	  
            if (candidates.size()==0) {
                cairo_md_sb(cr,  D2<SBasis>(Linear(A[X],B[X]),Linear(A[Y],B[Y])));
                cairo_stroke(cr);
            } else {
                //TODO: I'm sure std algorithm could do that for me...
                double error = -1;
                unsigned best = 0;
                for (unsigned i=0; i<candidates.size(); i++){

                    cairo_md_sb(cr, candidates[i]);
                    double l = length(candidates[i]);
	  
                    if ( l < error || error < 0 ){
                        error = l;
                        best = i;
                    }
                }
                cairo_md_sb(cr, candidates[best]);
                cairo_stroke(cr);
            }
        }
        Toy::draw(cr, notify, width, height, save);
    }        
  
    void canvas_click(Geom::Point at, int button) {
        std::cout << "clicked at:" << at << " with button " << button << std::endl;
        if(button == 1) {
            curve_handle.push_back(at+Point(100,100));
            curve_handle.push_back(at);
        }
    }

public:
    CurvatureTester(){
        if(handles.empty()) {
            handles.push_back(&curve_handle);
            for(unsigned i = 0; i < 4; i++)
                curve_handle.push_back(150+uniform()*300,150+uniform()*300);
        }
    }
};

int main(int argc, char **argv) {
    std::cout << "testing unit_normal(multidim_sbasis) based offset." << std::endl;
    init(argc, argv, new CurvatureTester);
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

 	  	 
