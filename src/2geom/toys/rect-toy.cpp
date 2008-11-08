/*
 * Line Toy
 *
 * Copyright 2008  njh <>
 * multitoy approach
 * Copyright 2008  Marco <>
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


#include <2geom/line.h>
#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/transforms.h>

#include <2geom/angle.h>

#include <vector>
#include <string>

using namespace Geom;



std::string angle_formatter(double angle)
{
    return default_formatter(decimal_round(rad_to_deg(angle),2));
}

void cairo_rectangle(cairo_t *cr, Rect &r) {
  cairo_rectangle(cr, r[0][0], r[1][0], r[0].extent(), r[1].extent());
}

class LineToy : public Toy
{
    enum menu_item_t
    {
        SHOW_MENU = 0,
        TEST_CREATE,
        TEST_PROJECTION,
        TEST_ORTHO,
        TEST_DISTANCE,
        TEST_POSITION,
        TEST_SEG_BISEC,
        TEST_ANGLE_BISEC,
        TEST_COLLINEAR,
        TEST_INTERSECTIONS,
        TOTAL_ITEMS // this one must be the last item
    };

    enum handle_label_t
    {
    };

    enum toggle_label_t
    {
    };

    enum slider_label_t
    {
        ANGLE_SLIDER = 0,
    };

    static const char* menu_items[TOTAL_ITEMS];
    static const char keys[TOTAL_ITEMS];

    virtual void first_time(int /*argc*/, char** /*argv*/)
    {
        draw_f = &LineToy::draw_menu;
    }

    void init_common()
    {
        set_common_control_geometry = true;
        set_control_geometry = true;

        sliders.clear();
        toggles.clear();
        handles.clear();
    }


    virtual void draw_common( cairo_t *cr, std::ostringstream *notify,
                              int width, int height, bool /*save*/ )
    {
        init_common_ctrl_geom(cr, width, height, notify);
    }


    void init_create()
    {
        init_common();

        p1.pos = Point(400, 50);
        p2.pos = Point(450, 450);

        sliders.push_back(Slider(0, 2*M_PI, 0, 0, "angle"));
        sliders[ANGLE_SLIDER].formatter(&angle_formatter);

        handles.push_back(&p1);
        handles.push_back(&p2);
        handles.push_back(&(sliders[ANGLE_SLIDER]));
    }

    void draw_create(cairo_t *cr, std::ostringstream *notify,
                      int width, int height, bool save)
    {
        draw_common(cr, notify, width, height, save);
        init_create_ctrl_geom(cr, notify, width, height);

        Rect r1(p1.pos, p2.pos);

        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
        cairo_set_line_width(cr, 0.3);
        cairo_rectangle(cr, r1);

	Matrix  rot = Translate(-r1.midpoint())*Rotate(sliders[ANGLE_SLIDER].value())*Translate(r1.midpoint());
        cairo_rectangle(cr, r1*rot);
	cairo_move_to(cr, r1.corner(3)*rot);
	for(int i = 0; i < 4; i++) {
	  cairo_line_to(cr, r1.corner(i)*rot);
	}
	
        cairo_stroke(cr);

        draw_label(cr, p1, "P1");
        draw_label(cr, p2, "P2");
    }



    void init_intersections()
    {
        init_common();
        p1.pos = Point(400, 50);
        p2.pos = Point(450, 450);
        p3.pos = Point(100, 250);
        p4.pos = Point(200, 450);
        p5.pos = Point(50, 150);
        p6.pos = Point(480, 60);

        handles.push_back(&p1);
        handles.push_back(&p2);
        handles.push_back(&p3);
        handles.push_back(&p4);
        handles.push_back(&p5);
        handles.push_back(&p6);
    }

    void draw_intersections(cairo_t *cr, std::ostringstream *notify,
                            int width, int height, bool save)
    {
        draw_common(cr, notify, width, height, save);

        Rect r1(p1.pos, p2.pos);
        Rect r2(p3.pos, p4.pos);
        Line l1(p5.pos, p6.pos);

        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
        cairo_set_line_width(cr, 0.3);
        cairo_rectangle(cr, r1);
        cairo_stroke(cr);

        cairo_set_source_rgba(cr, 0.0, 0.3, 0.0, 1.0);
        cairo_rectangle(cr, r2);
        cairo_stroke(cr);

        OptRect r1xr2 = intersect(r1, r2);
        if(r1xr2) {
            cairo_set_source_rgba(cr, 1.0, 0.7, 0.7, 1.0);
            cairo_rectangle(cr, *r1xr2);
            cairo_fill(cr);
        }

        cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
        cairo_set_line_width(cr, 0.3);
        draw_line(cr, l1);
        cairo_stroke(cr);


        boost::optional<LineSegment> ls = rect_line_intersect(r1, LineSegment(p5.pos, p6.pos));
        *notify << "intersects: " << ((ls)?"true":"false") << std::endl;
        if(ls) {
            draw_handle(cr, (*ls)[0]);
            draw_handle(cr, (*ls)[1]);
        }


    }

    void init_common_ctrl_geom(cairo_t* /*cr*/, int /*width*/, int /*height*/, std::ostringstream* /*notify*/)
    {
        if ( set_common_control_geometry )
        {
            set_common_control_geometry = false;
        }
    }

    void init_create_ctrl_geom(cairo_t* /*cr*/, std::ostringstream* /*notify*/, int /*width*/, int height)
    {
        if ( set_control_geometry )
        {
            set_control_geometry = false;

            sliders[ANGLE_SLIDER].geometry(Point(50, height - 50), 180);
        }
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

    void draw_segment(cairo_t* cr, LineSegment const& ls)
    {
        draw_segment(cr, ls[0], ls[1]);
    }

    void draw_ray(cairo_t* cr, Ray const& r)
    {
        double angle = r.angle();
        draw_segment(cr, r.origin(), angle, m_length);
    }

    void draw_line(cairo_t* cr, Line const& l)
    {
        double angle = l.angle();
        draw_segment(cr, l.origin(), angle, m_length);
        draw_segment(cr, l.origin(), angle, -m_length);
    }

    void draw_label(cairo_t* cr, PointHandle const& ph, const char* label)
    {
        draw_text(cr, ph.pos+op, label);
    }

    void draw_label(cairo_t* cr, Line const& l, const char* label)
    {
        draw_text(cr, projection(Point(m_width/2-30, m_height/2-30), l)+op, label);
    }

    void draw_label(cairo_t* cr, LineSegment const& ls, const char* label)
    {
        draw_text(cr, middle_point(ls[0], ls[1])+op, label);
    }

    void draw_label(cairo_t* cr, Ray const& r, const char* label)
    {
        Point prj = r.pointAt(r.nearestPoint(Point(m_width/2-30, m_height/2-30)));
        if (L2(r.origin() - prj) < 100)
        {
            prj = r.origin() + 100*r.versor();
        }
        draw_text(cr, prj+op, label);
    }

    void init_menu()
    {
        handles.clear();
        sliders.clear();
        toggles.clear();
    }

    void draw_menu( cairo_t * /*cr*/, std::ostringstream *notify,
                    int /*width*/, int /*height*/, bool /*save*/ )
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
                draw_f = &LineToy::draw_menu;
                break;
            case 'B':
                init_create();
                draw_f = &LineToy::draw_create;
                break;
            case 'C':
                init_intersections();
                draw_f = &LineToy::draw_intersections;
                break;
        }
        redraw();
    }

    virtual void draw( cairo_t *cr, std::ostringstream *notify,
                       int width, int height, bool save )
    {
        m_width = width;
        m_height = height;
        m_length = (m_width > m_height) ? m_width : m_height;
        m_length *= 2;
        (this->*draw_f)(cr, notify, width, height, save);
        Toy::draw(cr, notify, width, height, save);
    }

  public:
    LineToy()
    {
        op = Point(5,5);
    }

  private:
    typedef void (LineToy::* draw_func_t) (cairo_t*, std::ostringstream*, int, int, bool);
    draw_func_t draw_f;
    bool set_common_control_geometry;
    bool set_control_geometry;
    PointHandle p1, p2, p3, p4, p5, p6, O;
    std::vector<Toggle> toggles;
    std::vector<Slider> sliders;
    Point op;
    double m_width, m_height, m_length;

}; // end class LineToy


const char* LineToy::menu_items[] =
{
    "show this menu",
    "rect generation",
    "intersection with a line"
};

const char LineToy::keys[] =
{
     'A', 'B', 'C'
};



int main(int argc, char **argv)
{
    init( argc, argv, new LineToy());
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
