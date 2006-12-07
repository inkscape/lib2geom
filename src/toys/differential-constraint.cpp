#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis.h"

#include "path.h"
#include "path-cairo.h"
#include "path-builder.h"

#include "toy-framework.h"

using std::vector;
using namespace Geom;

unsigned total_pieces_sub;
unsigned total_pieces_inc;

void cairo_md_sb(cairo_t *cr, multidim_sbasis<2> const &B) {
    std::vector<Geom::Point> bez = sbasis_to_bezier(B, 2);
    cairo_move_to(cr, bez[0]);
    cairo_curve_to(cr, bez[1], bez[2], bez[3]);
}

#include <stdio.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>

vector<Geom::Point> *handlesptr = NULL;

class SBez: public Toy {
    static int
    func (double t, const double y[], double f[],
          void *params)
    {
        double mu = *(double *)params;
        multidim_sbasis<2> B = bezier_to_sbasis<2, 3>(handlesptr->begin());
        multidim_sbasis<2> dB = derivative(B);
        Geom::Point tan = point_at(dB,y[0]);//Geom::unit_vector();
        tan /= dot(tan,tan);
        Geom::Point yp = point_at(B, y[0]);
        double dtau = -dot(tan, yp - (*handlesptr)[4]);
        f[0] = dtau;
        
        return GSL_SUCCESS;
    }
     
    static int
    jac (double t, const double y[], double *dfdy, 
         double dfdt[], void *params)
    {
        double mu = *(double *)params;
        gsl_matrix_view dfdy_mat 
            = gsl_matrix_view_array (dfdy, 2, 2);
        gsl_matrix * m = &dfdy_mat.matrix; 
        gsl_matrix_set (m, 0, 0, 0.0);
        gsl_matrix_set (m, 0, 1, 1.0);
        gsl_matrix_set (m, 1, 0, -2.0*mu*y[0]*y[1] - 1.0);
        gsl_matrix_set (m, 1, 1, -mu*(y[0]*y[0] - 1.0));
        dfdt[0] = 0.0;
        dfdt[1] = 0.0;
        return GSL_SUCCESS;
    }
     
    double y[2];

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        handlesptr = &handles;
        cairo_set_line_width (cr, 0.5);
    
        multidim_sbasis<2> B = bezier_to_sbasis<2, 3>(handles.begin());
        cairo_md_sb(cr, B);
    
        const gsl_odeiv_step_type * T 
            = gsl_odeiv_step_rk8pd;
     
        gsl_odeiv_step * s 
            = gsl_odeiv_step_alloc (T, 1);
        gsl_odeiv_control * c 
            = gsl_odeiv_control_y_new (1e-6, 0.0);
        gsl_odeiv_evolve * e 
            = gsl_odeiv_evolve_alloc (1);
     
        double mu = 10;
        gsl_odeiv_system sys = {func, jac, 1, &mu};
     
        static double t = 0.0;
        double t1 = t + 1;
        double h = 1e-6;
     
        while (t < t1)
        {
            int status = gsl_odeiv_evolve_apply (e, c, s,
                                                 &sys, 
                                                 &t, t1,
                                                 &h, y);
     
            if (status != GSL_SUCCESS)
                break;
        
            //printf ("%.5e %.5e %.5e\n", t, y[0], y[1]);
        }
    
        draw_cross(cr, point_at(B, y[0]));
     
        gsl_odeiv_evolve_free (e);
        gsl_odeiv_control_free (c);
        gsl_odeiv_step_free (s);

        Toy::draw(cr, notify, width, height, save);
    }
public:
    SBez() {
        y[0] = 0;
        handles.push_back(Geom::Point(uniform()*400, uniform()*400));
        handles.push_back(Geom::Point(uniform()*400, uniform()*400));
        handles.push_back(Geom::Point(uniform()*400, uniform()*400));
        handles.push_back(Geom::Point(uniform()*400, uniform()*400));
        handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "s-bez", new SBez());

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
