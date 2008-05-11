#include "basic-intersection.h"
#include "d2.h"
#include "sbasis.h"
#include "bezier-to-sbasis.h"

#include "path-cairo.h"
#include "toy-framework.h"

using std::vector;
using namespace Geom;

cairo_t *g_cr = 0;
const double eps = 0.1;

double nearest(Point& P, D2<SBasis> Dom) {
    D2<SBasis> dst = Dom - P;
    SBasis dst2 = derivative(dot(dst, dst));
    std::vector<double> rts = roots(dst2);
    Point closest = Dom.at0();
    double t = 0;
    for(unsigned i = 0; i < rts.size(); i++) {
        if(L2(P - closest) > L2(P - Dom(rts[i]))) {
            closest = Dom(rts[i]);
            t = rts[i];
        }
    }
    if( L2(P - closest) > L2(P - Dom.at1()) ) {
        closest = Dom.at1();
        t = 1;
    }
    P = closest; // stupid lack of tuples :)
    return t;
}

class Circle{
public:
    Point center;
    double radius;
    Circle(D2<SBasis> c, double t) {
        std::vector<Point > ders = c.valueAndDerivatives(t, 3);
        std::cout << ders[0] << ", "
                  << ders[1] << ", "
                  << ders[2] << "\n";
        double num = cross(ders[1], ders[2]);
        double den = L2(ders[1]);
        //curvature = num /den^1.5;
        printf("%g %g\n", num, den);
        radius = -den*den*den / num;
        center = ders[0] + rot90(unit_vector(ders[1])) * radius;
        std::cout << radius << center <<std::endl;
    }
    void draw(cairo_t* cr) {
        if (radius < 1e5) {
            cairo_new_sub_path(cr);
            cairo_arc(cr, center[0], center[1], fabs(radius), 0, M_PI*2);
            cairo_stroke(cr);
        }
    }
};

#include "numeric/linear_system.h"

class PairShuttle: public Toy {
unsigned A_bez_ord;
unsigned B_bez_ord;
virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    cairo_set_line_width (cr, 0.5);
    
    D2<SBasis> A = handles_to_sbasis(handles.begin(), A_bez_ord-1);
    cairo_md_sb(cr, A);
    
    D2<SBasis> B = handles_to_sbasis(handles.begin()+A_bez_ord, B_bez_ord-1);
    cairo_md_sb(cr, B);
    vector<double> Asects, Bsects;
    g_cr = cr;
    // cubic system
    int sample_size = 100;
    int order = 6;
    NL::Matrix m(sample_size, order, 0);
    NL::Vector v(sample_size);
    int i = 0;
    double od = 0, ot = 0;
    for(double t = 0; t < 1; t+= 0.003) {
        Point P = A(t);
        double pt = nearest(P, B);
        double d = distance(A(t), P);
        if((i > 14) && (i < (14+sample_size))) {
            cairo_set_source_rgb(cr, 1,0,0);
            double tp = 1;
            for(int j = 0; j < order; j++) {
                m(i-14, j) = tp;
                tp *= t;
            }
            v[i-14] = d;
        } else{
            cairo_set_source_rgb(cr, 0,0,0);
        }
        if (t > 0) {
            cairo_move_to(cr, ot*width, od*100);
            cairo_line_to(cr, t*width, d*100);
            cairo_stroke(cr);
        }
        od = d;
        ot = t;
        i++;
    }
    cairo_stroke(cr);
    if(1) {
    NL::LinearSystem ls(m, v);
    NL::Vector coeff = ls.SV_solve();
    for(double t = 0; t < 1; t+= 0.003) {
        double d = 0;
        double tp = 1;
        for(int j = 0; j < order; j++) {
            d += coeff[j]*tp;
            tp *= t;
        }
        if (t == 0)
            cairo_move_to(cr, t*width, d*100);
        else
            cairo_line_to(cr, t*width, d*100);
    }
    cairo_stroke(cr);
    }
    Toy::draw(cr, notify, width, height, save);
}
public:
    PairShuttle (unsigned A_bez_ord, unsigned B_bez_ord) :
        A_bez_ord(A_bez_ord), B_bez_ord(B_bez_ord) {
    for(unsigned i = 0; i < A_bez_ord; i++) handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    for(unsigned i = 0; i < A_bez_ord; i++)
        handles.push_back(Geom::Point(handles[i][0]+uniform()*4, handles[i][1]+uniform()*4));
    
    for(unsigned i = 0; i < 2; i++) handles.push_back(Geom::Point(uniform()*400, uniform()*400));
}
};

int main(int argc, char **argv) {
unsigned A_bez_ord=4;
unsigned B_bez_ord=4;
    if(argc > 2)
        sscanf(argv[2], "%d", &B_bez_ord);
    if(argc > 1)
        sscanf(argv[1], "%d", &A_bez_ord);
    init(argc, argv, new PairShuttle(A_bez_ord, B_bez_ord));

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
