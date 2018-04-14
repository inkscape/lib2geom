#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-geometric.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#include <time.h>
using std::vector;
using namespace Geom;
using namespace std;

//-----------------------------------------------

class CurvatureTester: public Toy {
    PointSetHandle curve_handle;
    Path current_curve;
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        cairo_set_line_width (cr, 1);
        current_curve = Path();
        
        for(int base_i = 0; base_i < int(curve_handle.pts.size()/2) - 1; base_i++) {
            for(int i = 0; i < 2; i++) {
                Geom::Point center=curve_handle.pts[1+2*i+base_i*2];
                Geom::Point normal=center- curve_handle.pts[2*i+base_i*2];
                double radius = Geom::L2(normal);
                *notify<<"K="<<radius<<std::endl;
                if (fabs(radius)>1e-4){
                    
                    double ang = atan2(-normal);
                    cairo_arc(cr, center[0], center[1],fabs(radius), ang-0.3, ang+0.3);
                    cairo_set_source_rgba (cr, 0.75, 0.89, 1., 1);
                    cairo_stroke(cr);
                    
                    
                    //draw_handle(cr, center);
                }else{
                }
                {
                cairo_save(cr);
                double dashes[2] = {10, 10};
                cairo_set_dash(cr, dashes, 2, 0);
                cairo_move_to(cr, center);
                cairo_line_to(cr, center-normal);
                cairo_stroke(cr);
                cairo_restore(cr);
                }
            }
            cairo_set_source_rgba (cr, 0., 0, 0., 1);
                Geom::Point A = curve_handle.pts[0+base_i*2];
                Geom::Point B = curve_handle.pts[2+base_i*2];
            D2<SBasis> best_c = D2<SBasis>(SBasis(Linear(A[X],B[X])),SBasis(Linear(A[Y],B[Y])));
            double error = -1;
            for(int i = 0; i < 16; i++) {
                Geom::Point dA = curve_handle.pts[1+base_i*2]-A;
                Geom::Point dB = curve_handle.pts[3+base_i*2]-B;
                std::vector<D2<SBasis> > candidates = 
                    cubics_fitting_curvature(curve_handle.pts[0+base_i*2],curve_handle.pts[2+base_i*2],
                                             (i&2)?rot90(dA):-rot90(dA),
                                             (i&1)?rot90(dB):-rot90(dB),
                                             ((i&4)?-1:1)*L2sq(dA), ((i&8)?-1:1)*L2sq(dB));
	  
                if (candidates.empty()) {
                } else {
                    //TODO: I'm sure std algorithm could do that for me...
                    unsigned best = 0;
                    for (unsigned i=0; i<candidates.size(); i++){
                        Piecewise<SBasis> K = arcLengthSb(candidates[i]);
        
                        double l = Geom::length(candidates[i]);
                        //double l = K.segs.back().at1();//Geom::length(candidates[i]);
                        //printf("l = %g\n", l);
                        if ( l < error || error < 0 ){
                            error = l;
                            best = i;
                            best_c = candidates[best];
                        }
                    }
                }
            }
            if(error >= 0) {
                //cairo_d2_sb(cr, best_c);
                current_curve.append(best_c);
            }
        }
        
        cairo_path(cr, current_curve);
        cairo_stroke(cr);
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }        
  
    void canvas_click(Geom::Point at, int button) override {
        std::cout << "clicked at:" << at << " with button " << button << std::endl;
        if(button == 1) {
            double dist;
            double t = current_curve.nearestTime(at, &dist).asFlatTime();
            if(dist > 5) {
                curve_handle.push_back(at);
                curve_handle.push_back(at+Point(100,100));
            } else {
                // split around t
                Piecewise<D2<SBasis> > pw =  current_curve.toPwSb();
                std::vector<Point > vnd = pw.valueAndDerivatives(t, 2);
                Point on_curve = current_curve(t);
                Point normal = rot90(vnd[1]);
                Piecewise<SBasis > K = curvature(pw);
                Point ps[2] = {on_curve, on_curve+unit_vector(normal)/K(t)};
                curve_handle.pts.insert(curve_handle.pts.begin()+2*(int(t)+1), ps, ps+2);
            }
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

 	  	 
