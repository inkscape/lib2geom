#include <iostream>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/path-intersection.h>
#include <2geom/basic-intersection.h>
#include <2geom/pathvector.h>

#include <cstdlib>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <algorithm>

using namespace Geom;
using namespace std;

struct CurveIx {
    unsigned path, ix;
    CurveIx(unsigned p, unsigned i) : path(p), ix(i) {}
    Curve const &get(std::vector<Path> const &ps) const {
        return ps[path][ix];
    }
};

/*
struct WEvent {
    CurveIx curve;
    double time;
    Point point;
    unsigned vert;
    WEvent(CurveIx c, double t, Point p, unsigned v) : curve(c), time(t), point(p), vert(v) {}
}; */

struct Section {
    CurveIx curve;
    double f, t;
    Point fp, tp;
    unsigned fv, tv;
    Section(Curve const &c, Dim2 d, CurveIx cix, double fd, double td, unsigned fve, unsigned tve) : curve(cix), f(fd), t(td), fv(fve), tv(tve) {
        fp = c.pointAt(f), tp = c.pointAt(t);
        //TODO: decide which direction the second case should work
        if(fp[d] > tp[d] || (fp[d] == tp[d] && fp[1-d] < tp[1-d])) {
            //swap from and to
            std::swap(f, t);
            std::swap(fp, tp);
            std::swap(fv, tv);
        }
    }
};

void draw_section(cairo_t *cr, Section const &s, std::vector<Path> const &ps) {
    Curve *ret = s.curve.get(ps).portion(Interval(s.f, s.t));
    draw_curve(cr, *ret);
    delete ret;
}

// A little sugar for appending a list to another
template<typename T>
void append(T &a, T const &b) {
    a.insert(a.end(), b.begin(), b.end());
}

//yield a sorted, unique list of monotonic cuts of a curve, including 0 and 1
std::vector<double> mono_splits(Curve const &d) {
    Curve* deriv = d.derivative();
    std::vector<double> splits = deriv->roots(0, X);
    append(splits, deriv->roots(0, Y));
    delete deriv;
    splits.push_back(0);
    splits.push_back(1);
    std::sort(splits.begin(), splits.end());
    std::unique(splits.begin(), splits.end());
    return splits;
}

struct SectionSorter {
    Dim2 dim;
    SectionSorter(Dim2 d) : dim(d) {}
    bool operator()(Section x, Section y) {
        if(x.fp[d] < y.fp[d]) return true;
        if(x.fp[d] > y.fp[d]) return false;
        if(x.fp[1-d] < y.fp[1-d]) return true;
        if(x.fp[1-d] > y.fp[1-d]) return false;
        return x.vert < y.vert;
    }
};

std::vector<std::vector<Section> > sweep_window(std::vector<Path> const &ps) {
    std::vector<std::vector<Section> > contexts;
    std::vector<Section> context;
    
    unsigned vert = 0;
    //TODO: also store a priority queue of sectioned off bits, and do mergepass
    std::dequeue<Section> monos;
    
    for(unsigned i = 0; i < ps.size(); i++) {
        //TODO: necessary? can we have empty paths?
        if(ps[i].size()) {
            unsigned first = vert;
            for(unsigned j = 0; j < ps[i].size(); j++) {
                std::vector<double> splits = mono_splits(ps[i][j]);
                for(unsigned k = 1; k < splits.size(); k++) {
                   monos.push_back(Section(ps[i][j], d, CurveIx(i,j), splits[k-1], splits[k], vert, vert+1);
                   vert++;
                }
            }
            if(monos.back().fv == vert) monos.back().fv = first; else monos.back().tv = first;
        }
    }
    std::sort(monos.begin(), monos.end(), SectionSorter(X));
    
    while(!monos.empty())
        Section s = monos.front();
        monos.pop_front();
        
        
        
        contexts.push_back(context);
    }
    
    return contexts;
}

class SweepWindow: public Toy {
    vector<Path> path;
    std::vector<Toggle> toggles;
    PointHandle p;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        draw_toggles(cr, toggles);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 0.5);
        
        std::vector<Section> sections = paths_sections(path, X);
        
        std::vector<std::vector<Section> > contexts = sweep_window(sections, path);
        int cix = (int) p.pos[X] / 10;
        if(cix >= 0 && cix < contexts.size()) {
            for(unsigned i = 0; i < contexts[cix].size(); i++) {
                draw_section(cr, contexts[cix][i], path);
            }
        }
        
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
        toggles[0].bounds = Rect(Point(10,100), Point(100, 130));
        p = PointHandle(Point(100,300));
        handles.push_back(&p);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new SweepWindow());
    return 0;
}

/*
// Returns a vector of sections, breaking the path into monotonic parts
// The from / to parts of the sections are such that increasing t yields increaasing x or y
std::vector<Section> paths_sections(std::vector<Path> const &ps, Dim2 d) {
    std::vector<Section> ret;
    for(unsigned i = 0; i < ps.size(); i++) {
        for(unsigned j = 0; j < ps[i].size(); j++) {
            std::vector<double> splits = mono_splits(ps[i][j]);
            for(unsigned k = 1; k < splits.size(); k++)
               ret.push_back(Section(ps[i][j], d, CurveIx(i,j), splits[k-1], splits[k]);
        }
    }
    return ret;
}
*/

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
