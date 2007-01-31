#include "s-basis.h"
#include "point.h"

#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "path2.h"
#include "path-cairo.h"
#include "multidim-sbasis.h"

#include "toy-framework.h"

using std::string;
using std::vector;
using namespace Geom;

class RatBez: public Toy {
virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
    cairo_set_line_width (cr, 0.5);
    cairo_stroke(cr);
    
    MultidimSBasis<2> Bz = bezier_to_sbasis<2, 3>(handles.begin());
        Geom::Path2::Path pb;
        pb.append(Bz);
        pb.close(false);
        cairo_path(cr, pb);
    cairo_stroke(cr);
        
    MultidimSBasis<3> B;
    for(int dim = 0; dim < 2; dim++) {
        B[dim] = Bz[dim];
    }
    B[2] =  (BezOrd(1, 0)*BezOrd(1, sqrt(2))) +
        (BezOrd(0, 1)*BezOrd(sqrt(2), 1));
    
    for(int ti = 0; ti <= 30; ti++) {
        double t = (double(ti))/(30);
        double w = B[2].point_at(t);
        double x = B[0].point_at(t)/w;
        double y = B[1].point_at(t)/w;
        if(ti)
            cairo_line_to(cr, x, y);
        else
            cairo_move_to(cr, x, y);
    }    
    cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
    cairo_stroke(cr);
    cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
    
    int N = 2;
    for(int subdivi = 0; subdivi < N; subdivi++) {
        double dsubu = 1./N;
        double subu = dsubu*subdivi;
        MultidimSBasis<3> Bp;
        for(int dim = 0; dim < 3; dim++) {
            Bp[dim] = compose(B[dim], BezOrd(subu, dsubu+subu));
        }
        
        MultidimSBasis<2> Bu;
        for(int dim = 0; dim < 2; dim++) {
	    Bu[dim] = divide(Bp[dim], Bp[2], 1);
        }
        
        Geom::Path2::Path pb;
        pb.append(Bu);
        pb.close(false);
        cairo_path(cr, pb);
    }
    Toy::draw(cr, notify, width, height, save);
}
public:
RatBez () {
    for(int i = 0; i < 4; i++) handles.push_back(Geom::Point(uniform()*400, uniform()*400));
}
};

int main(int argc, char **argv) {
    init(argc, argv, "Converting NURBs to sbasis", new RatBez());

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
