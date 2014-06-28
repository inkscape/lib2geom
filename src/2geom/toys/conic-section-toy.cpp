/*
 * Conic Section Toy
 *
 * Authors:
 *      Marco Cecchetti <mrcekets at gmail.com>
 *
 * Copyright 2009  authors
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


#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

//#define CLIP_WITH_CAIRO_SUPPORT
#ifdef CLIP_WITH_CAIRO_SUPPORT
    #include <2geom/conic_section_clipper_cr.h>
#endif


#include <2geom/conicsec.h>
#include <2geom/line.h>

//#include <iomanip>


using namespace Geom;


class ConicSectionToy : public Toy
{
    enum menu_item_t
    {
        SHOW_MENU = 0,
        TEST_VERTEX_FOCI,
        TEST_FITTING,
        TEST_ECCENTRICITY,
        TEST_DEGENERATE,
        TEST_ROOTS,
        TEST_NEAREST_POINT,
        TEST_BOUND,
        TEST_CLIP,
        TEST_TANGENT,
        TEST_DUAL,
        TOTAL_ITEMS // this one must be the last item
    };

    static const char* menu_items[TOTAL_ITEMS];
    static const char keys[TOTAL_ITEMS];

    virtual void first_time(int /*argc*/, char** /*argv*/)
    {
        draw_f = &ConicSectionToy::draw_menu;
    }


    void init_common()
    {
        init_vertex_foci();
        set_control_geometry = true;
    }

    void draw_common (cairo_t *cr, std::ostringstream *notify,
                         int width, int height, bool save,
                         std::ostringstream * timer_stream)
    {
        draw_vertex_foci(cr, notify, width, height, save, timer_stream);
    }


/*
 *  TEST VERTEX FOCI
 */
    void init_vertex_foci()
    {
        set_common_control_geometry = true;
        handles.clear();

        focus1.pos = Point(300, 300);
        focus2.pos = Point(500, 300);
        vertex.pos = Point(200, 300);

        parabola_toggle = Toggle("set/unset F2 to infinity", false);

        handles.push_back (&vertex);
        handles.push_back (&focus1);
        handles.push_back (&focus2);
        handles.push_back (&parabola_toggle);
    }

    void draw_vertex_foci (cairo_t *cr, std::ostringstream *notify,
                         int width, int height, bool /*save*/,
                         std::ostringstream * /*timer_stream*/)
    {
        init_vertex_foci_ctrl_geom(cr, notify, width, height);


        if (!parabola_toggle.on )
        {
            if (focus2.pos == Point(infinity(),infinity()))
                focus2.pos = Point(m_width/2, m_height/2);
            double df1f2 = distance (focus1.pos, focus2.pos);
            if (df1f2 > 20 )
            {
                Line axis(focus1.pos,focus2.pos);
                vertex.pos = projection (vertex.pos, axis);
            }
            else if (df1f2 > 1)
            {
                Line axis(focus1.pos,vertex.pos);
                focus2.pos = projection (focus2.pos, axis);
            }
            else
            {
                Line axis(focus1.pos,vertex.pos);
                focus2.pos = focus1.pos;
            }
        }
        else
        {
            focus2.pos = Point(infinity(),infinity());
        }

        cs.set (vertex.pos, focus1.pos, focus2.pos);
        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
        cairo_set_line_width (cr, 0.5);
        draw (cr, cs, m_window);
        cairo_stroke(cr);

        draw_label(cr, focus1, "F1");
        if (!parabola_toggle.on)  draw_label(cr, focus2, "F2");
        draw_label(cr, vertex, "V");
        cairo_stroke(cr);

        *notify << cs.categorise() << ": " << cs << std::endl;
    }

    void init_vertex_foci_ctrl_geom (cairo_t* /*cr*/,
                                     std::ostringstream* /*notify*/,
                                     int width, int /*height*/)
    {
        if ( set_common_control_geometry )
        {
            set_common_control_geometry = false;

            Point toggle_sp( width - 200, 10);
            parabola_toggle.bounds = Rect (toggle_sp, toggle_sp + Point(190,30));
        }
    }


/*
 *  TEST FITTING
 */
    void init_fitting()
    {
        set_common_control_geometry = true;
        handles.clear();

        psh.pts.resize(5);
        psh.pts[0] = Point(450, 250);
        psh.pts[1] = Point(250, 100);
        psh.pts[2] = Point(250, 400);
        psh.pts[3] = Point(400, 320);
        psh.pts[4] = Point(50, 250);

        fitting_slider.set (0, 5, 1, 0, "more handles");

        handles.push_back(&psh);
        handles.push_back(&fitting_slider);
    }

    void draw_fitting (cairo_t *cr, std::ostringstream *notify,
                         int width, int height, bool /*save*/,
                         std::ostringstream * /*timer_stream*/)
    {
        init_fitting_ctrl_geom(cr, notify, width, height);

        size_t n = (size_t)(fitting_slider.value()) + 5;
        if (n < psh.pts.size())
        {
            psh.pts.resize(n);
        }
        else if (n > psh.pts.size())
        {
            psh.push_back (std::fabs (width - 100) * uniform() + 50,
                           std::fabs (height - 100) * uniform() + 50);
        }

        cs.set (psh.pts);

        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
        cairo_set_line_width (cr, 0.5);
        draw (cr, cs, m_window);
        cairo_stroke(cr);
        *notify << cs.categorise() << ": " << cs << std::endl;
    }

    void init_fitting_ctrl_geom (cairo_t* /*cr*/,
                                 std::ostringstream* /*notify*/,
                                 int /*width*/, int height)
    {
        if ( set_common_control_geometry )
        {
            set_common_control_geometry = false;
            fitting_slider.geometry (Point(50, height - 50), 100);
        }
    }


/*
 *  TEST ECCENTRICITY
 */
    void init_eccentricity()
    {
        set_common_control_geometry = true;
        handles.clear();

        p1 = Point (100, 100);
        p2 = Point (100, 400);
        focus1 = Point (300, 250);

        eccentricity_slider.set (0, 3, 0, 1, "eccentricity");

        handles.push_back (&p1);
        handles.push_back (&p2);
        handles.push_back (&focus1);
        handles.push_back (&eccentricity_slider);
    }

    void draw_eccentricity (cairo_t *cr, std::ostringstream *notify,
                            int width, int height, bool /*save*/,
                            std::ostringstream * /*timer_stream*/)
    {
        init_eccentricity_ctrl_geom(cr, notify, width, height);

        Line directrix (p1.pos, p2.pos);

        cs.set (focus1.pos, directrix, eccentricity_slider.value());

        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
        cairo_set_line_width (cr, 0.5);
        draw (cr, cs, m_window);
        cairo_stroke(cr);

        draw_label (cr, focus1, "F");
        draw_line (cr, directrix, m_window);
        draw_label(cr, p1, "directrix");
        cairo_stroke(cr);

        *notify << cs.categorise() << ": " << cs << std::endl;
    }

    void init_eccentricity_ctrl_geom (cairo_t* /*cr*/,
                                      std::ostringstream* /*notify*/,
                                      int /*width*/, int height)
    {
        if ( set_common_control_geometry )
        {
            set_common_control_geometry = false;
            eccentricity_slider.geometry (Point (10, height - 50), 300);
        }
    }



/*
 *  TEST DEGENERATE
 */
    void init_degenerate()
    {
        set_common_control_geometry = true;
        handles.clear();

        psh.pts.resize(4);
        psh.pts[0] = Point(450, 250);
        psh.pts[1] = Point(250, 100);
        psh.pts[2] = Point(250, 400);
        psh.pts[3] = Point(400, 320);



        handles.push_back(&psh);
    }

    void draw_degenerate (cairo_t *cr, std::ostringstream *notify,
                          int width, int height, bool /*save*/,
                          std::ostringstream * /*timer_stream*/)
    {
        init_degenerate_ctrl_geom(cr, notify, width, height);

        Line l1 (psh.pts[0], psh.pts[1]);
        Line l2 (psh.pts[2], psh.pts[3]);
        cs.set (l1, l2);
        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
        cairo_set_line_width (cr, 0.5);
        draw (cr, cs, m_window);
        cairo_stroke(cr);

        *notify << cs.categorise() << ": " << cs << std::endl;
    }

    void init_degenerate_ctrl_geom (cairo_t* /*cr*/,
                                    std::ostringstream* /*notify*/,
                                    int /*width*/, int /*height*/)
    {
        if ( set_common_control_geometry )
        {
            set_common_control_geometry = false;
        }
    }


/*
 *  TEST ROOTS
 */
    void init_roots()
    {
        init_common();

        p1.pos = Point(180, 50);

        x_y_toggle = Toggle("X/Y roots", true);

        handles.push_back(&p1);
        handles.push_back(&x_y_toggle);
    }

    void draw_roots (cairo_t *cr, std::ostringstream *notify,
                         int width, int height, bool save,
                         std::ostringstream * timer_stream)
    {
        draw_common(cr, notify, width, height, save, timer_stream);
        init_roots_ctrl_geom(cr, notify, width, height);


        Dim2 DIM = x_y_toggle.on ? X : Y;
        Line l(p1.pos, DIM * (-M_PI/2) + M_PI/2);

        cairo_set_line_width(cr, 0.2);
        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
        draw_line(cr, l, m_window);
        cairo_stroke(cr);

        cairo_set_source_rgba(cr, 0.8, 0.0, 0.0, 1.0);
        std::vector<double> values;
        try
        {
            cs.roots(values, p1.pos[DIM], DIM);
        }
        catch(Geom::Exception e)
        {
            std::cerr << e.what() << std::endl;
        }
        for ( unsigned int i = 0; i < values.size(); ++i )
        {
            Point p(values[i], values[i]);
            p[DIM] = p1.pos[DIM];
            draw_handle(cr, p);
        }
        cairo_stroke(cr);

        *notify << "                 ";
        for ( unsigned int i = 0; i < values.size(); ++i )
        {
            *notify << "v" << i << " = " << values[i] << "  ";
        }
    }

    void init_roots_ctrl_geom (cairo_t* /*cr*/, std::ostringstream* /*notify*/,
                               int width, int height)
    {
        if ( set_control_geometry )
        {
            set_control_geometry = false;

            Point T(width - 120, height - 60);
            x_y_toggle.bounds = Rect( T, T + Point(100,25) );
        }
    }

/*
 *  TEST NEAREST POINT
 */

    void init_nearest_point()
    {
        init_common();
        p1.pos = Point(180, 50);
        handles.push_back(&p1);
    }

    void draw_nearest_point (cairo_t *cr, std::ostringstream *notify,
                             int width, int height, bool save,
                             std::ostringstream * timer_stream)
    {
        draw_common(cr, notify, width, height, save, timer_stream);

        Point P;
        try
        {
            P = cs.nearestTime (p1.pos);
        }
        catch (LogicalError e)
        {
            std::cerr << e.what() << std::endl;
        }


        cairo_set_source_rgba(cr, 0.8, 0.1, 0.1, 1.0);
        draw_line_seg(cr, p1.pos, P);
        cairo_stroke(cr);

        cairo_set_source_rgba(cr, 0.1, 0.1, 0.9, 1.0);
        draw_handle(cr, P);
        cairo_stroke(cr);
        cairo_set_source_rgba(cr, 0.1, 0.1, 0.1, 1.0);
        draw_label(cr, p1, "Q");
        draw_text(cr, P + Point(5, 5), "P");
        cairo_stroke(cr);
    }

/*
 *  TEST BOUND
 */
    void init_bound()
    {
        init_common();
        p1.pos = Point(50, 200);
        p2.pos = Point(50, 400);
        p3.pos = Point(50, 500);
        handles.push_back(&p1);
        handles.push_back(&p2);
        handles.push_back(&p3);
    }

    void draw_bound (cairo_t *cr, std::ostringstream *notify,
                         int width, int height, bool save,
                         std::ostringstream * timer_stream)
    {
        draw_common(cr, notify, width, height, save, timer_stream);

        try
        {
            p1.pos = cs.nearestTime (p1.pos);
            p2.pos = cs.nearestTime (p2.pos);
            p3.pos = cs.nearestTime (p3.pos);
        }
        catch (LogicalError e)
        {
            std::cerr << e.what() << std::endl;
        }

        Rect bound = cs.arc_bound (p1.pos, p2.pos, p3.pos);
        cairo_set_source_rgba(cr, 0.8, 0.1, 0.1, 1.0);
        cairo_set_line_width (cr, 0.5);
        cairo_rectangle (cr, bound);
        cairo_stroke(cr);

        cairo_set_source_rgba(cr, 0.1, 0.1, 0.1, 1.0);
        draw_label (cr, p1, "initial");
        draw_label (cr, p2, "inner");
        draw_label (cr, p3, "final");
        cairo_stroke(cr);

    }

/*
 *  TEST CLIP
 */
    void init_clip()
    {
        init_common();
    }

    void draw_clip (cairo_t *cr, std::ostringstream *notify,
                         int width, int height, bool save,
                         std::ostringstream * timer_stream)
    {
        draw_common(cr, notify, width, height, save, timer_stream);
        //init_clip_ctrl_geom(cr, notify, width, height);


        Rect R(Point (100, 100),Point (width-100, height-100));
        std::vector<RatQuad> rq;
#ifdef  CLIP_WITH_CAIRO_SUPPORT
        clipper_cr aclipper(cr, cs, R);
        aclipper.clip (rq);
#else
        clip (rq, cs, R);
#endif
        cairo_set_source_rgba(cr, 0.8, 0.1, 0.1, 1.0);
        cairo_set_line_width (cr, 0.5);
        cairo_rectangle (cr, Rect (Point (100, 100),Point (width-100, height-100)));
        for (size_t i = 0; i < rq.size(); ++i)
        {
            cairo_d2_sb (cr, rq[i].toCubic().toSBasis());
        }
        cairo_stroke(cr);
    }

/*
 *  TEST TANGENT
 */
    void init_tangent()
    {
        init_common();

        p1.pos = Point(180, 50);

        handles.push_back(&p1);
    }

    void draw_tangent (cairo_t *cr, std::ostringstream *notify,
                         int width, int height, bool save,
                         std::ostringstream * timer_stream)
    {
        draw_common(cr, notify, width, height, save, timer_stream);

        p1.pos = cs.nearestTime (p1.pos);
        Line l = cs.tangent(p1.pos);

        draw_label (cr, p1, "P");
        cairo_set_source_rgba(cr, 0.8, 0.0, 0.0, 1.0);
        draw_line(cr, l, m_window);
        cairo_stroke(cr);
    }

/*
 *  TEST DUAL
 */
    void init_dual()
    {
        init_common();
    }

    void draw_dual (cairo_t *cr, std::ostringstream *notify,
                         int width, int height, bool save,
                         std::ostringstream * timer_stream)
    {
        draw_common(cr, notify, width, height, save, timer_stream);

        cairo_set_source_rgba(cr, 0.8, 0.0, 0.0, 1.0);
        xAx dc = cs.dual();
        // we need some trick to make the dual visible in the window
        std::string dckind = dc.categorise();
        boost::optional<Point> T = dc.centre();
        if (T)  dc = dc.translate (-*T);
        dc = dc.scale (1e-5, 1e-5);
        dc = dc.translate (Point(width/2, height/2));
        draw (cr, dc, m_window);
        cairo_stroke(cr);
        *notify << "\n dual: " << dckind << ": " << dc;
    }


    void draw_segment (cairo_t* cr, Point const& p1, Point const&  p2)
    {
        cairo_move_to(cr, p1);
        cairo_line_to(cr, p2);
    }

    void draw (cairo_t* cr, xAx const& cs, const Rect& _window)
    {

        // offset
        //Point O(400, 300);
        Point O(0, 0);
        std::vector<double> r1, r2;

        size_t N = (size_t)(2 *_window.width());
        double dx = 0.5;//width / N;
        //std::cout << "dx = " << dx << std::endl;
        double x = _window.left() - O[X];
        for (size_t i = 0; i < N; ++i, x += dx)
        {
            if (r1.empty())
            {
                cs.roots(r1, x, X);
                if (r1.size() == 1)
                {
                    r1.push_back(r1.front());
                }
                if (i != 0 && r1.size() == 2)
                {
                    Point p1(x-dx, r1[0]);
                    Point p2(x-dx, r1[1]);
                    p1 += O; p2 += O;
                    if (_window.contains(p1) && _window.contains(p2))
                        draw_segment(cr, p1, p2);
                }
                continue;
            }
            cs.roots(r2, x, X);
            if (r2.empty())
            {
                Point p1(x-dx, r1[0]);
                Point p2(x-dx, r1[1]);
                p1 += O; p2 += O;
                if (_window.contains(p1) && _window.contains(p2))
                    draw_segment(cr, p1, p2);
                r1.clear();
                continue;
            }
            if (r2.size() == 1)
            {
                r2.push_back(r2.front());
            }

            Point p1(x-dx, r1[0]);
            Point p2(x, r2[0]);
            p1 += O; p2 += O;
            if (_window.contains(p1) && _window.contains(p2))
                draw_segment(cr, p1, p2);

            p1 = Point(x-dx, r1[1]) + O;
            p2 = Point(x, r2[1]) + O;
            if (_window.contains(p1) && _window.contains(p2))
                draw_segment(cr, p1, p2);

            using std::swap;
            swap(r1, r2);
        }
    }

    void draw_label(cairo_t* cr, PointHandle const& ph, const char* label)
    {
        draw_text(cr, ph.pos+op, label);
    }

//    void draw_label(cairo_t* cr, Line const& l, const char* label)
//    {
//        draw_text(cr, projection(Point(m_width/2-30, m_height/2-30), l)+op, label);
//    }

    void init_menu()
    {
        handles.clear();
    }

    void draw_menu( cairo_t * /*cr*/, std::ostringstream *notify,
                    int /*width*/, int /*height*/, bool /*save*/,
                    std::ostringstream */*timer_stream*/ )
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
                draw_f = &ConicSectionToy::draw_menu;
                break;
            case 'B':
                init_common();
                draw_f = &ConicSectionToy::draw_vertex_foci;
                break;
            case 'C':
                init_fitting();
                draw_f = &ConicSectionToy::draw_fitting;
                break;
            case 'D':
                init_eccentricity();
                draw_f = &ConicSectionToy::draw_eccentricity;
                break;
            case 'E':
                init_degenerate();
                draw_f = &ConicSectionToy::draw_degenerate;
                break;
            case 'F':
                init_roots();
                draw_f = &ConicSectionToy::draw_roots;
                break;
            case 'G':
                init_nearest_point();
                draw_f = &ConicSectionToy::draw_nearest_point;
                break;
            case 'H':
                init_bound();
                draw_f = &ConicSectionToy::draw_bound;
                break;
            case 'K':
                init_clip();
                draw_f = &ConicSectionToy::draw_clip;
                break;
            case 'J':
                init_tangent();
                draw_f = &ConicSectionToy::draw_tangent;
                break;
            case 'I':
                init_dual();
                draw_f = &ConicSectionToy::draw_dual;
                break;

        }
        redraw();
    }

    virtual void draw( cairo_t *cr, std::ostringstream *notify,
                       int width, int height, bool save,
                       std::ostringstream *timer_stream )
    {
        if (timer_stream == 0)  timer_stream = notify;
        m_width = width;
        m_height = height;
        m_length = (m_width > m_height) ? m_width : m_height;
        m_length *= 2;
        m_window = Rect (Point (0, 0), Point (m_width, m_height));
        (this->*draw_f)(cr, notify, width, height, save, timer_stream);
        Toy::draw(cr, notify, width, height, save, timer_stream);
    }

public:
    ConicSectionToy()
    {
        op = Point(5,5);
    }

private:
    typedef void (ConicSectionToy::* draw_func_t) (cairo_t*, std::ostringstream*, int, int, bool, std::ostringstream*);
    draw_func_t draw_f;
    bool set_common_control_geometry;
    bool set_control_geometry;
    Point op;
    double m_width, m_height, m_length;
    Rect m_window;

    xAx cs;
    PointHandle vertex, focus1, focus2;
    Toggle parabola_toggle;

    PointSetHandle psh;
    Slider fitting_slider;

    PointHandle p1, p2, p3;
    Toggle x_y_toggle;

    Slider eccentricity_slider;
};


const char* ConicSectionToy::menu_items[] =
{
    "show this menu",
    "vertex and foci",
    "fitting",
    "eccentricity",
    "degenerate",
    "roots",
    "nearest point",
    "bound",
    "clip",
    "tangent",
    "dual"
};

const char ConicSectionToy::keys[] =
{
     'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'K', 'J', 'I'
};


int main(int argc, char **argv)
{
    //std::cout.precision(20);
    init( argc, argv, new ConicSectionToy(), 800, 600 );
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
