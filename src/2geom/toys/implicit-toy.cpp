

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <2geom/geom.h>
#include <2geom/d2.h>
#include <2geom/poly.h>
#include <2geom/sbasis-poly.h>
#include <2geom/transforms.h>

#include <2geom/symbolic/implicit.h>

#include <aa.h>

#include <algorithm>
#include <ctime>

#include <boost/optional.hpp>
#include <boost/function.hpp>


using namespace Geom;



struct PtLexCmp{
    bool operator()(const Point &a, const Point &b) {
        return (a[0] < b[0]) || ((a[0] == b[0]) and (a[1] < b[1]));
    }
};

//typedef AAF (*implicit_curve_t)(AAF, AAF);
typedef boost::function<AAF (AAF const&, AAF const&)> implicit_curve_t;

// draw ax + by + c = 0
void draw_line_in_rect(cairo_t*cr, Rect &r, Point n, double c)
{
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
    if(result.size() == 2)
    {
        cairo_move_to(cr, result[0]);
        cairo_line_to(cr, result[1]);
        cairo_stroke(cr);
    }
}

OptRect tighten(Rect const&r, Point n, Interval lu)
{
    vector<Geom::Point> result;
    Point resultp;
    for(int i = 0; i < 4; i++)
    {
        Point cnr = r.corner(i);
        double z = dot(cnr, n);
        if ((z > lu[0]) && (z < lu[1]))
            result.push_back(cnr);
    }
    for(int i = 0; i < 2; i++)
    {
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
        return OptRect;
    Rect nr(result[0], result[1]);
    for(size_t i = 2; i < result.size(); i++)
    {
        nr.expandTo(result[i]);
    }
    return intersect(nr, r);
}

static const unsigned int DEG = 5;
double bvp[DEG+1][DEG+1]
    = {{-1, 0.00945115, -4.11799e-05, 1.01365e-07, -1.35037e-10, 7.7868e-14},
       {0.00837569, -6.24676e-05, 1.96093e-07, -3.09683e-10, 1.95681e-13, 0},
       {-2.39448e-05, 1.3331e-07, -2.65787e-10, 1.96698e-13, 0, 0},
       {2.76173e-08, -1.01069e-10, 9.88596e-14, 0, 0, 0},
       {-1.43584e-11, 2.48433e-14, 0, 0, 0, 0}, {2.49723e-15, 0, 0, 0, 0, 0}};


AAF trial_eval(AAF const& x, AAF const& y) {
//    AAF x = _x/100;
//    AAF y = _y/100;
    //return x*x - 1;
    //return y - pow(x,3);
    //return y - pow_sample_based(x,2.5);
    //return y - log_sample_based(x);
    //return y - log(x);
    //return y - exp_sample_based(x*log(x));
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
    //return sin(x*y) + cos(pow(x, 3)) - atan(x);
    //return pow((x*x + y*y), 2) - (x*x-y*y);
    //return x*x-y;
    //return (x*x*x-y*x)*sin(x) + (x-y*y)*cos(y)-0.5;
//    return -120.75 +(-64.4688 +(-16.6875 +(0.53125 -0.00390625*y)*y)*y)*y
//            + (-15.9375 + ( 1.5 +( 4.375 -0.0625*y)*y)*y
//            + (17 +( 9.5 -0.375*y)*y + (2 + -1*y -1*x)*x)*x)*x;

//    AAF v(0);
//    for (size_t i = DEG; i > 0; --i)
//    {
//        AAF vy(0);
//        for (size_t j = DEG - i; j > 0; --j)
//        {
//            vy += bvp[i][j];
//            vy *= y;
//        }
//        vy += bvp[i][0];
//        v += vy;
//        v *= x;
//    }
//    AAF vy(0);
//    for (size_t j = DEG; j > 0; --j)
//    {
//        vy += bvp[0][j];
//        vy *= y;
//    }
//    vy += bvp[0][0];
//    v += vy;
//    return v;

    int i = DEG;
    int j = DEG - i;
    AAF vy(bvp[i][j]);
    --j;
    for (; j >= 0; --j)
    {
        vy *= y;
        vy += bvp[DEG][j];
    }
    AAF v(vy);
    --i;
    for (; i >= 0; --i)
    {
        int j = DEG - i;
        AAF vy(bvp[i][j]);
        --j;
        for (; j >= 0; --j)
        {
            vy *= y;
            vy += bvp[i][j];
        }
        v *= x;
        v += vy;
    }
    return v;

//    return
//      -1 +( 0.00945115 +( -4.11799e-05 +( 1.01365e-07 +( -1.35037e-10 + 7.7868e-14*y)*y)*y)*y)*y
//        + (0.00837569 +( -6.24676e-05 +( 1.96093e-07 +( -3.09683e-10 + 1.95681e-13*y)*y)*y)*y
//        + (-2.39448e-05 +( 1.3331e-07 +( -2.65787e-10 + 1.96698e-13*y)*y)*y
//        + (2.76173e-08 +( -1.01069e-10 + 9.88596e-14*y)*y
//        + (-1.43584e-11 + 2.48433e-14*y  + 2.49723e-15*x)*x)*x)*x)*x;
}



double max_modulus (SL::MVPoly2 const& p)
{
    double a, m = 1;

    for (size_t i = 0; i < p.get_poly().size(); ++i)
        for (size_t j = 0; j < p[i].size(); ++j)
        {
            a = std::abs(p[i][j]);
            if (m < a) m = a;
        }
    return m;
}

void poly_to_mvpoly1(SL::MVPoly1& p, Geom::Poly const& q)
{
    for (size_t i = 0; i < q.size(); ++i)
    {
        p.coefficient(i, q[i]);
    }
    p.normalize();
}

void make_implicit_curve (SL::MVPoly2& ic, D2<SBasis> const& pc)
{
    Geom::Poly pc0 = sbasis_to_poly(pc[0]);
    Geom::Poly pc1 = sbasis_to_poly(pc[1]);

//    std::cerr << "parametrization: \n";
//    std::cerr << "pc0 = " << pc0 << std::endl;
//    std::cerr << "pc1 = " << pc1 << "\n\n";

    SL::MVPoly1 f, g;
    poly_to_mvpoly1(f, pc0);
    poly_to_mvpoly1(g, pc1);

//    std::cerr << "parametrization: \n";
//    std::cerr << "f = " << f << std::endl;
//    std::cerr << "g = " << g << "\n\n";

    Geom::SL::basis_type b;
    microbasis(b, f, g);

    Geom::SL::MVPoly3 p, q;
    basis_to_poly(p, b[0]);
    basis_to_poly(q, b[1]);

//    std::cerr << "generators as polynomial in R[t,x,y] : \n";
//    std::cerr << "p = " << p << std::endl;
//    std::cerr << "q = " << q << "\n\n";


    Geom::SL::Matrix<Geom::SL::MVPoly2> B = make_bezout_matrix(p, q);
    ic = determinant_minor(B);
    ic.normalize();
    double m = max_modulus(ic);
    ic /= m;

//    std::cerr << "Bezout matrix: (entries are bivariate polynomials) \n";
//    std::cerr << "B = " << B << "\n\n";
//    std::cerr << "determinant: \n";
//    std::cerr << "r(x, y) = " << ic << "\n\n";

}

//namespace Geom{ namespace SL{
//
//template<>
//struct zero<AAF, false>
//{
//    AAF operator() () const
//    {
//        return AAF(0);
//    }
//};
//
//} }

class ImplicitToy : public Toy
{
    bool contains_zero (implicit_curve_t const& eval,
                        Rect r, double w=1e-5)
    {
        ++iters;
        AAF x(interval(r.left(), r.right()));
        AAF y(interval(r.top(), r.bottom()));
        AAF f = eval(x, y);
        double a = f.index_coeff(x.get_index(0)) / x.index_coeff(x.get_index(0));
        double b = f.index_coeff(y.get_index(0)) / y.index_coeff(y.get_index(0));
        AAF d = a*x + b*y - f;
        interval ivl(d);
        Point n(a,b);
        OptRect out = tighten(r, n, Interval(ivl.min(), ivl.max()));
        if (f.straddles_zero())
        {
            if ((r.width() > w) || (r.height() > w))
            {
                Point c = r.midpoint();
                Rect oldr = r;
                if (out)  r = *out;
                // Three possibilities:
                // 1) the trim operation buys us enough that we should just iterate
                if (1 && (r.area() < oldr.area()*0.25))
                {
                    return contains_zero(eval, r,  w);
                }
                // 2) one dimension is significantly smaller
                else if (1 && (r[1].extent() < oldr[1].extent()*0.5))
                {
                    return contains_zero (eval,
                                          Rect(Interval(r.left(), r.right()),
                                               Interval(r.top(), c[1])), w)
                        || contains_zero (eval,
                                          Rect(Interval(r.left(), r.right()),
                                               Interval(c[1], r.bottom())), w);
                }
                else if (1 && (r[0].extent() < oldr[0].extent()*0.5))
                {
                    return contains_zero (eval,
                                          Rect(Interval(r.left(), c[0]),
                                               Interval(r.top(), r.bottom())), w)
                        || contains_zero (eval,
                                          Rect(Interval(c[0], r.right()),
                                               Interval(r.top(), r.bottom())), w);
                }
                // 3) to ensure progress we must do a four way split
                else
                {
                    return contains_zero (eval,
                                          Rect(Interval(r.left(), c[0]),
                                               Interval(r.top(), c[1])), w)
                        || contains_zero (eval, 
                                          Rect(Interval(c[0], r.right()),
                                               Interval(r.top(), c[1])), w)
                        || contains_zero (eval, 
                                          Rect(Interval(r.left(), c[0]),
                                               Interval(c[1], r.bottom())), w)
                        || contains_zero (eval, 
                                          Rect(Interval(c[0], r.right()),
                                               Interval(c[1], r.bottom())), w);
                }
            }
            //std::cout << w << " < " << r.width() << " , " << r.height() << std::endl;
            //std::cout << r.min() << " - " << r.max() << std::endl;
            return true;
        }
        return false;
    }  // end recursive_implicit

    
    void draw_implicit_curve (cairo_t*cr, implicit_curve_t const& eval,
                              Point const& origin, Rect r, double w)
    {
        ++iters;
        AAF x(interval(r.left(), r.right()));
        AAF y(interval(r.top(), r.bottom()));
        //assert(x.rad() > 0);
        //assert(y.rad() > 0);
//        time(&t0);
        AAF f = eval(x-origin[X], y-origin[Y]);
//        time(&t1);
//        d1 += std::difftime(t1, t0);
        // pivot
//        time(&t2);
        double a = f.index_coeff(x.get_index(0)) / x.index_coeff(x.get_index(0));
        double b = f.index_coeff(y.get_index(0)) / y.index_coeff(y.get_index(0));
        AAF d = a*x + b*y - f;
        interval ivl(d);
        Point n(a,b);
        OptRect out = tighten(r, n, Interval(ivl.min(), ivl.max()));
        if (ivl.extent() < 0.5*L2(n))
        {
            draw_line_in_rect(cr, r, n, ivl.middle());
            return;
        }
//        time(&t3);
//        d2 += std::difftime(t3, t2);
        if ((r.width() > w) || (r.height() > w))
        {
            if (f.straddles_zero())
            {
                Point c = r.midpoint();
                Rect oldr = r;
                if (out)  r = *out;
                // Three possibilities:
                // 1) the trim operation buys us enough that we should just iterate
                if (1 && (r.area() < oldr.area()*0.25))
                {
                    draw_implicit_curve(cr, eval, origin, r,  w);
                }
                // 2) one dimension is significantly smaller
                else if (1 && (r[1].extent() < oldr[1].extent()*0.5))
                {
                    draw_implicit_curve (cr, eval, origin,
                                         Rect(Interval(r.left(), r.right()),
                                              Interval(r.top(), c[1])), w);
                    draw_implicit_curve (cr, eval, origin,
                                         Rect(Interval(r.left(), r.right()),
                                              Interval(c[1], r.bottom())), w);
                }
                else if (1 && (r[0].extent() < oldr[0].extent()*0.5))
                {
                    draw_implicit_curve (cr, eval, origin,
                                         Rect(Interval(r.left(), c[0]),
                                              Interval(r.top(), r.bottom())), w);
                    draw_implicit_curve (cr, eval, origin,
                                         Rect(Interval(c[0], r.right()),
                                              Interval(r.top(), r.bottom())), w);
                }
                // 3) to ensure progress we must do a four way split
                else
                {
                    draw_implicit_curve (cr, eval, origin,
                                         Rect(Interval(r.left(), c[0]),
                                              Interval(r.top(), c[1])), w);
                    draw_implicit_curve (cr, eval, origin,
                                         Rect(Interval(c[0], r.right()),
                                              Interval(r.top(), c[1])), w);
                    draw_implicit_curve (cr, eval, origin,
                                         Rect(Interval(r.left(), c[0]),
                                              Interval(c[1], r.bottom())), w);
                    draw_implicit_curve (cr, eval, origin,
                                         Rect(Interval(c[0], r.right()),
                                              Interval(c[1], r.bottom())), w);
                }
            }
        } else {
            if(contains_zero(eval, r*Geom::Translate(-origin))) {
                cairo_save(cr);
                cairo_set_source_rgb(cr, 0,0.5,0);
                cairo_rectangle(cr, r);
                cairo_fill(cr);
                cairo_restore(cr);
            }
        }
    }  // end recursive_implicit

    void draw( cairo_t *cr, std::ostringstream *notify,
               int width, int height, bool save )
    {
        iters = 0;
        d1 = d2 = 0;
        cairo_set_line_width (cr, 0.3);
        D2<SBasis> A = pshA.asBezier();
        cairo_md_sb(cr, A);
        cairo_stroke(cr);

        SL::MVPoly2 ic;
        make_implicit_curve(ic, A);

        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 0.8);
        draw_implicit_curve (cr, ic, orig_handle.pos,
                             Rect(Interval(0,width), Interval(0, height)), 1);
        cairo_stroke(cr);

//        std::cerr << "D1 = " << d1 << std::endl;
//        std::cerr << "D2 = " << d2 << std::endl;

        *notify << "iter: " << iters;
        Toy::draw(cr, notify, width, height, save);
    }


public:
    ImplicitToy(unsigned int _A_bez_ord)
        : A_bez_ord(_A_bez_ord)
    {
        handles.push_back(&orig_handle);
        orig_handle.pos = Point(0,0); //Point(300,300);

        handles.push_back(&pshA);
        for (unsigned int i = 0; i < A_bez_ord; ++i)
            pshA.push_back(Geom::Point(uniform()*400, uniform()*400));
    }

private:
    unsigned int A_bez_ord;
    PointHandle orig_handle;
    PointSetHandle pshA;
    time_t t0, t1, t2, t3;
    double d1, d2;
    unsigned int iters;
};


int main(int argc, char **argv)
{
    unsigned int A_bez_ord=5;
    if(argc > 1)
        sscanf(argv[1], "%d", &A_bez_ord);


    init( argc, argv, new ImplicitToy(A_bez_ord));
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
