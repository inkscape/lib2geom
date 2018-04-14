#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/sbasis-math.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/path-intersection.h>
#include <2geom/bezier-curve.h>
#include <2geom/transforms.h>
#include <2geom/angle.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>
#include <sstream>

using std::vector;
using namespace Geom;
using namespace std;

// TODO:
// use path2
// replace Ray stuff with path2 line segments.

//-----------------------------------------------

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

int num = 30;
vector<double> c1_bot;
vector<double> c1_top;
vector<double> c2_bot;
vector<double> c2_top;
vector<double> c3_bot;
vector<double> c3_top;
vector<double> c4_bot;
vector<double> c4_top;

CubicBezier create_bezier(Point const &anchor, double angle /* in degrees */,
                             double length, double dx1, double dx2, cairo_t *cr = NULL) {
    Point A = anchor;
    Point dir = Point(1.0, 0) * Rotate(-angle) * length;
    Point B = anchor + dir;

    Point C = A - Point(1.0, 0) * dx1;
    Point D = B + Point(1.0, 0) * dx1;
    Point E = A + Point(1.0, 0) * dx2;
    Point F = B - Point(1.0, 0) * dx2;

    if (cr) {
        draw_cross(cr, A);
        draw_cross(cr, B);
        draw_cross(cr, C);
        draw_cross(cr, D);
        draw_cross(cr, E);
        draw_cross(cr, F);
    }

    return CubicBezier(C, E, F, D);
}

/*
 * Draws a single "scribble segment" (we use many of these to cover the whole curve).
 *
 * Let I1, I2 be two adjacent intervals (bounded by the points A1, A2, A3) on the lower and J1, J2
 * two adjacent intervals (bounded by B1, B2, B3) on the upper parallel. Then we specify:
 *
 * - The point in I1 where the scribble line starts (given by a value in [0,1])
 * - The point in J2 where the scribble line ends (given by a value in [0,1])
 * - A point in I2 (1st intermediate point of the Bezier curve)
 * - A point in J1 (2nd intermediate point of the Bezier curve)
 *
 */
CubicBezier
create_bezier_again(Point const &anchor1, Point const &anchor2, Point const &dir1, Point const &dir2,
                    double /*c1*/, double /*c2*/, double c3, double c4, double mu, cairo_t *cr = NULL) {
    Point A = anchor1;// - dir * c1;
    Point B = anchor1 + dir1 * (c3 + mu);
    Point C = anchor2 - dir2 * (c4 + mu);
    Point D = anchor2;// + dir * c2;

    if (cr) {
        draw_cross(cr, A);
        //draw_cross(cr, B);
        //draw_cross(cr, C);
        //draw_cross(cr, D);
    }

    return CubicBezier(A, B, C, D);
}

CubicBezier
create_bezier_along_curve(Piecewise<D2<SBasis> > const &curve1,
                          Piecewise<D2<SBasis> > const &curve2,
                          double segdist,
                          Coord const t1, Coord const t2, Point const &n,
                          double c1, double c2, double /*c3*/, double /*c4*/, double /*mu*/, cairo_t *cr = NULL) {
    cout << "create_bezier_along_curve -- start" << endl;
    /*
    Point A = curve1.valueAt(t1 - c1);
    Point B = curve1.valueAt(t1) + n * (c3 + mu);
    Point C = curve2.valueAt(t2) - n * (c4 + mu);
    Point D = curve2.valueAt(t2 + c2);
    */
    Point A = curve1.valueAt(t1 - c1 * segdist);
    Point B = curve1.valueAt(t1) + n * 0.1;
    Point C = curve2.valueAt(t2) - n * 0.1;
    Point D = curve2.valueAt(t2 + c2 * segdist);

    if (cr) {
        draw_cross(cr, A);
        //draw_cross(cr, B);
        //draw_cross(cr, C);
        //draw_cross(cr, D);
    }

    cout << "create_bezier_along_curve -- end" << endl;
    return CubicBezier(A, B, C, D);
}

class OffsetTester: public Toy {
    PointSetHandle psh;
    PointSetHandle psh_rand;

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        double w = 600;
        double slider_top = w/4.;
        double slider_bot = w*3./4.;
        double slider_margin = 40;
        double slider_middle = (slider_top + slider_bot) / 2;

        if(psh.pts.empty()) {
            psh.pts.push_back(Point(200,300));
            psh.pts.push_back(Point(350,250));
            psh.pts.push_back(Point(500,280));
            psh.pts.push_back(Point(700,300));

            psh.pts.push_back(Point(400,300));
            psh.pts.push_back(Point(550,250));
            psh.pts.push_back(Point(700,280));
            psh.pts.push_back(Point(900,300));

            psh.pts.push_back(Point(900,500));
            psh.pts.push_back(Point(700,480));
            psh.pts.push_back(Point(550,450));
            psh.pts.push_back(Point(400,500));

            psh_rand.pts.push_back(Geom::Point(slider_margin,slider_bot));
            psh_rand.pts.push_back(Geom::Point(slider_margin,slider_top));
            psh_rand.pts.push_back(Geom::Point(slider_margin,slider_top));
            psh_rand.pts.push_back(Geom::Point(slider_margin,slider_top));
            psh_rand.pts.push_back(Geom::Point(slider_margin,slider_top));
            psh_rand.pts.push_back(Geom::Point(slider_margin,slider_bot));
            psh_rand.pts.push_back(Geom::Point(slider_margin,slider_middle));
        }

        psh_rand.pts[0][X] = slider_margin;
        if (psh_rand.pts[0][Y]<slider_top) psh_rand.pts[0][Y] = slider_top;
        if (psh_rand.pts[0][Y]>slider_bot) psh_rand.pts[0][Y] = slider_bot;
        psh_rand.pts[1][X] = slider_margin + 15;
        if (psh_rand.pts[1][Y]<slider_top) psh_rand.pts[1][Y] = slider_top;
        if (psh_rand.pts[1][Y]>slider_bot) psh_rand.pts[1][Y] = slider_bot;
        psh_rand.pts[2][X] = slider_margin + 30;
        if (psh_rand.pts[2][Y]<slider_top) psh_rand.pts[2][Y] = slider_top;
        if (psh_rand.pts[2][Y]>slider_bot) psh_rand.pts[2][Y] = slider_bot;
        psh_rand.pts[3][X] = slider_margin + 45;
        if (psh_rand.pts[3][Y]<slider_top) psh_rand.pts[3][Y] = slider_top;
        if (psh_rand.pts[3][Y]>slider_bot) psh_rand.pts[3][Y] = slider_bot;
        psh_rand.pts[4][X] = slider_margin + 60;
        if (psh_rand.pts[4][Y]<slider_top) psh_rand.pts[4][Y] = slider_top;
        if (psh_rand.pts[4][Y]>slider_bot) psh_rand.pts[4][Y] = slider_bot;
        psh_rand.pts[5][X] = slider_margin + 75;
        if (psh_rand.pts[5][Y]<slider_top) psh_rand.pts[5][Y] = slider_top;
        if (psh_rand.pts[5][Y]>slider_bot) psh_rand.pts[5][Y] = slider_bot;
        psh_rand.pts[6][X] = slider_margin + 90;
        if (psh_rand.pts[6][Y]<slider_top) psh_rand.pts[6][Y] = slider_top;
        if (psh_rand.pts[6][Y]>slider_bot) psh_rand.pts[6][Y] = slider_bot;

        *notify << "Sliders:" << endl << endl << endl << endl;
        *notify << "0 - segment distance" << endl;
        *notify << "1 - start anchor randomization" << endl;
        *notify << "2 - end anchor randomization" << endl;
        *notify << "3 - start rounding randomization" << endl;
        *notify << "4 - end rounding randomization" << endl;
        *notify << "5 - start/end rounding increase randomization" << endl;
        *notify << "6 - additional offset of the upper anchors (to modify the segment angle)" << endl;

        for(unsigned i = 0; i < psh_rand.size(); ++i) {
            cairo_move_to(cr,Geom::Point(slider_margin + 15.0 * i, slider_bot));
            cairo_line_to(cr,Geom::Point(slider_margin + 15.0 * i, slider_top));
        }
        cairo_set_line_width(cr,.5);
        cairo_set_source_rgba (cr, 0., 0.3, 0., 1.);
        cairo_stroke(cr);

        cairo_set_line_width (cr, 2);
        cairo_set_source_rgba (cr, 0., 0., 0.8, 1);

        // Draw the curve and its offsets
        D2<SBasis> B = psh.asBezier();
        cairo_d2_sb(cr, B);
        cairo_stroke(cr);

        Coord offset = 30;
        Piecewise<D2<SBasis> > n = rot90(unitVector(derivative(B)));
        Piecewise<D2<SBasis> > offset_curve1 = Piecewise<D2<SBasis> >(B)+n*offset;
        Piecewise<D2<SBasis> > offset_curve2 = Piecewise<D2<SBasis> >(B)-n*offset;
        PathVector offset_path1 = path_from_piecewise(offset_curve1, 0.1);
        PathVector offset_path2 = path_from_piecewise(offset_curve2, 0.1);
        Piecewise<D2<SBasis> > tangent1 = unitVector(derivative(offset_curve1));
        Piecewise<D2<SBasis> > tangent2 = unitVector(derivative(offset_curve2));
        cairo_set_line_width (cr, 1);
        cairo_path(cr, offset_path1);
        cairo_path(cr, offset_path2);
        cairo_stroke(cr);

        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);

        double lambda1 = 1.0 - (psh_rand.pts[1][Y] - slider_top) * 2.0/w;
        double lambda2 = 1.0 - (psh_rand.pts[2][Y] - slider_top) * 2.0/w;
        double lambda3 = 1.0 - (psh_rand.pts[3][Y] - slider_top) * 2.0/w;
        double lambda4 = 1.0 - (psh_rand.pts[4][Y] - slider_top) * 2.0/w;
        double mu = 1.0 - (psh_rand.pts[5][Y] - slider_top) * 2.0/w;
        //Point dir = Point(1,0) * (slider_bot - psh_rand.pts[0][Y]) / 2.5;
        double off = 0.5 - (psh_rand.pts[6][Y] - slider_top) * 2.0/w;

        double segdist = (slider_bot - psh_rand.pts[0][Y]) / (slider_bot - slider_top) * 0.1;
        if (segdist < 0.01) {
            segdist = 0.01;
        }

        vector<Point> pts_bot;
        vector<Point> pts_top;
        vector<Point> dirs_bot;
        vector<Point> dirs_top;
        int counter = 0;
        for(double i = 0.0; i < 1.0; i += segdist) {
            draw_cross(cr, offset_curve1.valueAt(i));
            pts_bot.push_back(offset_curve1.valueAt(i + segdist * c1_bot[counter] * lambda1));
            pts_top.push_back(offset_curve2.valueAt(i + segdist * (c2_top[counter] * lambda2 + 1/2.0) + off));
            dirs_bot.push_back(tangent1.valueAt(i) * 20);
            dirs_top.push_back(tangent2.valueAt(i) * 20);
            ++counter;
        }

        for(int i = 0; i < num; ++i) {
            cout << "c1_bot[" << i << "]: " << c1_bot[i] << endl;
        }

        for (int i = 0; i < num-1; ++i) {
            Path path1;
            //cout << "dirs_bot[" << i << "]: " << dirs_bot[i] << endl;
            cout << "c3_bot[" << i << "]: " << c3_bot[i] << endl;
            CubicBezier bc = create_bezier_again(pts_bot[i], pts_top[i],
                                                    dirs_bot[i], dirs_top[i],
                                                    0, 0, c3_bot[i] * lambda3, c4_top[i] * lambda4, mu, cr);
                                                    //c1_bot[i] * lambda1,
                                                    //c2_top[i] * lambda2,
                                                    //c3_bot[i] * lambda3,
                                                    //c4_top[i] * lambda4, mu, cr);

            path1.append(bc);
            cairo_path(cr, path1);

            Path path2;
            bc = create_bezier_again(pts_top[i], pts_bot[i+1],
                                     dirs_top[i], dirs_bot[i+1],
                                     0, 0, c4_bot[i] * lambda4, c3_top[i] * lambda3, mu, cr);
            /*
            bc = create_bezier_again(pts_top[i+1], pts_bot[i], dir,
                                     1.0 - c2_top[i] * lambda2,
                                     1.0 - c1_bot[i+1] * lambda1,
                                     c3_top[i] * lambda3,
                                     c4_bot[i] * lambda4, mu, cr);
            */
            path2.append(bc);
            cairo_path(cr, path2);
        }

        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

public:
    OffsetTester() {
        handles.push_back(&psh);
        handles.push_back(&psh_rand);
        /*
        psh.pts.clear();
        Point A(100,300);
        //Point B(100,200);
        //Point C(200,330);
        Point D(200,100);
        psh.push_back(A);
        //psh.push_back(B);
        //psh.push_back(C);
        psh.push_back(D);
        psh.push_back(Geom::Point(slider_margin,slider_bot));
        */

        for (int i = 0; i < num; ++i) {
            c1_bot.push_back(uniform() - 0.5);
            c1_top.push_back(uniform() - 0.5);
            //c1_bot.push_back(1.0);
            //c1_top.push_back(1.0);
            c2_bot.push_back(uniform() - 0.5);
            c2_top.push_back(uniform() - 0.5);

            c3_bot.push_back(uniform());
            c3_top.push_back(uniform());
            c4_bot.push_back(uniform());
            c4_top.push_back(uniform());
        }

        /*
        for (int i = 0; i < num; ++i) {
            c1_bot[i] = c1_bot[i] / 10.0;
            c2_bot[i] = c2_bot[i] / 10.0;
            c3_bot[i] = c3_bot[i] / 10.0;
            c4_bot[i] = c4_bot[i] / 10.0;

            c1_top[i] = c1_top[i] / 10.0;
            c2_top[i] = c2_top[i] / 10.0;
            c3_top[i] = c3_top[i] / 10.0;
            c4_top[i] = c4_top[i] / 10.0;
        }
        */
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new OffsetTester);
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
//vim:filetype = cpp:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :

 	  	 
