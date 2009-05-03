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
    }
            
    Geom::NL::LinearSystem ls(M, V);
    
    Geom::NL::Vector x = ls.SV_solve();
    return Geom::xAx(x[0], x[1], x[2], x[3], x[4], 1);
    
}


static double det(Point a, Point b) {
    return a[0]*b[1] - a[1]*b[0];
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
        //for(unsigned i = 0; i < rts.size(); i++) {
        if(!rts.empty()) {
            unsigned i = 0;
            double t = T.valueAt(rts[i]);
            double s = S.valueAt(rts[i]);
            *notify << t << "; " << s << std::endl;
            double C0[3][3] = {{t*C1.c[0]+s*C2.c[0], (t*C1.c[1]+s*C2.c[1])/2, (t*C1.c[3]+s*C2.c[3])/2},
                               {(t*C1.c[1]+s*C2.c[1])/2, t*C1.c[2]+s*C2.c[2], (t*C1.c[4]+s*C2.c[4])/2},
                               {(t*C1.c[3]+s*C2.c[3])/2, (t*C1.c[4]+s*C2.c[4])/2, t*C1.c[5]+s*C2.c[5]}};
            xAx xC0 = C1*t + C2*s;
            ::draw(cr, xC0, screen_rect); // degen
            
            double m = 0;
            int mi = 0;
            std::cout << "det(C0) = "<< det3(C0) << "\n";
            for(int c = 0; c < 3; c++) {
                double mx = std::max(fabs(C0[0][c]), 
                                     fabs(C0[1][c]));
                mx = std::max(mx, fabs(C0[2][c]));
                if(mx > m) {
                    mi = c;
                    m = mx;
                }
                std::cout << C0[0][c] << " "
                        << C0[1][c] << " "
                        << C0[2][c] << "\n";
                    
            }
            *notify << mi << m << "\n";
            bool prop = true;
            for(int c = 0; c < 3; c++) {
                std::cout << C0[0][c]/C0[0][mi] << " "
                        << C0[1][c]/C0[1][mi] << " "
                        << C0[2][c]/C0[2][mi] << "\n";
                if(c == mi) continue;
                
                if(fabs(C0[c][0]*C0[mi][1] - C0[c][1]*C0[mi][0]) > 1e-1)
                    prop = false;
                if(fabs(C0[c][0]*C0[mi][2] - C0[c][2]*C0[mi][0]) > 1e-1)
                    prop = false;
                    
            }
            double p[3];
            double Mp[3][3] = {{0, p[3], -p[2]},
                               {-p[3], 0, p[1]},
                               {p[2], -p[1], 0}};
            
            *notify << prop << "\n";
        }

        ::draw(cr, C1*sliders[0].value() + C2*sliders[1].value(), screen_rect);
        

        
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
