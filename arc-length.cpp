#include "path-find-points-of-interest.h"
#include "cubic_bez_util.h"
using namespace Geom;


double cubic_length_subdividing(Geom::SubPath::SubPathElem e, double tol) {
    Geom::Point v[3];
    for(int i = 0; i < 3; i++)
        v[i] = e[i+1] - e[0];
    Geom::Point orth = v[2]; // unit normal to path line
    rot90(orth);
    orth.normalize();
    double err = fabs(dot(orth, v[1])) + fabs(dot(orth, v[0]));
    if(err < tol) {
        return Geom::L2(e.last() - e.first()); // approximately a line
    } else {
        Geom::Point mid[3];
        double result;
        for(int i = 0; i < 3; i++)
            mid[i] = Lerp(0.5, e[i], e[i+1]);
        Geom::Point midmid[2];
        for(int i = 0; i < 2; i++)
            midmid[i] = Lerp(0.5, mid[i], mid[i+1]);
        Geom::Point midmidmid = Lerp(0.5, midmid[0], midmid[1]);
        {
            Geom::Point curve[4] = {e[0], mid[0], midmid[0], midmidmid};
            Geom::SubPath::SubPathElem e0(Geom::cubicto, std::vector<Geom::Point>::const_iterator(curve), std::vector<Geom::Point>::const_iterator(curve) + 4);
            result = cubic_length_subdividing(e0, tol);
        } {
            Geom::Point curve[4] = {midmidmid, midmid[1], mid[2], e[3]};
            Geom::SubPath::SubPathElem e1(Geom::cubicto, std::vector<Geom::Point>::const_iterator(curve), std::vector<Geom::Point>::const_iterator(curve) + 4);
            return result + cubic_length_subdividing(e1, tol);
        }
    }
}

double arc_length_subdividing(Geom::SubPath p, double tol) {
    double result = 0;

    for(Geom::SubPath::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        switch(iter.cmd()) {
        case Geom::moveto:
            break;
        case Geom::lineto:
        {
            result += L2((*iter).first() - (*iter).last());
            break;
        }
        case Geom::cubicto:
        {
            result += cubic_length_subdividing(*iter, tol);
            break;
        }
        default:
            break;
        }
    }
    
    return result;
}

#include <gsl/gsl_integration.h>
// int gsl_integration_qng (const gsl_function * f, double a, double b, double epsabs, double epsrel, double * result, double * abserr, size_t * neval)
// int gsl_integration_qag (const gsl_function * f, double a, double b, double epsabs, double epsrel, size_t limit, int key, gsl_integration_workspace * workspace, double * result, double * abserr)
double cubic_length_integrating(double t, void* param) {
    Point* pc = (Point*)param;
    Point p = t*(3*t*pc[3] + 2*pc[2]) + pc[1];
    return sqrt(dot(p,p));
}

void arc_length_integrating(Geom::SubPath::SubPathElem pe, double tol, double &result, double &abs_error) {
    switch(pe.op) {
    case Geom::lineto:
    {
        result += L2(pe.first() - pe.last());
        break;
    }
    case Geom::cubicto:
    {
        Geom::Point pc[4];
        for(int i = 0; i < 4; i++)
            pc[i] = Point(0,0);
            
        cubic_bezier_poly_coeff(pe.begin(), pc);

        gsl_function F;
        gsl_integration_workspace * w 
            = gsl_integration_workspace_alloc (20);
        F.function = &cubic_length_integrating;
        F.params = (void*)pc;
        double quad_result, err;
        /* We could probably use the non adaptive code here if we removed any cusps first. */
        int returncode = 
            gsl_integration_qag (&F, 0, 1, 0, tol, 20, 
                                 GSL_INTEG_GAUSS21, w, &quad_result, &err);
            
        abs_error += fabs(err);
            
        result += quad_result;
        break;
    }
    default:
        return;
    }
}

double arc_length_integrating(Geom::SubPath p, double tol) {
    double result = 0, abserr = 0;

    for(Geom::SubPath::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        arc_length_integrating(*iter, tol, result, abserr);
    }
    //printf("got %g with err %g\n", result, abserr);
    
    return result;
}

double arc_length_integrating(Geom::SubPath p, Geom::SubPath::SubPathLocation pl, double tol) {
    double result = 0, abserr = 0;

    for(Geom::SubPath::const_iterator iter(p.begin()), end(p.end()); 
        (iter != end); ++iter) {
        arc_length_integrating(*iter, tol, result, abserr);
    }
    
    return result;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
