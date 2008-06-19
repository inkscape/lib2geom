#include "d2.h"
#include "sbasis.h"
#include "bezier-to-sbasis.h"
#include "path.h"
#include "point.h"

#include "path-cairo.h"
#include "toy-framework.h"

using std::string;
using std::vector;
using namespace Geom;

class RatBez: public Toy {
virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
    cairo_set_line_width (cr, 1);
    cairo_stroke(cr);
    
    // Convert 4 handles to md_sb
    D2<SBasis> Bz = handles_to_sbasis<3>(handles.begin());
    
    // Draw it in orange
    Geom::Path pb;
    pb.append(Bz);
    pb.close(false);
    cairo_path(cr, pb);
    cairo_set_source_rgba (cr, 1., 0.5, 0, 0.8);
    cairo_stroke(cr);
    
    // We make a 3d md_sb that represents 2d curve and weight function
    MultidimSBasis<3> B;
    // first two are curve
    for(int dim = 0; dim < 2; dim++) {
        B[dim] = Bz[dim];
    }
    // last is weighting function.  This supposedly creates a bezier
    B[2] =  (Linear(1, 0)*Linear(1, sqrt(2))) +
        (Linear(0, 1)*Linear(sqrt(2), 1));
    
    // draw a pointwise approximation - compute exact values for points and join with line segments
    for(int ti = 0; ti <= 30; ti++) {
        double t = (double(ti))/(30);
        double w = B[2].point_at(t);
        double x = B[0].point_at(t)/w; // this is the rational part
        double y = B[1].point_at(t)/w;
        if(ti)
            cairo_line_to(cr, x, y);
        else
            cairo_move_to(cr, x, y);
    }    
    // dark green
    cairo_set_line_width (cr, 2);
    cairo_set_source_rgba (cr, 0., 0.125, 0, 0.5);
    cairo_stroke(cr);
    cairo_set_line_width (cr, 1);

    // purple
    cairo_set_source_rgba (cr, 0.5, 0., 1, 1);
    
    // subdivide the curve into N pieces
    int N = 2;
    for(int subdivi = 0; subdivi < N; subdivi++) {
        double dsubu = 1./N;
        double subu = dsubu*subdivi;
        MultidimSBasis<3> Bp;
        for(int dim = 0; dim < 3; dim++) {
            Bp[dim] = compose(B[dim], Linear(subu, dsubu+subu));
        }
        // subdivided
        
        // find cubic approximation to division
        D2<SBasis> Bu;
        for(int dim = 0; dim < 2; dim++) {
	    Bu[dim] = divide(Bp[dim], Bp[2], 1);
        }
        
        // draw it
        Geom::Path pb;
        pb.append(Bu);
        pb.close(false);
        cairo_path(cr, pb);
    }
    cairo_stroke(cr);
    Toy::draw(cr, notify, width, height, save);
}
public:
RatBez () {
    for(int i = 0; i < 4; i++) handles.push_back(Geom::Point(uniform()*400, uniform()*400));
}
};

int main(int argc, char **argv) {
    init(argc, argv, new RatBez());

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
