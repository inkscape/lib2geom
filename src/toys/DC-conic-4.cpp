#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis.h"

#include "path.h"
#include "path-cairo.h"
#include "path-builder.h"

#include <iterator>
#include "translate.h"

#include "toy-framework.h"

using std::vector;
using namespace Geom;

const double w = 1./3;
const double cwp = cos(w*M_PI);
const double swp = sin(w*M_PI);
double phi(double t, double w) { return sin(w*t) - w*sin(t); }
double phih(double t, double w) { return sin(w*t) + w*sin(t); }
double b4(double t, double w) {return phi(t/2,w)*phih(t/2,w)/(swp*swp);}
double b3(double t, double w) {return cwp*phi(t,w)/(2*swp) - cwp*cwp*b4(t,w); }
double b2(double t, double w) {return 2*w*w*sin(t/2)*sin(t/2);}
double b1(double t, double w) {return b3(2*M_PI - t, w);}
double b0(double t, double w) {return b4(2*M_PI - t, w);}

class arc_basis{
public:
    SBasis basis[5];
    double w;
    int k;
    
    SBasis phi(BezOrd const &d, double w) { 
        return sin(w*d, k) - w*sin(d, k); 
    }
    SBasis phih(BezOrd const &d, double w) { 
        return sin(w*d, k) + w*sin(d, k); 
    }
    SBasis b4(BezOrd const &d, double w) {
        return (1./(swp*swp))*phi(0.5*d,w)*phih(0.5*d,w);
    }
    SBasis b3(BezOrd const &d, double w) {
        return (cwp/(2*swp))*phi(d,w) - cwp*cwp*b4(d,w); 
    }

    SBasis b2(BezOrd const &d, double w) {
        return 2*w*w*sin(0.5*d, k)*sin(0.5*d, k);
    }
    SBasis b1(BezOrd const &d, double w) {
        return b3(reverse(d), w);
    }
    SBasis b0(BezOrd const &d, double w) {
        return b4(reverse(d), w);
    }
    
    
    arc_basis(double w) {
        //basis[5] = {b4, b3, b2, b1, b0};
        k = 2; // 2 seems roughly right
        const BezOrd dom(0, 2*M_PI);
        basis[0] = b4(dom, w);
        basis[1] = b3(dom, w);
        basis[2] = b2(dom, w);
        basis[3] = b1(dom, w);
        basis[4] = b0(dom, w);
    }

};

class Conic4: public Toy {
    public:
    Conic4() {
        double sc = 30;
        Geom::Point c(6*sc, 6*sc);
        handles.push_back(sc*Geom::Point(0,0)+c);
        handles.push_back(sc*Geom::Point(tan(w*M_PI)/w, 0)+c);
        handles.push_back(sc*Geom::Point(0, 1/(w*w))+c);
        handles.push_back(sc*Geom::Point(-tan(w*M_PI)/w, 0)+c);
        handles.push_back(sc*Geom::Point(0,0)+c);
    }

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
        cairo_set_line_width (cr, 0.5);
        cairo_stroke(cr);

        std::vector<Geom::Point> e_h = handles;
        for(int i = 0; i < 5; i++) {
            Geom::Point p = e_h[i];
        
            if(i)
                cairo_line_to(cr, p);
            else
                cairo_move_to(cr, p);
        }
        cairo_stroke(cr);
        
        arc_basis ab(1./3);       
        for(unsigned i  = 0; i < 5; i++)
		*notify << ab.basis[i] << std::endl;
        MultidimSBasis<2> B;
        
        for(unsigned dim  = 0; dim < 2; dim++)
            for(unsigned i  = 0; i < 5; i++)
                B[dim] += e_h[i][dim]*ab.basis[i];
        
        cairo_md_sb(cr, B);

        Toy::draw(cr, notify, width, height, save);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "conic-4.cpp", new Conic4());

    return 0;
}

/*
#include <stdio.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>
     
class SBez: public Toy {
    static int
    func (double t, const double y[], double f[],
          void *params)
    {
        double mu = *(double *)params;
        MultidimSBasis<2> B = bezier_to_sbasis<2, 3>(handles.begin());
        MultidimSBasis<2> dB = derivative(B);
        Geom::Point tan = Geom::unit_vector(point_at(dB,y[0]));
        Geom::Point yp = point_at(B, y[0]);
        double dtau = -dot(tan, yp - handles[4])/10;
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
        cairo_set_line_width (cr, 0.5);
    
        MultidimSBasis<2> B = bezier_to_sbasis<2, 3>(handles.begin());
        //cairo_md_sb(cr, B);
    
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
    }
};
*/
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
