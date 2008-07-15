/*
 * Ellipse and Elliptical Arc Fitting Example
 *
 * Authors:
 *      Marco Cecchetti <mrcekets at gmail.com>
 *
 * Copyright 2008  authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */


#include <2geom/numeric/fitting-tool.h>
#include <2geom/numeric/fitting-model.h>

#include <2geom/ellipse.h>
#include <2geom/svg-elliptical-arc.h>
#include <2geom/utils.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <stdio.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_min.h>




using namespace Geom;


class LFMEllipseArea
    : public NL::LinearFittingModelWithFixedTerms<Point, double, Ellipse>
{
  public:
    LFMEllipseArea(double coeff)
        : m_coeff(coeff*coeff)
    {
    }
    void feed( NL::VectorView & coeff, double & fixed_term, Point const& p ) const
    {
        coeff[0] = p[X] * p[Y];
        coeff[1] = p[X];
        coeff[2] = p[Y];
        coeff[3] = 1;
        fixed_term = p[X] * p[X] + m_coeff * p[Y] * p[Y];
    }

    size_t size() const
    {
        return 4;
    }

    void instance(Ellipse & e, NL::ConstVectorView const& coeff) const
    {
//        std::cerr << " B = " << coeff[0]
//                  << " C = " << decimal_round(m_coeff,10)
//                  << " D = " << coeff[1]
//                  << " E = " << coeff[2]
//                  << " F = " << coeff[3]
//                  << std::endl;
        e.set(1, coeff[0], m_coeff, coeff[1], coeff[2], coeff[3]);
    }

  private:
      double m_coeff;
};

inline
Ellipse fitting(std::vector<Point> const& points, double coeff)
{
    size_t sz = points.size();
    if (sz != 4)
    {
        THROW_RANGEERROR("fitting error: too few points passed");
    }
    LFMEllipseArea model(coeff);
    NL::least_squeares_fitter<LFMEllipseArea> fitter(model, sz);

    for (size_t i = 0; i < sz; ++i)
    {
        fitter.append(points[i]);
    }
    fitter.update();

    NL::Vector z(sz, 0.0);
    Ellipse e;
    model.instance(e, fitter.result(z));
    return e;
}


inline
double area_goal(double coeff, void* params)
{
    typedef std::vector<Point> point_set_t;
    const point_set_t & points = *static_cast<point_set_t*>(params);
    Ellipse e;
    try
    {
        e = fitting(points, coeff);
    }
    catch(LogicalError exc)
    {
        //std::cerr << exc.what() << std::endl;
        return 1e18;
    }
    return e.ray(X) * e.ray(Y);
}


inline
double perimeter_goal(double coeff, void* params)
{
    typedef std::vector<Point> point_set_t;
    const point_set_t & points = *static_cast<point_set_t*>(params);
    Ellipse e;
    try
    {
        e = fitting(points, coeff);
    }
    catch(LogicalError exc)
    {
        //std::cerr << exc.what() << std::endl;
        return 1e18;
    }
    return e.ray(X) + e.ray(Y);
}

void no_minimum_error_handler (const char * reason,
                               const char * file,
                               int line,
                               int gsl_errno)
{
    if (gsl_errno == GSL_EINVAL)
    {
        std::cerr << "gsl: " << file << ":" << line << " ERROR: " << reason << std::endl;
    }
    else
    {
        gsl_error(reason, file, line, gsl_errno);
    }
}

typedef  double goal_function_type(double coeff, void* params);

double minimizer (std::vector<Point> & points, goal_function_type* gf)
{
    int status;
    int iter = 0, max_iter = 1e3;
    const gsl_min_fminimizer_type *T;
    gsl_min_fminimizer *s;
    double m = 1.0;
    double a = 1e-2, b = 1e2;
    gsl_function F;

    F.function = gf;
    F.params = static_cast<void*>(&points);

    //T = gsl_min_fminimizer_goldensection;
    T = gsl_min_fminimizer_brent;
    s = gsl_min_fminimizer_alloc (T);
    gsl_min_fminimizer_set (s, &F, m, a, b);

//    printf ("using %s method\n",
//            gsl_min_fminimizer_name (s));
//
//    printf ("%5s [%9s, %9s] %9s %10s %9s\n",
//            "iter", "lower", "upper", "min",
//            "err", "err(est)");
//
//    printf ("%5d [%.7f, %.7f] %.7f %+.7f %.7f\n",
//            iter, a, b,
//            m, m - m_expected, b - a);

    do
    {
        iter++;
        status = gsl_min_fminimizer_iterate (s);

        m = gsl_min_fminimizer_x_minimum (s);
        a = gsl_min_fminimizer_x_lower (s);
        b = gsl_min_fminimizer_x_upper (s);

        status
            = gsl_min_test_interval (a, b, 1e-3, 0.0);

//        if (status == GSL_SUCCESS)
//            printf ("Converged:\n");
//
//        printf ("%5d [%.7f, %.7f] "
//                "%.7f %+.7f %.7f\n",
//                iter, a, b,
//                m, m - m_expected, b - a);
    }
    while (status == GSL_CONTINUE && iter < max_iter);

    gsl_min_fminimizer_free (s);

    if (status != GSL_SUCCESS) return 0;

    return m;
}



class EllipseAreaMinimizer : public Toy
{
  public:
    void draw( cairo_t *cr, std::ostringstream *notify,
               int width, int height, bool save )
    {
        Point toggle_sp( 300, height - 50);
        toggles[0].bounds = Rect( toggle_sp, toggle_sp + Point(135,25) );

        goal_function_type* gf = &area_goal;
        if (!toggles[0].on) gf = &perimeter_goal;
        double coeff = minimizer(psh.pts, gf);

        try
        {
            e = fitting(psh.pts, coeff);
        }
        catch(LogicalError exc)
        {
            std::cerr << exc.what() << std::endl;
            Toy::draw(cr, notify, width, height, save);
            return;
        }

        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
        cairo_set_line_width (cr, 0.3);
        draw_elliptical_arc_with_cairo( cr,
                                        e.center(X), e.center(Y),
                                        e.ray(X), e.ray(Y),
                                        0, 2*M_PI,
                                        e.rot_angle() );
        if (toggles[0].on)
            *notify << "Area: " << e.ray(X)*e.ray(Y);
        else
            *notify << "Perimeter: " << 2* (e.ray(X) + e.ray(Y));
        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save);
    }

    void draw_elliptical_arc_with_cairo(
            cairo_t *cr,
            double _cx, double _cy,
            double _rx, double _ry,
            double _sa, double _ea,
            double _ra = 0
            ) const
    {
        double cos_rot_angle = std::cos(_ra);
        double sin_rot_angle = std::sin(_ra);
        cairo_matrix_t transform_matrix;
        cairo_matrix_init( &transform_matrix,
                           _rx * cos_rot_angle, _rx * sin_rot_angle,
                          -_ry * sin_rot_angle, _ry * cos_rot_angle,
                           _cx,                 _cy
                         );
        cairo_save(cr);
        cairo_transform(cr, &transform_matrix);
        cairo_arc(cr, 0, 0, 1, _sa, _ea);
        cairo_restore(cr);
    }

  public:
    EllipseAreaMinimizer()
    {
        gsl_set_error_handler(&no_minimum_error_handler);

        first_time = true;

        psh.pts.resize(4);
        psh.pts[0] = Point(450, 250);
        psh.pts[1] = Point(250, 100);
        psh.pts[2] = Point(250, 400);
        psh.pts[3] = Point(400, 320);

        handles.push_back(&psh);

        toggles.push_back(Toggle("Area/Perimeter", true));
        handles.push_back(&(toggles[0]));
    }

  public:
    Ellipse e;
    bool first_time;
    PointSetHandle psh;
    std::vector<Toggle> toggles;
};




int main(int argc, char **argv)
{
    init( argc, argv, new EllipseAreaMinimizer(), 600, 600 );
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
