/**
 * Contour fitting of arbitrary SB2ds using minimisation of varations.
 * unstable - needs work (njh)
 */
#include "sbasis-to-bezier.h"
#include "multidim-sbasis.h"
#include "s-basis-2d.h"
#include "sbasis-poly.h"
#include "bezier-to-sbasis.h"

#include "path.h"
#include "path-cairo.h"
#include "path-builder.h"

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_min.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>

#include <iostream>
#include <iterator>

#include "toy-framework.h"
#include "path-cairo.h"
using namespace Geom;

class curve_min{
public:
    MultidimSBasis<2> &B;
    MultidimSBasis<2> out;
    SBasis2d& sb2;
    unsigned n;
    unsigned par;
    Geom::Point start;
    curve_min(MultidimSBasis<2> &B,
              SBasis2d& sb2) :B(B), sb2(sb2) {}
};

double
my_f (const gsl_vector *v, void *params)
{
    curve_min &p = *(curve_min*)params;
    for(int dim = 0; dim < 2; dim++) {
        p.out[dim] = p.B[dim];
        double s = 1;
        for(int i = 0; i < p.n; i++) {
            p.out[dim] += shift(BezOrd(gsl_vector_get(v, 2*dim + 2*i)*s,
                                       gsl_vector_get(v, 2*dim + 2*i + 1)*s), i+1);
            s *= 0.25;
        }
    }
    SBasis l = compose(p.sb2, p.out);
    l = integral(l*l);
    return l[0][1] - l[0][0];
}

double fn1 (double x, void * params)
{
    curve_min &p = *(curve_min*)params;
    if((p.par &1) == 0)
        for(int dim = 0; dim < 2; dim++) {
            p.out[dim] = p.B[dim] + shift(BezOrd(x,0), p.par/2 + 1);
        }
    else 
        for(int dim = 0; dim < 2; dim++) {
            p.out[dim] = p.B[dim] + shift(BezOrd(0,x), p.par/2 + 1);
        }
    MultidimSBasis<2> dp = derivative(p.out);
    SBasis ds = L2(dp, 5);
    //SBasis l = compose(p.sb2, p.out);
    SBasis l = integral( ds);
    return l[0][1] - l[0][0];
}

double fn2 (double x, void * params)
{
    curve_min &p = *(curve_min*)params;
    if((p.par &1) == 0)
        for(int dim = 0; dim < 2; dim++) {
            p.out[dim] = p.B[dim] + shift(BezOrd(x), p.par/2 + 1);
        }
    else 
        for(int dim = 0; dim < 2; dim++) {
            p.out[dim] = p.B[dim] + shift(BezOrd(Hat(0), Tri(x)), p.par/2 + 1);
        }
    SBasis l = compose(p.sb2, p.out);// * L2(p.out, 3);
    l = integral(l*l);
    return l[0][1] - l[0][0];
}

vector<Geom::Point> zero_handles;

Geom::Point
gradient(SBasis2d & sb2, double u, double v) {
    return Geom::Point(derivative(extract_v(sb2, v))(v),
                       derivative(extract_u(sb2, u))(u));

}

int
func (double t, const double y[], double f[], void *params) {
    curve_min &p = *(curve_min*)params;
    
    Geom::Point grad = unit_vector(gradient(p.sb2, y[0], y[1]));
    double value = p.sb2.apply(y[0], y[1]);
    f[0] = grad[1];// - 0.01*grad[0]*value*(value);
    f[1] = -grad[0];// - 0.01*grad[1]*value*(value);
    
    return GSL_SUCCESS;
}
     
int jac (double t, const double y[], double *dfdy, double dfdt[], void *params) {
    return GSL_FAILURE;
}
     
class Contour : public Toy {
virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    SBasis2d sb2;
    sb2.us = 2;
    sb2.vs = 2;
    const int depth = sb2.us*sb2.vs;
    const int surface_handles = 4*depth;
    sb2.resize(depth, BezOrd2d(0));
    vector<Geom::Point> display_handles(surface_handles);
    Geom::Point dir = (handles[surface_handles] - Geom::Point(3*width/4., width/4.)) / 30;
    cairo_move_to(cr, 3*width/4., width/4.);
    cairo_line_to(cr, handles[surface_handles]);
    for(int vi = 0; vi < sb2.vs; vi++)
    for(int ui = 0; ui < sb2.us; ui++)
    for(int iv = 0; iv < 2; iv++)
    for(int iu = 0; iu < 2; iu++) {
        unsigned corner = iu + 2*iv;
        unsigned i = ui + vi*sb2.us;
        Geom::Point base((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
                         (2*(iv+vi)/(2.*vi+1)+1)*width/4.);
        double dl = dot((handles[corner+4*i] - base), dir)/dot(dir,dir);
        display_handles[corner+4*i] = dl*dir + base;
        sb2[i][corner] = dl*10/(width/2)*pow(4,ui+vi);
    }
    draw_sb2d(cr, sb2, dir*0.1, width);
    cairo_set_source_rgba (cr, 0., 0., 0, 0.7);
    cairo_stroke(cr);
    zero_handles.clear();
    for(int ui = 0; ui <= 1; ui++) {
        SBasis sb = extract_u(sb2, ui);
        vector<double> r = roots(sb);
        *notify << "sbasis sub (%d, 0): ";
        std::copy(r.begin(), r.end(), std::ostream_iterator<double>(*notify, ",\t"));
        *notify << std::endl;
        for(int i = 0; i < r.size(); i++) {
            zero_handles.push_back(Geom::Point(ui, r[i]));
        }
    }
    for(int vi = 0; vi <= 1; vi++) {
        SBasis sb = extract_v(sb2, vi);
        vector<double> r = roots(sb);
        *notify << "sbasis sub (0, %d): ";
        std::copy(r.begin(), r.end(), std::ostream_iterator<double>(*notify, ",\t"));
        *notify << std::endl;
        for(int i = 0; i < r.size(); i++) {
            zero_handles.push_back(Geom::Point(r[i], vi));
        }
    }
        std::copy(zero_handles.begin(), zero_handles.end(), std::ostream_iterator<Geom::Point>(*notify, ",\t"));
    
    for(int i = 0; i < zero_handles.size(); i++) {
        Geom::Point p = zero_handles[i];
        zero_handles[i] = p = p*(width/2) + Geom::Point(width/4., width/4.);
        draw_cross(cr, p);
        cairo_stroke(cr);
    }
    
    MultidimSBasis<2> B = bezier_to_sbasis<2, 1>(handles.begin() + surface_handles+1);
    B += Geom::Point(-width/4., -width/4.);
    B *= (2./width);

    for(int dim = 0; dim < 2; dim++) {
        B[dim] += shift(BezOrd(0.1), 1);
    }
#if 0
//  *** Minimiser
    curve_min cm(B, sb2);
    for(cm.par = 0; cm.par < 3; cm.par++) {
    int status;
    int iter = 0, max_iter = 100;
    const gsl_min_fminimizer_type *T;
    gsl_min_fminimizer *s;
    double m = 0.0;
    double a = -1000.0, b = 1000.0;
    gsl_function F;
     
    F.function = &fn2;
    F.params = &cm;
     
    T = gsl_min_fminimizer_brent;
    s = gsl_min_fminimizer_alloc (T);
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
    cm.B = cm.out;
    }
    
    B = cm.out;
#elif 0
    const gsl_multimin_fminimizer_type *T =
        gsl_multimin_fminimizer_nmsimplex;
    gsl_multimin_fminimizer *s = NULL;
    gsl_vector *ss, *x;
    gsl_multimin_function minex_func;
    
    size_t iter = 0, i;
    int status;
    double size;
    curve_min cm(B, sb2);
     
    /* Initial vertex size vector */
    cm.n = 1;
    size_t np = cm.n*4;
    ss = gsl_vector_alloc (np);
    gsl_vector_set_all (ss, 0.1);
     
    /* Starting point */
    x = gsl_vector_alloc (np);
    gsl_vector_set_all (x, 0.0);
     
    /* Initialize method and iterate */
    minex_func.f = &my_f;
    minex_func.n = np;
    minex_func.params = (void *)&cm;
     
    s = gsl_multimin_fminimizer_alloc (T, np);
    gsl_multimin_fminimizer_set (s, &minex_func, x, ss);
     
    do
    {
        iter++;
        status = gsl_multimin_fminimizer_iterate(s);
     
        if (status)
            break;
     
        size = gsl_multimin_fminimizer_size (s);
        status = gsl_multimin_test_size (size, 1e-2);
        /*
        if (status == GSL_SUCCESS)
        {
            printf ("converged to minimum at\n");
        }
     
        printf ("%5d ", iter);
        for (i = 0; i < np; i++)
        {
            printf ("%10.3e ", gsl_vector_get (s->x, i));
        }
        printf ("f() = %7.3f size = %.3f\n", s->fval, size);*/
    }
    while (status == GSL_CONTINUE && iter < 100);
     
    gsl_vector_free(x);
    gsl_vector_free(ss);
    gsl_multimin_fminimizer_free (s);
     
    B = cm.out;
#elif 1
    SBasis sb = extract_u(sb2, 0);
    vector<double> r = roots(sb);
    if(!r.empty()) {
    curve_min cm(B, sb2);
    double rt = r[0];
    cm.start = Geom::Point(width/4, BezOrd(width/4, width*3/4)(rt));
    cairo_move_to(cr, cm.start);
    
    const gsl_odeiv_step_type * T 
        = gsl_odeiv_step_rk8pd;
     
    gsl_odeiv_step * s 
        = gsl_odeiv_step_alloc (T, 2);
    gsl_odeiv_control * c 
        = gsl_odeiv_control_y_new (1e-6, 0.0);
    gsl_odeiv_evolve * e 
        = gsl_odeiv_evolve_alloc (2);
     
    gsl_odeiv_system sys = {func, jac, 2, &cm};
     
    double t = 0.0;
    double h = 1e-6;
    double y[2] = { 0, rt };
    
    for(int ti = 0; ti < 3000; ti++) {
        double t1 = 0.01*ti;
        while (t < t1) {
            int status = gsl_odeiv_evolve_apply (e, c, s, &sys, &t, t1, &h, y);
            if(y[0] < 0 ||
               y[0] > 1 || 
               y[1] < 0 || 
               y[1] > 1)
                break;
            if (status != GSL_SUCCESS)
                goto oops;
        }
        cairo_line_to(cr, BezOrd(width/4, width*3/4)(y[0]), BezOrd(width/4, width*3/4)(y[1]));
            if(y[0] < 0 ||
               y[0] > 1 || 
               y[1] < 0 || 
               y[1] > 1)
                break;
    }
oops:
    
    gsl_odeiv_evolve_free (e);
    gsl_odeiv_control_free (c);
    gsl_odeiv_step_free (s);
  }
#endif
     
#if 0
    cairo_md_sb(cr, (width/2)*B + Geom::Point(width/4., width/4.));
    SBasis l = compose(sb2, B);
    notify << "l = " << l << std::endl ;
    l = integral(l*l);
    notify << "cost = " << l[0][1] - l[0][0] ;
#endif
Toy::draw(cr, notify, width, height, save);
}

    public:
    Contour() {
        SBasis2d sb2;
        sb2.us = 2;
        sb2.vs = 2;
        sb2.resize(4, BezOrd2d(0));
        Geom::Point dir(1,-2);
        for(int vi = 0; vi < sb2.vs; vi++)
        for(int ui = 0; ui < sb2.us; ui++)
        for(int iv = 0; iv < 2; iv++)
        for(int iu = 0; iu < 2; iu++) {
            Geom::Point p((2*(iu+ui)/(2.*ui+1)+1),
                          (2*(iv+vi)/(2.*vi+1)+1));
            if(ui == 0 && vi == 0) {
                if(iu == 0 && iv == 0)
                    p[1] += 0.1;
                if(iu == 1 && iv == 1 )
                    p[1] -= 0.1;
                /*if(iu != iv)
                    p[1] += 0.1;
                if(vi == sb2.vs - 1)
                    p[1] += 0.1;*/
            }
            handles.push_back((600/4.)*p);
        }
        
        handles.push_back(Geom::Point(3*600/4., 600/4.) + 30*dir);
        for(int i = 0; i < 2; i++)
            handles.push_back(Geom::Point(3*600/8., (1+6*i)*600/8.));
    }
};

/*
on_open_activate(GtkMenuItem *menuitem, gpointer user_data) {
    //TODO: show open dialog, get filename
    
    char const *const filename = "banana.svgd";

    FILE* f = fopen(filename, "r");
    if (!f) {
        perror(filename);
        return;
    }
    
    gtk_widget_queue_draw(canvas); // globals are probably evil
}*/

int main(int argc, char **argv) {
    init(argc, argv, "Contour", new Contour());
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
