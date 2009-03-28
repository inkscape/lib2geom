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
#include <queue>
#include <functional>

using namespace Geom;

struct CurveIx {
    unsigned path, ix;
    CurveIx(unsigned p, unsigned i) : path(p), ix(i) {}
    Curve const &get(std::vector<Path> const &ps) const {
        return ps[path][ix];
    }
};

struct Section {
    CurveIx curve;
    double f, t;
    Point fp, tp;
    int winding;
    Section(Curve const &c, Dim2 d, CurveIx cix, double fd, double td) : curve(cix), f(fd), t(td) {
        fp = c.pointAt(f), tp = c.pointAt(t);
        //TODO: decide which direction the second case should work
        if(fp[d] > tp[d] || (fp[d] == tp[d] && fp[1-d] < tp[1-d])) {
            //swap from and to
            std::swap(f, t);
            std::swap(fp, tp);
        }
    }
    void set_to(Curve const &c, Dim2 d, double ti) {
        //we're assuming the to-val isn't before our from
        t = ti;
        tp = c(ti);
        assert(tp[d] >= fp[d]);
    }
    Rect bbox() const {
        return Rect(fp, tp);
    }
};

std::vector<Rect> section_rects(std::vector<Section> const &s) {
    std::vector<Rect> ret;
    for(unsigned i = 0; i < s.size(); i++) {
        ret.push_back(s[i].bbox());
    }
    return ret;
}

void draw_section(cairo_t *cr, Section const &s, std::vector<Path> const &ps) {
    Interval ti = Interval(s.f, s.t);
    Curve *curv = s.curve.get(ps).portion(ti.min(), ti.max());
    cairo_curve(cr, *curv);
    {
        Point h = s.curve.get(ps).pointAt(ti.min());
        int x = int(h[Geom::X]);
        int y = int(h[Geom::Y]);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, 2, 0, M_PI*2);
    }{
        Point h = s.curve.get(ps).pointAt(ti.max());
        int x = int(h[Geom::X]);
        int y = int(h[Geom::Y]);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x, y, 2, 0, M_PI*2);
    }
    cairo_stroke(cr);
    delete curv;
}

struct NearPredicate {
    bool operator()(double x, double y) { return are_near(x, y); }
};

// ensures that f and t are elements of a vector, sorts and uniqueifies
// also asserts that no values fall outside of f and t
// if f is greater than t, the sort is in reverse
void process_splits(std::vector<double> &splits, double f, double t) {
    splits.push_back(f);
    splits.push_back(t);
    std::sort(splits.begin(), splits.end());
    std::vector<double>::iterator end = std::unique(splits.begin(), splits.end(), NearPredicate());
    splits.resize(end - splits.begin());
    if(f > t) std::reverse(splits.begin(), splits.end());
    for(unsigned i = 0; i < splits.size(); i++) cout << splits[i] << " ";
    cout << endl;
    //TODO: reenable these asserts
   // while(splits.front() != f) splits.erase(splits.begin());
    //while(splits.back() != t) splits.erase(splits.end() - 1);
    //assert(splits.front() == f);
    //assert(splits.back() == t);
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
    process_splits(splits, 0, 1);
    return splits;
}

std::vector<Section> mono_sections(std::vector<Path> const &ps) {
    std::vector<Section> monos;
    for(unsigned i = 0; i < ps.size(); i++) {
        //TODO: necessary? can we have empty paths?
        if(ps[i].size()) {
            for(unsigned j = 0; j < ps[i].size(); j++) {
                std::vector<double> splits = mono_splits(ps[i][j]);
                for(unsigned k = 1; k < splits.size(); k++) {
                   monos.push_back(Section(ps[i][j], X, CurveIx(i,j), splits[k-1], splits[k]));
                }
            }
        }
    }
    return monos;
}

//splits a section into bits, mutating it to represent the first bit, and returning the rest
std::vector<Section> split_section(Section &s, std::vector<Path> const &ps, std::vector<double> &cuts, Dim2 d) {
    std::vector<Section> ret;
    process_splits(cuts, s.f, s.t);
    Curve const &c = s.curve.get(ps);
    s.set_to(c, d, cuts[1]);
    for(unsigned i = 2; i < cuts.size(); i++) ret.push_back(Section(c, d, s.curve, cuts[i-1], cuts[i]));
    return ret;
}

bool lexo_point(Point const &a, Point const &b, Dim2 d) {
    if(d)
        return a[Y] < b[Y] || (a[Y] == b[Y] && a[X] < b[X]);
    else
        return a[X] < b[X] || (a[X] == b[X] && a[Y] < b[Y]);
}

class SweepSorter {
    Dim2 dim;
  public:
    //binary adapter typedefs
    typedef Section const& first_argument_type;
    typedef Section const& second_argument_type;
    typedef bool result_type;
    
    SweepSorter(Dim2 d) : dim(d) {}
    bool operator()(Section const &a, Section const &b) {
        return lexo_point(a.fp, b.fp, dim);
    }
};

double section_root(Section const &s, std::vector<Path> const &ps, double v, Dim2 d) {
    std::vector<double> roots = s.curve.get(ps).roots(v, d);
    for(unsigned j = 0; j < roots.size(); j++)
        if(Interval(s.f, s.t).contains(roots[j])) return roots[j];
    return -1;
}

class SectionSorter {
    const std::vector<Path> &ps;
    Dim2 dim;
    bool section_order(Section const &a, double at, Section const &b, double bt) {
        Point ap = a.curve.get(ps).pointAt(at);
        Point bp = b.curve.get(ps).pointAt(bt);
        if(are_near(ap[dim], bp[dim])) {
            if(a.tp[dim] < ap[dim] && b.tp[dim] > ap[dim]) return true;
            if(a.tp[dim] > ap[dim] && b.tp[dim] < ap[dim]) return false;
            //TODO: sampling / higher derivatives when unit tangents match
            Point ad = a.curve.get(ps).unitTangentAt(a.f);
            Point bd = b.curve.get(ps).unitTangentAt(b.f);
            // tangent can point backwards
            if(ad[1-dim] < 0) ad = -ad;
            if(bd[1-dim] < 0) bd = -bd;
            return ad[dim] < bd[dim];
        }
        return ap[dim] < bp[dim];
    }
  public:
    SectionSorter(const std::vector<Path> &rs, Dim2 d) : ps(rs), dim(d) {}
    bool operator()(Section const &a, Section const &b) {
        Rect ra = a.bbox(), rb = b.bbox();
        if(ra[dim].max() <= rb[dim].min()) return true;
        if(rb[dim].max() <= ra[dim].min()) return false;
        //we know that the rects intersect on dim
        //by referencing f / t we are assuming that the section was constructed with 1-dim
        if(ra[1-dim].intersects(rb[1-dim])) {
            if(are_near(a.fp[1-dim], b.fp[1-dim])) {
                return section_order(a, a.f > a.t ? a.f - 0.1 : a.f + 0.1,
                                     b, b.f > b.t ? b.f - 0.1 : b.f + 0.1);
            } else if(a.fp[1-dim] < b.fp[1-dim]) {
                //b inside a
                double ta = section_root(a, ps, b.fp[1-dim], Dim2(1-dim));
                //TODO: fix bug that necessitates this
                if(ta == -1) ta = (a.t + a.f) / 2;
                return section_order(a, ta, b, b.f);
            } else {
                //a inside b
                double tb = section_root(b, ps, a.fp[1-dim], Dim2(1-dim));
                //TODO: fix bug that necessitates this
                if(tb == -1) tb = (b.t + b.f) / 2;
                return section_order(a, a.f, b, tb);
            }
        }
        
        //assert(false);
        return false;
        //return a.fp < b.fp;
    }
};

void merge_monos(std::vector<Section> &monos, std::vector<Section> const &mergers, Dim2 d) {
  //TODO: there may be more efficient, more STLish ways of doing this
    //reserve to prevent iterator invalidation
    monos.reserve(monos.size() + mergers.size());
    std::vector<Section>::iterator it = monos.begin();
    for(unsigned i = 0; i < mergers.size(); i++) {
        it = std::lower_bound(it, monos.end(), mergers[i], SweepSorter(d));
        monos.insert(it, mergers[i]);
    }
}

template<typename T>
class SwapParams {
    T inner;
  public:
    typedef typename T::first_argument_type second_argument_type;
    typedef typename T::second_argument_type first_argument_type;
    typedef typename T::result_type result_type;
    SwapParams(T x) : inner(x) {}
    result_type operator()(first_argument_type x, second_argument_type y) { return inner(x, y); }
};

template<typename T>
typename T::value_type pop_it(T &x) {
    typename T::value_type ret = x.top();
    x.pop();
    return ret;
}

template<typename T>
void push_them(T &x, std::vector<typename T::value_type> const &xs) {
    for(unsigned i = 0; i < xs.size(); i++) x.push(xs[i]);
}

std::vector<std::vector<Section> > monoss;
std::vector<std::vector<Section> > chopss;
std::vector<std::vector<Section> > contexts;

std::vector<Section> sweep_window(std::vector<Path> const &ps) {
    //TODO: also store a priority queue of sectioned off bits, and do mergepass
    //the monotonic sections to process
    std::vector<Section> monos = mono_sections(ps);
    std::sort(monos.begin(), monos.end(), SweepSorter(X));
    
    //priority queue of chopped off sections
    std::priority_queue<Section, std::vector<Section>, SwapParams<SweepSorter> >
        chops((SwapParams<SweepSorter>(SweepSorter(X))));
    
    //the current operating context
    std::vector<Section> context;
    
    //the returned, output sections
    std::vector<Section> ret;
    
    unsigned mi = 0;
    while(mi < monos.size() || !chops.empty()) {
        //grab the next section, be it from chops or monos
        //if its from monos, mi, the index, is incremented
        Section s = (mi < monos.size() && (chops.empty() || lexo_point(monos[mi].fp, chops.top().fp, X))) ?
                      monos[mi++] : pop_it(chops);
        
        //iterate the contexts in reverse, looking for sections which are finished
        for(int i = context.size() - 1; i >= 0; i--) {
            if(context[i].tp[X] < s.fp[X] || are_near(context[i].tp, s.fp)) {
                //figure out this section's winding
                int wind = 0;
                for(int j = 0; j < i; j++) {
                    if(context[j].f < context[j].t) wind++;
                    if(context[j].f > context[j].t) wind--;
                }
                //add it to the output
                Section r = context[i];
                r.winding = wind;
                ret.push_back(r);
                //remove it from the context
                context.erase(context.begin() + i);
            }
        }
          
        //insert section into context, in the proper location
        unsigned context_ix = std::lower_bound(context.begin(), context.end(), s, SectionSorter(ps, Y)) - context.begin();
        context.insert(context.begin() + context_ix, s);
        
        // Now we intersect with neighbors - do a sweep!
        std::vector<Rect> sing;
        sing.push_back(s.bbox());
        std::vector<unsigned> others = sweep_bounds(sing, section_rects(context), Y)[0];
        for(unsigned i = 0; i < others.size(); i++) {
            if(others[i] == context_ix) continue;
            
            Section other = context[others[i]];
            
            Crossings xs = mono_intersect(s.curve.get(ps),     Interval(s.f, s.t),
                                          other.curve.get(ps), Interval(other.f, other.t));
            if(xs.empty()) continue;
            
            std::vector<double> tas, tbs;
            for(unsigned j = 0; j < xs.size(); j++) {
                tas.push_back(xs[j].ta);
                tbs.push_back(xs[j].tb);
            }
            
            push_them(chops, split_section(context[context_ix], ps, tas, X));
            push_them(chops, split_section(context[others[i]], ps, tbs, X));
        }
        std::vector<Section> rem;
        for(unsigned i = mi; i < monos.size(); i++) {
            rem.push_back(monos[i]);
        }
        monoss.push_back(rem);
        contexts.push_back(context);
    }
    
    return ret;
}

class SweepWindow: public Toy {
    vector<Path> path;
    std::vector<Toggle> toggles;
    PointHandle p;
    std::vector<colour> colours;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 2);
        
        monoss.clear();
        contexts.clear();
        chopss.clear();
        std::vector<Section> output = sweep_window(path);
        int cix = (int) p.pos[X] / 10;
        if(cix >= 0 && cix < (int)contexts.size()) {
            while(colours.size() < contexts[cix].size()) {
                double c = colours.size();
                colours.push_back(colour::from_hsl(c*0.5, 1, 0.5, 0.75));
            }
            char* buf = (char*)malloc(10);
            for(unsigned i = 0; i < contexts[cix].size(); i++) {
                cairo_set_source_rgba(cr, colours[i]);
                //cairo_set_line_width(cr, (i%2+1)*2);
                draw_section(cr, contexts[cix][i], path);
                sprintf(buf, "%i", i);
                draw_text(cr, contexts[cix][i].curve.get(path)
                              ((contexts[cix][i].t + contexts[cix][i].f) / 2), buf);
                cairo_stroke(cr);
            }
            free(buf);
            cairo_set_source_rgba(cr, 0,0,0,1);
            cairo_set_line_width(cr, 1);
            for(unsigned i = 0; i < monoss[cix].size(); i++) {
                draw_section(cr, monoss[cix][i], path);
                cairo_stroke(cr);
            }
        }

        //some marks to indicate section breaks
        for(unsigned i = 0; i < path.size(); i++) {
            for(unsigned j = 0; j < path[i].size(); j++) {
                //draw_cross(cr, path[i][j].initialPoint());
            }
        }
        
        *notify << cix << " "; //<< ixes[cix] << endl;
        
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    void mouse_pressed(GdkEventButton* e) {
        toggle_events(toggles, e);
        Toy::mouse_pressed(e);
    }
    
    void key_hit(GdkEventKey* e) {
        if(e->keyval == 'a') p.pos[X] = 0;
        else if(e->keyval == '[') p.pos[X] -= 10;
        else if(e->keyval == ']') p.pos[X] += 10;
        if (p.pos[X] < 0) {
            p.pos[X] = 0;
        }
        redraw();
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
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
