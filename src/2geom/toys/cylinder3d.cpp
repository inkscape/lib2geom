#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/sbasis-2d.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/transforms.h>
#include <2geom/sbasis-math.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>

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
        cairo_d2_sb(cr, B);
    }
}

static const int segments = 50;
Geom::Point orig;

static void draw_box (cairo_t *cr, Geom::Point corners[8]);
static void draw_slider_lines (cairo_t *cr);
static Geom::Point proj_image (cairo_t *cr, const double pt[4], const vector<Geom::Point> &handles);

double tmat[3][4];
double c[8][4];
Geom::Point corners[8];

class Box3d: public Toy {
    std::vector<Toggle> togs;
    Path path_a;
    Piecewise<D2<SBasis> >  path_a_pw;
    PointSetHandle hand;
    
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        orig = hand.pts[7];
	
        Geom::Point dir(1,-2);

	cairo_set_source_rgba (cr, 0., 0.125, 0, 1);

        // draw vertical lines for the VP sliders and keep the sliders at their horizontal positions
        draw_slider_lines (cr);
        hand.pts[4][0] = 30;
        hand.pts[5][0] = 45;
        hand.pts[6][0] = 60;

        // draw the curve that is supposed to be projected on the box's front face
        vector<Geom::Point>::iterator it = hand.pts.begin();
        for (int j = 0; j < 7; ++j) ++it;

        /* create the transformation matrix for the map  P^3 --> P^2 that has the following effect:
              (1 : 0 : 0 : 0) --> vanishing point in x direction (= handle #0)
              (0 : 1 : 0 : 0) --> vanishing point in y direction (= handle #1)
              (0 : 0 : 1 : 0) --> vanishing point in z direction (= handle #2)
              (0 : 0 : 0 : 1) --> origin (= handle #3)
        */
        for (int j = 0; j < 4; ++j) {
            tmat[0][j] = hand.pts[j][0];
            tmat[1][j] = hand.pts[j][1];
            tmat[2][j] = 1;
        }

        *notify << "Projection matrix:" << endl;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 4; ++j) {
                *notify << tmat[i][j] << " ";
            }
            *notify << endl;
        }

        // draw the projective images of the box's corners
        for (int i = 0; i < 8; ++i) {
            corners[i] = proj_image (cr, c[i], hand.pts);
        }
        draw_box(cr, corners);
        cairo_set_line_width (cr, 2);
	cairo_stroke(cr);

        {
            D2<Piecewise<SBasis> > B = make_cuts_independent(path_a_pw);
            Piecewise<SBasis> preimage[4];
	    
	    if(togs[0].on) {
		    preimage[0] = sin((B[0] - orig[0]) / 100);
		    preimage[1] = -(B[1] - orig[1]) / 100;
		    preimage[2] = cos((B[0] - orig[0]) / 100);
	    } else { //if(togs[1].state) {
		    Piecewise<SBasis> sphi = sin((B[0] - orig[0]) / 200);
		    Piecewise<SBasis> cphi = cos((B[0] - orig[0]) / 200);
		    
		    preimage[0] = -sphi*sin((B[1] - orig[1]) / 200);
		    preimage[1] = -sphi*cos((B[1] - orig[1]) / 200);
		    preimage[2] = -cphi;
	    }
	    Piecewise<SBasis> res[3];
            for (int j = 0; j < 3; ++j) {
                res[j] =
                    (preimage[0]) * tmat[j][0]
                    + (preimage[1] - ((hand.pts[5][1]-300)/100)) * tmat[j][1]
                    + (preimage[2] - ((hand.pts[6][1]-00)/100)) * tmat[j][2]
                    +( - (hand.pts[4][1]-300)/100) * tmat[j][0] + tmat[j][3];
            }
            //if (fabs (res[2]) > 0.000001) {
            D2<Piecewise<SBasis> > result(divide(res[0],res[2], 4), 
                                          divide(res[1],res[2], 4));
            
            cairo_d2_pw_sb(cr, result);
            cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
            cairo_stroke(cr);
        }
        draw_toggles(cr, togs);
        
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
    void first_time(int argc, char** argv) {
        const char *path_a_name="star.svgd";
        if(argc > 1)
            path_a_name = argv[1];
        std::vector<Path> paths_a = read_svgd(path_a_name);
        assert(paths_a.size() > 0);
        path_a = paths_a[0];
        
	path_a.close(true);
        path_a_pw = path_a.toPwSb();

        // Finite images of the three vanishing points and the origin
        hand.pts.push_back(Point(150,300));
        hand.pts.push_back(Point(380,40));
        hand.pts.push_back(Point(550,350));
        hand.pts.push_back(Point(340,450));

        // Hand.Pts for moving in axes directions
        hand.pts.push_back(Point(30,300));
        hand.pts.push_back(Point(45,300));
        hand.pts.push_back(Point(60,300));
        
        // Box corners
        for (int i = 0; i < 8; ++i) {
            c[i][0] = ((i & 1) ? 1 : 0);
            c[i][1] = ((i & 2) ? 1 : 0);
            c[i][2] = ((i & 4) ? 1 : 0);
            c[i][3] = 1;
        }

        // Origin handle
	hand.pts.push_back(Point(180,70));
        togs.push_back(Toggle("S", true));
        handles.push_back(&hand);
    }
    void key_hit(GdkEventKey *e) {
        if(e->keyval == 'c') togs[0].set(1); else
        if(e->keyval == 's') togs[0].set(0);
        redraw();
    }
    void mouse_pressed(GdkEventButton* e) {
        toggle_events(togs, e);
        Toy::mouse_pressed(e);
    }
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
              tmat[j][0] * (pt[0] - (handles[4][1]-300)/100)
            + tmat[j][1] * (pt[1] - (handles[5][1]-300)/100)
            + tmat[j][2] * (pt[2] - (handles[6][1]-300)/100)
            + tmat[j][3] * pt[3];
    }
    if (fabs (res[2]) > 0.000001) {
        Geom::Point result = Geom::Point (res[0]/res[2], res[1]/res[2]);
        draw_handle(cr, result);
        return result;
    }
    assert(0); // unclipped point
    return Geom::Point(0,0);
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
