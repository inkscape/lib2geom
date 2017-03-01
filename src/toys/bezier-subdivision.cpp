#include <random>

#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/bezier.h>
#include <2geom/sbasis-geometric.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>
#include "path-cairo.h"
#include <cairo/cairo.h>
#include <2geom/svg-path-writer.h>
#include <2geom/bezier-utils.h>

#include <vector>
using std::vector;
using namespace Geom;

double asymmetric_furthest_distance(Path const& a, Path const& b) {

    double result = 0;
    for (auto const& curve : a) {
        for (double t = 0; t <= 1; t += .01) {
            double current_dist = 0;
            b.nearestTime(curve.pointAt(t), &current_dist);
            result = std::max(result, current_dist);
        }
    }

    return result;
}

double symmetric_furthest_distance(Path const& a, Path const& b) {
    return std::max(
                asymmetric_furthest_distance(a,b),
                asymmetric_furthest_distance(b,a)
                );
}

static void dot_plot(cairo_t *cr, Piecewise<D2<SBasis> > const &M, double space=10){
    //double dt=(M[0].cuts.back()-M[0].cuts.front())/space;
    Piecewise<D2<SBasis> > Mperp = rot90(derivative(M)) * 2;
    for( double t = M.cuts.front(); t < M.cuts.back(); t += space) {
        Point pos = M(t), perp = Mperp(t);
        draw_line_seg(cr, pos + perp, pos - perp);
    }
    cairo_pw_d2_sb(cr, M);
    cairo_stroke(cr);
}

static void cross_plot(
        cairo_t *cr,
        std::vector<Geom::Point> const& data,
        Geom::Point offset = Geom::Point(0,0),
        double const size = 10){
    for (auto point : data) {
        Geom::Point dx(size/2, 0);
        Geom::Point dy(0, size/2);
        draw_line_seg(cr, offset + point + dx, offset + point - dx);
        draw_line_seg(cr, offset + point + dy, offset + point - dy);
    }
}

void plot_bezier_with_handles(cairo_t *cr, CubicBezier const& bez, Geom::Point offset = Geom::Point(0,0), double size = 10) {
    Path tmp_path;
    Affine translation;
    translation.setTranslation(offset);
    tmp_path.append(bez);
    tmp_path *= translation;

    cairo_path(cr, tmp_path);
    draw_line_seg(cr, offset + bez[0], offset + bez[1]);
    draw_line_seg(cr, offset + bez[2], offset + bez[3]);
    for (size_t ii = 0; ii < 4; ++ii) {
        Geom::Point const dx(size/2, 0);
        Geom::Point const dy(0, size/2);
        Geom::Point point = bez[ii];
        draw_line_seg(cr, offset + point + dx, offset + point - dx);
        draw_line_seg(cr, offset + point + dy, offset + point - dy);
    }
}

void plot_bezier_with_handles(cairo_t *cr, Path const& path, Geom::Point offset = Geom::Point(0,0), double size = 10) {
    for (auto const& curve : path) {
        CubicBezier const& bez = dynamic_cast<CubicBezier const&>(curve);
        plot_bezier_with_handles(cr, bez, offset, size);
    }
}

Geom::Path subdivide(CubicBezier const& bez, std::vector<double> const& times_in) {
    Path result;
    // First we need to sort the times ascending.
    std::vector<CubicBezier> curves = bez.subdivide(times_in);
    for (const auto& c : curves) {
        result.append(c);
    }
    return result;
}

#define SIZE 4

class BezierFitTester: public Toy {
public:
    PointSetHandle b_handle;
    void draw(cairo_t *cr,
              std::ostringstream *notify,
              int width, int height, bool save, std::ostringstream *timer_stream) {

        if (first_time)
        {
            first_time = false;
            sliders[0].geometry(Point(50, 50), 100);
            sliders[1].geometry(Point(50, 100), 100);
            sliders[2].geometry(Point(50, 150), 100);
        }

        double const t1 = sliders[0].value();
        double const t2 = sliders[1].value();
        double const t3 = sliders[2].value();

        std::vector<double> const t_set {t1, t2};
        std::vector<double> const t_set2 {t1, t2, t3};

        D2<SBasis> B1 = b_handle.asBezier();
        Piecewise<D2<SBasis> >B;
        B.concat(Piecewise<D2<SBasis> >(B1));

        // testing fuse_nearby_ends
        std::vector< Piecewise<D2<SBasis> > > pieces;
        pieces = fuse_nearby_ends(split_at_discontinuities(B),9);
        Piecewise<D2<SBasis> > C;
        for (unsigned i=0; i<pieces.size(); i++){
            C.concat(pieces[i]);
        }
        // testing fuse_nearby_ends

        cairo_set_line_width (cr, 2.);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
        //cairo_d2_sb(cr, B1);
        //cairo_pw_d2_sb(cr, C);
        //cairo_pw_d2_sb(cr, B);
        cairo_stroke(cr);

        Timer tm;
        Timer::Time als_time = tm.lap();

        cairo_set_source_rgba (cr, 0., 0., 0.9, 1);
        //dot_plot(cr,uniform_B);
        cairo_stroke(cr);

        std::cout << B[0] << std::endl;

        Geom::Affine translation;

        Geom::Path original_path;
        //original_bezier.append(B[0]);
        //original_bezier.appendNew<CubicBezier> (B[0]);
        CubicBezier original_bezier(b_handle.pts);
        original_path.append(original_bezier);



        cairo_set_source_rgba (cr, 0., 0., .9, 1);
        cairo_path(cr, original_path);


        if(1) {
            tm.ask_for_timeslice();
            tm.start();
            auto result = original_bezier.subdivide(t1);
            als_time = tm.lap();

            Point translation(Geom::Point(0,300));

            cairo_set_source_rgba (cr, .0, .5, .0, 1);
            plot_bezier_with_handles(cr, original_path, translation);

            cairo_set_source_rgba (cr, .0, .0, .9, 1);
            plot_bezier_with_handles(cr, result.first, translation);
            plot_bezier_with_handles(cr, result.second, translation);
        }

        if(1) {
            tm.ask_for_timeslice();
            tm.start();
            Path result = subdivide(original_bezier, t_set);
            std::cout << "Difference between original and divided curve ("
                      << result.size() << "): " << symmetric_furthest_distance(original_path, result) << std::endl;
            als_time = tm.lap();

            Point translation(Geom::Point(300,300));

            cairo_set_source_rgba (cr, .0, .5, .0, 1);
            plot_bezier_with_handles(cr, original_path, translation);

            cairo_set_source_rgba (cr, .0, .0, .9, 1);
            plot_bezier_with_handles(cr, result, translation);
        }

        if(1) {
            tm.ask_for_timeslice();
            tm.start();
            Path result = subdivide(original_bezier, t_set2);
            std::cout << "Difference between original and divided curve ("
                      << result.size() << "): " << symmetric_furthest_distance(original_path, result) << std::endl;
            als_time = tm.lap();

            Point translation(Geom::Point(300,0));

            cairo_set_source_rgba (cr, .0, .5, .0, 1);
            plot_bezier_with_handles(cr, original_path, translation);

            cairo_set_source_rgba (cr, .0, .0, .9, 1);
            plot_bezier_with_handles(cr, result, translation);
        }

        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

public:
    BezierFitTester(){
        for(int i = 0; i < SIZE; i++) {
            b_handle.push_back(150+uniform()*300,150+uniform()*300);
        }
        b_handle.pts[0] = Geom::Point(70,250);
        b_handle.pts[1] = Geom::Point(200,150);
        b_handle.pts[2] = Geom::Point(200,350);
        b_handle.pts[3] = Geom::Point(350,200);
        handles.push_back(&b_handle);
        // M 70 250 C 860 766 200 350 350 200
        // M 70 250 C 906 833 200 350 350 200
        // M 70 250 C 800 738 200 350 350 200
        sliders.push_back(Slider(0, 1, 0, .3, "time 1"));
        sliders.push_back(Slider(0, 1, 0, .5, "time 2"));
        sliders.push_back(Slider(0, 1, 0, .7, "time 3"));
        handles.push_back(&(sliders[0]));
        handles.push_back(&(sliders[1]));
        handles.push_back(&(sliders[2]));
    }
private:
    std::vector<Slider> sliders;
    bool first_time = true;
    bool randomize_times = false;
    std::random_device rd;
    std::default_random_engine generator;
};

int main(int argc, char **argv) {
    init(argc, argv, new BezierFitTester);
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
//vim:filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99:
