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
    unsigned path, ix;
    CurveIx(unsigned p, unsigned i) : path(p), ix(i) {}
};

struct CEvent {
    CurveIx curve;
    double t, v;
    CEvent(CurveIx c, double ti, double va) : curve(c), t(ti), v(va) {}
    // Lexicographic ordering by value
    bool operator<(CEvent const &other) const {
        if(v < other.v) return true;
        if(v > other.v) return false;
        return false;
    }
};

struct Section {
    CurveIx curve;
    double f, t;
    OptRect rect;
    Section(CurveIx c, double fd, double td) : curve(c), f(fd), t(td) {}
    Section(Curve const &c, Dim2 d, CurveIx cix, double fd, double td) : curve(cix), f(fd), t(td) {
        Point fp = c.pointAt(f), tp = c.pointAt(t);
        //TODO: decide which direction the second case should work
        if(fp[d] > tp[d] || (fp[d] == tp[d] && fp[1-d] < tp[1-d])) {
            //swap from and to
            double temp = f;
            f = t;
            t = temp;
        }
        rect = OptRect(Rect(fp, tp));
    }
};

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

std::vector<Section> paths_sections(std::vector<Path> const &ps, Dim2 d) {
    std::vector<Section> ret;
    for(unsigned i = 0; i < ps.size(); i++) {
        for(unsigned j = 0; j < ps[i].size(); j++) {
            std::vector<double> deriv = curve_mono_splits(ps[i][j]);
            //ret.push_back(CEvent(CurveIx(i, j), 0, ps[i][j].initialPoint()[d]));
            //ret.push_back(CEvent(CurveIx(i, j), 1, ps[i][j].finalPoint()[d]));
            //ret.push_back(CEvent(CurveIx(i, j), deriv[k], ps[i][j].valueAt(deriv[k],d)));
            if(deriv.size() == 0) {
                ret.push_back(Section(ps[i][j], d, CurveIx(i, j), 0, 1));
            } else {
                if(deriv[0] != 0) ret.push_back(Section(ps[i][j], d, CurveIx(i,j), 0, deriv[0]));
                for(unsigned k = 1; k < deriv.size(); k++) {
                   ret.push_back(Section(ps[i][j], d, CurveIx(i,j), deriv[k-1], deriv[k]));
                }
                if(deriv[deriv.size() - 1] != 1) ret.push_back(Section(ps[i][j], d, CurveIx(i,j), deriv[deriv.size() - 1], 1));
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
        
        std::vector<Section> es = paths_sections(path, X);
        for(unsigned i = 0; i < es.size(); i++) {
            cairo_rectangle(cr, *es[i].rect);
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
