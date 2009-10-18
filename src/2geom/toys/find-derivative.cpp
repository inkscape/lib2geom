#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-geometric.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

using std::vector;
using namespace Geom;
using namespace std;

// Author: Johan Engelen, 2009
//
// Shows how to find the locations on a path where the derivative is parallel to a certain vector.
//-----------------------------------------------

class FindDerivative: public Toy {
    PointSetHandle curve_handle;
    PointHandle sample_point;
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {

        D2<SBasis> B = curve_handle.asBezier();

        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
        cairo_d2_sb(cr, B);
        cairo_stroke(cr);

        Point vector = sample_point.pos - Geom::Point(400,400);
        cairo_move_to(cr, Geom::Point(400,400));
        cairo_line_to(cr, sample_point.pos);
        cairo_set_source_rgba (cr, 0., 0., 0.5, 0.8);
        cairo_stroke(cr);

        // How to find location of points with certain derivative along a path:
        D2<SBasis> deriv = derivative(B);
        SBasis dotp = dot(deriv, rot90(vector));
        std::vector<double> sol = roots(dotp);
        for (unsigned i = 0; i < sol.size(); ++i) {
            draw_handle(cr, B.valueAt(sol[i]));         // the solutions are in vector 'sol'
        }

        cairo_set_source_rgba (cr, 0.5, 0.2, 0., 0.8);
        cairo_stroke(cr);
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

public:
    FindDerivative(){
        if(handles.empty()) {
            handles.push_back(&curve_handle);
            handles.push_back(&sample_point);
            for(unsigned i = 0; i < 4; i++)
                curve_handle.push_back(150+uniform()*300,150+uniform()*300);
            sample_point.pos = Geom::Point(250,300);
        }
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new FindDerivative);
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


