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
    Point fp, tp;
    Section(CurveIx c, double fd, double td) : curve(c), f(fd), t(td) {}
    Section(Curve const &c, Dim2 d, CurveIx cix, double fd, double td) : curve(cix), f(fd), t(td) {
        fp = c.pointAt(f), tp = c.pointAt(t);
        //TODO: decide which direction the second case should work
        if(fp[d] > tp[d] || (fp[d] == tp[d] && fp[1-d] < tp[1-d])) {
            //swap from and to
            double temp = f;
            f = t;
            t = temp;
        }
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

std::vector<Section>
subdivide_sections(std::vector<Path> const &ps, Dim2 d,
                   std::vector<Section> const &xs,
                   std::vector<std::vector<double> > &cuts) {
    std::vector<Section> ret;
    for(unsigned i = 0; i < cuts.size(); i++) {
        std::sort(cuts[i].begin(), cuts[i].end());
        bool rev = xs[i].f > xs[i].t;
        cuts[i].insert(cuts[i].begin(), rev ? xs[i].t : xs[i].f);
        cuts[i].push_back(rev ? xs[i].f : xs[i].t);
        std::unique(cuts[i].begin(), cuts[i].end());
        /*for(int j = rev ? cuts[i].size() - 1 : 0;
                    rev ? (j >= 0) : (j < cuts[i].size());
                    rev ? (j--) : (j++)) {*/
        for(unsigned j = 1; j < cuts[i].size(); j++) {
            ret.push_back(Section(ps[xs[i].curve.path][xs[i].curve.ix], d, xs[i].curve, cuts[i][j-1], cuts[i][j]));
        }
    }
    return ret;
}

class SweepWindow: public Toy {
    vector<Path> path;
    std::vector<Toggle> toggles;
    PointHandle p;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 0.5);
        cairo_save(cr);
        cairo_path(cr, path);
        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_stroke(cr);
        cairo_restore(cr);
        
        std::vector<Section> es = paths_sections(path, X);
        std::vector<Rect> rs;
        for(unsigned i = 0; i < es.size(); i++) {
            Rect r = Rect(es[i].fp, es[i].tp);
            if(!toggles[0].on) {
                cairo_rectangle(cr, r);
                cairo_stroke(cr);
            }
            rs.push_back(r);
        }
        
        std::vector<std::vector<unsigned> > inters = sweep_bounds(rs);
        std::vector<std::vector<double> > tvals(es.size());
        for(unsigned i = 0; i < es.size(); i++) tvals[i] = std::vector<double>();
        for(unsigned i = 0; i < inters.size(); i++) {
            for(unsigned j = 0; j < inters[i].size(); j++) {
                Section other = es[inters[i][j]];
                
                Crossings xs = pair_intersect(path[es[i].curve.path][es[i].curve.ix], Interval(es[i].f, es[i].t),
                                              path[other.curve.path][other.curve.ix], Interval(other.f, other.t));
                for(unsigned k = 0; k < xs.size(); k++) {
                    if((are_near(xs[k].ta, es[i].f) || are_near(xs[k].ta, es[i].t)) && 
                       (are_near(xs[k].tb, other.f) || are_near(xs[k].tb, other.t))) continue;
                    draw_cross(cr, path[es[i].curve.path][es[i].curve.ix].pointAt(xs[k].ta));
                    tvals[i].push_back(xs[k].ta);
                    tvals[inters[i][j]].push_back(xs[k].tb);
                }
                
            }
        }

        if(toggles[0].on) {
            es = subdivide_sections(path, X, es, tvals);
            for(unsigned i = 0; i < es.size(); i++) {
                Rect r = Rect(es[i].fp, es[i].tp);
                cairo_rectangle(cr, r);
                cairo_stroke(cr);
            }
        }
        
        double t = fmod(p.pos[X], 20) / 20.0;
        int n = floor(p.pos[X]) / 20;
        cout << t << " " << n << endl;
        if(t > 0 && n < es.size()) {
            Section s = es[n];
            draw_cross(cr, path[s.curve.path][s.curve.ix].pointAt(lerp(t, s.f, s.t)));
            cairo_stroke(cr);
        }
        
        draw_toggles(cr, toggles);
        
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    void mouse_pressed(GdkEventButton* e) {
        toggle_events(toggles, e);
        Toy::mouse_pressed(e);
    }
    public:
    SweepWindow () {}
    void first_time(int argc, char** argv) {
        const char *path_name="sanitize_examples.svgd";
        if(argc > 1)
            path_name = argv[1];
        path = read_svgd(path_name);
        OptRect bounds = bounds_exact(path);
        if(bounds)
            path += Point(10,10)-bounds->min();
        toggles.push_back(Toggle("Intersect", true));
        toggles[0].bounds = Rect(Point(10,10), Point(100, 30));
        p = PointHandle(Point(100,300));
        handles.push_back(&p);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new SweepWindow());
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
