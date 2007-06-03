#include "d2.h"
#include "s-basis.h"
#include "s-basis-2d.h"
#include "bezier-to-sbasis.h"
#include "choose.h"
#include "convex-cover.h"

#include "path.h"

#include "path-cairo.h"
#include "toy-framework.h"

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
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
        
        if(!save) {
            for(unsigned i = 0; i < handles.size(); i++) {
                handles[i][0] = width*handle_to_sb_t(i, handles.size())/2 + width/4;
                if(i)
                    cairo_line_to(cr, handles[i]);
                else
                    cairo_move_to(cr, handles[i]);
            }
        }
        
        D2<SBasis> B;
        B[0] = Linear(width/4, 3*width/4);
        B[1].resize(handles.size()/2);
        for(unsigned i = 0; i < B[1].size(); i++) {
            B[1][i] = Linear(0);
        }
        for(unsigned i = 0; i < handles.size(); i++) {
            handle_to_sb(i, handles.size(), B[1]) = 3*width/4 - handles[i][1];
        }
        for(unsigned i = 1; i < B[1].size(); i++) {
            B[1][i] = B[1][i]*choose<double>(2*i+1, i);
        }
        
        Interval bs = bounds_fast(B[1]);
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
    
        Geom::ConvexHull ch(handles);
    
        cairo_move_to(cr, ch.boundary.back());
        for(unsigned i = 0; i < ch.boundary.size(); i++) {
            cairo_line_to(cr, ch.boundary[i]);
        }
        Toy::draw(cr, notify, width, height, save);
    }
public:
Sb1d () {
    handles.push_back(Geom::Point(0,450));
    for(unsigned i = 0; i < 4; i++)
        handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    handles.push_back(Geom::Point(0,450));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
