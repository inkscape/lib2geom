/*
 * Show off collinear normals between two Bezier curves.
 * The intersection points are found by using Bezier clipping.
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

#include <2geom/d2.h>
#include <2geom/basic-intersection.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/ray.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

using namespace Geom;



class CurveIntersect : public Toy
{

    void draw( cairo_t *cr, std::ostringstream *notify,
               int width, int height, bool save, std::ostringstream *timer_stream) override
    {
        m_width = width;
        m_height = height;
        m_length = (m_width > m_height) ? m_width : m_height;
        m_length *= 2;

        cairo_set_line_width (cr, 0.3);
        cairo_set_source_rgba (cr, 0.8, 0., 0, 1);
        D2<SBasis> A = pshA.asBezier();
        cairo_d2_sb(cr, A);
        cairo_stroke(cr);
        cairo_set_source_rgba (cr, 0.0, 0., 0, 1);
        D2<SBasis> B = pshB.asBezier();
        cairo_d2_sb(cr, B);
        cairo_stroke(cr);
        draw_text(cr, A.at0(), "A");
        draw_text(cr, B.at0(), "B");

        Timer tm;
        tm.ask_for_timeslice();
        tm.start();
        
        find_collinear_normal(xs, pshA.pts, pshB.pts, m_precision);
        Timer::Time als_time = tm.lap();
        *timer_stream << "find_collinear_normal " << als_time << std::endl;
        cairo_set_line_width (cr, 0.3);
        cairo_set_source_rgba (cr, 0.0, 0.0, 0.7, 1);
        for (auto & x : xs)
        {
            Point At = A(x.first);
            Point Bu = B(x.second);
            draw_axis(cr, At, Bu);
            draw_handle(cr, At);
            draw_handle(cr, Bu);

        }
        cairo_stroke(cr);
        
        double h_a_t = 0, h_b_t = 0;

        double h_dist = hausdorfl( A, B, m_precision, &h_a_t, &h_b_t);
        {
            Point At = A(h_a_t);
            Point Bu = B(h_b_t);
            cairo_move_to(cr, At);
            cairo_line_to(cr, Bu);
            draw_handle(cr, At);
            draw_handle(cr, Bu);
            cairo_save(cr);
            cairo_set_line_width (cr, 1);
            cairo_set_source_rgba (cr, 0.7, 0.0, 0.0, 1);
            cairo_stroke(cr);
            cairo_restore(cr);
        }
        /*h_dist = hausdorf( A, B, m_precision, &h_a_t, &h_b_t);
        {
            Point At = A(h_a_t);
            Point Bu = B(h_b_t);
            draw_axis(cr, At, Bu);
            draw_handle(cr, At);
            draw_handle(cr, Bu);
            cairo_save(cr);
            cairo_set_line_width (cr, 0.3);
            cairo_set_source_rgba (cr, 0.0, 0.7, 0.0, 1);
            cairo_stroke(cr);
            cairo_restore(cr);
            }*/
        *notify << "Hausdorf distance = " << h_dist 
                << "occurring at " << h_a_t 
                << " B=" << h_b_t << std::endl;

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    void draw_segment(cairo_t* cr, Point const& p1, Point const&  p2)
    {
        cairo_move_to(cr, p1);
        cairo_line_to(cr, p2);
    }

    void draw_segment(cairo_t* cr, Point const& p1, double angle, double length)
    {
        Point p2;
        p2[X] = length * std::cos(angle);
        p2[Y] = length * std::sin(angle);
        p2 += p1;
        draw_segment(cr, p1, p2);
    }

    void draw_ray(cairo_t* cr, Ray const& r)
    {
        double angle = r.angle();
        draw_segment(cr, r.origin(), angle, m_length);
    }

    void draw_axis(cairo_t* cr, Point const& p1, Point const&  p2)
    {
        double d = Geom::distance(p1, p2);
        d = d + d/4;
        Point q1 = Ray(p1, p2).pointAt(d);
        Point q2 = Ray(p2, p1).pointAt(d);
        draw_segment(cr, q1, q2);
    }

public:
    CurveIntersect(unsigned int _A_bez_ord, unsigned int _B_bez_ord)
        : A_bez_ord(_A_bez_ord), B_bez_ord(_B_bez_ord)
    {
        handles.push_back(&pshA);
        for (unsigned int i = 0; i <= A_bez_ord; ++i)
            pshA.push_back(Geom::Point(uniform()*400, uniform()*400)+Point(200,200));
        handles.push_back(&pshB);
        for (unsigned int i = 0; i <= B_bez_ord; ++i)
            pshB.push_back(Geom::Point(uniform()*400, uniform()*400)+Point(200,200));

        m_precision = 1e-6;
    }

private:
    unsigned int A_bez_ord, B_bez_ord;
    PointSetHandle pshA, pshB, pshC;
    std::vector< std::pair<double, double> > xs;
    double m_precision;
    double m_width, m_height, m_length;
};


int main(int argc, char **argv)
{
    unsigned int A_bez_ord = 4;
    unsigned int B_bez_ord = 6;
    if(argc > 1)
        sscanf(argv[1], "%d", &A_bez_ord);
    if(argc > 2)
        sscanf(argv[2], "%d", &B_bez_ord);


    init( argc, argv, new CurveIntersect(A_bez_ord, B_bez_ord), 800, 800);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :


