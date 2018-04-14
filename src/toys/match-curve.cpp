#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/path.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#define ZROOTS_TEST 0
#if ZROOTS_TEST
#include <2geom/zroots.c>
#endif

#include <vector>
using std::vector;
using namespace Geom;

//#define HAVE_GSL

template <typename T>
void shift(T &a, T &b, T const &c) {
    a = b;
    b = c;
}
template <typename T>
void shift(T &a, T &b, T &c, T const &d) {
    a = b;
    b = c;
    c = d;
}

extern unsigned total_steps, total_subs;

class MatchCurve: public Toy {
public:
    double timer_precision;
    double units;
    PointSetHandle psh;
    
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgb(cr, 0,0,0);
        std::vector<Geom::Point> trans;
        trans.resize(psh.size());
        for(unsigned i = 0; i < psh.size(); i++) {
            trans[i] = psh.pts[i] - Geom::Point(0, 3*width/4);
        }
        
        std::vector<double> solutions;
        
        D2<SBasis> test_sb = psh.asBezier();

    
        D2<SBasis> B = psh.asBezier();
        Geom::Path pb;
        pb.append(B);
        pb.close(false);
        cairo_path(cr, pb);
        cairo_stroke(cr);
        
        D2<SBasis> m;
        D2<SBasis> dB = derivative(B);
        D2<SBasis> ddB = derivative(dB);
        D2<SBasis> dddB = derivative(ddB);
        
        Geom::Point pt = B(0);
        Geom::Point tang = dB(0);
        Geom::Point dtang = ddB(0);
        Geom::Point ddtang = dddB(0);
        for(int dim = 0; dim < 2; dim++) {
            m[dim] = Linear(pt[dim],pt[dim]+tang[dim]);
            m[dim] += Linear(0, 1)*Linear(0, 1*dtang[dim])/2;
            m[dim] += Linear(0, 1)*Linear(0, 1)*Linear(0, ddtang[dim])/6;
        }
        
        double lo = 0, hi = 1;
        double eps = 5;
        while(hi - lo > 0.0001) {
            double mid = (hi + lo)/2;
            //double Bmid = (Bhi + Blo)/2;
            
            m = truncate(compose(B, Linear(0, mid)), 2);
            // perform golden section search
            double best_f = 0, best_x = 1;
            for(int n = 2; n < 4; n++) {
            Geom::Point m_pt = m(double(n)/6);
            double x0 = 0, x3 = 1.; // just a guess!
            const double R = 0.61803399;
            const double C = 1 - R;
            double x1 = C*x0 + R*x3;
            double x2 = C*x1 + R*x3;
            double f1 = Geom::distance(m_pt, B(x1));
            double f2 = Geom::distance(m_pt, B(x2));
            while(fabs(x3 - x0) > 1e-3*(fabs(x1) + fabs(x2))) {
                if(f2 < f1) {
                    shift(x0, x1, x2, R*x1 + C*x3);
                    shift(f1, f2, Geom::distance(m_pt, B(x2)));
                } else {
                    shift(x3, x2, x1, R*x2 + C*x0);
                    shift(f2, f1, Geom::distance(m_pt, B(x2)));
                }
                std::cout << x0 << "," 
                          << x1 << ","
                          << x2 << ","
                          << x3 << ","
                          << std::endl;
            }
            if(f2 < f1) {
                f1 = f2;
                x1 = x2;
            }
            if(f1 > best_f) {
                best_f = f1;
                best_x = x1;
            }
            }
            std::cout << mid << ":" << best_x << "->" << best_f << std::endl;
            //draw_cross(cr, point_at(B, x1));
            
            if(best_f > eps) {
                hi = mid;
            } else {
                lo = mid;
            }
        }
        std::cout << std::endl;
        //draw_cross(cr, point_at(B, hi));
        draw_circ(cr, m(hi));
        {
            Geom::Path pb;
            pb.append(m);
            pb.close(false);
            cairo_path(cr, pb);
        }
        
        cairo_stroke(cr);
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
    MatchCurve() : timer_precision(0.1), units(1e6) // microseconds
    {
        handles.push_back(&psh);
        for(int i = 0; i < 6; i++)
            psh.push_back(uniform()*400, uniform()*400);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new MatchCurve());

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
