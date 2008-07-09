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

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <stdio.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_min.h>
     
class EllipseFitting;

using namespace Geom;

double fn1 (double x, void * params);
     
int
mins (EllipseFitting*ef)
{
    int status;
    int iter = 0, max_iter = 100;
    const gsl_min_fminimizer_type *T;
    gsl_min_fminimizer *s;
    double m = 2.0, m_expected = M_PI;
    double a = 0.0, b = 6.0;
    gsl_function F;
     
    F.function = &fn1;
    F.params = (void*)ef;
     
    T = gsl_min_fminimizer_brent;
    s = gsl_min_fminimizer_alloc (T);
    gsl_min_fminimizer_set (s, &F, m, -1000, 1000);
     
    printf ("using %s method\n",
            gsl_min_fminimizer_name (s));
     
    printf ("%5s [%9s, %9s] %9s %10s %9s\n",
            "iter", "lower", "upper", "min",
            "err", "err(est)");
     
    printf ("%5d [%.7f, %.7f] %.7f %+.7f %.7f\n",
            iter, a, b,
            m, m - m_expected, b - a);
     
    do
    {
        iter++;
        status = gsl_min_fminimizer_iterate (s);
     
        m = gsl_min_fminimizer_x_minimum (s);
        a = gsl_min_fminimizer_x_lower (s);
        b = gsl_min_fminimizer_x_upper (s);
     
        status 
            = gsl_min_test_interval (a, b, 0.001, 0.0);
     
        if (status == GSL_SUCCESS)
            printf ("Converged:\n");
     
        printf ("%5d [%.7f, %.7f] "
                "%.7f %+.7f %.7f\n",
                iter, a, b,
                m, m - m_expected, b - a);
    }
    while (status == GSL_CONTINUE && iter < max_iter);
     
    gsl_min_fminimizer_free (s);
     
    return m;
}



class EllipseFitting : public Toy
{
  public:
    void draw( cairo_t *cr, std::ostringstream *notify,
               int width, int height, bool save )
    {
        if (first_time)
        {
            first_time = false;
            Point toggle_sp( 300, height - 50);
            toggles[0].bounds = Rect( toggle_sp, toggle_sp + Point(120,25) );
            sliders[0].geometry(Point(50, height - 50), 100);
        }

        size_t n = (size_t)(sliders[0].value()) + 5;
        if (n < psh.pts.size())
        {
            psh.pts.resize(n);
        }
        else if (n > psh.pts.size())
        {
            psh.push_back(400*uniform()+50, 300*uniform()+50);
        }
        
        double x = mins(this);
        
        psh.pts[4][X] = x;
        e.set(psh.pts);
        if(0) {
            try
            {
                e.set(psh.pts);
            }
            catch(LogicalError exc)
            {
                std::cerr << exc.what() << std::endl;
                Toy::draw(cr, notify, width, height, save);
                return;
            }
        }
        if (toggles[0].on)
        {
            try
            {
                ea = e.arc(psh.pts[0], psh.pts[2], psh.pts[4]);
            }
            catch(RangeError exc)
            {
                std::cerr << exc.what() << std::endl;
                Toy::draw(cr, notify, width, height, save);
                return;
            }
        }

        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
        cairo_set_line_width (cr, 0.3);
        if (!toggles[0].on)
        {
            draw_elliptical_arc_with_cairo( cr,
                                            e.center(X), e.center(Y),
                                            e.ray(X), e.ray(Y),
                                            0, 2*M_PI,
                                            e.rot_angle() );
            *notify << "Area:" << e.ray(X)*e.ray(Y);
        }
        else
        {
            draw_text(cr, psh.pts[0], "initial");
            draw_text(cr, psh.pts[2], "inner");
            draw_text(cr, psh.pts[4], "final");

            D2<SBasis> easb = ea.toSBasis();
            cairo_md_sb(cr, easb);
        }
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
    EllipseFitting()
    {
        first_time = true;

        psh.pts.resize(5);
        psh.pts[0] = Point(450, 250);
        psh.pts[1] = Point(250, 100);
        psh.pts[2] = Point(250, 400);
        psh.pts[3] = Point(400, 320);
        psh.pts[4] = Point(50, 250);


        toggles.push_back(Toggle(" arc / ellipse ", false));
        sliders.push_back(Slider(0, 5, 1, 0, "more handles"));

        handles.push_back(&psh);
        handles.push_back(&(toggles[0]));
        handles.push_back(&(sliders[0]));
    }

  public:
    Ellipse e;
    SVGEllipticalArc ea;
    bool first_time;
    PointSetHandle psh;
    std::vector<Toggle> toggles;
    std::vector<Slider> sliders;
};



int main(int argc, char **argv)
{
    init( argc, argv, new EllipseFitting(), 600, 600 );
    return 0;
}

double fn1 (double x, void * params)
{
    EllipseFitting* ef = (EllipseFitting*)params;
    Ellipse e;
    ef->psh.pts[4][X] = x;
    try
    {
        e.set(ef->psh.pts);
    }
    catch(LogicalError exc)
    {
        return 1e18;
    }
    return e.ray(X)*e.ray(Y);
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
