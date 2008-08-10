#include <2geom/convex-cover.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/d2.h>
#include <2geom/geom.h>
#include <2geom/numeric/linear_system.h>

#include <aa.h>
#include <complex>
#include <algorithm>

using std::vector;
using namespace Geom;
using namespace std;

typedef std::complex<AAF> CAAF;

struct PtLexCmp{
    bool operator()(const Point &a, const Point &b) {
        return (a[0] < b[0]) || ((a[0] == b[0]) and (a[1] < b[1]));
    }
};

// draw ax + by + c = 0
void draw_line_in_rect(cairo_t*cr, Rect &r, Point n, double c) {
    vector<Geom::Point> result;
    Point resultp;
    if(intersects == line_intersection(Point(1, 0), r.left(),
				       n, c,
				       resultp) && r[1].contains(resultp[1]))
	result.push_back(resultp);
    if(intersects == line_intersection(Point(1, 0), r.right(),
				       n, c,
				       resultp) && r[1].contains(resultp[1]))
	result.push_back(resultp);
    if(intersects == line_intersection(Point(0, 1), r.top(),
				       n, c,
				       resultp) && r[0].contains(resultp[0]))
	result.push_back(resultp);
    if(intersects == line_intersection(Point(0, 1), r.bottom(),
				       n, c,
				       resultp) && r[0].contains(resultp[0]))
	result.push_back(resultp);
    if(result.size() > 2) {
        std::sort(result.begin(), result.end(), PtLexCmp());
        vector<Geom::Point>::iterator new_end = std::unique(result.begin(), result.end());
        result.resize(new_end-result.begin());
    }
    if(result.size() == 2) {
	cairo_move_to(cr, result[0]);
	cairo_line_to(cr, result[1]);
	cairo_stroke(cr);
    } else {
        ;//cout << result.size() << endl;
    }


}

void fill_line_in_rect(cairo_t*cr, Rect &r, Point n, double c) {
    vector<Geom::Point> result;
    Point resultp;
    if(intersects == line_intersection(Point(1, 0), r.left(),
				       n, c,
				       resultp) && r[1].contains(resultp[1]))
	result.push_back(resultp);
    if(intersects == line_intersection(Point(1, 0), r.right(),
				       n, c,
				       resultp) && r[1].contains(resultp[1]))
	result.push_back(resultp);
    if(intersects == line_intersection(Point(0, 1), r.top(),
				       n, c,
				       resultp) && r[0].contains(resultp[0]))
	result.push_back(resultp);
    if(intersects == line_intersection(Point(0, 1), r.bottom(),
				       n, c,
				       resultp) && r[0].contains(resultp[0]))
	result.push_back(resultp);
    if(result.size() > 2) {
        std::sort(result.begin(), result.end(), PtLexCmp());
        vector<Geom::Point>::iterator new_end = std::unique(result.begin(), result.end());
        result.resize(new_end-result.begin());
    }
        ConvexHull ch(result);
        for(int i = 0; i < 4; i++) {
            Point p = r.corner(i);
            if(dot(n,p) < c) {
                ch.boundary.push_back(p);
            }
        }
        ch.graham();
	cairo_convex_hull(cr, ch.boundary);
}

boost::optional<Rect> tighten(Rect &r, Point n, Interval lu) {
    vector<Geom::Point> result;
    Point resultp;
    for(int i = 0; i < 4; i++) {
        Point cnr = r.corner(i);
        double z = dot(cnr, n);
        if((z > lu[0]) and (z < lu[1]))
            result.push_back(cnr);
    }
    for(int i = 0; i < 2; i++) {
        double c = lu[i];
        if(intersects == line_intersection(Point(1, 0), r.left(),
                                           n, c,
                                           resultp) && r[1].contains(resultp[1]))
            result.push_back(resultp);
        if(intersects == line_intersection(Point(1, 0), r.right(),
                                           n, c,
                                           resultp) && r[1].contains(resultp[1]))
            result.push_back(resultp);
        if(intersects == line_intersection(Point(0, 1), r.top(),
                                           n, c,
                                           resultp) && r[0].contains(resultp[0]))
            result.push_back(resultp);
        if(intersects == line_intersection(Point(0, 1), r.bottom(),
                                           n, c,
                                           resultp) && r[0].contains(resultp[0]))
            result.push_back(resultp);
    }
    if(result.size() < 2)
        return boost::optional<Rect>();
    Rect nr(result[0], result[1]);
    for(size_t i = 2; i < result.size(); i++) {
        nr.expandTo(result[i]);
    }
    return intersect(nr, r);
}

AAF ls_sample_based(AAF x, vector<Point> pts) {
    NL::Matrix m(pts.size(), 2);
    NL::Vector v(pts.size());
    NL::LinearSystem ls(m, v);

    m.set_all(0);
    v.set_all(0);
    for (unsigned int k = 0; k < pts.size(); ++k)
    {
        m(k,0) += pts[k][0];
        m(k,1) += 1;
        //std::cout << pts[k] << " ";

        v[k] += pts[k][1];
        //v[1] += pts[k][1];
        //v[2] += y2;
    }

    ls.SV_solve();

    double A = ls.solution()[0];
    double B = ls.solution()[1];
    // Ax + B = y
    Interval bnd(0,0);
    for (unsigned int k = 0; k < pts.size(); ++k)
    {
        bnd.extendTo(A*pts[k][0]+B - pts[k][1]);
    }
    //std::cout << A << "," << B << std::endl;
    return AAF(x, A, B, bnd.extent(),
               x.special);
}

AAF md_sample_based(AAF x, vector<Point> pts) {
    Geom::ConvexHull ch1(pts);
    Point a, b, c;
    ch1.narrowest_diameter(a, b, c);
    Point db = c-b;
    double A = db[1]/db[0];
    Point aa = db*(dot(db, a-b)/dot(db,db))+b;
    Point mid = (a+aa)/2;
    double B = mid[1] - A*mid[0];
    double dB = (a[1] - A*a[0]) - B;
    // Ax + B = y
    //std::cout << A << "," << B << std::endl;
    return AAF(x, A, B, dB,
               x.special);
}

AAF atan_sample_based(AAF x) {
    interval ab(x);
    const double a = ab.min(); // [a,b] is our interval
    const double b = ab.max();

    const double ea = atan(a);
    const double eb = atan(b);
    vector<Point> pts;
    pts.push_back(Point(a,ea));
    pts.push_back(Point(b,eb));
    const double alpha = (eb-ea)/(b-a);
    double xs = sqrt(1/alpha-1);
    if((a < xs) and (xs < b))
        pts.push_back(Point(xs,atan(xs)));
    xs = -xs;
    if((a < xs) and (xs < b))
        pts.push_back(Point(xs,atan(xs)));
    
    return md_sample_based(x, pts);
}

AAF log_sample_based(AAF x) {
    interval ab(x);
    const double a = ab.min(); // [a,b] is our interval
    const double b = ab.max();
    AAF_TYPE type;
    if(a > 0)
        type = AAF_TYPE_AFFINE;
    else if(b < 0) { // no point in continuing
        type = AAF_TYPE_NAN;
        return AAF(type);
    }
    else if(a <= 0) { // undefined, can we do better?
        type = (AAF_TYPE)(AAF_TYPE_AFFINE | AAF_TYPE_NAN);
        return AAF(type);
        // perhaps we should make a = 0+eps and try to continue?
    }

    const double ea = log(a);
    const double eb = log(b);
    vector<Point> pts;
    pts.push_back(Point(a,ea));
    pts.push_back(Point(b,eb));
    const double alpha = (eb-ea)/(b-a);
    // dlog(xs) = alpha
    double xs = 1/(alpha);
    if((a < xs) and (xs < b))
        pts.push_back(Point(xs,log(xs)));
    
    return md_sample_based(x, pts);
}

AAF exp_sample_based(AAF x) {
    interval ab(x);
    const double a = ab.min(); // [a,b] is our interval
    const double b = ab.max();

    const double ea = exp(a);
    const double eb = exp(b);
    vector<Point> pts;
    pts.push_back(Point(a,ea));
    pts.push_back(Point(b,eb));
    const double alpha = (eb-ea)/(b-a);
    // dexp(xs) = alpha
    double xs = log(alpha);
    if((a < xs) and (xs < b))
        pts.push_back(Point(xs,exp(xs)));
    
    return md_sample_based(x, pts);
}

double
desy_lambert_W(double x) {
    if (x <= 500.0) {
        double lx1 = log(x + 1.0);
        return 0.665 * (1 + 0.0195 * lx1) * lx1 + 0.04;
    }
    return log(x - 4.0) - (1.0 - 1.0/log(x)) * log(log(x));
}

double lambertW(double x, double prec = 1E-12, int maxiters = 100) {
    double w = desy_lambert_W(x);
    const double e = exp(1);
    for(int i = 0; i < maxiters; i++) {
        double we = w * pow(e,w);
        double w1e = (w + 1) * pow(e,w);
        if( prec > abs((x - we) / w1e))
            return w;
        w -= (we - x) / (w1e - (w+2) * (we-x) / (2*w+2));
    }
    //raise ValueError("W doesn't converge fast enough for abs(z) = %f" % abs(x))
}

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_min.h>

typedef struct{
    double a, b;
} param_W;

double fn1 (double x, void * params)
{
    param_W *pw = (param_W*)params;
    return (pw->a*x+pw->b) - lambertW(x);
}
     
double optimise(void * params, double a, double b) {
    int status;
    param_W *pw = (param_W*)params;
    int iter = 0, max_iter = 100;
    double m = (a+b)/2;
    gsl_function F;
     
    F.function = &fn1;
    F.params = params;
     
    const gsl_min_fminimizer_type *T = gsl_min_fminimizer_brent;
    gsl_min_fminimizer *s = gsl_min_fminimizer_alloc (T);
    if(a+1e-10 >= b) return m;
    cout << a << " " << b << " " << m << endl;
    cout << "fn:" <<  fn1(a, params) << " " << fn1(b, params) << " " << fn1(m, params) << endl;
    cout << "fn:" <<  (pw->a*a+pw->b) << " " << (pw->a*b+pw->b) << " " << (pw->a*m+pw->b) << endl;
    
    gsl_min_fminimizer_set (s, &F, m, a, b);
    do
    {
        iter++;
        status = gsl_min_fminimizer_iterate (s);
     
        m = gsl_min_fminimizer_x_minimum (s);
        a = gsl_min_fminimizer_x_lower (s);
        b = gsl_min_fminimizer_x_upper (s);
     
        status 
            = gsl_min_test_interval (a, b, 0.001, 0.0);
     
    }
    while (status == GSL_CONTINUE && iter < max_iter);
     
    gsl_min_fminimizer_free (s);
     
    return m;
}


AAF W_sample_based(AAF x) {
    interval ab(x);
    const double a = ab.min(); // [a,b] is our interval
    const double b = ab.max();
    const double e = exp(1);
    AAF_TYPE type;
    if(a >= -1./e)
        type = AAF_TYPE_AFFINE;
    else if(b < 0) { // no point in continuing
        type = AAF_TYPE_NAN;
        return AAF(type);
    }
    else if(a <= 0) { // undefined, can we do better?
        type = (AAF_TYPE)(AAF_TYPE_AFFINE | AAF_TYPE_NAN);
        return AAF(type);
        // perhaps we should make a = 0+eps and try to continue?
    }
    const double ea = lambertW(a);
    const double eb = lambertW(b);
    vector<Point> pts;
    pts.push_back(Point(a,ea));
    pts.push_back(Point(b,eb));
    const double alpha = (eb-ea)/(b-a);
    // d(W(xs)) = alpha
    // W = 
    param_W pw;
    pw.a = alpha;
    pw.b = ea - alpha*a;
    if(a < b) {
        double xs = optimise(&pw, a, b);
        if((a < xs) and (xs < b))
            pts.push_back(Point(xs,lambertW(xs)));
    }
    return md_sample_based(x, pts);
}

AAF pow_sample_based(AAF x, double p) {
    interval ab(x);
    const double a = ab.min(); // [a,b] is our interval
    const double b = ab.max();
    AAF_TYPE type;
    if(floor(p) != p) {
        if(a >= 0)
            type = AAF_TYPE_AFFINE;
        else if(b < 0) { // no point in continuing
            type = AAF_TYPE_NAN;
            return AAF(type);
        }
        else if(a <= 0) { // undefined, can we do better?
            type = (AAF_TYPE)(AAF_TYPE_AFFINE | AAF_TYPE_NAN);
            return AAF(type);
            // perhaps we should make a = 0+eps and try to continue?
        }
    }
    const double ea = pow(a, p);
    const double eb = pow(b, p);
    vector<Point> pts;
    pts.push_back(Point(a,ea));
    pts.push_back(Point(b,eb));
    const double alpha = (eb-ea)/(b-a);
    // d(xs^p) = alpha
    // p xs^(p-1) = alpha
    // xs = (alpha/p)^(1-p)
    double xs = pow(alpha/p, 1./(p-1));
    if((a < xs) and (xs < b))
        pts.push_back(Point(xs,pow(xs, p)));
    xs = -xs;
    if((a < xs) and (xs < b))
        pts.push_back(Point(xs,pow(xs, p)));
    
    return md_sample_based(x, pts);
}

Point origin;
AAF trial_eval(AAF x, AAF y) {
    x = x-origin[0];
    y = y-origin[1];
    x = x/200;
    y = y/200;
    AAF x2 = pow_sample_based(x,2);
    AAF y2 = pow_sample_based(y,2);
    //return x2 + y2 - 1;
    //return y - pow(x,3);
    return y + W_sample_based(x);
    //return y - pow_sample_based(x,2.5);
    //return y - log_sample_based(x);
    //return y - log(x);
    //return -y + -exp_sample_based(x*log(x));
    return -x + -exp_sample_based(y*log(y));
    //return y - sqrt(sin(x));
    //return sqrt(y)*x - sqrt(x) - y - 1;
    //return y-1/x;
    //return exp(x)-y;
    //return sin(x)-y;
    //return exp_sample_based(x)-y;
    //return atan(x)-y;
    //return atan_sample_based(x)-y;
    //return atanh(x)-y;
    //return x*y;
    //return 4*x+3*y-1;
    //return x*x + y*y - 1;
    return pow_sample_based((x2 + y2), 2) - (x2-y2);
    //return x*x-y;
    //return (x*x*x-y*x)*sin(x) + (x-y*y)*cos(y)-0.5;
}

AAF xaxis(AAF x, AAF y) {
    (void)x;
    y = y-origin[1];
    y = y/200;
    return y;
}

AAF yaxis(AAF x, AAF y) {
    (void)y;
    x = x-origin[0];
    x = x/200;
    return x;
}

class ConvexTest: public Toy {
public:
    PointSetHandle test_window;
    PointSetHandle samples;
    PointHandle orig_handle;
    ConvexTest () {
        toggles.push_back(Toggle("Show trials", false));
        handles.push_back(&test_window);
        handles.push_back(&samples);
        handles.push_back(&orig_handle);
        orig_handle.pos = Point(00,300);
        test_window.push_back(Point(100,100));
        test_window.push_back(Point(200,200));
        for(int i = 0; i < 0; i++) {
            samples.push_back(Point(i*100, i*100+25));
        }
    }
    int iters;
    int splits[4];
    bool show_splits;
    std::vector<Toggle> toggles;
    AAF (*eval)(AAF, AAF);
    void recursive_implicit(Rect r, cairo_t*cr, double w) {
        if(show_splits) {
            cairo_save(cr);
            cairo_set_line_width(cr, 0.3);
            /*if(f.is_partial())
                cairo_set_source_rgba(cr, 1, 0, 1, 0.25);
                else*/
            cairo_set_source_rgba(cr, 0, 1, 0, 0.25);
            cairo_rectangle(cr, r);
            cairo_stroke(cr);
            cairo_restore(cr);
        }
        iters++;
	AAF x(interval(r.left(), r.right()));
	AAF y(interval(r.top(), r.bottom()));
        //assert(x.rad() > 0);
        //assert(y.rad() > 0);
	AAF f = (*eval)(x, y);
        double a = f.index_coeff(x.get_index(0))/x.index_coeff(x.get_index(0));
        double b = f.index_coeff(y.get_index(0))/y.index_coeff(y.get_index(0));
        AAF d = a*x + b*y - f;
        interval ivl(d);
        Point n(a,b);
        boost::optional<Rect> out = tighten(r, n, Interval(ivl.min(), ivl.max()));
        if(ivl.extent() < 0.5*L2(n)) {
            
            cairo_save(cr);
            cairo_set_source_rgba(cr, 0,1,0,0.125);
            fill_line_in_rect(cr, r, n, ivl.middle());
            cairo_fill(cr);
            cairo_restore(cr);
            draw_line_in_rect(cr, r, n, ivl.middle());
            return;
        }
        if(f.strictly_neg()) {
            cairo_save(cr);
            cairo_set_source_rgba(cr, 0,1,0,0.125);
            cairo_rectangle(cr, r);
            cairo_fill(cr);
            cairo_restore(cr);
            return;
        }
        if(!f.is_partial() and f.is_indeterminate()) {
            cairo_save(cr);
            cairo_set_line_width(cr, 0.3);
            if(f.is_infinite()) {
                cairo_set_source_rgb(cr, 1, 0.5, 0.5);
            } else if(f.is_nan()) {
                cairo_set_source_rgb(cr, 1, 1, 0);
            } else {
                cairo_set_source_rgb(cr, 1, 0, 0);
            }
            cairo_rectangle(cr, r);
            if(show_splits) {
                cairo_stroke(cr);
            } else {
                cairo_fill(cr);
            }
            cairo_restore(cr);
            return;
        }

	if((r.width() > w) or (r.height()>w)) {
	    if(f.strictly_neg() or f.straddles_zero()) {
                // Three possibilities:
                // 1) the trim operation buys us enough that we should just iterate
                Point c = r.midpoint();
                Rect oldr = r;
                if(1 && out) {
                    r = *out;
                    for(int i = 0; i < 4; i++) {
                        Point p = oldr.corner(i);
                        if(dot(n,p) < ivl.middle()) {
                            r.expandTo(p);
                        }
                    }
                }
                if(1 && out && (r.area() < oldr.area()*0.25)) {
                    splits[0] ++;
                    recursive_implicit(r, cr, w);
                // 2) one dimension is significantly smaller
                } else if(1 && (r[1].extent() < oldr[1].extent()*0.5)) {
                    splits[1]++;
                    recursive_implicit(Rect(Interval(r.left(), r.right()),
                                            Interval(r.top(), c[1])), cr,w);
                    recursive_implicit(Rect(Interval(r.left(), r.right()),
                                            Interval(c[1], r.bottom())), cr,w);
                } else if(1 && (r[0].extent() < oldr[0].extent()*0.5)) {
                    splits[2]++;
                    recursive_implicit(Rect(Interval(r.left(), c[0]),
                                            Interval(r.top(), r.bottom())), cr,w);
                    recursive_implicit(Rect(Interval(c[0], r.right()),
                                            Interval(r.top(), r.bottom())), cr,w);
                // 3) to ensure progress we must do a four way split
                } else {
                    splits[3]++;
                    recursive_implicit(Rect(Interval(r.left(), c[0]),
                                            Interval(r.top(), c[1])), cr,w);
                    recursive_implicit(Rect(Interval(c[0], r.right()),
                                            Interval(r.top(), c[1])), cr,w);
                    recursive_implicit(Rect(Interval(r.left(), c[0]),
                                            Interval(c[1], r.bottom())), cr,w);
                    recursive_implicit(Rect(Interval(c[0], r.right()),
                                            Interval(c[1], r.bottom())), cr,w);
                }
	    }
	} else {
        }
    }
    
    void key_hit(GdkEventKey *e) {
        if(e->keyval == 'w') toggles[0].toggle(); else
        if(e->keyval == 'a') toggles[1].toggle(); else
        if(e->keyval == 'q') toggles[2].toggle(); else
        if(e->keyval == 's') toggles[3].toggle();
        redraw();
    }
    void mouse_pressed(GdkEventButton* e) {
        toggle_events(toggles, e);
        Toy::mouse_pressed(e);
    }
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        origin = orig_handle.pos;
        if(1) {
            cairo_save(cr);
            cairo_set_line_width(cr, 0.3);
            cairo_set_source_rgb(cr, 0.5, 0.5, 1);
            eval = xaxis;
            //recursive_implicit(Rect(Interval(0,width), Interval(0, height)), cr, 3);
            eval = yaxis;
            //recursive_implicit(Rect(Interval(0,width), Interval(0, height)), cr, 3);
            cairo_restore(cr);
            iters = 0;
            for(int i = 0; i < 4; i++)
                splits[i] = 0;
            show_splits = toggles[0].on;
            eval = trial_eval;
            recursive_implicit(Rect(Interval(0,width), Interval(0, height)), cr, 3);
            for(int i = 0; i < 4; i++)
                *notify << splits[i] << " + ";
            *notify << " = " << iters;
        }
        if(1) {
            Rect r(test_window.pts[0], test_window.pts[1]);
            AAF x(interval(r.left(), r.right()));
            AAF y(interval(r.top(), r.bottom()));
            //AAF f = md_sample_based(x, samples.pts)-y;
            if(0) {
                x = x-500;
                y = y-300;
                x = x/200;
                y = y/200;
                AAF f = atan_sample_based(x)-y;
                cout << f << endl;
            }
            AAF f = (*eval)(x, y);
            double a = f.index_coeff(x.get_index(0))/x.index_coeff(x.get_index(0));
            double b = f.index_coeff(y.get_index(0))/y.index_coeff(y.get_index(0));
            AAF d = a*x + b*y - f;
            //cout << d << endl;
            interval ivl(d);
            Point n(a,b);
            boost::optional<Rect> out = tighten(r, n, Interval(ivl.min(), ivl.max()));
            if(out)
                cairo_rectangle(cr, *out);
            cairo_rectangle(cr, r);
            draw_line_in_rect(cr, r, n, ivl.min());
            if(f.strictly_neg()) {
                cairo_save(cr);
                cairo_set_source_rgba(cr, .5, 0.5, 0, 0.125);
                cairo_fill(cr);
                cairo_restore(cr);
            } else
                cairo_stroke(cr);
            cairo_save(cr);
            cairo_set_line_width(cr, 0.3);
            cairo_set_source_rgb(cr, 0.5, 0.5, 0);
            draw_line_in_rect(cr, r, n, ivl.middle());
            cairo_stroke(cr);
            cairo_restore(cr);
            //fill_line_in_rect(cr, r, n, c);
            draw_line_in_rect(cr, r, n, ivl.max());
            cairo_stroke(cr);
        }
        if(0) {
            Geom::ConvexHull gm(samples.pts);
            cairo_convex_hull(cr, gm);
            cairo_stroke(cr);
            Point a, b, c;
            gm.narrowest_diameter(a, b, c);
            cairo_save(cr);
            cairo_set_line_width(cr, 2);
            cairo_set_source_rgba(cr, 1, 0, 0, 0.5);
            cairo_move_to(cr, b);
            cairo_line_to(cr, c);
            cairo_move_to(cr, a);
            cairo_line_to(cr, (c-b)*dot(a-b, c-b)/dot(c-b,c-b)+b);
            cairo_stroke(cr);
            //std::cout << a << ", " << b << ", " << c << ": " << dia << "\n";
            cairo_restore(cr);
        }
        Toy::draw(cr, notify, width, height, save);
        Point d(25,25);
        toggles[0].bounds = Rect(Point(10, height-80)+d,
                                 Point(10+120, height-80+d[1])+d);

        draw_toggles(cr, toggles);
    }

};

int main(int argc, char **argv) {
    init(argc, argv, new ConvexTest());

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
