/*
 * SVG Elliptical Arc Class
 *
 * Copyright 2008  Marco Cecchetti <mrcekets at gmail.com>
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



#include "path.h"
#include "svg-elliptical-arc.h"
#include "path-cairo.h"
#include "toy-framework-2.h"
#include <vector>


using namespace Geom;



std::string angle_formatter(double angle)
{
    return default_formatter(decimal_round(rad_to_deg(angle),2));
}



class SVGEllipticToy: public Toy
{
  public:
    virtual void draw(
                cairo_t *cr,
                std::ostringstream *notify,
                int width, int height,
                bool save
            )
    {
        draw_controls(cr, width, height, notify);

        // draw axes passing through the center of the ellipse supporting the arc
        cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
        cairo_set_line_width(cr, 0.2);
        draw_axes(cr);

        ea.set( initial_point.pos,
                sliders[0].value(),
                sliders[1].value(),
                sliders[2].value(),
                toggles[0].on,
                toggles[1].on,
                final_point.pos );

        std::pair<Point,Point> centers
            = calculate_ellipse_centers( ea.initialPoint(), ea.finalPoint(),
                                         ea.ray(X), ea.ray(Y), ea.rotation_angle(),
                                         ea.large_arc_flag(), ea.sweep_flag() );
        // draw the the 2 ellipse with rays rx, ry passing through
        // the 2 given point and with the x-axis inclined of rot_angle
        if ( !(are_near(ea.ray(X), 0) || are_near(ea.ray(Y), 0)) )
        {
            draw_elliptical_arc_with_cairo( cr, centers.first[X], centers.first[Y],
                                            ea.ray(X), ea.ray(Y), 0, 2*M_PI, ea.rotation_angle() );
            cairo_stroke(cr);
            draw_elliptical_arc_with_cairo( cr, centers.second[X], centers.second[Y],
                                            ea.ray(X), ea.ray(Y), 0, 2*M_PI, ea.rotation_angle() );
            cairo_stroke(cr);
        }

        D2<SBasis> easb = ea.toSBasis();
        cairo_set_line_width(cr, 0.2);
        cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
        cairo_md_sb(cr, easb);
        cairo_stroke(cr);

        draw_text(cr, ea.initialPoint() + Point(5, -15), "initial");
        draw_text(cr, ea.finalPoint() + Point(5, 0), "final");
        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save);
    }


  public:
    SVGEllipticToy()
    {
        initial_point.pos = Point(500, 300);
        final_point.pos = Point(300, 450);
        ea.set(initial_point.pos, 200, 150, 0, 0, 0, final_point.pos);

        toggles.push_back(Toggle("Large Arc Flag", ea.large_arc_flag()));
        toggles.push_back(Toggle("Sweep Flag", ea.sweep_flag()));

        sliders.push_back(Slider(0, 500, 0, ea.ray(X), "ray X"));
        sliders.push_back(Slider(0, 500, 0, ea.ray(Y), "ray Y"));
        sliders.push_back(Slider(0, 2*M_PI, 0, ea.rotation_angle(), "rot angle"));
        sliders[2].formatter(&angle_formatter);

        handles.push_back(&initial_point);
        handles.push_back(&final_point);
        handles.push_back(&(toggles[0]));
        handles.push_back(&(toggles[1]));
        handles.push_back(&(sliders[0]));
        handles.push_back(&(sliders[1]));
        handles.push_back(&(sliders[2]));
    }

  private:

    void draw_axes(cairo_t* cr) const
    {
        Point D(std::cos(ea.rotation_angle()), std::sin(ea.rotation_angle()));
        Point Dx = (ea.ray(X) + 20) * D;
        Point Dy = (ea.ray(Y) + 20) * D.cw();
        Point C(ea.center(X),ea.center(Y));
        Point LP = C - Dx;
        Point RP = C + Dx;
        Point UP = C - Dy;
        Point DP = C + Dy;

        cairo_move_to(cr, LP[X], LP[Y]);
        cairo_line_to(cr, RP[X], RP[Y]);
        cairo_move_to(cr, UP[X], UP[Y]);
        cairo_line_to(cr, DP[X], DP[Y]);
        cairo_move_to(cr, 0, 0);
        cairo_stroke(cr);
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

    void draw_controls(cairo_t* cr, int width, int height, std::ostringstream *notify)
    {
        sliders[0].geometry(Point(50, height-100), 250);
        sliders[1].geometry(Point(50, height-70), 250);
        sliders[2].geometry(Point(50, height-40), 180);

        toggles[0].bounds = Rect(Point(400, height-100), Point(540, height-75));
        toggles[1].bounds = Rect(Point(400, height-50), Point(520, height-25));
    }

    std::pair<Point,Point>
    calculate_ellipse_centers( Point _initial_point, Point _final_point,
                               double m_rx,           double m_ry,
                               double m_rot_angle,
                               bool m_large_arc, bool m_sweep
                             )
    {
        std::pair<Point,Point> result;
        if ( _initial_point == _final_point )
        {
            result.first = result.second = _initial_point;
            return result;
        }

        m_rx = std::fabs(m_rx);
        m_ry = std::fabs(m_ry);

        Point d = _initial_point - _final_point;

        if ( are_near(m_rx, 0) || are_near(m_ry, 0) )
        {
            result.first = result.second
                = middle_point(_initial_point, _final_point);
        }

        double sin_rot_angle = std::sin(m_rot_angle);
        double cos_rot_angle = std::cos(m_rot_angle);


        Matrix m( cos_rot_angle, -sin_rot_angle,
                  sin_rot_angle, cos_rot_angle,
                  0,             0              );

        Point p = (d / 2) * m;
        double rx2 = m_rx * m_rx;
        double ry2 = m_ry * m_ry;
        double rxpy = m_rx * p[Y];
        double rypx = m_ry * p[X];
        double rx2py2 = rxpy * rxpy;
        double ry2px2 = rypx * rypx;
        double num = rx2 * ry2;
        double den = rx2py2 + ry2px2;
        assert(den != 0);
        double rad = num / den;
        Point c(0,0);
        if (rad > 1)
        {
            rad -= 1;
            rad = std::sqrt(rad);

            if (m_large_arc == m_sweep) rad = -rad;
            c = rad * Point(rxpy / m_ry, -rypx / m_rx);

            m[1] = -m[1];
            m[2] = -m[2];

            c = c * m;
        }

        d = middle_point(_initial_point, _final_point);

        result.first = c + d;
        result.second = -c + d;
        return result;

    }
  private:
      PointHandle initial_point, final_point;
      std::vector<Toggle> toggles;
      std::vector<Slider> sliders;
      SVGEllipticalArc ea;

};


int main(int argc, char **argv)
{
    init( argc, argv, new SVGEllipticToy(), 850, 780 );
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
