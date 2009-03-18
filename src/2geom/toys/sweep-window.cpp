#include <iostream>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/path-intersection.h>
#include <2geom/basic-intersection.h>
#include <2geom/pathvector.h>

#include <cstdlib>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/ord.h>

#include <algorithm>

using namespace Geom;
using namespace std;

void draw_rect(cairo_t *cr, Point tl, Point br) {
    cairo_move_to(cr, tl[X], tl[Y]);
    cairo_line_to(cr, br[X], tl[Y]);
    cairo_line_to(cr, br[X], br[Y]);
    cairo_line_to(cr, tl[X], br[Y]);
    cairo_close_path(cr);
}

struct CurveIx {
    unsigned px, ix;
    CurveIx(unsigned p, unsigned i) : px(p), ix(i) {}
};

struct CEvent {
    CurveIx curve;
    double t, v;
    CEvent(CurveIx c, double ti, double va) : curve(c), t(ti), v(va) {}
};

struct Section {
    CurveIx curve;
    double f, t;
    OptRect bounds;
};

// returns nested vectors of rects, representing curve bounds
std::vector<std::vector<Rect> > paths_rects(std::vector<Path> const &ps) {
    std::vector<std::vector<Rect> > ret;
    for(unsigned i = 0; i < ps.size(); i++) {
        std::vector<Rect> inner;
        for(Path::const_iterator it = ps[i].begin(); it != ps[i].end(); it++)
            inner.push_back(*(it->boundsExact()));
        ret.push_back(inner);
    }
    return ret;
}

// A little sugar for appending a list to another
template<typename T>
void append(T &a, T const &b) {
    a.insert(a.end(), b.begin(), b.end());
}

/** This returns the times when the x or y derivative is 0 in the curve. */
std::vector<double> curve_mono_splits(Curve const &d) {
    Curve* deriv = d.derivative();
    std::vector<double> rs = deriv->roots(0, X);
    append(rs, deriv->roots(0, Y));
    delete deriv;
    std::sort(rs.begin(), rs.end());
    return rs;
}

std::vector<CEvent> paths_events(std::vector<Path> const &ps, Dim2 d = X) {
    std::vector<CEvent> ret;
    for(unsigned i = 0; i < ps.size(); i++) {
        for(unsigned j = 0; j < ps[i].size(); j++) {
            std::vector<double> deriv = curve_mono_splits(ps[i][j]);
            ret.push_back(CEvent(CurveIx(i, j), 0, ps[i][j].initialPoint()[d]));
            ret.push_back(CEvent(CurveIx(i, j), 1, ps[i][j].finalPoint()[d]));
            for(unsigned k = 0; k < deriv.size(); k++) {
                if(deriv[k] == 0 || deriv[k] == 1) continue;
                ret.push_back(CEvent(CurveIx(i, j), deriv[k], ps[i][j].valueAt(deriv[k],d)));
            }
        }
    }
    return ret;
}

class WindingTest: public Toy {
    vector<Path> path;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 0.5);
        cairo_save(cr);
        cairo_path(cr, path);
        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_stroke(cr);
        cairo_restore(cr);
        
        std::vector<CEvent> es = paths_events(path);
        for(unsigned i = 0; i < es.size(); i++) {
            if(es[i].t == 0.0 || es[i].t == 1.0) {
                cairo_set_source_rgb(cr, 0, 1, 0);
            } else {
                cairo_set_source_rgb(cr, 1, 0, 0);
            }
            draw_cross(cr, path[es[i].curve.px][es[i].curve.ix].pointAt(es[i].t));
            cairo_stroke(cr);
        }

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    public:
    WindingTest () {}
    void first_time(int argc, char** argv) {
        const char *path_name="winding.svgd";
        if(argc > 1)
            path_name = argv[1];
        path = read_svgd(path_name);
        OptRect bounds = bounds_exact(path);
        if(bounds)
            path += Point(10,10)-bounds->min();
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new WindingTest());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
