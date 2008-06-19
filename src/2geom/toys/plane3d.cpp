#include "d2.h"
#include "sbasis.h"
#include "sbasis-geometric.h"
#include "sbasis-2d.h"
#include "bezier-to-sbasis.h"
#include "transforms.h"
#include "sbasis-math.h"

#include "path-cairo.h"
#include "toy-framework-2.h"
#include "path.h"
#include "svg-path-parser.h"

#include <gsl/gsl_matrix.h>

#include <vector>
using std::vector;
using namespace Geom;
using namespace std;

class Box3d: public Toy {
    double tmat[3][4];
    PointHandle origin_handle;
    PointSetHandle vanishing_points_handles;
    std::vector<Path> paths_a;
    
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {

        Geom::Point orig = origin_handle.pos;
	cairo_set_source_rgba (cr, 0., 0.125, 0, 1);

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

        for(unsigned i = 0; i < paths_a.size(); i++) {
            Piecewise<D2<SBasis> >  path_a_pw = paths_a[i].toPwSb();

            D2<Piecewise<SBasis> > B = make_cuts_independant(path_a_pw);
            Piecewise<SBasis> preimage[4];
                
            preimage[0] =  (B[0] - orig[0]) / 100;
            preimage[1] = -(B[1] - orig[1]) / 100;
            Piecewise<SBasis> res[3];
            for (int j = 0; j < 3; ++j) {
                res[j] = preimage[0] * tmat[j][0]
                    + preimage[1] * tmat[j][1]
                    + tmat[j][3];
            }
            
            //if (fabs (res[2]) > 0.000001) {
            D2<Piecewise<SBasis> > result(divide(res[0],res[2], 2), 
                                          divide(res[1],res[2], 2));
            
            cairo_d2_pw(cr, result);
            cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
            cairo_stroke(cr);
        }
        
        Toy::draw(cr, notify, width, height, save);
    }
    void first_time(int argc, char** argv) {
        const char *path_a_name="ptitle.svgd";
        if(argc > 1)
            path_a_name = argv[1];
        paths_a = read_svgd(path_a_name);
        assert(paths_a.size() > 0);

        // Finite images of the three vanishing points and the origin
        handles.push_back(&origin_handle);
        handles.push_back(&vanishing_points_handles);
        vanishing_points_handles.push_back(550,350);
        vanishing_points_handles.push_back(150,300);
        vanishing_points_handles.push_back(380,40);
        vanishing_points_handles.push_back(340,450);
        // plane origin
        origin_handle.pos = Point(180,65);
    }
    //int should_draw_bounds() {return 1;}
    
    Geom::Point proj_image (cairo_t *cr, const double pt[4], const vector<Geom::Point> &handles)
    {
        double res[3];
        for (int j = 0; j < 3; ++j) {
            res[j] = 0;
            for (int i = 0; i < 3; ++i)
                res[j] += tmat[j][i] * pt[i];
        }
        if (fabs (res[2]) > 0.000001) {
            Geom::Point result = Geom::Point (res[0]/res[2], res[1]/res[2]);
            draw_handle(cr, result);
            return result;
        }
        assert(0); // unclipped point
        return Geom::Point(0,0);
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
