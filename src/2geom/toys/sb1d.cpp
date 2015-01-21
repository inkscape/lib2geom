#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/choose.h>
#include <2geom/convex-hull.h>

#include <2geom/path.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <vector>
using std::vector;
using namespace Geom;

extern unsigned total_steps, total_subs;

double& handle_to_sb(unsigned i, unsigned n, SBasis &sb) {
    assert(i < n);
    assert(n <= sb.size()*2);
    unsigned k = i;
    if(k >= n/2) {
        k = n - k - 1;
        return sb[k][1];
    } else
        return sb[k][0];
}

double handle_to_sb_t(unsigned i, unsigned n) {
    double k = i;
    if(i >= n/2)
        k = n - k - 1;
    double t = k/(2*k+1);
    if(i >= n/2)
        return 1 - t;
    else
        return t;
}

class Sb1d: public Toy {
public:
    PointSetHandle hand;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
        
        if(!save) {
            for(unsigned i = 0; i < hand.pts.size(); i++) {
                hand.pts[i][0] = width*handle_to_sb_t(i, hand.pts.size())/2 + width/4;
                if(i)
                    cairo_line_to(cr, hand.pts[i]);
                else
                    cairo_move_to(cr, hand.pts[i]);
            }
        }
        
        D2<SBasis> B;
        B[0] = Linear(width/4, 3*width/4);
        B[1].resize(hand.pts.size()/2);
        for(unsigned i = 0; i < B[1].size(); i++) {
            B[1][i] = Linear(0);
        }
        for(unsigned i = 0; i < hand.pts.size(); i++) {
            handle_to_sb(i, hand.pts.size(), B[1]) = 3*width/4 - hand.pts[i][1];
        }
        for(unsigned i = 1; i < B[1].size(); i++) {
            B[1][i] = B[1][i]*choose<double>(2*i+1, i);
        }
        
        Interval bs = *bounds_fast(B[1]);
        double lo, hi;
        lo = 3*width/4 - bs.min();
        hi = 3*width/4 - bs.max();
        cairo_move_to(cr, B[0](0), lo);
        cairo_line_to(cr, B[0](1), lo);
        cairo_move_to(cr, B[0](0), hi);
        cairo_line_to(cr, B[0](1), hi);
        cairo_stroke(cr);
        *notify << "sb bounds = "<<lo << ", " <<hi<<std::endl;
        //B[1] = SBasis(Linear(3*width/4)) - B[1];
        *notify << B[0] << ", "; 
        *notify << B[1];
        Geom::Path pb;
        B[1] = SBasis(Linear(3*width/4)) - B[1];
        pb.append(B);
        pb.close(false);
        cairo_path(cr, pb);
    
        cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
        cairo_stroke(cr);
    
        Geom::ConvexHull ch(hand.pts);
    
        cairo_move_to(cr, ch.back());
        for(unsigned i = 0; i < ch.size(); i++) {
            cairo_line_to(cr, ch[i]);
        }
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
public:
Sb1d () {
    hand.pts.push_back(Geom::Point(0,450));
    for(unsigned i = 0; i < 4; i++)
        hand.pts.push_back(Geom::Point(uniform()*400, uniform()*400));
    hand.pts.push_back(Geom::Point(0,450));
    handles.push_back(&hand);
}
};

int main(int argc, char **argv) {
    init(argc, argv, new Sb1d());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
