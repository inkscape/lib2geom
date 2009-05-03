#include <iostream>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/path-intersection.h>
#include <2geom/basic-intersection.h>
#include <2geom/pathvector.h>
#include <2geom/exception.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/path-intersection.h>
#include <2geom/nearest-point.h>
#include <2geom/line.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>

#include <cstdlib>
#include <map>
#include <vector>
#include <algorithm>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/ord.h>

#include <2geom/numeric/linear_system.h>


#include <2geom/conicsec.h>

using namespace Geom;
using namespace std;


// File: convert.h
#include <sstream>
#include <stdexcept>
 
class BadConversion : public std::runtime_error {
public:
    BadConversion(const std::string& s)
        : std::runtime_error(s)
    { }
};
 
template <typename T>
inline std::string stringify(T x)
{
    std::ostringstream o;
    if (!(o << x))
        throw BadConversion("stringify(T)");
    return o.str();
}

namespace Geom{
xAx degen;
};

void draw_hull(cairo_t*cr, RatQuad rq) {
    cairo_move_to(cr, rq.P[0]);
    cairo_line_to(cr, rq.P[1]);
    cairo_line_to(cr, rq.P[2]);
    cairo_stroke(cr);
}
  


void draw(cairo_t* cr, xAx C, Rect bnd) {
    vector<double> prev_rts;
    double py = bnd[1][0];
    for(int i = 0; i < 100; i++) {
        double t = i/100.;
        double y = (1-t)*bnd[1][0] + t*bnd[1][1];
        vector<double> rts = C.roots(Point(1, 0), Point(0, y));
        if(rts.size() == prev_rts.size()) {
            for(unsigned j = 0; j < rts.size(); j++) {
                cairo_move_to(cr, prev_rts[j], py);
                cairo_line_to(cr, rts[j], y);
            }
        } else if(prev_rts.size() == 1) {
            for(unsigned j = 0; j < rts.size(); j++) {
                cairo_move_to(cr, prev_rts[0], py);
                cairo_line_to(cr, rts[j], y);
            }
        } else if(rts.size() == 1) {
            for(unsigned j = 0; j < prev_rts.size(); j++) {
                cairo_move_to(cr, prev_rts[j], py);
                cairo_line_to(cr, rts[0], y);
            }
        } else {
            for(unsigned j = 0; j < rts.size(); j++) {
                cairo_move_to(cr, rts[j], y);
                cairo_rel_line_to(cr, 1,1);
            }
        }
        cairo_stroke(cr);
        prev_rts = rts;
        py = y;
    }
}


xAx fromHandles(std::vector<Geom::Point> const &pt) {
    Geom::NL::Vector V(pt.size(), -1.0);
    Geom::NL::Matrix M(pt.size(), 5);
    for(unsigned i = 0; i < pt.size(); i++) {
        Geom::Point P = pt[i];
        Geom::NL::VectorView vv = M.row_view(i);
        vv[0] = P[0]*P[0];
        vv[1] = P[0]*P[1];
        vv[2] = P[1]*P[1];
        vv[3] = P[0];
        vv[4] = P[1];
        std::cout << vv << std::endl;
    }
            
    Geom::NL::LinearSystem ls(M, V);
    
    Geom::NL::Vector x = ls.SV_solve();
    return Geom::xAx(x[0], x[1], x[2], x[3], x[4], 1);
    
}


class Conic6: public Toy {
    PointSetHandle C1H, C2H;
    std::vector<Slider> sliders;

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        Rect screen_rect(Interval(10, width-10), Interval(10, height-10));
        
        Geom::xAx C1 = fromHandles(C1H.pts);
        ::draw(cr, C1, screen_rect);
        *notify << C1;
        
        Geom::xAx C2 = fromHandles(C2H.pts);
        ::draw(cr, C2, screen_rect);
        *notify << C2;
        
        cairo_stroke(cr);
	
        //*notify << "w = " << w << "; lambda = " << rq.lambda() << "\n";
        Toy::draw(cr, notify, width, height, save, timer_stream);
    }

public:
    Conic6() {
        for(int j = 0; j < 5; j++){
            C1H.push_back(uniform()*400, 100+ uniform()*300);
            C2H.push_back(uniform()*400, 100+ uniform()*300);
        }
        handles.push_back(&C1H);
        handles.push_back(&C2H);
        sliders.push_back(Slider(0.0, 5.0, 0, 0.0, "a"));
        sliders.push_back(Slider(0.0, 5.0, 0, 0.0, "b"));
        sliders.push_back(Slider(0.0, 5.0, 0, 0.0, "c"));
        handles.push_back(&(sliders[0]));
        handles.push_back(&(sliders[1]));
        handles.push_back(&(sliders[2]));
        sliders[0].geometry(Point(50, 20), 250);
        sliders[1].geometry(Point(50, 50), 250);
        sliders[2].geometry(Point(50, 80), 250);
    }

    void first_time(int argc, char** argv) {

    }
};

int main(int argc, char **argv) {
    init(argc, argv, new Conic6());
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
