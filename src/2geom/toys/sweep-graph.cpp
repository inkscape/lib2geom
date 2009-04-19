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
    std::vector<int> windings;
    Section(CurveIx cix, double fd, double td, std::vector<Path> ps, Dim2 d) : curve(cix), f(fd), t(td) {
        fp = curve.get(ps).pointAt(f), tp = curve.get(ps).pointAt(t);
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
    Curve *get_portion(std::vector<Path> const &ps) const {
        Interval ti(f, t);
        return curve.get(ps).portion(ti.min(), ti.max());
    }
};

struct Edge {
    Section const *section;
    unsigned other;
    Edge(Section const *s, unsigned o) : section(s), other(o) {}
};

struct Vertex {
    std::vector<Edge> enters;
    std::vector<Edge> exits;
    Point avg;
    Vertex(Point p) : avg(p) {}
    
    unsigned degree() const { return enters.size() + exits.size(); }
    
    Edge const &lookup(unsigned &i) const {
        i = i % degree();
        return i < enters.size() ? enters[i] : exits[i - enters.size()];
    }
    
    unsigned find(Section const *sect) const {
        for(unsigned i = 0; i < enters.size(); i++)
            if(enters[i].section == sect) return i;
        for(unsigned i = 0; i < exits.size(); i++)
            if(exits[i].section == sect) return i + enters.size();
        return degree();
    }
};

typedef std::vector<Vertex> Graph;

void free_graph(Graph const &g) {
    for(unsigned i = 0; i < g.size(); i++)
        for(unsigned j = 0; j < g[i].exits.size(); j++)
            delete g[i].exits[j].section;
}

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
    Curve *curv = s.get_portion(ps);
    cairo_curve(cr, *curv);
    draw_node(cr, curv->initialPoint());
    draw_node(cr, curv->finalPoint());
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

std::vector<Section> mono_sections(std::vector<Path> const &ps, Dim2 d) {
    std::vector<Section> monos;
    for(unsigned i = 0; i < ps.size(); i++) {
        //TODO: necessary? can we have empty paths?
        if(ps[i].size()) {
            for(unsigned j = 0; j < ps[i].size(); j++) {
                std::vector<double> splits = mono_splits(ps[i][j]);
                for(unsigned k = 1; k < splits.size(); k++) {
                   monos.push_back(Section(CurveIx(i,j), splits[k-1], splits[k], ps, d));
                }
            }
        }
    }
    return monos;
}

double section_root(Section const &s, std::vector<Path> const &ps, double v, Dim2 d) {
    std::vector<double> roots = s.curve.get(ps).roots(v, d);
    for(unsigned j = 0; j < roots.size(); j++)
        if(Interval(s.f, s.t).contains(roots[j])) return roots[j];
    return -1;
}

class SectionSorter {
    const std::vector<Path> &ps;
    Dim2 dim;
    double tol;
    bool section_order(Section const &a, double at, Section const &b, double bt) const {
        Point ap = a.curve.get(ps).pointAt(at);
        Point bp = b.curve.get(ps).pointAt(bt);
        if(are_near(ap[dim], bp[dim], tol)) {
            // since the sections are monotonic, if the endpoints are on opposite sides of this
            // coincidence, the order is determinable
            if(a.tp[dim] < ap[dim] && b.tp[dim] > bp[dim]) return true;
            if(a.tp[dim] > ap[dim] && b.tp[dim] < bp[dim]) return false;
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
    typedef Section first_argument_type;
    typedef Section second_argument_type;
    typedef bool result_type;
  
    SectionSorter(const std::vector<Path> &rs, Dim2 d, double t) : ps(rs), dim(d), tol(t) {}
    bool operator()(Section const &a, Section const &b) const {
        if(&a == &b) return false;
        Rect ra = a.bbox(), rb = b.bbox();
        //TODO: should we use tol in these conditions?
        if(ra[dim].max() <= rb[dim].min()) return true;
        if(rb[dim].max() <= ra[dim].min()) return false;
        //we know that the rects intersect on dim
        //by referencing f / t we are assuming that the section was constructed with 1-dim
        if(ra[1-dim].intersects(rb[1-dim])) {
            if(are_near(a.fp[1-dim], b.fp[1-dim], tol)) {
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

//splits a section into bits, mutating it to represent the first bit, and returning the rest
//TODO: output iterator?
std::vector<Section*> split_section(Section *s, std::vector<Path> const &ps, std::vector<double> &cuts, Dim2 d) {
    std::vector<Section*> ret;
    
    process_splits(cuts, s->f, s->t);
    if(cuts.size() <= 2) return ret;
    Curve const &c = s->curve.get(ps);
    s->set_to(c, d, cuts[1]);
    
    ret.reserve(cuts.size() - 2);
    for(int i = cuts.size() - 1; i > 1; i--) ret.push_back(new Section(s->curve, cuts[i-1], cuts[i], ps, d));
    return ret;
}

template<typename X, typename Z>
void push_them(std::vector<X> &c, std::vector<X> const &xs, Z const &z) {
     c.reserve(c.size() + xs.size());
     typename std::vector<X>::iterator it = c.begin();
     for(unsigned i = 0; i < xs.size(); i++) {
         it = std::lower_bound(it, c.end(), xs[i], z);
         c.insert(it, xs[i]);
     }
}

//TODO: make this faster than linear
unsigned get_vertex(std::vector<Vertex> &vertices, std::vector<unsigned> &v_to_do, Point p, double tol) {
    unsigned i = 0;
    for(; i < vertices.size(); i++)
        if(are_near(vertices[i].avg, p, tol)) break;
    if(i == vertices.size()) {
        v_to_do.push_back(vertices.size());
        vertices.push_back(Vertex(p));
    }
    return i;
}

template<typename T>
std::vector<T> deref_vector(std::vector<T*> const &xs, unsigned from = 0) {
    std::vector<T> ret;
    ret.reserve(xs.size() - from);
    for(unsigned i = from; i < xs.size(); i++)
        ret.push_back(T(*xs[i]));
    return ret;
}

struct SweepSorter {
    typedef Section first_argument_type;
    typedef Section second_argument_type;
    typedef bool result_type;
    Dim2 dim;
    SweepSorter(Dim2 d) : dim(d) {}
    bool operator()(const Section &a, const Section &b) const { return lexo_point(a.fp, b.fp, dim); }
};

template<typename C>
struct ReverseAdapter {
    typedef typename C::second_argument_type first_argument_type;
    typedef typename C::first_argument_type second_argument_type;
    typedef typename C::result_type result_type;
    const C &comp;
    ReverseAdapter(const C &c) : comp(c) {}
    result_type operator()(const first_argument_type &a, const second_argument_type &b) const { return comp(b, a); }
};

template<typename C>
struct DerefAdapter {
    typedef typename C::first_argument_type* first_argument_type;
    typedef typename C::second_argument_type* second_argument_type;
    typedef typename C::result_type result_type;
    const C &comp;
    DerefAdapter(const C &c) : comp(c) {}
    result_type operator()(const first_argument_type a, const second_argument_type b) const {
        if(!a) return false;
        if(!b) return true;
        return comp(*a, *b);
    }
};

std::vector<std::vector<Section> > monoss;
std::vector<std::vector<Section> > chopss;
std::vector<std::vector<Section> > contexts;

Graph sweep_graph(std::vector<Path> const &ps, Dim2 d = X, double tol = 0.00001) {
    //s_sort = vertical section order
    DerefAdapter<SectionSorter> s_sort = DerefAdapter<SectionSorter>(SectionSorter(ps, (Dim2)(1-d), tol));
    //sweep_sort = horizontal sweep order
    DerefAdapter<SweepSorter> sweep_sort = DerefAdapter<SweepSorter>(SweepSorter(d));
    //heap_sort = reverse horizontal sweep order
    ReverseAdapter<DerefAdapter<SweepSorter> > heap_sort = ReverseAdapter<DerefAdapter<SweepSorter> >(sweep_sort);
    
    //sections = input monotonic sections, sorted on sweep_sort
    //chops = portions of sections yielded by intersection handling, sorted on heap_sort
    //context = the current operating context, sorted on s_sort
    std::vector<Section> sections = mono_sections(ps, d);
    std::vector<Section*> chops, context;
    std::sort(sections.begin(), sections.end(), SweepSorter(d));
    
    //vix = index of the start vertices of each context member
    //v_to_do = list of vertexes which need to be finalized
    std::vector<unsigned> vix, v_to_do;
    
    //the returned, output vertices
    std::vector<Vertex> vertices;
    
    //vector used to hold temporary windings vectors
    std::vector<int> windings(ps.size(), 0);
    
    //TODO: preprocess input segs, and remove everything that's too small?
    //  vertex finalization stuff relies on this a bit, probably erroneously.
    
    //TODO: figure out how to handle vertical / horizontal exits with vertex finalization
    
    //TODO: allocate one big block of memory for section data?
    
    unsigned mix = 0;
    while(mix < sections.size() || !chops.empty() || !v_to_do.empty()) {
        //mergepass of monos and chops
        Section* s = NULL;
        if(mix < sections.size() && (chops.empty() || lexo_point(sections[mix].fp, chops.back()->fp, d))) {
            s = new Section(sections[mix++]);
        } else if(!chops.empty()) {
            s = chops.back();
            chops.pop_back();
        }
        
        double lim = s ? s->fp[d] : 1.0 / 0.0;
        
        //process all of the vertices we're done with
        for(unsigned i = 0; i < v_to_do.size(); i++) {
            if(vertices[v_to_do[i]].avg[d] + tol <= lim) {
                for(unsigned j = 0; j < vix.size(); j++)
                    if(vix[j] == v_to_do[i]) vertices[vix[j]].exits.push_back(Edge(context[j], 0));
                v_to_do.erase(v_to_do.begin() + i);
                i--;
            }
        }
        
        //move all of the segments were done with
        for(int i = context.size() - 1; i >= 0; i--) {
            if(context[i]->tp[d] < lim || are_near(context[i]->tp[d], lim)) {
                //figure out this section's winding
                std::fill(windings.begin(), windings.end(), 0);
                for(int j = 0; j < i; j++) {
                    Section *sec = context[j];
                    unsigned k = sec->curve.path;
                    if(k >= windings.size()) windings.resize(k+1);
                    if(sec->fp[d] == sec->tp[d]) continue;
                    if(sec->f < sec->t) windings[k]++;
                    if(sec->f > sec->t) windings[j]--;
                }
                
                context[i]->windings = windings;
                
                unsigned vert = get_vertex(vertices, v_to_do, context[i]->tp, tol);
                
                //TODO: erase section?
                //if(vert == vix[i]) continue;  // remove tiny things
                
                //add edges to vertices
                for(unsigned j = 0; j < vertices[vix[i]].exits.size(); j++) {
                    if(vertices[vix[i]].exits[j].section == context[i]) {
                        vertices[vix[i]].exits[j].other = vert;
                        break;
                    }
                }
                vertices[vert].enters.push_back(Edge(context[i], vix[i]));
                
                //remove it from the context
                context.erase(context.begin() + i);
                vix.erase(vix.begin() + i);
            }
        }
        
        
        if(s) {
            //insert section into context, in the proper location
            unsigned context_ix = std::lower_bound(context.begin(), context.end(), s, s_sort) - context.begin();
            context.insert(context.begin() + context_ix, s);
            vix.insert(vix.begin() + context_ix, get_vertex(vertices, v_to_do, s->fp, tol));
            
            Interval si = Interval(s->fp[1-d], s->tp[1-d]);
            
            // Now we intersect with neighbors - do a sweep!
            std::vector<double> this_splits;
            for(unsigned i = 0; i < context.size(); i++) {
                if(i == context_ix) continue;
                
                Section *sec = context[i];
                if(si.intersects(Interval(sec->fp[1-d], sec->tp[1-d]))) {
                    std::vector<double> other_splits;
                    Crossings xs = mono_intersect(s->curve.get(ps), Interval(s->f, s->t),
                                                  sec->curve.get(ps), Interval(sec->f, sec->t));
                    
                    if(!xs.empty()) {
                        for(unsigned j = 0; j < xs.size(); j++) {
                            this_splits.push_back(xs[j].ta);
                            other_splits.push_back(xs[j].tb);
                        }
                        
                        push_them(chops, split_section(sec, ps, other_splits, d), heap_sort);
                    }
                }
            }
            if(!this_splits.empty())
                push_them(chops, split_section(context[context_ix], ps, this_splits, d), heap_sort);
            std::sort(chops.begin(), chops.end(), heap_sort);
        }
        
//      std::vector<Section> rem;
//      for(unsigned i = mix; i < sections.size(); i++) rem.push_back(Section(sections[i]));
//      monoss.push_back(rem);
        chopss.push_back(deref_vector(chops));
        contexts.push_back(deref_vector(context));
    }
    
    for(unsigned i = 0; i < v_to_do.size(); i++)
        for(unsigned j = 0; j < vix.size(); j++)
            if(vix[j] == v_to_do[i]) vertices[vix[j]].exits.push_back(Edge(context[j], 0));
    
    return vertices;
}

void draw_graph(cairo_t *cr, std::vector<Vertex> const &vertices) {
    for(unsigned i = 0; i < vertices.size(); i++) {
        std::cout << i << " " << vertices[i].avg << " [";
        cairo_set_source_rgba(cr, colour::from_hsl(i*0.5, 1, 0.5, 0.75));
        for(unsigned j = 0; j < vertices[i].enters.size(); j++) {
            draw_ray(cr, vertices[i].avg, 10*unit_vector(vertices[vertices[i].enters[j].other].avg - vertices[i].avg));
            cairo_stroke(cr);
            std::cout << vertices[i].enters[j].section << "_" << vertices[i].enters[j].other << ", ";
        }
        std::cout  << "|";
        for(unsigned j = 0; j < vertices[i].exits.size(); j++) {
            draw_ray(cr, vertices[i].avg, 20*unit_vector(vertices[vertices[i].exits[j].other].avg - vertices[i].avg));
            cairo_stroke(cr);
            std::cout << vertices[i].exits[j].section << "_" << vertices[i].exits[j].other << ", ";
        }
        std::cout << "]\n";
    }
    std::cout << "=======\n";
}

void draw_edges(cairo_t *cr, std::vector<Edge> const &edges, std::vector<Path> const &ps, double t) {
    for(unsigned j = 1; j < edges.size(); j++) {
        const Section * const s1 = edges[j-1].section,
                *s2 = edges[j].section;
        Point fp = s1->curve.get(ps)(lerp(t, s1->f, s1->t));
        Point tp = s2->curve.get(ps)(lerp(t, s2->f, s2->t));
        draw_ray(cr, fp, tp - fp);
        cairo_stroke(cr);
    }
}

void draw_edge_orders(cairo_t *cr, std::vector<Vertex> const &vertices, std::vector<Path> const &ps) {
    for(unsigned i = 0; i < vertices.size(); i++) {
        cairo_set_source_rgba(cr, colour::from_hsl(i*0.5, 1, 0.5, 0.75));
        draw_edges(cr, vertices[i].enters, ps, 0.6);
        draw_edges(cr, vertices[i].exits, ps, 0.4);
    }
}

//planar area finding
std::vector<std::vector<const Section*> > traverse_areas(Graph const &g, std::vector<std::vector<std::vector<bool> > > &visiteds) {
    std::vector<std::vector<const Section*> > ret;
    
    //stores which edges we've visited
    std::vector<std::vector<bool> > visited;
    for(unsigned i = 0; i < g.size(); i++) visited.push_back(std::vector<bool>(g[i].degree(), false));
    
    for(unsigned vix = 0; vix < g.size(); vix++) {
        while(true) {
            //find an unvisited edge to start on
            
            unsigned e_ix = std::find(visited[vix].begin(), visited[vix].end(), false) - visited[vix].begin();
            if(e_ix == g[vix].degree()) break;
            Edge e = g[vix].lookup(e_ix);
            
            unsigned start = e_ix;
            unsigned cur = vix;
            
            std::vector<const Section*> area;
            //std::vector<std::vector<bool> > before(visited);
            while(!visited[cur][e_ix]) {
                visited[cur][e_ix] = true;
                
                area.push_back(e.section);
                
                //go to clockwise edge
                cur = e.other;
                e_ix = g[cur].find(e.section);
                if(g[cur].degree() == 1) {
                   visited[cur][e_ix] = true;
                   break;
                }
                assert(e_ix != g[cur].degree());
                e = g[cur].lookup(++e_ix);
                
                if(cur == vix && start == e_ix) break;
            }
            //if(vix == cur && start == e_ix) {
                ret.push_back(area);
                visiteds.push_back(visited);
            //} else visited = before;
        }
    }
    return ret;
}

Path sectionsToPath(std::vector<Path> const &ps, std::vector<const Section*> const & sections) {
    Path ret;
    for(unsigned i = 0; i < sections.size(); i++) {
        Interval ti(sections[i]->f, sections[i]->t);
        Curve *curv = sections[i]->curve.get(ps).portion(ti.min(), ti.max());
        ret.insert(ret.end(), *curv);
        delete(curv);
    }
    return ret;
}

enum {
  BOOLOP_JUST_A  = 1,
  BOOLOP_JUST_B  = 2,
  BOOLOP_BOTH    = 4,
  BOOLOP_NEITHER = 8
};

enum {
  BOOLOP_NULL         = 0,
  BOOLOP_INTERSECT    = BOOLOP_BOTH,
  BOOLOP_SUBTRACT_A_B = BOOLOP_JUST_B,
  BOOLOP_IDENTITY_A   = BOOLOP_JUST_A | BOOLOP_BOTH,
  BOOLOP_SUBTRACT_B_A = BOOLOP_JUST_A,
  BOOLOP_IDENTITY_B   = BOOLOP_JUST_B | BOOLOP_BOTH,
  BOOLOP_EXCLUSION    = BOOLOP_JUST_A | BOOLOP_JUST_B,
  BOOLOP_UNION        = BOOLOP_JUST_A | BOOLOP_JUST_B | BOOLOP_BOTH
};

//std::vector<Path>
std::vector<std::vector<Curve*> > unio(std::vector<Path> const &p1, bool nz1, std::vector<Path> const &p2, bool nz2) {
    std::vector<Path> acc(p1);
    append(acc, p2);
    Graph g = sweep_graph(acc);
    
    std::vector<std::vector<std::vector<bool> > > visits;
    std::vector<std::vector<const Section*> > areas = traverse_areas(g, visits);
    std::vector<std::vector<Curve*> > ret;
    SweepSorter sort = SweepSorter(Y);
    for(unsigned i = 0; i < areas.size(); i++) {
        //find the maximal section
        const Section *rep = areas[i][0];
        for(unsigned j = 1; j < areas[i].size(); j++)
            if(sort(*rep, *areas[i][j])) rep = areas[i][j];  
        //const Section &rep = **std::max_element(areas[i].begin(), areas[i].end(), sort);
        
        int w1 = 0, w2 = 0;
        for(unsigned j = 0; j < p1.size(); j++) w1 += (*rep).windings[j];
        for(unsigned j = p1.size(); j < acc.size(); j++) w2 += (*rep).windings[j];
        bool on1 = (nz1 ? w1 : w1 % 2) != 0,
              on2 = (nz2 ? w2 : w2 % 2) != 0;
        if(on1 && on2) {
            std::vector<Curve*> area_curves;
            for(unsigned j = 0; j < areas[i].size(); j++)
                area_curves.push_back(areas[i][j]->get_portion(acc));
            ret.push_back(area_curves);
        }
    }
    return ret;
}

class SweepWindow: public Toy {
    vector<Path> path, path2;
    std::vector<Toggle> toggles;
    PointHandle p, p2;
    std::vector<colour> colours;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 3);

        monoss.clear();
        contexts.clear();
        chopss.clear();
        
        Graph output = sweep_graph(path,Y);
        
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
            /* for(unsigned i = 0; i < monoss[cix].size(); i++) {
                draw_section(cr, monoss[cix][i], path);
                cairo_fill(cr);
            } */
            /* cairo_set_source_rgba(cr,0,0,0,0.5);
            cairo_set_line_width(cr, 10);
            std::cout << "!!!!!!!!!!! " << chopss[cix].size() << std::endl;
            for(unsigned i = 0; i < chopss[cix].size(); i++) {
                draw_section(cr, chopss[cix][i], path);
                cairo_stroke(cr);
            } */
        }
        
        *notify << cix << std::endl;
        
        //draw_graph(cr, output);
        
        cairo_set_line_width(cr, 1);
        draw_edge_orders(cr, output, path);
        
        std::vector<std::vector<std::vector<bool> > > visiteds;
        
        std::vector<std::vector<const Section*> > areas = traverse_areas(output, visiteds);
        /*for(unsigned i = 0; i < areas.size(); i++) {
            for(unsigned j = 0; j < areas[i].size(); j++) {
                std::cout << areas[i][j] << ", ";
            }
            std::cout << std::endl;
        }*/
        
        /*std::vector<std::vector<Curve*> > curves = unio(path, true, path2 + p2.pos, true);
        
        for(unsigned i = 0; i < curves.size(); i++) {
            for(unsigned j = 0; j < curves[i].size(); j++) {
                cairo_curve(cr, *curves[i][j]);
                cairo_stroke(cr);
            }
        } */
        
        
        unsigned area_ix = cix < (int)areas.size() ? cix : areas.size() - 1;
        cairo_set_line_width(cr, 5);
        cairo_set_source_rgba(cr, 1, 1,0,1);
        for(unsigned i = 0; i < areas[area_ix].size(); i++) {
            draw_section(cr, *areas[area_ix][i], path);
        }
        
        cairo_set_line_width(cr, 1);
        cairo_set_source_rgba(cr, 0, 0, 0, 1);
        for(unsigned i = 0; i < visiteds[area_ix].size(); i++) {
            for(unsigned j = 0; j < visiteds[area_ix][i].size(); j++) {
                if(visiteds[area_ix][i][j]) {
                    const Section *s = output[i].lookup(j).section;
                    draw_ray(cr, output[i].avg, lerp(0.5, s->curve.get(path)(lerp(0.35, s->f, s->t)) - output[i].avg, Point()));
                    cairo_stroke(cr);
                }
            }
        }
        
        free_graph(output);
        
        /*
        std::vector<Edge> sects = fill(output, X);
        for(unsigned i = 0; i < sects.size(); i++) {
            cairo_stroke(cr);
            Section s = output.sections[sects[i].section];
            draw_section(cr, s, path);
            draw_number(cr, s.curve.get(path)((s.f + s.t) / 2), i);
        } */
        
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
        const char *path_name="sanitize_examples.svgd",
                     *path2_name="sanitize_examples.svgd";
        if(argc > 1)
            path_name = argv[1];
        if(argc > 2)
            path2_name = argv[2];
        path = read_svgd(path_name); //* Scale(3);
        path2 = read_svgd(path2_name);
        OptRect bounds = bounds_exact(path);
        if(bounds) path += Point(10,10)-bounds->min();
        bounds = bounds_exact(path2);
        if(bounds) path2 += Point(10,10)-bounds->min();
        p = PointHandle(Point(100,300));
        handles.push_back(&p);
        p2 = PointHandle(Point(200,300));
        handles.push_back(&p2);
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
