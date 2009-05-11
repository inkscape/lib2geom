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

void draw_hull(cairo_t*cr, RatQuad rq) {
    cairo_move_to(cr, rq.P[0]);
    cairo_line_to(cr, rq.P[1]);
    cairo_line_to(cr, rq.P[2]);
    cairo_stroke(cr);
}
  


void draw(cairo_t* cr, xAx C, Rect bnd) {
    if(bnd[1].extent() < 5) return;
    vector<double> prev_rts;
    double py = bnd[1][0];
    for(int i = 0; i < 100; i++) {
        double t = i/100.;
        double y = (1-t)*bnd[1][0] + t*bnd[1][1];
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
        } else {
            draw(cr, C, Rect(bnd[0], Interval(py, y)));
        }
        prev_rts = rts;
        py = y;
    }
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

double xAx_descr(xAx const & C) {
    double mC[3][3] = {{C.c[0], (C.c[1])/2, (C.c[3])/2},
                       {(C.c[1])/2, C.c[2], (C.c[4])/2},
                       {(C.c[3])/2, (C.c[4])/2, C.c[5]}};
    
    return det3(mC);
}

void draw_ratquad(cairo_t*cr, RatQuad rq, double tol=1e-1) {
    CubicBezier cb = rq.toCubic();
    // I tried using the nearest point to 0.5 for the error, but the improvement was negligible
    if(L2(cb.pointAt(0.5) - rq.pointAt(0.5)) > tol) {
        RatQuad a, b;
        rq.split(a, b);
        draw_ratquad(cr, a, tol);
        draw_ratquad(cr, b, tol);
    } else {
        cairo_curve(cr, cb);
        //draw_cross(cr, cb.pointAt(0));
        //draw_cross(cr, cb.pointAt(1));
    }
}


class Conic5: public Toy {
    PointSetHandle path_handles;
    PointHandle oncurve;
    PointSetHandle cutting_plane;
    std::vector<Slider> sliders;
    RectHandle rh;

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        
        if(0) {
            Path path;
            path = Path(path_handles.pts[0]);
            D2<SBasis> c = handles_to_sbasis(path_handles.pts.begin(), 2);
            path.append(c);
	
            cairo_save(cr);
            cairo_path(cr, path);
            cairo_set_source_rgba (cr, 0., 1., 0, 0.3);
            cairo_set_line_width (cr, 3);
            cairo_stroke(cr);
            cairo_restore(cr);

            //double w = exp(sliders[0].value());
        }
        Point A = path_handles.pts[0];
        Point B = path_handles.pts[1];
        Point C = path_handles.pts[2];
      
        if(1) {
            QuadraticBezier qb(A, B, C);
            //double abt = qb.nearestPoint(oncurve.pos);
            //oncurve.pos = qb.pointAt(abt);
      
            RatQuad rq = RatQuad::fromPointsTangents(A, B-A, oncurve.pos, C, B -C); //( A, B, C, w);
	
            cairo_save(cr);
            cairo_set_source_rgba (cr, 0., 0., 0, 1);
            cairo_set_line_width (cr, 1);
            draw_ratquad(cr, rq);
            //cairo_d2_sb(cr, rq.hermite());
            cairo_stroke(cr);
            cairo_restore(cr);
        }      

        if(0) {
            RatQuad rq = RatQuad::circularArc(A, B, C);
	
            cairo_save(cr);
            cairo_set_source_rgba (cr, 0., 0., 0, 1);
            cairo_set_line_width (cr, 1);
            RatQuad a, b;
            rq.split(a,b);
            cairo_curve(cr, a.toCubic());
            cairo_curve(cr, b.toCubic());
            cairo_stroke(cr);
            cairo_restore(cr);
        }      

        Rect screen_rect(Interval(10, width-10), Interval(10, height-10));
        Line cutLine(cutting_plane.pts[0], cutting_plane.pts[1]);
        double dist;
        Point norm = cutLine.normalAndDist(dist);
      
        const unsigned N = 3;
        xAx sources[N] = {
            xAx::fromPoint(A)*(exp(-sliders[0].value())),
            xAx::fromPoint(B)*(exp(-sliders[1].value())),
            xAx::fromPoint(C)*(exp(-sliders[2].value()))
            //xAx::fromLine(Line(A, oncurve.pos))
        };
        for(unsigned i = 0; i < N; i++) {
            //*notify << sources[i] << "\n";
        }
        for(unsigned i = 0; i < N; i++) {
            for(unsigned j = i+1; j < N; j++) {
                xAx Q = sources[i]-sources[j];
                *notify << Q << " is a " << Q.categorise() << "\n";
            }
        }
        {
            cairo_save(cr);
            cairo_set_source_rgba(cr, 0, 0, 1, 0.5);
            
            ::draw(cr, (sources[0]-sources[1]), screen_rect);
            ::draw(cr, (sources[0]-sources[2]), screen_rect);
            ::draw(cr, (sources[1]-sources[2]), screen_rect);
            cairo_restore(cr);
        }
        {
            string os;
            for(unsigned i = 0; i < N; i++) {
                for(unsigned j = i+1; j < N; j++) {
                    xAx Q = sources[i]-sources[j];
                    Interval iQ = Q.extrema(rh.pos);
                    if(iQ.contains(0)) {
                        os += stringify(iQ) + "\n";

                        Q.toCurve(rh.pos);
                        vector<Point> crs = Q.crossings(rh.pos);
                        for(unsigned ei = 0; ei < crs.size(); ei++) {
                            draw_cross(cr, crs[ei]);
                        }

                    }
                }
            }
            
            draw_text(cr, rh.pos.midpoint(), 
                      os);
        }
        if(1) {
            xAx oxo=sources[0] - sources[2];
            Timer tm;
            
            tm.ask_for_timeslice();
            tm.start();
                
            std::vector<Point> intrs = intersect(oxo, sources[0] - sources[1]);
            Timer::Time als_time = tm.lap();
            *notify << "intersect time = " << als_time << std::endl;
            for(unsigned i = 0; i < intrs.size(); i++) {
                cairo_save(cr);
                cairo_set_source_rgb(cr, 1, 0,0);
                draw_cross(cr, intrs[i]);
                cairo_stroke(cr);
                cairo_restore(cr);
            }
      
            boost::optional<RatQuad> orq = oxo.toCurve(rh.pos);
            if(orq) {
                RatQuad rq = *orq;
                draw_hull(cr, rq);
                vector<SBasis> hrq = rq.homogenous();
                SBasis vertex_poly = (sources[0] - sources[1]).evaluate_at(hrq[0], hrq[1], hrq[2]);
                //*notify << "\n0: " << hrq[0];
                //*notify << "\n1: " << hrq[1];
                //*notify << "\n2: " << hrq[2];
                vector<double> rts = roots(vertex_poly);
                //*notify << "\nvertex poly:" << vertex_poly << '\n';
                for(unsigned i = 0; i < rts.size(); i++) {
                    draw_circ(cr, Point(rq.pointAt(rts[i])));
                    *notify << "\nrq" << i << ":" << rts[i];
                }

                cairo_save(cr);
                cairo_set_source_rgb(cr, 1, 0, 0);
                RatQuad a, b;
                rq.split(a,b);
                cairo_curve(cr, a.toCubic());
                cairo_curve(cr, b.toCubic());
                cairo_stroke(cr);
                cairo_restore(cr);
            }
        }      
        if(0) {
            RatQuad a, b;
            //rq.split(a,b);
            //cairo_move_to(cr, rq.toCubic().pointAt(0.5));
            cairo_line_to(cr, a.P[2]);
            cairo_stroke(cr);
	
            cairo_curve(cr, a.toCubic());
            cairo_curve(cr, b.toCubic());
      
        }
        cairo_stroke(cr);
	
        //*notify << "w = " << w << "; lambda = " << rq.lambda() << "\n";
        Toy::draw(cr, notify, width, height, save, timer_stream);
    }

public:
    Conic5() {
        handles.push_back(&path_handles);
        handles.push_back(&rh);
        rh.pos = Rect(Point(100,100), Point(200,200));
        rh.show_center_handle = true;
        handles.push_back(&oncurve);
        for(int j = 0; j < 3; j++){
            path_handles.push_back(uniform()*400, 100+ uniform()*300);
        }
        oncurve.pos = ((path_handles.pts[0]+path_handles.pts[1]+path_handles.pts[2])/3);
        handles.push_back(&cutting_plane);
        for(int j = 0; j < 2; j++){
            cutting_plane.push_back(uniform()*400, 100+ uniform()*300);
        }
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
    init(argc, argv, new Conic5());
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
