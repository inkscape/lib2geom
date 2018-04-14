/**
 * elliptics via 5 point w-pi basis. (njh)
 * Affine, endpoint, tangent, exact circle
 * full circle.  Convex containment implies small circle.
 * Also represents lumpy polar type curves
 */

#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-math.h>
#include <2geom/bezier-to-sbasis.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

using std::vector;
using namespace Geom;

const double w = 1./3;
const double cwp = cos(w*M_PI);
const double swp = sin(w*M_PI);
/*double phi(double t, double w) { return sin(w*t) - w*sin(t); }
double phih(double t, double w) { return sin(w*t) + w*sin(t); }
double b4(double t, double w) {return phi(t/2,w)*phih(t/2,w)/(swp*swp);}
double b3(double t, double w) {return cwp*phi(t,w)/(2*swp) - cwp*cwp*b4(t,w); }
double b2(double t, double w) {return 2*w*w*sin(t/2)*sin(t/2);}
double b1(double t, double w) {return b3(2*M_PI - t, w);}
double b0(double t, double w) {return b4(2*M_PI - t, w);}*/

class arc_basis{
public:
    Piecewise<SBasis> basis[5];
    double w;
    
    Piecewise<SBasis> phi(Piecewise<SBasis> const &d, double w) { 
        return sin(d*w) - sin(d)*w; 
    }
    Piecewise<SBasis> phih(Piecewise<SBasis> const &d, double w) { 
        return sin(d*w) + sin(d)*w; 
    }
    Piecewise<SBasis> b4(Piecewise<SBasis> const &d, double w) {
        return phi(d*.5,w)/(swp*swp)*phih(d*.5,w);
    }
    Piecewise<SBasis> b3(Piecewise<SBasis> const &d, double w) {
        return phi(d,w)*(cwp/(2*swp)) - b4(d,w)*(cwp*cwp); 
    }

    Piecewise<SBasis> b2(Piecewise<SBasis> const &d, double w) {
        return sin(d*.5)*(2*w*w)*sin(d*.5);
    }
    Piecewise<SBasis> b1(Piecewise<SBasis> const &d, double w) {
        return b3(reverse(d), w);
    }
    Piecewise<SBasis> b0(Piecewise<SBasis> const &d, double w) {
        return b4(reverse(d), w);
    }
    
    
    arc_basis(double w) {
        Piecewise<SBasis> dom(Linear(0, 2*M_PI));
        basis[0] = b4(dom, w);
        basis[1] = b3(dom, w);
        basis[2] = b2(dom, w);
        basis[3] = b1(dom, w);
        basis[4] = b0(dom, w);
    }

};

class Conic4: public Toy {
    PointSetHandle psh;
    public:
    Conic4 () {
        double sc = 30;
        Geom::Point c(6*sc, 6*sc);
        psh.push_back(sc*Geom::Point(0,0)+c);
        psh.push_back(sc*Geom::Point(tan(w*M_PI)/w, 0)+c);
        psh.push_back(sc*Geom::Point(0, 1/(w*w))+c);
        psh.push_back(sc*Geom::Point(-tan(w*M_PI)/w, 0)+c);
        psh.push_back(sc*Geom::Point(0,0)+c);
        handles.push_back(&psh);
    }

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        std::vector<Geom::Point> e_h = psh.pts;
        for(int i = 0; i < 5; i++) {
            Geom::Point p = e_h[i];
        
            if(i)
                cairo_line_to(cr, p);
            else
                cairo_move_to(cr, p);
        }
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        cairo_stroke(cr);
        
        arc_basis ab(1./3);
        D2<Piecewise<SBasis> > B;
        
        for(unsigned dim  = 0; dim < 2; dim++)
            for(unsigned i  = 0; i < 5; i++)
                B[dim] += ab.basis[i]*e_h[i][dim];
        
        cairo_d2_pw_sb(cr, B);
        cairo_set_source_rgba (cr, 1., 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new Conic4());

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
