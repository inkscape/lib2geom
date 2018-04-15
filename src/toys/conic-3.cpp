/**
 * elliptics via C-curves. (njh)
 * Limited to 180 degrees (by end point and tangent matching criteria)
 * Also represents cycloids
 */

#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/sbasis-math.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

using std::vector;
using namespace Geom;

Linear z0(0.5,1.);

unsigned total_pieces;

double sinC(double t) { return t - sin(t);}
double cosC(double t) { return 1 - cos(t);}
double tanC(double t) { return sinC(t) / cosC(t);}

class Conic3: public Toy {
    PointSetHandle psh;
    public:
    Conic3 () {
        psh.push_back(100, 500);
        psh.push_back(100, 500 - 200*M_PI/2);
        psh.push_back(500, 500 - 200*M_PI/2);
        psh.push_back(500, 500);
        handles.push_back(&psh);
    }

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
        cairo_set_line_width (cr, 0.5);
        cairo_stroke(cr);
        
        Geom::Point a[2] = {psh.pts[0] - psh.pts[1],
                        psh.pts[2] - psh.pts[1]};
        double angle = Geom::angle_between(a[0], a[1]);
        double len = std::max(Geom::L2(a[0]),
                                Geom::L2(a[1]));
        for(auto & p : a) 
        p = len*unit_vector(p);
        *notify << "angle = " << angle;
        *notify << " sinC = " << sinC(angle);
        *notify << " cosC = " << cosC(angle);
        *notify << " tanC = " << tanC(angle);
        vector<Geom::Point> e_a_h = psh.pts;
        
        double alpha = M_PI;
        Piecewise<SBasis> pw(Linear(0, alpha));
        Piecewise<SBasis> C = cos(pw);
        Piecewise<SBasis> S = sin(pw);
        Piecewise<SBasis> sinC = pw - S;
        Piecewise<SBasis> cosC = Piecewise<SBasis>(1) - C;
        Piecewise<SBasis> Z3 = sinC/sinC(1);
        Piecewise<SBasis> Z0 = reverse(Z3);
        Piecewise<SBasis> Z2 = cosC/cosC(1) - Z3;
        Piecewise<SBasis> Z1 = reverse(Z2);
        
        Piecewise<SBasis> Z[4] = {Z0, Z1, Z2, Z3};
        
        D2<Piecewise<SBasis> > B;
        for(unsigned dim  = 0; dim < 2; dim++) {
            B[dim] = Piecewise<SBasis>(0);
            for(unsigned i  = 0; i < 4; i++) {
                B[dim] += Z[i]*e_a_h[i][dim];
            }
        }
        cairo_d2_pw_sb(cr, B);
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new Conic3());

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
