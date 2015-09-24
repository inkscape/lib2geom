#include <2geom/cairo-path-sink.h>
#include <2geom/ellipse.h>
#include <2geom/line.h>
#include <2geom/polynomial.h>
#include <toys/toy-framework-2.h>

using namespace Geom;

class CircleIntersect : public Toy {
    PointSetHandle psh[2];
    Ellipse ea, eb;
    Line l[6];

    void intersect() {
        // This is code is almost the same as in ellipse.cpp.
        // We use it here to get the lines.
        double A, B, C, D, E, F;
        double a, b, c, d, e, f;

        ea.coefficients(A, E, B, C, D, F);
        eb.coefficients(a, e, b, c, d, f);

        double I, J, K, L;
        I = (-E*E*F + 4*A*B*F + C*D*E - A*D*D - B*C*C) / 4;
        J = -((E*E - 4*A*B) * f + (2*E*F - C*D) * e + (2*A*D - C*E) * d +
              (2*B*C - D*E) * c + (C*C - 4*A*F) * b + (D*D - 4*B*F) * a) / 4;
        K = -((e*e - 4*a*b) * F + (2*e*f - c*d) * E + (2*a*d - c*e) * D +
              (2*b*c - d*e) * C + (c*c - 4*a*f) * B + (d*d - 4*b*f) * A) / 4;
        L = (-e*e*f + 4*a*b*f + c*d*e - a*d*d - b*c*c) / 4;

        std::vector<double> mu = solve_cubic(I, J, K, L);

        for (unsigned i = 0; i < mu.size(); ++i) {
            double aa = mu[i] * A + a;
            double bb = mu[i] * B + b;
            double cc = mu[i] * C + c;
            double dd = mu[i] * D + d;
            double ee = mu[i] * E + e;
            double ff = mu[i] * F + f;
            double delta = ee*ee - 4*aa*bb;
            if (delta < 0) {
                continue;
            }

            if (aa != 0) {
                bb /= aa; cc /= aa; dd /= aa; ee /= aa; ff /= aa;
                double s = (ee + std::sqrt(ee*ee - 4*bb)) / 2;
                double q = ee - s;
                double alpha = (dd - cc*q) / (s - q);
                double beta = cc - alpha;

                l[i*2]   = Line(1, q, alpha);
                l[i*2+1] = Line(1, s, beta);
            } else if (bb != 0) {
                cc /= bb; dd /= bb; ee /= bb; ff /= bb;
                double s = ee;
                double q = 0;
                double alpha = cc / ee;
                double beta = ff * ee / cc;

                l[i*2]   = Line(q, 1, alpha);
                l[i*2+1] = Line(s, 1, beta);
            } else {
                // both aa and bb are zero
                l[i*2]   = Line(ee, 1, dd);
                l[i*2+1] = Line(0, 1, cc/ee);
            }
        }
    }

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {

        Rect all(Point(0,0), Point(width, height));

        double r1x = Geom::distance(psh[0].pts[0], psh[0].pts[1]);
        double r1y = Geom::distance(psh[0].pts[0], psh[0].pts[2]);
        double rot1 = Geom::atan2(psh[0].pts[1] - psh[0].pts[0]);

        double r2x = Geom::distance(psh[1].pts[0], psh[1].pts[1]);
        double r2y = Geom::distance(psh[1].pts[0], psh[1].pts[2]);
        double rot2 = Geom::atan2(psh[1].pts[1] - psh[1].pts[0]);

        ea = Ellipse(psh[0].pts[0], Point(r1x, r1y), rot1);
        eb = Ellipse(psh[1].pts[0], Point(r2x, r2y), rot2);

        for (unsigned i = 0; i < 6; ++i) {
            l[i] = Line(0, 0, 0);
        }

        cairo_set_line_width(cr, 1.0);

        cairo_set_source_rgb(cr, 0, 0, 0);
        Geom::CairoPathSink cps(cr);
        cps.feed(ea);
        cps.feed(eb);
        cairo_stroke(cr);

        try {
            intersect();
            std::vector<ShapeIntersection> result = ea.intersect(eb);

            if (!l[0].isDegenerate() && !l[1].isDegenerate()) {
                cairo_set_source_rgba(cr, 1, 0, 0, 0.2);
                draw_line(cr, l[0], all);
                draw_line(cr, l[1], all);
                cairo_stroke(cr);
            }
            if (!l[2].isDegenerate() && !l[3].isDegenerate()) {
                cairo_set_source_rgba(cr, 0, 1, 0, 0.2);
                draw_line(cr, l[2], all);
                draw_line(cr, l[3], all);
                cairo_stroke(cr);
            }
            if (!l[4].isDegenerate() && !l[5].isDegenerate()) {
                cairo_set_source_rgba(cr, 0, 0, 1, 0.2);
                draw_line(cr, l[4], all);
                draw_line(cr, l[5], all);
                cairo_stroke(cr);
            }

            cairo_set_source_rgb(cr, 1, 0, 0);
            for (unsigned i = 0; i < result.size(); ++i) {
                draw_handle(cr, result[i].point());
            }
            cairo_stroke(cr);
        } catch(...) {
            *notify << "Exception";
        }

        // TODO: draw_handle at intersections

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    public:
    CircleIntersect(){
        psh[0].push_back(300,300); psh[0].push_back(450,150); psh[0].push_back(250, 250);
        psh[1].push_back(350,300); psh[1].push_back(500,500); psh[1].push_back(300, 350);
        handles.push_back(&psh[0]);
        handles.push_back(&psh[1]);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new CircleIntersect());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
