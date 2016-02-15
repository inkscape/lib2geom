#include <iostream>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/path-intersection.h>
#include <2geom/basic-intersection.h>
#include <2geom/pathvector.h>
#include <2geom/exception.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/path-intersection.h>
#include <2geom/nearest-time.h>
#include <2geom/line.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>

#include <cstdlib>
#include <map>
#include <vector>
#include <algorithm>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/ord.h>

#include <2geom/conicsec.h>

std::vector<Geom::RatQuad> xAx_to_RatQuads(Geom::xAx const &/*C*/, Geom::Rect const &/*bnd*/) {
    // find points on boundary
    // if there are exactly 0 points return
    // if there are exactly 2 points fit ratquad and return
    // if there are an odd number, split bnd on the point with the smallest dot(unit_vector(grad), rect_edge)
    // sort into clockwise order ABCD
    // compute corresponding tangents
    // test boundary points against the line through A
    // if all on one side
    //
    // if A,X and Y,Z
    // ratquad from A,X and Y,Z
    return std::vector<Geom::RatQuad>();
}



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
    if(bnd[1].extent() < 5) return;
    vector<double> prev_rts;
    double py = bnd[Y].min();
    for(int i = 0; i < 100; i++) {
        double t = i/100.;
        double y = bnd[Y].valueAt(t);
        vector<double> rts = C.roots(Point(1, 0), Point(0, y));
        int top = 0;
        for(unsigned j = 0; j < rts.size(); j++) {
            if(bnd[0].contains(rts[j])) {
                rts[top] = rts[j];
                top++;
            }
        }
        rts.erase(rts.begin()+top, rts.end());
        
        if(rts.size() == prev_rts.size()) {
            for(unsigned j = 0; j < rts.size(); j++) {
                cairo_move_to(cr, prev_rts[j], py);
                cairo_line_to(cr, rts[j], y);
                cairo_stroke(cr);
            }
/*        } else if(prev_rts.size() == 1) {
            for(unsigned j = 0; j < rts.size(); j++) {
                cairo_move_to(cr, prev_rts[0], py);
                cairo_line_to(cr, rts[j], y);
                cairo_stroke(cr);
            }
        } else if(rts.size() == 1) {
            for(unsigned j = 0; j < prev_rts.size(); j++) {
                cairo_move_to(cr, prev_rts[j], py);
                cairo_line_to(cr, rts[0], y);
                cairo_stroke(cr);
                }*/
        } else {
            draw(cr, C, Rect(bnd[0], Interval(py, y)));
            /*for(unsigned j = 0; j < rts.size(); j++) {
                cairo_move_to(cr, rts[j], y);
                cairo_rel_line_to(cr, 1,1);
                }*/
        }
        prev_rts = rts;
        py = y;
    }
}

template <typename T>
static T det(T a, T b, T c, T d) {
    return a*d - b*c;
}

template <typename T>
static T det(T M[2][2]) {
    return M[0][0]*M[1][1] - M[1][0]*M[0][1];
}

template <typename T>
static T det3(T M[3][3]) {
    return ( M[0][0] * det(M[1][1], M[1][2],
                           M[2][1], M[2][2])
             -M[1][0] * det(M[0][1], M[0][2],
                            M[2][1], M[2][2])
             +M[2][0] * det(M[0][1], M[0][2],
                            M[1][1], M[1][2]));
}

class Conic6: public Toy {
    PointSetHandle C1H, C2H;
    std::vector<Slider> sliders;
    Point    mouse_sampler;

    virtual void mouse_moved(GdkEventMotion* e) {
        mouse_sampler = Point(e->x, e->y);
        Toy::mouse_moved(e);
    }
    
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        Rect screen_rect(Interval(10, width-10), Interval(10, height-10));
        
        Geom::xAx C1 = xAx::fromPoints(C1H.pts);
        ::draw(cr, C1, screen_rect);
        *notify << C1;
        
        Geom::xAx C2 = xAx::fromPoints(C2H.pts);
        ::draw(cr, C2, screen_rect);
        *notify << C2;


        SBasis T(Linear(-1,1));
        SBasis S(Linear(1,1));
        SBasis C[3][3] = {{T*C1.c[0]+S*C2.c[0], (T*C1.c[1]+S*C2.c[1])/2, (T*C1.c[3]+S*C2.c[3])/2},
                          {(T*C1.c[1]+S*C2.c[1])/2, T*C1.c[2]+S*C2.c[2], (T*C1.c[4]+S*C2.c[4])/2},
                          {(T*C1.c[3]+S*C2.c[3])/2, (T*C1.c[4]+S*C2.c[4])/2, T*C1.c[5]+S*C2.c[5]}};
    
        SBasis D = det3(C);
        std::vector<double> rts = Geom::roots(D);
        if(rts.empty()) {
            T = Linear(1,1);
            S = Linear(-1,1);
            SBasis C[3][3] = {{T*C1.c[0]+S*C2.c[0], (T*C1.c[1]+S*C2.c[1])/2, (T*C1.c[3]+S*C2.c[3])/2},
                              {(T*C1.c[1]+S*C2.c[1])/2, T*C1.c[2]+S*C2.c[2], (T*C1.c[4]+S*C2.c[4])/2},
                              {(T*C1.c[3]+S*C2.c[3])/2, (T*C1.c[4]+S*C2.c[4])/2, T*C1.c[5]+S*C2.c[5]}};
        
            D = det3(C);
            rts = Geom::roots(D);
        }
        // at this point we have a T and S and perhaps some roots that represent our degenerate conic
        // Let's just pick one randomly (can we do better?)
        //for(unsigned i = 0; i < rts.size(); i++) {
        if(!rts.empty()) {
            cairo_save(cr);

            unsigned i = 0;
            double t = T.valueAt(rts[i]);
            double s = S.valueAt(rts[i]);
            *notify << t << "; " << s << std::endl;
            /*double C0[3][3] = {{t*C1.c[0]+s*C2.c[0], (t*C1.c[1]+s*C2.c[1])/2, (t*C1.c[3]+s*C2.c[3])/2},
                               {(t*C1.c[1]+s*C2.c[1])/2, t*C1.c[2]+s*C2.c[2], (t*C1.c[4]+s*C2.c[4])/2},
                               {(t*C1.c[3]+s*C2.c[3])/2, (t*C1.c[4]+s*C2.c[4])/2, t*C1.c[5]+s*C2.c[5]}};*/
            xAx xC0 = C1*t + C2*s;
            //::draw(cr, xC0, screen_rect); // degen
            
            boost::optional<Point> oB0 = xC0.bottom();
            
            Point B0 = *oB0;
            //*notify << B0 << " = " << C1.gradient(B0);
            draw_circ(cr, B0);
            
            Point n0, n1;
            // Are these just the eigenvectors of A11?
            if(fabs(xC0.c[0]) > fabs(xC0.c[2])) {
                double b = 0.5*xC0.c[1]/xC0.c[0];
                double c = xC0.c[2]/xC0.c[0];
                double d =  std::sqrt(b*b-c);
                n0 = Point(1, b+d);
                n1 = Point(1, b-d);
            } else {
                
                double b = 0.5*xC0.c[1]/xC0.c[2];
                double c = xC0.c[0]/xC0.c[2];
                double d =  std::sqrt(b*b-c);
                n0 = Point(b+d, 1);
                n1 = Point(b-d, 1);
            }
            cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
            
            Line L0 = Line::from_origin_and_vector(B0, rot90(n0));
            draw_line(cr, L0, screen_rect);
            Line L1 = Line::from_origin_and_vector(B0, rot90(n1));
            draw_line(cr, L1, screen_rect);
            
            cairo_set_source_rgb(cr, 1, 0., 0.);
            rts = C1.roots(L0);
            for(unsigned i = 0; i < rts.size(); i++) {
                Point P = L0.pointAt(rts[i]);
                draw_cross(cr, P);
                *notify << C1.valueAt(P) << "; " << C2.valueAt(P) << "\n";
            }
            rts = C1.roots(L1);
            for(unsigned i = 0; i < rts.size(); i++) {
                Point P = L1.pointAt(rts[i]);
                draw_cross(cr, P);
                *notify << C1.valueAt(P) << "; "<< C2.valueAt(P) << "\n";
            }
            cairo_stroke(cr);
            cairo_restore(cr);
        }

        ::draw(cr, C1*sliders[0].value() + C2*sliders[1].value(), screen_rect);
        
        std::vector<Point> res = intersect(C1, C2);
        for(unsigned i = 0; i < res.size(); i++) {
            draw_circ(cr, res[i]);
        }
        
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
        sliders.push_back(Slider(-1.0, 1.0, 0, 0.0, "a"));
        sliders.push_back(Slider(-1.0, 1.0, 0, 0.0, "b"));
        sliders.push_back(Slider(0.0, 5.0, 0, 0.0, "c"));
        handles.push_back(&(sliders[0]));
        handles.push_back(&(sliders[1]));
        handles.push_back(&(sliders[2]));
        sliders[0].geometry(Point(50, 20), 250);
        sliders[1].geometry(Point(50, 50), 250);
        sliders[2].geometry(Point(50, 80), 250);
    }

    void first_time(int /*argc*/, char**/* argv*/) {

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
