#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-geometric.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <2geom/angle.h>

using std::vector;
using namespace Geom;
using namespace std;

// Author: Johan Engelen, 2009
//
// Shows how to find the locations on a path where the derivative is parallel to a certain vector.
//-----------------------------------------------


std::string angle_formatter(double angle)
{
    return default_formatter(decimal_round(rad_to_deg(angle),2));
}



class FindDerivatives : public Toy
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
        TEST_COEFFICIENTS,
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
        END_SHARED_SLIDERS = 0,
        ANGLE_SLIDER = END_SHARED_SLIDERS,
        A_COEFF_SLIDER = END_SHARED_SLIDERS,
        B_COEFF_SLIDER,
        C_COEFF_SLIDER
    };

    static const char* menu_items[TOTAL_ITEMS];
    static const char keys[TOTAL_ITEMS];

    PointSetHandle curve_handle;
    PointHandle sample_point;

    virtual void first_time(int /*argc*/, char** /*argv*/)
    {
        draw_f = &FindDerivatives::draw_menu;
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
        O.pos = Point(50, 400);

        sliders.push_back(Slider(0, 2*M_PI, 0, 0, "angle"));
        sliders[ANGLE_SLIDER].formatter(&angle_formatter);

        handles.push_back(&p1);
        handles.push_back(&p2);
        handles.push_back(&O);
        handles.push_back(&(sliders[ANGLE_SLIDER]));
    }

    void draw_create(cairo_t *cr, std::ostringstream *notify,
                      int width, int height, bool save, std::ostringstream */*timer_stream*/)
    {
        draw_common(cr, notify, width, height, save);
        init_create_ctrl_geom(cr, notify, width, height);

        Line l1(p1.pos, p2.pos);
        Line l2(O.pos, sliders[ANGLE_SLIDER].value());

        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
        cairo_set_line_width(cr, 0.3);
        draw_line(cr, l1);
        draw_line(cr, l2);
        cairo_stroke(cr);

        draw_label(cr, p1, "P1");
        draw_label(cr, p2, "P2");
        draw_label(cr, O, "O");
        draw_label(cr, l1, "L(P1,P2)");
        draw_label(cr, l2, "L(O,angle)");
    }


    void init_projection()
    {
        init_common();
        p1.pos = Point(400, 50);
        p2.pos = Point(450, 450);
        p3.pos = Point(100, 250);
        p4.pos = Point(200, 450);
        O.pos = Point(50, 150);

        handles.push_back(&p1);
        handles.push_back(&p2);
        handles.push_back(&p3);
        handles.push_back(&p4);
        handles.push_back(&O);
    }

    void draw_projection(cairo_t *cr, std::ostringstream *notify,
                      int width, int height, bool save, std::ostringstream */*timer_stream*/)
    {
        draw_common(cr, notify, width, height, save);

        Line l1(p1.pos, p2.pos);
        LineSegment ls(p3.pos, p4.pos);

        Point np = projection(O.pos, l1);
        LineSegment lsp = projection(ls, l1);

        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
        cairo_set_line_width(cr, 0.2);
        draw_line(cr, l1);
        draw_segment(cr, ls);
        cairo_stroke(cr);

        cairo_set_line_width(cr, 0.3);
        cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
        draw_segment(cr, lsp);
        draw_handle(cr, lsp[0]);
        draw_handle(cr, lsp[1]);
        cairo_stroke(cr);

        cairo_set_source_rgba(cr, 0.8, 0.0, 0.0, 1.0);
        draw_circ(cr, np);
        cairo_stroke(cr);

        cairo_set_source_rgba(cr, 0.5, 0.5, 0.5, 1.0);
        draw_label(cr, p1, "P1");
        draw_label(cr, p2, "P2");
        draw_label(cr, ls, "S");
        draw_label(cr, lsp, "prj(S)");
        draw_label(cr, O, "P");
        draw_text(cr, np, "prj(P)");

        cairo_stroke(cr);
    }

    void init_derivative() {
        init_common();
        handles.push_back(&curve_handle);
        handles.push_back(&sample_point);
        for(unsigned i = 0; i < 4; i++)
            curve_handle.push_back(150+uniform()*300,150+uniform()*300);
        sample_point.pos = Geom::Point(250,300);
    }

    void draw_derivative(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {

        D2<SBasis> B = curve_handle.asBezier();

        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
        cairo_d2_sb(cr, B);
        cairo_stroke(cr);

        Point vector = sample_point.pos - Geom::Point(400,400);
        cairo_move_to(cr, Geom::Point(400,400));
        cairo_line_to(cr, sample_point.pos);
        cairo_set_source_rgba (cr, 0., 0., 0.5, 0.8);
        cairo_stroke(cr);

        // How to find location of points with certain derivative along a path:
        D2<SBasis> deriv = derivative(B);
        SBasis dotp = dot(deriv, rot90(vector));
        std::vector<double> sol = roots(dotp);
        for (unsigned i = 0; i < sol.size(); ++i) {
            draw_handle(cr, B.valueAt(sol[i]));         // the solutions are in vector 'sol'
        }

        cairo_set_source_rgba (cr, 0.5, 0.2, 0., 0.8);
        cairo_stroke(cr);
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    void init_find_tangents() {
        init_common();
        handles.push_back(&curve_handle);
        handles.push_back(&sample_point);

        toggles.push_back(Toggle(" tangent / normal ", false));
        handles.push_back(&(toggles[0]));
        for(unsigned i = 0; i < 4; i++)
            curve_handle.push_back(150+uniform()*300,150+uniform()*300);
        sample_point.pos = Geom::Point(250,300);
        Point toggle_sp( 30, 30);
        toggles[0].bounds = Rect( toggle_sp, toggle_sp + Point(200,25) );
    }

    void draw_find_tangents(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {

        D2<SBasis> B = curve_handle.asBezier();

        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
        cairo_d2_sb(cr, B);
        cairo_stroke(cr);

        std::vector<double> sol = toggles[0].on ?
            find_tangents(sample_point.pos, B)
            : find_normals(sample_point.pos, B);
        for (unsigned i = 0; i < sol.size(); ++i) {
            draw_handle(cr, B.valueAt(sol[i]));         // the solutions are in vector 'sol'
            draw_segment(cr, B.valueAt(sol[i]), sample_point.pos);
        }

        cairo_set_source_rgba (cr, 0.5, 0.2, 0., 0.8);
        cairo_stroke(cr);
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    void init_ortho()
    {
        init_common();
        p1.pos = Point(400, 50);
        p2.pos = Point(450, 450);
        p3.pos = Point(100, 50);
        p4.pos = Point(150, 450);

        handles.push_back(&p1);
        handles.push_back(&p2);
        handles.push_back(&p3);
        handles.push_back(&p4);
    }

    void draw_ortho(cairo_t *cr, std::ostringstream *notify,
                      int width, int height, bool save, std::ostringstream */*timer_stream*/)
    {
        draw_common(cr, notify, width, height, save);

        Line l1(p1.pos, p2.pos);
        Line l2 = make_orthogonal_line(p3.pos, l1);
        Line l3 = make_parallel_line(p4.pos, l1);

        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
        cairo_set_line_width(cr, 0.3);
        draw_line(cr, l1);
        draw_line(cr, l2);
        draw_line(cr, l3);
        cairo_stroke(cr);

        draw_label(cr, p1, "P1");
        draw_label(cr, p2, "P2");
        draw_label(cr, p3, "O1");
        draw_label(cr, p4, "O2");

        draw_label(cr, l1, "L");
        draw_label(cr, l2, "L1 _|_ L");
        draw_label(cr, l3, "L2 // L");

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

    void init_coefficients_ctrl_geom(cairo_t* /*cr*/, std::ostringstream* /*notify*/, int /*width*/, int height)
    {
        if ( set_control_geometry )
        {
            set_control_geometry = false;

            sliders[A_COEFF_SLIDER].geometry(Point(50, height - 160), 400);
            sliders[B_COEFF_SLIDER].geometry(Point(50, height - 110), 400);
            sliders[C_COEFF_SLIDER].geometry(Point(50, height - 60), 400);
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
        Point prj = r.pointAt(r.nearestTime(Point(m_width/2-30, m_height/2-30)));
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
                draw_f = &FindDerivatives::draw_menu;
                break;
            case 'B':
                init_derivative();
                draw_f = &FindDerivatives::draw_derivative;
                break;
            case 'C':
                init_find_tangents();
                draw_f = &FindDerivatives::draw_find_tangents;
                break;
            case 'D':
                init_ortho();
                draw_f = &FindDerivatives::draw_ortho;
                break;
        }
        redraw();
    }

    virtual void draw( cairo_t *cr, std::ostringstream *notify,
                       int width, int height, bool save, std::ostringstream *timer_stream)
    {
        m_width = width;
        m_height = height;
        m_length = (m_width > m_height) ? m_width : m_height;
        m_length *= 2;
        (this->*draw_f)(cr, notify, width, height, save, timer_stream);
        Toy::draw(cr, notify, width, height, save, timer_stream);
    }

  public:
    FindDerivatives()
    {
        op = Point(5,5);
    }

  private:
    typedef void (FindDerivatives::* draw_func_t) (cairo_t*, std::ostringstream*, int, int, bool, std::ostringstream*);
    draw_func_t draw_f;
    bool set_common_control_geometry;
    bool set_control_geometry;
    PointHandle p1, p2, p3, p4, p5, p6, O;
    std::vector<Toggle> toggles;
    std::vector<Slider> sliders;
    Point op;
    double m_width, m_height, m_length;

}; // end class FindDerivatives


const char* FindDerivatives::menu_items[] =
{
    "show this menu",
    "derivative matching on curve",
    "find normals",
    "find tangents"
};

const char FindDerivatives::keys[] =
{
     'A', 'B', 'C', 'D'
};



int main(int argc, char **argv)
{
    init( argc, argv, new FindDerivatives());
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
//vim:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :


