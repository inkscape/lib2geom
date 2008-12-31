#include <2geom/toys/toy-framework-2.h>

#include <vector>
using std::vector;
using namespace Geom;
using namespace std;

class Box3d: public Toy {
    Point orig;
    
    double tmat[3][4];
    double c[8][4];
    Point corners[8];

    PointHandle origin_handle;
    PointSetHandle vanishing_points_handles, axes_handles;
    
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        Geom::Point dir(1,-2);

	cairo_set_source_rgba (cr, 0., 0.125, 0, 1);

        // draw vertical lines for the VP sliders and keep the sliders at their horizontal positions
        draw_slider_lines (cr);
        axes_handles.pts[0][0] = 30;
        axes_handles.pts[1][0] = 45;
        axes_handles.pts[2][0] = 60;

        /* create the transformation matrix for the map  P^3 --> P^2 that has the following effect:
           (1 : 0 : 0 : 0) --> vanishing point in x direction (= handle #0)
           (0 : 1 : 0 : 0) --> vanishing point in y direction (= handle #1)
           (0 : 0 : 1 : 0) --> vanishing point in z direction (= handle #2)
           (0 : 0 : 0 : 1) --> origin (= handle #3)
        */
        for (int j = 0; j < 4; ++j) {
            tmat[0][j] = vanishing_points_handles.pts[j][0];
            tmat[1][j] = vanishing_points_handles.pts[j][1];
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
            corners[i] = proj_image (cr, c[i]);
        }
        draw_box(cr, corners);
        cairo_set_line_width (cr, 2);
	cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
    void first_time(int /*argc*/, char** /*argv*/) {
        // Finite images of the three vanishing points and the origin
        handles.push_back(&origin_handle);
        handles.push_back(&vanishing_points_handles);
        handles.push_back(&axes_handles);
        vanishing_points_handles.push_back(550,350);
        vanishing_points_handles.push_back(150,300);
        vanishing_points_handles.push_back(380,40);
        vanishing_points_handles.push_back(340,450);
        // plane origin
        origin_handle.pos = Point(180,65);

        // Handles for moving in axes directions
        axes_handles.push_back(30,300);
        axes_handles.push_back(45,300);
        axes_handles.push_back(60,300);
        
        // Box corners
        for (int i = 0; i < 8; ++i) {
            c[i][0] = ((i & 1) ? 1 : 0);
            c[i][1] = ((i & 2) ? 1 : 0);
            c[i][2] = ((i & 4) ? 1 : 0);
            c[i][3] = 1;
        }

        orig = origin_handle.pos;
    }
    Geom::Point proj_image (cairo_t *cr, const double pt[4]) {
        double res[3];
        for (int j = 0; j < 3; ++j) {
            res[j] =
                tmat[j][0] * (pt[0] - (axes_handles.pts[0][1]-300)/100)
                + tmat[j][1] * (pt[1] - (axes_handles.pts[1][1]-300)/100)
                + tmat[j][2] * (pt[2] - (axes_handles.pts[2][1]-300)/100)
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

        for(int i = 0 ; i < 4; i++) {
            cairo_move_to(cr,corners[i]);
            cairo_line_to(cr,corners[i+4]);
        }
    }

    void draw_slider_lines (cairo_t *cr) {
        cairo_move_to(cr, Geom::Point(20,300));
        cairo_line_to(cr, Geom::Point(70,300));
    
        for(int i = 0; i < 3; i++) {
            cairo_move_to(cr, Geom::Point(30 + 15*i,00));
            cairo_line_to(cr, Geom::Point(30 + 15*i,450));
        }
        
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 1);
        cairo_stroke(cr);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new Box3d);
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
