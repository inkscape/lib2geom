#include "toy-framework.h"
#include "point-ops.h"

static double alpha = 1;

class BSpline : public Toy {
    vector<float> knots;
    int find_span(int p, double u) {
        const int n = handles.size();
        if(u >= knots.back())
            return n;
        int lo = p;
        int hi = n+1;
        int mid = (lo + hi)/2;
        while((lo < hi) && ((u < knots[mid]) || (u >= knots[mid+1]))) {
            if(u < knots[mid])
                hi = mid;
            else
                lo = mid;
            mid = (lo + hi)/2;
        }
        return mid;
    }

    void basis_fns(int i, double u, int p, double *N) {
        const int n = handles.size();
        N[0] = 1;
        static vector<double> left, right;
        left.resize(p+1);
        right.resize(p+1);
        for(int j = 1; j <= p; j++) {
            left[j] = u-knots[i+1-j];
            right[j] = knots[i+j]-u;
            double saved = 0.0;
            for(int r = 0; r < j; r++) {
                double temp = N[r]/(right[r+1]+left[j-r]);
                N[r] = saved + right[r+1]*temp;
                saved = left[j-r]*temp;
            }
            N[j] = saved;
        }
    }

    Geom::Point curve_point(double u) {
        const int n = handles.size();
        const int p = 3;
        int span = find_span(p, u);
        vector<double> N;
        N.resize(p+1);
        basis_fns(span, u, p, &N[0]);
        Geom::Point C;
        for(int i = 0; i <= p; i++)
            C += N[i]*handles[span-p+i];
        return C;
    }

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_line_width (cr, 1);
        for(int i = 0; i < handles.size()-1; i++)
            draw_line_seg(cr, handles[i], handles[i+1]);
    
        Geom::Point C = curve_point(0);
        cairo_move_to(cr, C);
        for(int i = 0; i < 100; i++) {
            double u = double(i+1)/100;
            Geom::Point C = curve_point(u);
            cairo_line_to(cr, C);
        }
        cairo_stroke(cr);
        
        *notify << "Alpha: " << alpha;
        Toy::draw(cr, notify, width, height, save);
    }

    public:
    BSpline() {
        for(int i = 0; i < 8; i++) {
            handles.push_back(Geom::Point((rand() & 0xff) + 1, (rand() & 0xff) + 1));
            knots.push_back(uniform());
        }
        knots[0] = 0;
        knots.push_back(1.0);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "B-Spline", new BSpline());
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
