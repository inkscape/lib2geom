/** @file
 * @brief Demonstration of elliptical arc functions
 *//*
 * Authors:
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * Copyright 2008-2015 Authors
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

#include <2geom/elliptical-arc.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>
#include <2geom/cairo-path-sink.h>

#include <vector>
#include <string>


using namespace Geom;



std::string angle_formatter(double angle)
{
    return default_formatter(decimal_round(deg_from_rad(angle),2));
}



class EllipticalArcToy: public Toy
{

    enum menu_item_t
    {
        SHOW_MENU = 0,
        TEST_BASIC,
        TEST_COMPARISON,
        TEST_PORTION,
        TEST_REVERSE,
        TEST_NEAREST_POINTS,
        TEST_FURHEST_POINTS,
        TEST_DERIVATIVE,
        TEST_ROOTS,
        TEST_BOUNDS,
        TEST_FITTING,
        TEST_TRANSFORM,
        TOTAL_ITEMS // this one must be the last item
    };

    enum handle_label_t
    {
        START_POINT = 0,
        END_POINT,
        POINT
    };

    enum toggle_label_t
    {
        LARGE_ARC_FLAG = 0,
        SWEEP_FLAG,
        X_Y_TOGGLE
    };

    enum slider_label_t
    {
        RX_SLIDER = 0,
        RY_SLIDER,
        ROT_ANGLE_SLIDER,
        T_SLIDER,
        FROM_SLIDER = T_SLIDER,
        TO_SLIDER,
        TM0_SLIDER = T_SLIDER,
        TM1_SLIDER,
        TM2_SLIDER,
        TM3_SLIDER
    };

    static const char* menu_items[TOTAL_ITEMS];
    static const char keys[TOTAL_ITEMS];

    virtual void first_time(int /*argc*/, char** /*argv*/)
    {
        draw_f = &EllipticalArcToy::draw_menu;
    }

    void init_common()
    {
        set_common_control_geometry = true;
        set_control_geometry = true;

        double start_angle = (10.0/6.0) * M_PI;
        double sweep_angle = (4.0/6.0) * M_PI;
        double end_angle = start_angle + sweep_angle;
        double rot_angle = (0.0/6.0) * M_PI;
        double rx = 200;
        double ry = 150;
        double cx = 300;
        double cy = 300;

        Point start_point( cx + rx * std::cos(start_angle),
                           cy + ry * std::sin(start_angle) );
        Point end_point( cx + rx * std::cos(end_angle),
                         cy + ry * std::sin(end_angle) );

        bool large_arc = false;
        bool sweep = true;


        initial_point.pos = start_point;
        final_point.pos = end_point;

        try
        {
            ea.set (initial_point.pos,
                    rx, ry, rot_angle,
                    large_arc, sweep,
                    final_point.pos);
        }
        catch( RangeError e )
        {
            no_solution = true;
            std::cerr << e.what() << std::endl;
        }

        sliders.clear();
        sliders.reserve(50);
        sliders.push_back(Slider(0, 500, 0, ea.ray(X), "ray X"));
        sliders.push_back(Slider(0, 500, 0, ea.ray(Y), "ray Y"));
        sliders.push_back(Slider(0, 2*M_PI, 0, ea.rotationAngle(), "rot angle"));
        sliders[ROT_ANGLE_SLIDER].formatter(&angle_formatter);

        toggles.clear();
        toggles.reserve(50);
        toggles.push_back(Toggle("Large Arc Flag", ea.largeArc()));
        toggles.push_back(Toggle("Sweep Flag", ea.sweep()));

        handles.clear();
        handles.push_back(&initial_point);
        handles.push_back(&final_point);
        handles.push_back(&(sliders[RX_SLIDER]));
        handles.push_back(&(sliders[RY_SLIDER]));
        handles.push_back(&(sliders[ROT_ANGLE_SLIDER]));
        handles.push_back(&(toggles[LARGE_ARC_FLAG]));
        handles.push_back(&(toggles[SWEEP_FLAG]));
    }

    virtual void draw_common( cairo_t *cr, std::ostringstream *notify,
                              int width, int height, bool /*save*/,
                              std::ostringstream *timer_stream=0)
    {
        if(timer_stream == 0)
            timer_stream = notify;
        init_common_ctrl_geom(cr, width, height, notify);

        no_solution = false;
        try
        {
            ea.set( initial_point.pos,
                    sliders[0].value(),
                    sliders[1].value(),
                    sliders[2].value(),
                    toggles[0].on,
                    toggles[1].on,
                    final_point.pos );
        }
        catch( RangeError e )
        {
            no_solution = true;
            std::cerr << e.what() << std::endl;
            return;
        }

        degenerate = ea.isDegenerate();

        point_overlap = false;
        if ( are_near(ea.initialPoint(), ea.finalPoint()) )
        {
            point_overlap = true;
        }

        // calculate the center of the two possible ellipse supporting the arc
        std::pair<Point,Point> centers
            = calculate_ellipse_centers( ea.initialPoint(), ea.finalPoint(),
                                         ea.ray(X), ea.ray(Y), ea.rotationAngle(),
                                         ea.largeArc(), ea.sweep() );


        // draw axes passing through the center of the ellipse supporting the arc
        cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
        cairo_set_line_width(cr, 0.5);
        draw_axes(cr);

        // draw the the 2 ellipse with rays rx, ry passing through
        // the 2 given point and with the x-axis inclined of rot_angle
        if ( !(are_near(ea.ray(X), 0) || are_near(ea.ray(Y), 0)) )
        {
            cairo_elliptiarc( cr,
                              centers.first[X], centers.first[Y],
                              ea.ray(X), ea.ray(Y),
                              0, 2*M_PI,
                              ea.rotationAngle() );
            cairo_stroke(cr);
            cairo_elliptiarc( cr,
                              centers.second[X], centers.second[Y],
                              ea.ray(X), ea.ray(Y),
                              0, 2*M_PI,
                              ea.rotationAngle() );
            cairo_stroke(cr);
        }

        // convert the elliptical arc to a sbasis path and draw it
        D2<SBasis> easb = ea.toSBasis();
        cairo_set_line_width(cr, 0.5);
        cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
        cairo_d2_sb(cr, easb);
        cairo_stroke(cr);

        // draw initial and final point labels
        draw_text(cr, ea.initialPoint() + Point(5, -15), "initial");
        draw_text(cr, ea.finalPoint() + Point(5, 0), "final");
        cairo_stroke(cr);

        // TODO reenable this
        //*notify << ea;
    }


    void draw_comparison(cairo_t *cr, std::ostringstream *notify,
                         int width, int height, bool save, std::ostringstream */*timer_stream*/)
    {
        draw_common(cr, notify, width, height, save);
        if ( no_solution || point_overlap ) return;

        // draw the arc with cairo in order to make a visual comparison
        cairo_set_line_width(cr, 1);
        cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);

        if (ea.isDegenerate())
        {
            cairo_move_to(cr, ea.initialPoint());
            cairo_line_to(cr, ea.finalPoint());
        }
        else
        {
            if ( ea.sweep() )
            {
                cairo_elliptiarc( cr,
                                  ea.center(X), ea.center(Y),
                                  ea.ray(X), ea.ray(Y),
                                  ea.initialAngle(), ea.finalAngle(),
                                  ea.rotationAngle() );
            }
            else
            {
                cairo_elliptiarc( cr,
                                  ea.center(X), ea.center(Y),
                                  ea.ray(X), ea.ray(Y),
                                  ea.finalAngle(), ea.initialAngle(),
                                  ea.rotationAngle() );
            }
        }
        cairo_stroke(cr);
    }


    void init_portion()
    {
        init_common();

        from_t = 0;
        to_t = 1;

        sliders.push_back( Slider(0, 1, 0, from_t, "from"));
        sliders.push_back( Slider(0, 1, 0, to_t, "to"));

        handles.push_back(&(sliders[FROM_SLIDER]));
        handles.push_back(&(sliders[TO_SLIDER]));
    }

    void draw_portion(cairo_t *cr, std::ostringstream *notify,
                      int width, int height, bool save, std::ostringstream */*timer_stream*/)
    {
        draw_common(cr, notify, width, height, save);
        init_portion_ctrl_geom(cr, notify, width, height);
        if ( no_solution || point_overlap ) return;

        from_t = sliders[FROM_SLIDER].value();
        to_t = sliders[TO_SLIDER].value();

        EllipticalArc* eapp
            = static_cast<EllipticalArc*>(ea.portion(from_t, to_t));
        EllipticalArc& eap = *eapp;

        cairo_set_line_width(cr, 0.8);
        cairo_set_source_rgba(cr, 0.0, 1.0, 1.0, 1.0);
        cairo_move_to(cr, eap.center(X), eap.center(Y));
        cairo_line_to(cr, eap.initialPoint()[X], eap.initialPoint()[Y]);
        cairo_move_to(cr, eap.center(X), eap.center(Y));
        cairo_line_to(cr, eap.finalPoint()[X], eap.finalPoint()[Y]);
        cairo_stroke(cr);
        D2<SBasis> sub_arc = eap.toSBasis();
        cairo_d2_sb(cr, sub_arc);
        cairo_stroke(cr);

        delete eapp;

    }


    void init_reverse()
    {
        init_common();
        time = 0;

        sliders.push_back( Slider(0, 1, 0, time, "t"));
        handles.push_back(&(sliders[T_SLIDER]));
    }

    void draw_reverse(cairo_t *cr, std::ostringstream *notify,
                      int width, int height, bool save, std::ostringstream */*timer_stream*/)
    {
        draw_common(cr, notify, width, height, save);
        init_reverse_ctrl_geom(cr, notify, width, height);
        if ( no_solution || point_overlap ) return;


        time = sliders[T_SLIDER].value();

        EllipticalArc* eapp = static_cast<EllipticalArc*>(ea.reverse());
        EllipticalArc& eap = *eapp;

        cairo_set_line_width(cr, 0.8);
        cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);

        cairo_move_to(cr, eap.center(X), eap.center(Y));
        cairo_line_to(cr, eap.valueAt(time,X), eap.valueAt(time,Y));
        draw_circ(cr, eap.pointAt(time));
        cairo_stroke(cr);
        cairo_set_source_rgba(cr, 0.0, 1.0, 1.0, 1.0);
        D2<SBasis> sub_arc = eap.toSBasis();
        cairo_d2_sb(cr, sub_arc);
        cairo_stroke(cr);

        delete eapp;

    }


    void init_np()
    {
        init_common();
        nph.pos = Point(10,10);
        handles.push_back(&nph);
    }

    void draw_np(cairo_t *cr, std::ostringstream *notify,
                 int width, int height, bool save, std::ostringstream */*timer_stream*/)
    {
        draw_common(cr, notify, width, height, save);
        if ( no_solution || point_overlap ) return;

        std::vector<double> times = ea.allNearestTimes( nph.pos );
        for ( unsigned int i = 0; i < times.size(); ++i )
        {
            cairo_move_to(cr,nph.pos);
            cairo_line_to( cr, ea.pointAt(times[i]) );
        }
        cairo_stroke(cr);
    }
    
    void init_fp()
    {
        init_common();
        fph.pos = Point(10,10);
        handles.push_back(&fph);
    }

    void draw_fp(cairo_t *cr, std::ostringstream *notify,
                 int width, int height, bool save, std::ostringstream */*timer_stream*/)
    {
        draw_common(cr, notify, width, height, save);
        if ( no_solution || point_overlap ) return;

        std::vector<double> times = ea.allFurthestTimes( fph.pos );
        for ( unsigned int i = 0; i < times.size(); ++i )
        {
            cairo_move_to(cr,fph.pos);
            cairo_line_to( cr, ea.pointAt(times[i]) );
        }
        cairo_stroke(cr);
    }

    void init_derivative()
    {
        init_common();
        time = 0;

        sliders.push_back( Slider(0, 1, 0, time, "t"));
        handles.push_back(&(sliders[T_SLIDER]));
    }

    void draw_derivative(cairo_t *cr, std::ostringstream *notify,
                         int width, int height, bool save, std::ostringstream */*timer_stream*/)
    {
        draw_common(cr, notify, width, height, save);
        init_reverse_ctrl_geom(cr, notify, width, height);
        if ( no_solution || point_overlap ) return;

        time = sliders[T_SLIDER].value();

        Curve* der = ea.derivative();
        Point p = ea.pointAt(time);
        Point v = der->pointAt(time) + p;
        delete der;
//      std::vector<Point> points = ea.pointAndDerivatives(time, 8);
//      Point p = points[0];
//      Point v = points[1] + p;
        cairo_move_to(cr, p);
        cairo_line_to(cr, v);
        cairo_stroke(cr);
    }


    void init_roots()
    {
        init_common();
        ph.pos = Point(10,10);
        toggles.push_back( Toggle("X/Y roots", true) );

        handles.push_back(&ph);
        handles.push_back(&(toggles[X_Y_TOGGLE]));
    }

    void draw_roots(cairo_t *cr, std::ostringstream *notify,
                    int width, int height, bool save, std::ostringstream */*timer_stream*/)
    {
        draw_common(cr, notify, width, height, save);
        init_roots_ctrl_geom(cr, notify, width, height);
        if ( no_solution || point_overlap ) return;

        Dim2 DIM = toggles[X_Y_TOGGLE].on ? X : Y;

        Point p1[2] = { Point(ph.pos[X], -1000),
                        Point(-1000, ph.pos[Y]) };
        Point p2[2] = { Point(ph.pos[X], 1000),
                        Point(1000, ph.pos[Y]) };
        cairo_set_line_width(cr, 0.5);
        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
        cairo_move_to(cr, p1[DIM]);
        cairo_line_to(cr, p2[DIM]);

        std::vector<double> times;
        try
        {
            times = ea.roots(ph.pos[DIM], DIM);
            *notify << "winding: " << ea.winding(ph.pos);
        }
        catch(Geom::Exception e)
        {
            std::cerr << e.what() << std::endl;
        }
        for ( unsigned int i = 0; i < times.size(); ++i )
        {
            draw_handle(cr, ea.pointAt(times[i]));
        }
        cairo_stroke(cr);
    }


    void init_bounds()
    {
        init_common();
    }

    void draw_bounds(cairo_t *cr, std::ostringstream *notify,
                     int width, int height, bool save, std::ostringstream */*timer_stream*/)
    {
        draw_common(cr, notify, width, height, save);
        if ( no_solution || point_overlap ) return;

//      const char* msg[] = { "xmax", "xmin", "ymax", "ymin" };

        Rect bb = ea.boundsFast();

//      for ( unsigned int i = 0; i < limits.size(); ++i )
//      {
//          std::cerr << "angle[" << i << "] = " <<  deg_from_rad(limits[i]) << std::endl;
//          Point extreme = ea.pointAtAngle(limits[i]);
//          draw_handle(cr, extreme );
//          draw_text(cr, extreme, msg[i]);
//      }
        cairo_rectangle( cr, bb.left(), bb.top(), bb.width(), bb.height() );
        cairo_stroke(cr);
    }


    void init_fitting()
    {
        init_common();
    }

    void draw_fitting(cairo_t * cr, std::ostringstream *notify,
                      int width, int height, bool save, std::ostringstream */*timer_stream*/)
    {
        draw_common(cr, notify, width, height, save);
        if ( no_solution || point_overlap ) return;

        D2<SBasis> easb = ea.toSBasis();
        try
        {
            EllipticalArc earc;
            if (!arc_from_sbasis(earc, easb, 0.1, 5)) return;

            D2<SBasis> arc = earc.toSBasis();
            arc[0] += Linear(50, 50);
            cairo_d2_sb(cr, arc);
            cairo_stroke(cr);
        }
        catch( RangeError e )
        {
            std::cerr << "conversion failure" << std::endl;
            std::cerr << e.what() << std::endl;
            return;
        }
    }

    void init_transform()
    {
        init_common();

        double max = 4;
        double min = -max;

        sliders.push_back( Slider(min, max, 0, 1, "TM0"));
        sliders.push_back( Slider(min, max, 0, 0, "TM1"));
        sliders.push_back( Slider(min, max, 0, 0, "TM2"));
        sliders.push_back( Slider(min, max, 0, 1, "TM3"));

        handles.push_back(&(sliders[TM0_SLIDER]));
        handles.push_back(&(sliders[TM1_SLIDER]));
        handles.push_back(&(sliders[TM2_SLIDER]));
        handles.push_back(&(sliders[TM3_SLIDER]));
    }

    void draw_transform(cairo_t *cr, std::ostringstream *notify,
                        int width, int height, bool save, std::ostringstream */*timer_stream*/)
    {
        draw_common(cr, notify, width, height, save);
        init_transform_ctrl_geom(cr, notify, width, height);
        if ( no_solution || point_overlap ) return;

        Affine TM(sliders[TM0_SLIDER].value(), sliders[TM1_SLIDER].value(),
                  sliders[TM2_SLIDER].value(), sliders[TM3_SLIDER].value(),
                  ea.center(X),                ea.center(Y));

        Affine tm( 1,            0,
                   0,            1,
                   -ea.center(X), -ea.center(Y) );


        EllipticalArc* tea = static_cast<EllipticalArc*>(ea.transformed(tm));
        EllipticalArc* eat = NULL;
        eat = static_cast<EllipticalArc*>(tea->transformed(TM));
        delete tea;
        if (eat == NULL)
        {
            std::cerr << "elliptiarc transformation failed" << std::endl;
            return;
        }

        CairoPathSink ps(cr);

        //D2<SBasis> sb = eat->toSBasis();
        cairo_set_line_width(cr, 0.8);
        cairo_set_source_rgba(cr, 0.8, 0.1, 0.1, 1.0);
        //cairo_d2_sb(cr, sb);
        ps.feed(*eat);
        cairo_stroke(cr);
        delete eat;
    }

    void init_common_ctrl_geom(cairo_t* /*cr*/, int /*width*/, int height, std::ostringstream* /*notify*/)
    {
        if ( set_common_control_geometry )
        {
            set_common_control_geometry = false;

            sliders[RX_SLIDER].geometry(Point(50, height-120), 250);
            sliders[RY_SLIDER].geometry(Point(50, height-85), 250);
            sliders[ROT_ANGLE_SLIDER].geometry(Point(50, height-50), 180);

            toggles[LARGE_ARC_FLAG].bounds = Rect(Point(400, height-120), Point(540, height-95));
            toggles[SWEEP_FLAG].bounds = Rect(Point(400, height-70), Point(520, height-45));
        }
    }

    void init_portion_ctrl_geom(cairo_t* /*cr*/, std::ostringstream* /*notify*/, int /*width*/, int height)
    {
        if ( set_control_geometry )
        {
            set_control_geometry = false;

            Point from_sp = Point(600, height - 120);
            Point to_sp = from_sp + Point(0,45);
            double from_to_len = 100;

            sliders[FROM_SLIDER].geometry(from_sp, from_to_len);
            sliders[TO_SLIDER].geometry(to_sp, from_to_len);
        }
    }

    void init_reverse_ctrl_geom(cairo_t* /*cr*/, std::ostringstream* /*notify*/, int /*width*/, int height)
    {
        if ( set_control_geometry )
        {
            set_control_geometry = false;

            Point t_sp = Point(600, height - 120);
            double t_len = 200;

            sliders[T_SLIDER].geometry(t_sp, t_len);
        }
    }

    void init_roots_ctrl_geom(cairo_t* /*cr*/, std::ostringstream* /*notify*/, int /*width*/, int height)
    {
        if ( set_control_geometry )
        {
            set_control_geometry = false;
            Point T(600, height - 120);
            toggles[X_Y_TOGGLE].bounds = Rect( T, T + Point(100,25) );
        }
    }

    void init_transform_ctrl_geom(cairo_t* /*cr*/, std::ostringstream* /*notify*/, int /*width*/, int height)
    {
        if ( set_control_geometry )
        {
            set_control_geometry = false;

            Point sp = Point(600, height - 140);
            Point op = Point(0, 30);
            double len = 200;

            sliders[TM0_SLIDER].geometry(sp, len);
            sliders[TM1_SLIDER].geometry(sp += op, len);
            sliders[TM2_SLIDER].geometry(sp += op, len);
            sliders[TM3_SLIDER].geometry(sp += op, len);
        }
    }

    void init_menu()
    {
        handles.clear();
        sliders.clear();
        toggles.clear();
    }

    void draw_menu( cairo_t * /*cr*/, std::ostringstream *notify,
                    int /*width*/, int /*height*/, bool /*save*/,
                    std::ostringstream */*timer_stream*/)
    {
        *notify << std::endl;
        for (int i = SHOW_MENU; i < TOTAL_ITEMS; ++i)
        {
            *notify << "   " << keys[i] << " -  " <<  menu_items[i] << std::endl;
        }
    }

    void key_hit(GdkEventKey *e)
    {
        char choice = std::toupper(e->keyval);
        switch ( choice )
        {
            case 'A':
                init_menu();
                draw_f = &EllipticalArcToy::draw_menu;
                break;
            case 'B':
                init_common();
                draw_f = &EllipticalArcToy::draw_common;
                break;
            case 'C':
                init_common();
                draw_f = &EllipticalArcToy::draw_comparison;
                break;
            case 'D':
                draw_f = &EllipticalArcToy::draw_menu;
                init_portion();
                draw_f = &EllipticalArcToy::draw_portion;
                break;
            case 'E':
                init_reverse();
                draw_f = &EllipticalArcToy::draw_reverse;
                break;
            case 'F':
                init_np();
                draw_f = &EllipticalArcToy::draw_np;
                break;
            case 'G':
                init_fp();
                draw_f = &EllipticalArcToy::draw_fp;
                break;
            case 'H':
                init_derivative();
                draw_f = &EllipticalArcToy::draw_derivative;
                break;
            case 'I':
                init_roots();
                draw_f = &EllipticalArcToy::draw_roots;
                break;
            case 'J':
                init_bounds();
                draw_f = &EllipticalArcToy::draw_bounds;
                break;
            case 'K':
                init_fitting();
                draw_f = &EllipticalArcToy::draw_fitting;
                break;
            case 'L':
                init_transform();
                draw_f = &EllipticalArcToy::draw_transform;
                break;
        }
        redraw();
    }


    void draw_axes(cairo_t* cr) const
    {
        Point D(std::cos(ea.rotationAngle()), std::sin(ea.rotationAngle()));
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

    void cairo_elliptiarc( cairo_t *cr,
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
            return result;
        }

        double sin_rot_angle = std::sin(m_rot_angle);
        double cos_rot_angle = std::cos(m_rot_angle);


        Affine m( cos_rot_angle, -sin_rot_angle,
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

    virtual void draw( cairo_t *cr, std::ostringstream *notify,
                       int width, int height, bool save, std::ostringstream *timer_stream)
    {
        (this->*draw_f)(cr, notify, width, height, save, timer_stream);
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

  public:
    EllipticalArcToy() {}

  private:
    typedef void (EllipticalArcToy::* draw_func_t) (cairo_t*, std::ostringstream*, int, int, bool, std::ostringstream*);
    draw_func_t draw_f;
    bool set_common_control_geometry;
    bool set_control_geometry;
    bool no_solution, point_overlap;
    bool degenerate;
    PointHandle initial_point, final_point;
    PointHandle nph, ph, fph;
    std::vector<Toggle> toggles;
    std::vector<Slider> sliders;
    EllipticalArc ea;

    double from_t;
    double to_t;
    double time;

};


const char* EllipticalArcToy::menu_items[] =
{
    "show this menu",
    "basic",
    "comparison",
    "portion, pointAt",
    "reverse, valueAt",
    "nearest points",
    "furthest points",
    "derivative",
    "roots",
    "bounding box",
    "fitting",
    "transformation"
};

const char EllipticalArcToy::keys[] =
{
     'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K'
};


int main(int argc, char **argv)
{
    init( argc, argv, new EllipticalArcToy(), 850, 780 );
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
