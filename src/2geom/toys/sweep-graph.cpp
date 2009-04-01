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

bool lexo_point(Point const &a, Point const &b, Dim2 d) {
    if(d)
        return a[Y] < b[Y] || (a[Y] == b[Y] && a[X] < b[X]);
    else
        return a[X] < b[X] || (a[X] == b[X] && a[Y] < b[Y]);
}

struct Section {
    CurveIx curve;
    double f, t;
    Point fp, tp;
    int winding;
    Section(Curve const &c, Dim2 d, CurveIx cix, double fd, double td) : curve(cix), f(fd), t(td) {
        fp = c.pointAt(f), tp = c.pointAt(t);
        if(lexo_point(tp, fp, d)) {
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

struct Edge {
    unsigned section, other;
    Edge(unsigned s, unsigned o) : section(s), other(o) {}
};

struct Vertex {
    std::vector<Edge> enters;
    std::vector<Edge> exits;
    Point avg;
    Vertex(Point p) : avg(p) {}
};

struct Graph {
    std::vector<Vertex> verts;
    std::vector<Section> edges;
    Graph(std::vector<Vertex> const &vs, std::vector<Section> const &es) : verts(vs), edges(es) {}
};

std::vector<Rect> section_rects(std::vector<Section> const &s) {
    std::vector<Rect> ret;
    for(unsigned i = 0; i < s.size(); i++) {
        ret.push_back(s[i].bbox());
    }
    return ret;
}

void draw_node(cairo_t *cr, Point h) {
    int x = int(h[Geom::X]);
    int y = int(h[Geom::Y]);
    cairo_new_sub_path(cr);
    cairo_arc(cr, x, y, 2, 0, M_PI*2);
}

void draw_section(cairo_t *cr, Section const &s, std::vector<Path> const &ps) {
    Interval ti = Interval(s.f, s.t);
    Curve *curv = s.curve.get(ps).portion(ti.min(), ti.max());
    cairo_curve(cr, *curv);
    draw_node(cr, s.curve.get(ps).pointAt(s.f));
    draw_node(cr, s.curve.get(ps).pointAt(s.t));
    cairo_stroke(cr);
    delete curv;
}

template<typename T>
struct NearPredicate {
    bool operator()(T x, T y) { return are_near(x, y); }
};

// ensures that f and t are elements of a vector, sorts and uniqueifies
// also asserts that no values fall outside of f and t
// if f is greater than t, the sort is in reverse
void process_splits(std::vector<double> &splits, double f, double t) {
    splits.push_back(f);
    splits.push_back(t);
    std::sort(splits.begin(), splits.end());
    std::vector<double>::iterator end = std::unique(splits.begin(), splits.end(), NearPredicate<double>());
    splits.resize(end - splits.begin());
    if(f > t) std::reverse(splits.begin(), splits.end());

    //remove any splits which fall outside t / f
    while(!splits.empty() && splits.front() != f) splits.erase(splits.begin());
    while(!splits.empty() && splits.back() != t) splits.erase(splits.end() - 1);
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
//TODO: output iterator?
std::vector<Section> split_section(Section &s, std::vector<Path> const &ps, std::vector<double> &cuts, Dim2 d) {
    std::vector<Section> ret;
    process_splits(cuts, s.f, s.t);
    Curve const &c = s.curve.get(ps);
    s.set_to(c, d, cuts[1]);
    for(unsigned i = 2; i < cuts.size(); i++) ret.push_back(Section(c, d, s.curve, cuts[i-1], cuts[i]));
    return ret;
}

//sorter which sorts sections for the monos heap
class HeapSorter {
    Dim2 dim;
  public:
    HeapSorter(Dim2 d) : dim(d) {}
    bool operator()(Section const &a, Section const &b) {
        return lexo_point(b.fp, a.fp, dim);
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
            // since the sections are monotonic, if the endpoints are on opposite sides of this
            // coincidence, the order is determinable
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
        if(&a == &b) return false;
        Rect ra = a.bbox(), rb = b.bbox();
        if(ra[dim].max() <= rb[dim].min()) return true;
        if(rb[dim].max() <= ra[dim].min()) return false;
        //we know that the rects intersect on dim
        //by referencing f / t we are assuming that the section was constructed with 1-dim
        if(ra[1-dim].intersects(rb[1-dim])) {
            if(are_near(a.fp[1-dim], b.fp[1-dim])) {
                return section_order(a, a.f > a.t ? a.f - 0.01 : a.f + 0.01,
                                     b, b.f > b.t ? b.f - 0.01 : b.f + 0.01);
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
        
        return lexo_point(a.fp, b.fp, dim);
    }
};

template<typename T, typename Z>
void push_them(T &x, typename T::iterator &it, std::vector<typename T::value_type> const &xs, Z const &z) {
    x.reserve(it - x.begin() + xs.size());
    for(unsigned i = 0; i < xs.size(); i++) {
        *(it++) = xs[i];
        std::push_heap(x.begin(), it, z);
    }
}

//TODO: make this faster than linear
unsigned find_vertex(std::vector<Vertex> &vertices, Point p) {
    unsigned i = 0;
    for(; i < vertices.size(); i++)
        if(are_near(vertices[i].avg, p)) break;
    if(i == vertices.size()) vertices.push_back(Vertex(p));
    return i;
}

//moves all the sections we're done with from the context to the output
//helper for sweep_graph, passes in a lot of sweep_graph locals
void move_to_output(std::vector<Section> &context, std::vector<Section> &sections,
                    std::vector<Vertex> &vertices, std::vector<unsigned> &vix, double v) {
    //iterate the contexts in reverse, looking for sections which are finished
    for(int i = context.size() - 1; i >= 0; i--) {
        if(context[i].tp[X] < v || are_near(context[i].tp[X], v)) {
            //figure out this section's winding
            int wind = 0;
            for(int j = 0; j < i; j++) {
                if(context[j].fp[X] == context[j].tp[X]) continue;
                if(context[j].f < context[j].t) wind++;
                if(context[j].f > context[j].t) wind--;
            }
            //add it to the output
            Section r = context[i];
            r.winding = wind;
            sections.push_back(r);
            //add it to vertices
            unsigned vert = find_vertex(vertices, r.tp);
            vertices[vix[i]].exits.push_back(Edge(sections.size()-1, vert));
            vertices[vert].enters.push_back(Edge(sections.size()-1, vix[i]));
            //remove it from the context
            context.erase(context.begin() + i);
            vix.erase(vix.begin() + i);
        }
    }
}

std::vector<std::vector<Section> > monoss;
std::vector<std::vector<Section> > chopss;
std::vector<std::vector<Section> > contexts;

Graph sweep_graph(std::vector<Path> const &ps, Dim2 d = X, double tol = 1) {
    SectionSorter s_sort = SectionSorter(ps, (Dim2)(1-d));
    HeapSorter heap_sort = HeapSorter(d);
    
    //context = the current operating context
    //sections = the returned, output sections
    //monos = a heap of monotonic sections to process
    std::vector<Section> context, sections, monos = mono_sections(ps);
    
    std::make_heap(monos.begin(), monos.end(), heap_sort);
    std::vector<Section>::iterator monos_end = monos.end();
    
    //index of the start vertices of each context member
    std::vector<unsigned> vix;
    
    //the returned, output vertices
    std::vector<Vertex> vertices;
    
    while(monos_end != monos.begin()) {
        std::pop_heap(monos.begin(), monos_end, heap_sort);
        monos_end--;
        Section s = *monos_end;
        
        move_to_output(context, sections, vertices, vix, s.fp[d]);
        
        //insert section into context, in the proper location
        unsigned context_ix = std::lower_bound(context.begin(), context.end(), s, s_sort) - context.begin();
        context.insert(context.begin() + context_ix, s);
        vix.insert(vix.begin() + context_ix, find_vertex(vertices, s.fp));
        
        Interval si = Interval(s.fp[1-d], s.tp[1-d]);
        // Now we intersect with neighbors - do a sweep!
        std::vector<double> this_splits;
        for(unsigned i = 0; i < context.size(); i++) {
            if(i == context_ix) continue;
            
            if(si.intersects(Interval(context[i].fp[1-d], context[i].tp[1-d]))) {
                std::vector<double> other_splits;
                Crossings xs = mono_intersect(s.curve.get(ps), Interval(s.f, s.t),
                                              context[i].curve.get(ps), Interval(context[i].f, context[i].t));
                for(unsigned j = 0; j < xs.size(); j++) {
                    this_splits.push_back(xs[j].ta);
                    other_splits.push_back(xs[j].tb);
                }
                
                push_them(monos, monos_end, split_section(context[i], ps, other_splits, d), heap_sort);
            }
        }
        push_them(monos, monos_end, split_section(context[context_ix], ps, this_splits, d), heap_sort);
        
        std::vector<Section> rem(monos.begin(), monos_end);
        std::sort_heap(rem.begin(), rem.end(), heap_sort);
        monoss.push_back(rem);
        contexts.push_back(context);
    }
    
    //move the rest to output.
    move_to_output(context, sections, vertices, vix, 1.0 / 0.0);
    
    return Graph(vertices, sections);
}

void draw_graph(cairo_t *cr, std::vector<Vertex> vertices) {
    for(unsigned i = 0; i < vertices.size(); i++) {
        std::cout << i << " " << vertices[i].avg << " [";
        cairo_set_source_rgba(cr, colour::from_hsl(i*0.5, 1, 0.5, 0.75));
        for(unsigned j = 0; j < vertices[i].enters.size(); j++) {
            draw_ray(cr, vertices[i].avg, 10*unit_vector(vertices[vertices[i].enters[j].other].avg - vertices[i].avg));
            cairo_stroke(cr);
            std::cout << vertices[i].enters[j].other << ", ";
        }
        for(unsigned j = 0; j < vertices[i].exits.size(); j++) {
            draw_ray(cr, vertices[i].avg, 20*unit_vector(vertices[vertices[i].exits[j].other].avg - vertices[i].avg));
            cairo_stroke(cr);
            std::cout << vertices[i].exits[j].other << ", ";
        }
        //draw_number(cr, vertices[i].avg + Point(uniform() * 5, uniform() * 5), (unsigned)vertices[i].edges.size());
        std::cout << "]\n";
    }
    std::cout << "=======\n";
}

void fill(std::vector<Section> const &sections, std::vector<Vertex> const &vertices) {
    
}

class SweepWindow: public Toy {
    vector<Path> path;
    std::vector<Toggle> toggles;
    PointHandle p;
    std::vector<colour> colours;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_set_line_width(cr, 3);

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 3);
        monoss.clear();
        contexts.clear();
        chopss.clear();
        Graph output = sweep_graph(path);
        int cix = (int) p.pos[X] / 10;
        if(cix >= 0 && cix < (int)contexts.size()) {
            while(colours.size() < contexts[cix].size()) {
                double c = colours.size();
                colours.push_back(colour::from_hsl(c*0.5, 1, 0.5, 0.75));
            }
            for(unsigned i = 0; i < contexts[cix].size(); i++) {
                cairo_set_source_rgba(cr, colours[i]);
                draw_section(cr, contexts[cix][i], path);
                draw_number(cr, contexts[cix][i].curve.get(path)
                                ((contexts[cix][i].t + contexts[cix][i].f) / 2), i);
                cairo_stroke(cr);
            }
            cairo_set_source_rgba(cr, 0,0,0,1);
            //cairo_set_line_width(cr, 1);
            for(unsigned i = 0; i < monoss[cix].size(); i++) {
                draw_section(cr, monoss[cix][i], path);
                cairo_stroke(cr);
            }
        }
        
        *notify << cix << " "; //<< ixes[cix] << endl;
        /*
        cairo_set_line_width(cr, 1);
        for(unsigned i = 0; i < output.size(); i++) {
            draw_number(cr, output[i].curve.get(path)
                            ((output[i].t + output[i].f) / 2), output[i].winding, "", false);
            cairo_stroke(cr);
            //if(abs((output[i].winding + (output[i].f > output[i].t ? 1 : 0)) % 2) == (toggles[0].on ? 0 : 1)) draw_section(cr, output[i], path);
            cairo_stroke(cr);
        }
        
        std::vector<Vertex> vertices = section_graph(output, 1);
        draw_graph(cr, vertices);
        
        uncross(output, vertices);
        */
        
        draw_graph(cr, output.verts);
        
       // draw_toggles(cr, toggles);
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
