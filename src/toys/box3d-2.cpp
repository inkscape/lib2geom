#include "d2.h"
#include "sbasis.h"
#include "sbasis-geometric.h"
#include "sbasis-2d.h"
#include "bezier-to-sbasis.h"
#include "transforms.h"
#include "sbasis-math.h"

#include "path-cairo.h"
#include "toy-framework.h"
#include "path.h"
#include "svg-path-parser.h"

#include <gsl/gsl_matrix.h>

#include <vector>
using std::vector;
using namespace Geom;
using namespace std;

unsigned total_pieces_sub;
unsigned total_pieces_inc;

void cairo_pw(cairo_t *cr, Piecewise<SBasis> p) {
    for(unsigned i = 0; i < p.size(); i++) {
        D2<SBasis> B;
        B[0] = Linear(p.cuts[i], p.cuts[i+1]);
        B[1] = p[i];
        cairo_md_sb(cr, B);
    }
}

static const int segments = 50;
Geom::Point orig;
bool draw_axis_hints = true;

static void draw_box (cairo_t *cr, Geom::Point corners[8]);
static void draw_slider_lines (cairo_t *cr);
static Geom::Point proj_image (cairo_t *cr, const double pt[4], const vector<Geom::Point> &handles);
static void preimage_of_curve_pt (Geom::Point const pt, double preimage[4], const vector<Geom::Point> &handles);

double tmat[3][4];
double c[8][4];
Geom::Point corners[8];

class Box3d: public Toy {
    Path path_a;
    Piecewise<D2<SBasis> >  path_a_pw;
    
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        Geom::Point dir(1,-2);

	cairo_set_source_rgba (cr, 0., 0.125, 0, 1);

        // draw vertical lines for the VP sliders and keep the sliders at their horizontal positions
        draw_slider_lines (cr);
        handles[4][0] = 30;
        handles[5][0] = 45;
        handles[6][0] = 60;

        // draw the curve that is supposed to be projected on the box's front face
        vector<Geom::Point>::iterator it = handles.begin();
        for (int j = 0; j < 7; ++j) ++it;
        D2<SBasis> B = handles_to_sbasis<3>(it);
        Piecewise<D2<SBasis> > curve = Piecewise<D2<SBasis> > (B);
        cairo_pw_d2(cr, curve);
        cairo_stroke(cr);

        /* create the transformation matrix for the map  P^3 --> P^2 that has the following effect:
              (1 : 0 : 0 : 0) --> vanishing point in x direction (= handle #0)
              (0 : 1 : 0 : 0) --> vanishing point in y direction (= handle #1)
              (0 : 0 : 1 : 0) --> vanishing point in z direction (= handle #2)
              (0 : 0 : 0 : 1) --> origin (= handle #3)
        */
        for (int j = 0; j < 4; ++j) {
            tmat[0][j] = handles[j][0];
            tmat[1][j] = handles[j][1];
            tmat[2][j] = 1;
        }

        *notify << "Projection matrix:" << endl;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                *notify << tmat[i][j] << " ";
            }
            *notify << endl;
        }


        // Box corners
        for (int i = 0; i < 8; ++i) {
            c[i][0] = ((i & 1) ? 1 : 0) + (-1)*(handles[4][1]-300)/100;
            c[i][1] = ((i & 2) ? 1 : 0) + (-1)*(handles[5][1]-300)/100;
            c[i][2] = ((i & 4) ? 1 : 0) + (-1)*(handles[6][1]-300)/100;
            c[i][3] = 1;
        }

        // draw the projective images of the box's corners
        for (int i = 0; i < 8; ++i) {
            corners[i] = proj_image (cr, c[i], handles);
        }
        draw_box(cr, corners);
        cairo_set_line_width (cr, 2);
	cairo_stroke(cr);

        // the curve is divided into a number of segments; we project the endpoints of these segments
        // and draw them as dots on the canvas; simply projecting the handles of the curve and using
        // these as the handles of a new Bezier curve obviously yields a wrong result; but what is
        // the perspective image of a Bezier curve, from a mathematical point of view?
        double preimage[4];
        double t;
        Geom::Point image;
        for (int i = 0; i <= segments; ++i) {
            t = (double) i/(double) segments;
            Geom::Point pt = curve (t);
            preimage_of_curve_pt (pt, preimage, handles);
            image = proj_image (cr, preimage, handles);
            draw_handle (cr, image);
        }

	cairo_set_source_rgba (cr, 0.75, 0, 0, 1);
        cairo_pw_d2(cr, curve);
	cairo_stroke(cr);

        /* draw auxiliary lines parallel to the axes to indicate the box's position relative to the origin */
        if (draw_axis_hints) {
            double pt0[4] = {0, 0, 0, 1};
            double pt1[4] = {c[0][0], 0, 0, 1};
            double pt2[4] = {c[0][0], c[0][1], 0, 1};
            double pt3[4] = {c[0][0], c[0][1], c[0][2], 1};
            Geom::Point pt0_proj, pt1_proj, pt2_proj, pt3_proj;

            pt0_proj = proj_image(cr, pt0, handles);
            pt1_proj = proj_image(cr, pt1, handles);
            pt2_proj = proj_image(cr, pt2, handles);
            pt3_proj = proj_image(cr, pt3, handles);

            cairo_move_to(cr, pt0_proj);
            cairo_line_to(cr, pt1_proj);
            cairo_line_to(cr, pt2_proj);
            cairo_line_to(cr, pt3_proj);
            cairo_set_source_rgba (cr, 0, 0, 0.8, 1);
            cairo_stroke(cr);
        }


        // Hint for keyboard shortcut
	cairo_set_source_rgba (cr, 0, 0, 0, 1);
        draw_text(cr, Point(width - 425, height - 45), "Press 'a' to toggle axis hints");
        draw_text(cr, Point(width - 425, height - 25), "(only visible when box is moved away from origin)");
        
        Toy::draw(cr, notify, width, height, save);
    }

    void key_hit(GdkEventKey *e) {
        if(e->keyval == 'a') draw_axis_hints = !draw_axis_hints;
        redraw();
    }

    void first_time(int argc, char** argv) {
        char *path_a_name="star.svgd";
        if(argc > 1)
            path_a_name = argv[1];
        std::vector<Path> paths_a = read_svgd(path_a_name);
        assert(paths_a.size() > 0);
        path_a = paths_a[0];
        
	path_a.close(true);
        path_a_pw = path_a.toPwSb();

        // Finite images of the three vanishing points and the origin
        handles.push_back(Point(150,300));
        handles.push_back(Point(380,40));
        handles.push_back(Point(550,350));
        handles.push_back(Point(340,450));

        // Handles for moving in axes directions
        handles.push_back(Point(30,300));
        handles.push_back(Point(45,300));
        handles.push_back(Point(60,300));
        
        // Box corners
        for (int i = 0; i < 8; ++i) {
            c[i][0] = ((i & 1) ? 1 : 0);
            c[i][1] = ((i & 2) ? 1 : 0);
            c[i][2] = ((i & 4) ? 1 : 0);
            c[i][3] = 1;
        }

        // Curve handles
        for(unsigned i = 0; i < 4; i++)
            handles.push_back(Point(180+50*i,450+70*uniform()));

        orig = handles[7];
    }
    int should_draw_bounds() {return 1;}
};

int main(int argc, char **argv) {
    init(argc, argv, new Box3d);
    return 0;
}

void draw_box (cairo_t *cr, Geom::Point corners[8]) {
    cairo_move_to(cr,corners[0]);
    cairo_line_to(cr,corners[1]);
    cairo_line_to(cr,corners[3]);
    cairo_line_to(cr,corners[2]);
    cairo_close_path(cr);

    cairo_move_to(cr,corners[4]);
    cairo_line_to(cr,corners[5]);
    cairo_line_to(cr,corners[7]);
    cairo_line_to(cr,corners[6]);
    cairo_close_path(cr);

    cairo_move_to(cr,corners[0]);
    cairo_line_to(cr,corners[4]);

    cairo_move_to(cr,corners[1]);
    cairo_line_to(cr,corners[5]);

    cairo_move_to(cr,corners[2]);
    cairo_line_to(cr,corners[6]);

    cairo_move_to(cr,corners[3]);
    cairo_line_to(cr,corners[7]);
}

void draw_slider_lines (cairo_t *cr) {
    cairo_move_to(cr, Geom::Point(20,300));
    cairo_line_to(cr, Geom::Point(70,300));

    cairo_move_to(cr, Geom::Point(30,00));
    cairo_line_to(cr, Geom::Point(30,450));

    cairo_move_to(cr, Geom::Point(45,00));
    cairo_line_to(cr, Geom::Point(45,450));

    cairo_move_to(cr, Geom::Point(60,00));
    cairo_line_to(cr, Geom::Point(60,450));
        
    cairo_set_line_width (cr, 1);
    cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 1);
    cairo_stroke(cr);
}

static Geom::Point proj_image (cairo_t *cr, const double pt[4], const vector<Geom::Point> &handles)
{
    double res[3];
    for (int j = 0; j < 3; ++j) {
        res[j] =
              tmat[j][0] * (pt[0])
            + tmat[j][1] * (pt[1])
            + tmat[j][2] * (pt[2])
            + tmat[j][3] * pt[3];

    }
    if (fabs (res[2]) > 0.000001) {
        Geom::Point result = Geom::Point (res[0]/res[2], res[1]/res[2]);
        draw_handle(cr, result);
        return result;
    }
    //assert(0); // unclipped point
    return Geom::Point(0,0);
}

static void
preimage_of_curve_pt (Geom::Point const pt, double preimage[4], const vector<Geom::Point> &handles)
{
    preimage[0] = 0 + (-1)*(handles[4][1]-300)/100;;
    preimage[1] = -(pt[1] - orig[1]) / 100 + (-1)*(handles[5][1]-300)/100;;
    preimage[2] =  (pt[0] - orig[0]) / 100 + (-1)*(handles[6][1]-300)/100;;
    preimage[3] = 1;
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
