#include "path-metric.h"
#include "arc-length.h"
#ifdef HAVE_GSL
#include <gsl/gsl_integration.h>
#endif

struct path_metric_data{
    Geom::Path const *a;
    Geom::Path const *b;
};

static double path_metric_integrand(double s, void* param) {
    path_metric_data &pmd(*(path_metric_data*)param);
    Geom::Path::Location a_l = 
        natural_parameterisation(*pmd.a, s, 1e-3);
    Geom::Point a_p = pmd.a->point_at(a_l);
    double dist = 0;
    pmd.b->nearest_location(a_p, dist);
    //std::cout << s << "->" << dist*dist << std::endl;
    return dist*dist;
}

double L2(Geom::Path const &a, Geom::Path const &b, double abstol, double reltol) {
#ifdef HAVE_GSL
    path_metric_data pmd;
    pmd.a = &a;
    pmd.b = &b;
    
    double sl = arc_length_integrating(a, abstol);
    gsl_function F;
    gsl_integration_workspace * w 
        = gsl_integration_workspace_alloc (1000);
    F.function = &path_metric_integrand;
    F.params = (void*)&pmd;
    double quad_result, err;
    int returncode = gsl_integration_qags (&F, 0, sl, abstol, reltol, 1000, 
                                           w, &quad_result, &err);
    //gsl_integration_qag (&F, 0, sl, 0, tol, 1000, 
    //GSL_INTEG_GAUSS21, w, &quad_result, &err);
    return sqrt(quad_result);
#endif
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
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
