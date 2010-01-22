#include <2geom/toys/toy-framework-2.h>

/*
 * Copyright cilix42
 * Kinematic template toy. The aim is to manipulate the cursor movement
 * so that it stays closer to a given shape (e.g., a circle or a line).
 * For details, see http://hci.uwaterloo.ca/Publications/Papers/uist222-fung.pdf
 *
 * Each kinematic template has a radius of action outside of which it
 * has no effect (this is indicated by a red circle).
 */

#include <vector>
#include <2geom/point.h>
#include <2geom/transforms.h>

using std::vector;
using namespace Geom;
using namespace std;

// I feel a little uneasy using a Point for polar coords.
Point cartesian_to_polar(Point const &pt, Point const &center = Point(0,0)) {
    Point rvec = pt - center;
    // use atan2 unless you want to measure between two vectors
    return Point(L2(rvec), atan2(rvec));
}

Point polar_to_cartesian(Point const &pt, Point const &center = Point(0,0)) {
    return center + Point(pt[0],0) * Rotate(pt[1]);
}

class KinematicTemplate {
public:
    KinematicTemplate(double const sx = 0.0, double const sy = 0.0, double const cx = 0.0, double const cy = 0.0);
    ~KinematicTemplate();

    /*
     * To facilitate the creation of templates, we can use different coordinates at each point
     * (e.g., radial coordinates around a fixed center)
     */
    virtual std::pair<Point, Point> local_coordinate_system(Point const &/*at*/) {
        // Return standard cartesian coordinates
        return std::make_pair(Point(1,0), Point(0,1));
    }

    virtual Point next_point(Point const &at, Point const &delta);// { return at; }
    virtual void draw_visual_cue(cairo_t *cr);

    Point const get_center() { return center; }
    void set_center(Point const &pos) { center = pos; }

    double get_radius_of_action() { return radius; }
    void set_radius_of_action(double const r) { radius = r; }
    void enlarge_radius_of_action(double const by) {
        if (radius > -by)
            radius += by;
        else
            radius = 0;
    }
            

protected:
    double sx, sy, cx, cy;
    Point center;
    double radius;
};

KinematicTemplate::KinematicTemplate(double const sx, double const sy, double const cx, double const cy)
    : sx(sx),
      sy(sy),
      cx(cx),
      cy(cy),
      center(300,300),
      radius(100)
{
}

KinematicTemplate::~KinematicTemplate()
{
}

Point
KinematicTemplate::next_point(Point const &last_pt, Point const &delta)
{
//    Point new_pt = last_pushed + kinematic_delta(last_pushed, delta, 0);

    /* Compute the "relative" coordinates w.r.t. the "local coordinate system" at the current point */
    Point v = local_coordinate_system(last_pt).first;
    Point w = local_coordinate_system(last_pt).second;
    double dotv = dot(v, delta);
    double dotw = dot(w, delta);

    Point new_delta;
    if (L2(last_pt + delta - center) < radius) {
        /*
         * We are withing the radius of action of the kinematic template.
         * Compute displacement w.r.t. the v/w-coordinate system.
         */
        new_delta = (dotv*sx + cx)*v + (dotw*sy + cy)*w;
    } else {
        new_delta = delta;
    }

    return last_pt + new_delta;
}

void
KinematicTemplate::draw_visual_cue(cairo_t *cr) {
    cairo_set_source_rgba (cr, 1, 0, 0, 1);
    cairo_set_line_width (cr, 0.5);
    cairo_new_sub_path(cr);
    cairo_arc(cr, center[X], center[Y], radius, 0, M_PI*2);
    cairo_stroke(cr);
}

class RadialKinematicTemplate : public KinematicTemplate {
public:
    RadialKinematicTemplate(Point const &center, double const sx, double const sy, double const cx, double const cy);

    virtual std::pair<Point, Point> local_coordinate_system(Point const &at) {
        /* Return 'radial' coordinates around polar_center */
        Point v = unit_vector(at - center);
        return std::make_pair(v, rot90(v));
    }

private:
    Point radial_center;
};

RadialKinematicTemplate::RadialKinematicTemplate(Point const &center, double const sx, double const sy,
                                                 double const cx = 0.0, double const cy = 0.0)
    : KinematicTemplate(sx, sy, cx, cy)
{
    radial_center = center;
}

class GridKinematicTemplate : public KinematicTemplate {
public:
    GridKinematicTemplate(double const sx = 0.0, double const sy = 0.0, double const cx = 0.0, double const cy = 0.0)
        : KinematicTemplate(sx, sy, cx, cy) {};
    virtual Point next_point(Point const &at, Point const &delta);// { return at; }
};

Point
GridKinematicTemplate::next_point(Point const &at, Point const &delta) {
    if (L2(at + delta - center) < radius) {
        // we are within the radius of action
        Point new_delta = delta;

        if (fabs(delta[0]) > fabs(delta[1]))
            new_delta[1] *= sy;
        else
            new_delta[0] *= sx;

        return at + new_delta;
    } else {
        return at + delta;
    }
}


// My idea was to compute the gradient of an arbitrary potential function as the transform.  Probably the right way to do this is to use the hessian as the integrand -- njh
class ImplicitKinematicTemplate : public KinematicTemplate {
public:
    ImplicitKinematicTemplate() {}

    virtual Point next_point(Point const &at, Point const &delta) {
        if (L2(at + delta - center) < radius) {
            // we are within the radius of action

            // the 0.7dx+1 includes a weakened version of the constraining force
            // I can't help but think this is really a form of differential constraint solver, let's discuss.
            return at + delta*Scale(0.7*sin(at[0]/10.0)+1, 0.7*cos(at[1]/10.0)+1);
        } else {
            return at + delta;
        }
    }
};



vector<KinematicTemplate*> kin;
KinematicTemplate *cur_kin;
std::string cur_choice = "A";

class KinematicTemplatesToy : public Toy {

    enum menu_item_t
    {
        KT_HORIZONTAL = 0,
        KT_VERTICAL,
        KT_GRID,
        KT_CIRCLE,
        KT_RADIAL,
        KT_CONVEYOR,
        KT_IMP,
        TOTAL_ITEMS // this one must be the last item
    };

    static const char* menu_items[TOTAL_ITEMS];
    static const char keys[TOTAL_ITEMS];

    Point cur, last_pushed;
    vector<vector<Point>*> pts;

    bool dragging_center; // to prevent drawing while dragging the center
    
    void draw_menu( cairo_t * /*cr*/, std::ostringstream *notify,
                    int /*width*/, int /*height*/, bool /*save*/,
                    std::ostringstream */*timer_stream*/ )
    {
        *notify << std::endl;
        for (int i = KT_HORIZONTAL; i < TOTAL_ITEMS; ++i)
        {
            *notify << "   " << keys[i] << " -  " <<  menu_items[i] << std::endl;
        }
        *notify << "+/-  -  enlarge/shrink radius of action" << endl << endl << endl << endl;
        *notify << "Current choice: " << cur_choice << endl;
    }

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
	cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
	cairo_set_line_width (cr, 1);

        //draw_handle(cr, cur_kin->get_center());
        draw_menu(cr, notify, width, height, save, timer_stream);

        // draw all points accumulated so far
        for (unsigned int i = 0; i < pts.size(); ++i) {
            if (pts[i]->size() > 0) {
                cairo_move_to(cr, (*pts[i])[0]);
            }
            for (unsigned int j = 0; j < pts[i]->size(); ++j) {
                //cout << "   --> drawing line to point #" << j << endl;
                cairo_line_to(cr, (*pts[i])[j]);
            }
        }

        cairo_stroke(cr);

        cur_kin->draw_visual_cue(cr);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    void first_time(int /*argc*/, char** /*argv*/) {
        p1.pos = Point(200, 200);
        handles.push_back(&p1);

        pts.clear();
        kin.push_back(new KinematicTemplate(1.0, 0.1)); // horizontal lines
        kin.push_back(new KinematicTemplate(0.1, 1.0)); // horizontal lines
        kin.push_back(new GridKinematicTemplate(0.1, 0.1));
        kin.push_back(new RadialKinematicTemplate(p1.pos, 0.1, 1.0));
        kin.push_back(new RadialKinematicTemplate(p1.pos, 1.0, 0.1));
        kin.push_back(new KinematicTemplate(1.0, 0.1, 1, 0)); // horiz conveyor
        kin.push_back(new ImplicitKinematicTemplate());
        cur_kin = kin[0];
        cur_kin->set_center(p1.pos);

        dragging_center = false;
    }

    void mouse_pressed(GdkEventButton *e) {
        Point at(e->x, e->y);

        if(L2(at - p1.pos) < 5) {
            dragging_center = true;
        } else {
            if(e->button == 1) {
                vector<Point> *vec = new vector<Point>;
                vec->clear();
                vec->push_back(at);
                last_pushed = at;
                pts.push_back(vec);
            }
        }

        Toy::mouse_pressed(e);
    }

    void mouse_released(GdkEventButton */*e*/) {
        dragging_center = false;
    }

    void mouse_moved(GdkEventMotion* e) {
        if (!dragging_center) {
            Point at(e->x, e->y);

            Point delta = at - cur;
            //cout << "Mouse moved to: " << at << " (difference: " << delta << ")" << endl;
            if(e->state & GDK_BUTTON1_MASK) {
                Point new_pt = cur_kin->next_point(last_pushed, delta);

                pts.back()->push_back(new_pt);
                last_pushed = new_pt;
            }
            cur = at;
        } else {
            cur_kin->set_center(p1.pos);
        }

        Toy::mouse_moved(e);
    }

    void key_hit(GdkEventKey *e)
    {
        char choice = std::toupper(e->keyval);
        // No need to copy and paste code
        if(choice >= 'A' and choice < 'A' + TOTAL_ITEMS) {
            cur_kin = kin[choice - 'A'];
            cur_choice = choice;

        } else
        switch (choice)
        {
            case '+':
                cur_kin->enlarge_radius_of_action(5);
                break;
            case '-':
                cur_kin->enlarge_radius_of_action(-5);
                break;
            default:
                break;
        }
        p1.pos = cur_kin->get_center();
        redraw();
    }

private:
    PointHandle p1;
};

const char* KinematicTemplatesToy::menu_items[] =
{
    "horizontal",
    "vertical",
    "grid",
    "circular",
    "radial",
    "conveyor",
    "implicit"
};

const char KinematicTemplatesToy::keys[] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G'
};

int main(int argc, char **argv) {
    init(argc, argv, new KinematicTemplatesToy);
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
