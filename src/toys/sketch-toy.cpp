#include "pw.h"
#include "d2.h"
#include "s-basis.h"
#include "matrix.h"

#include "path-cairo.h"
#include "toy-framework.h"

#include <vector>
using namespace Geom;

class SketchToy: public Toy {
    Piecewise<D2<SBasis> > ink;
    vector<Point> poly;
    vector<Point> old;
    
    void smoothify() {
        if(poly.size()<4) return;
        ink.push(crapfit(3), ink.cuts.back() + 1);
        Point temp = poly.back();
        old.insert(old.end(), poly.begin(), poly.end());
        poly.clear();
        poly.push_back(temp);
    }

    //most rediculous 'curve fitting' function ever
    D2<SBasis> crapfit(unsigned order) {
        /*D2<SBasis> best;
        double bestfit = 1000;
        for(unsigned i = poly.size()-1; i>=order*2-1; i++) {*/
            D2<SBasis> sb;
            for(unsigned d = 0; d<2; d++) {
                sb[d].push_back(Linear(poly[0][d], poly[poly.size()-1][d]));
                double m = (poly[1][d] - poly[0][d]) / poly.size();
                vector<double> diffs;
                for(unsigned i = 1; i < poly.size(); i++)
                    diffs.push_back(poly[i][d] - poly[i-1][d] - m * i);
                for(unsigned o = 1; o < order; o++) {
                    double lavg = 0, ravg = 0, ltot = 0, rtot = 0;
                    for(unsigned i = 1; i < poly.size() - o; i++) {
                        double t = double(i)/poly.size();
                        ltot += (1-t)/pow(t,o);
                        lavg += diffs[i] * (1-t);
                        rtot += t/pow(t,o);
                        ravg += diffs[i] * t;
                    }
                    //lavg /= ltot; ravg /= rtot;
                    sb[d].push_back(Linear(lavg, -ravg));
                    vector<double> temp(diffs);
                    diffs.clear();
                    for(unsigned i = 1; i < diffs.size(); i++)
                        diffs.push_back(temp[i] - temp[i-1]);
                    
                }
            }
            return sb;
            /*err = 0;
            for(unsigned j = 0; j<ink.size(); j++) {
                err += distanceSq(sb(double(j)/ink.size()), ink[j]); 
            }
            if(err*i<bestfit)
            if(i == 5) return best;
            if(err < 5) return sb;
        }*/
    }

    
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0, 0, 0, 1);
        cairo_pw_d2(cr, ink);
        cairo_set_line_width(cr,1);
        if(!save) {
            for(unsigned i = 0; i < ink.size(); i++)
                draw_cross(cr, ink[i].at0());
        }
        for(unsigned i = 1; i < poly.size(); i++)
            draw_line_seg(cr, poly[i-1], poly[i]);
        cairo_close_path(cr);

        cairo_set_line_width(cr,.5);
        for(unsigned i = 1; i < old.size(); i++)
            draw_line_seg(cr, old[i-1], old[i]);
        cairo_close_path(cr);
        
        Toy::draw(cr, notify, width, height, save);
    }
    void mouse_released(GdkEventButton* e) {
        if(e->state & (GDK_BUTTON1_MASK)) {
            smoothify();
            poly.clear();
            redraw();
        }
    }
    void mouse_moved(GdkEventMotion* e) {
        Geom::Point mouse(e->x, e->y);
        
        if(e->state & (GDK_BUTTON1_MASK)) {
            poly.push_back(mouse);
            if(poly.size()>100) smoothify();
            redraw();
        }
    }

    bool should_draw_numbers() { return false; }

    public:
    SketchToy () {
        ink.push_cut(0.);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "sketch-toy", new SketchToy());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
