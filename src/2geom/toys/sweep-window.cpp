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
#include <2geom/region.h>
#include <2geom/shape.h>

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
            Point temp2 = fp;
            fp = tp;
            tp = temp2;
        }
    }
    Curve const &get_curve(std::vector<Path> const &ps) const {
        return ps[curve.path][curve.ix];
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
            ret.push_back(Section(xs[i].get_curve(ps), d, xs[i].curve, cuts[i][j-1], cuts[i][j]));
        }
    }
    return ret;
}

struct WEvent {
    bool closing;
    unsigned ix;
    WEvent(bool c, unsigned i) : closing(c), ix(i) {}
};

struct EventSorter {
    std::vector<Section> *rs;
    Dim2 d;
    EventSorter(std::vector<Section> *r, Dim2 dim) : rs(r), d(dim) {}
    bool operator()(WEvent x, WEvent y) {
        Point xp = x.closing ? (*rs)[x.ix].tp : (*rs)[x.ix].fp,
              yp = y.closing ? (*rs)[y.ix].tp : (*rs)[y.ix].fp;
        if(xp[d] < yp[d]) return true;
        if(xp[d] > yp[d]) return false;
        if( x.closing && !y.closing ) return true;  //handle closings before openings
        if( !x.closing && y.closing ) return false;
        return xp[1-d] < yp[1-d];                   //sort in the other dimension as well
    }
};

/*  Commented out.. it was difficult to think about this out of context, so code is inlined

//checks if a is greater than b along dimension d.
//   assumes nonintersection, monotonic sections, overlap in opposite d, 
//   and that b is not inside a's bbox (though the converse may be true)
//these invariants allow us to assign a strict boolean answer to "is this section above this other one?"
//TODO: identical sections?
bool section_above(Section const &a, Section const &b, std::vector<Path> const &ps, Dim2 d) {
    Rect ra = Rect(a.fp, a.tp), rb = Rect(b.fp, b.tp);
    if(ra.contains(rb)) {
        //ambiguous case where we must use roots
    } else {
        //we should be able to look at how the sections are related to determine aboveneess
        if(rb[d].max() >= ra[d].max()
        if(ra[d].min() <= rb[d].min()) return true;
        if(ra[1-d].max() > 
    }
} */

double section_root(Section const &s, std::vector<Path> const &ps, double v, Dim2 d) {
    std::vector<double> roots = s.get_curve(ps).roots(v, d);
    for(unsigned j = 0; j < roots.size(); j++)
        if(Interval(s.f, s.t).contains(roots[j])) return roots[j];
    return -1;
}

std::vector<unsigned> sweep_window(cairo_t *cr, std::vector<Path> const &ps, std::vector<Section> &rs, Dim2 d) {
    std::vector<WEvent> events; events.reserve(rs.size()*2);
    std::vector<unsigned> ret(rs.size());
    
    for(int i = 0; i < rs.size(); i++) {
        ret.push_back(0);
        events.push_back(WEvent(false,i));
        events.push_back(WEvent(true,i));
    }
    EventSorter sorter(&rs, d);
    std::sort(events.begin(), events.end(), sorter);

    /* char* buf = (char*)malloc(10);
    for(unsigned i = 0; i < events.size(); i++) {
        sprintf(buf, "%i", i);
        draw_text(cr, events[i].closing ? rs[events[i].ix].tp : rs[events[i].ix].fp, buf);
    }
    free(buf); */ 

    std::vector<unsigned> open;
    for(unsigned i = 0; i < events.size(); i++) {
        unsigned ix = events[i].ix;
        if(events[i].closing) {
            std::vector<unsigned>::iterator iter = std::find(open.begin(), open.end(), ix);
            open.erase(iter);
        } else {
            Rect ir = Rect(rs[ix].fp, rs[ix].tp);
            unsigned count = 0;
            for(unsigned j = 0; j < open.size(); j++) {
                unsigned jx = open[j];
                Rect jr = Rect(rs[jx].fp, rs[jx].tp);
                //following code checks if jx is above ix, and if not, continues
                if(jr.contains(ir)) {
                    //ambiguous case where we must use roots
                    double rt = section_root(rs[jx], ps, rs[ix].fp[d] + 0.1, d);
                    bool rl;
                    if(rt == -1) { 
                        cairo_set_source_rgb(cr, 1, 0, 0);
                    } else {
                        rl = rs[ix].fp[1-d] < rs[jx].get_curve(ps)(rt)[1-d];
                        if(rl) cairo_set_source_rgb(cr, 0, 0, 1); else
                               cairo_set_source_rgb(cr, 0, 1, 0);
                    }
                    cairo_rectangle(cr, jr);
                    cairo_rectangle(cr, ir);
                    cairo_stroke(cr);
                    if(rt == -1 || rl) continue;
                } else {
                    if(ir[1-d].min() < jr[1-d].min()) continue;  //ix is above
                    if(ir[d].max() > jr[d].max() && rs[jx].fp[1-d] > rs[jx].tp[1-d]) continue; //ix is off to the side, and jx is decreasing
                }
                //otherwise, jx is above
                count++;
            }
            open.push_back(ix);
            ret[ix] = count;
        }
    }
    return ret;
}

std::vector<unsigned> naive_count(std::vector<Section> const &rs, std::vector<Path> const &ps, Dim2 d) {
    std::vector<unsigned> ret(rs.size());
    for(unsigned i = 0; i < rs.size(); i++) {
        double v = rs[i].fp[d] + 1;
        for(unsigned j = 0; j < rs.size(); j++) {
            if(j == i) continue;
            if(rs[j].tp[d] >= rs[i].fp[d] &&
               min(rs[j].tp[1-d], rs[j].fp[1-d]) <
               min(rs[i].tp[1-d], rs[i].fp[1-d])) {
                if(rs[j].fp[d] == v || rs[j].tp[d] == v) {
                    v += 0.01;
                }
            }
        }
        double vt = section_root(rs[i], ps, v, d);
        if(vt == -1) {
            cout << "DOH!\n";
            ret[i] = 0;
            continue;
        }
        double vp = rs[i].get_curve(ps)(vt)[1-d];
        unsigned count = 0;
        for(unsigned j = 0; j < rs.size(); j++) {
            if(j == i) continue;
            if(rs[j].tp[d] >= rs[i].fp[d] &&
               min(rs[j].tp[1-d], rs[j].fp[1-d]) <=
               min(rs[i].tp[1-d], rs[i].fp[1-d])) {
                double t = section_root(rs[j], ps, v, d);
                if(t == -1) continue;
                if(rs[j].get_curve(ps)(t)[1-d] < vp) count++;
            }
        }
        ret[i] = count;
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
            if(toggles[0].on) {
                //cairo_rectangle(cr, r);
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
                
                Crossings xs = pair_intersect(es[i].get_curve(path), Interval(es[i].f, es[i].t),
                                              other.get_curve(path), Interval(other.f, other.t));
                for(unsigned k = 0; k < xs.size(); k++) {
                    if((are_near(xs[k].ta, es[i].f) || are_near(xs[k].ta, es[i].t)) && 
                       (are_near(xs[k].tb, other.f) || are_near(xs[k].tb, other.t))) continue;
                    draw_cross(cr, es[i].get_curve(path).pointAt(xs[k].ta));
                    tvals[i].push_back(xs[k].ta);
                    tvals[inters[i][j]].push_back(xs[k].tb);
                }
                
            }
        }

        if(toggles[0].on) {
            es = subdivide_sections(path, X, es, tvals);
            for(unsigned i = 0; i < es.size(); i++) {
                Rect r = Rect(es[i].fp, es[i].tp);
                //cairo_rectangle(cr, r);
                //cairo_stroke(cr);
            }
        }
        std::vector<unsigned> info = sweep_window(cr, path, es, X);
        char* buf = (char*)malloc(10);
        for(unsigned i = 0; i < es.size(); i++) {
            sprintf(buf, "%i", info[i]);
            draw_text(cr, es[i].get_curve(path).pointAt((es[i].f + es[i].t) / 2), buf);
        }
        free(buf);
        
        double t = fmod(p.pos[X], 20) / 20.0;
        int n = floor(p.pos[X]) / 20;
        //cout << t << " " << n << endl;
        if(t > 0 && n < es.size()) {
            Section s = es[n];
            draw_cross(cr, s.get_curve(path).pointAt(lerp(t, s.f, s.t)));
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
