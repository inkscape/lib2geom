#include <2geom/toposweep.h>

#include <2geom/path-intersection.h>
#include <2geom/basic-intersection.h>

//using namespace Geom;

namespace Geom {

/*
unsigned Vertex::find(Section const *sect) const {
    for(unsigned i = 0; i < enters.size(); i++)
        if(enters[i].section == sect) return i;
    for(unsigned i = 0; i < exits.size(); i++)
        if(exits[i].section == sect) return i + enters.size();
    return degree();
}

Edge &Vertex::lookup_section(Section const *sect) {
    for(unsigned i = 0; i < enters.size(); i++)
        if(enters[i].section == sect) return enters[i];
    for(unsigned i = 0; i < exits.size(); i++)
        if(exits[i].section == sect) return exits[i];
    assert(false);
}

void Vertex::remove_edge(unsigned &i) {
    //TODO: make this an assert?
    if(degree()) {
        i %= degree();
        if(i < enters.size())
            enters.erase(enters.begin() + i);
        else
            exits.erase(exits.begin() + (i - enters.size()));
    }
} */

TopoGraph::Edge &TopoGraph::get_edge(unsigned ix, unsigned jx) {
    Vertex &v = vertices[ix];
    jx %= vertices[ix].degree();
    return edges[jx < v.enters.size() ? v.enters[jx] : v.exits[jx - v.enters.size()]];
}

TopoGraph::Edge TopoGraph::get_edge(unsigned ix, unsigned jx) const {
    Vertex const &v = vertices[ix];
    jx %= v.degree();
    if(jx < v.enters.size())
        return edges[v.enters[jx]];
    else
        return edges[v.exits[jx - v.enters.size()]];
}

void TopoGraph::cannonize() {
    std::vector<unsigned> vix;
    std::vector<Vertex> vs;
    std::vector<bool> e_used(edges.size(), false);
    unsigned ix = 0;
    for(unsigned i = 0; i < vertices.size(); i++) {
        vix.push_back(ix);
        if(vertices[i].degree() != 0) {
            for(unsigned j = 0; j < vertices[i].enters.size(); j++)
                e_used[vertices[i].enters[j]] = true;
            for(unsigned j = 0; j < vertices[i].exits.size(); j++)
                e_used[vertices[i].exits[j]] = true;
            vs.push_back(vertices[i]);
            ix++;
        }
    }
    
    std::vector<unsigned> eix;
    std::vector<Edge> es;
    ix = 0;
    for(unsigned i = 0; i < e_used.size(); i++) {
        eix.push_back(ix);
        if(e_used[i]) {
            es.push_back(edges[i]);
            ix++;
        }
    }
    
    for(unsigned i = 0; i < es.size(); i++) {
        es[i].other_edge = eix[es[i].other_edge];
        es[i].other_vert = vix[es[i].other_vert];
    }
    
    for(unsigned i = 0; i < vs.size(); i++) {
        for(unsigned j = 0; j < vs[i].exits.size(); j++)
            vs[i].exits[j] = eix[vs[i].exits[j]];
        for(unsigned j = 0; j < vs[i].enters.size(); j++)
            vs[i].enters[j] = eix[vs[i].enters[j]];
    }
    
    vertices = vs;
    edges = es;
}

//near predicate utilized in process_splits
template<typename T>
struct NearPredicate { bool operator()(T x, T y) { return are_near(x, y); } };

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
void concatenate(T &a, T const &b) { a.insert(a.end(), b.begin(), b.end()); }

//returns a list of monotonic sections of a path
//TODO: handle saddle points
std::vector<Section> mono_sections(PathVector const &ps, Dim2 d) {
    std::vector<Section> monos;
    for(unsigned i = 0; i < ps.size(); i++) {
        //TODO: necessary? can we have empty paths?
        if(ps[i].size()) {
            for(unsigned j = 0; j < ps[i].size(); j++) {
                //find the points of 0 derivative
                Curve* deriv = ps[i][j].derivative();
                std::vector<double> splits = deriv->roots(0, X);
                concatenate(splits, deriv->roots(0, Y));
                delete deriv;
                process_splits(splits, 0, 1);
                //split on points of 0 derivative
                for(unsigned k = 1; k < splits.size(); k++)
                    monos.push_back(Section(CurveIx(i,j), splits[k-1], splits[k], ps, d));
            }
        }
    }
    return monos;
}

//finds the t-value on a section, which corresponds to a particular horizontal or vertical line
//d indicates the dimension along which the roots is performed.
//-1 is returned if no root is found
double section_root(Section const &s, PathVector const &ps, double v, Dim2 d) {
    std::vector<double> roots = s.curve.get(ps).roots(v, d);
    for(unsigned j = 0; j < roots.size(); j++)
        if(Interval(s.f, s.t).contains(roots[j])) return roots[j];
    return -1;
}

bool SectionSorter::section_order(Section const &a, double at, Section const &b, double bt) const {
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

bool SectionSorter::operator()(Section const &a, Section const &b) const {
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

// splits a section into pieces, as specified by an array of doubles, mutating the section to
// represent the first part, and returning the rest
//TODO: output iterator?
std::vector<boost::shared_ptr<Section> > split_section(boost::shared_ptr<Section> s, PathVector const &ps, std::vector<double> &cuts, Dim2 d) {
    std::vector<boost::shared_ptr<Section> > ret;
    
    process_splits(cuts, s->f, s->t);
    if(cuts.size() <= 2) return ret;
    
    s->t = cuts[1];
    s->tp = s->curve.get(ps)(cuts[1]);
    assert(lexo_point(s->fp, s->tp, d));
    
    ret.reserve(cuts.size() - 2);
    for(int i = cuts.size() - 1; i > 1; i--) ret.push_back(boost::shared_ptr<Section>(new Section(s->curve, cuts[i-1], cuts[i], ps, d)));
    return ret;
}

//merges the sorted lists a and b according to comparison z
template<typename X, typename Z>
void merge(X &a, X const &b, Z const &z) {
     a.reserve(a.size() + b.size());
     unsigned start = a.size();
     concatenate(a, b);
     std::inplace_merge(a.begin(), a.begin() + start, a.end(), z);
}

//TODO: make this faster than linear
unsigned get_vertex(std::vector<TopoGraph::Vertex> &vertices, std::vector<unsigned> &v_to_do, Point p, double tol) {
    unsigned i = 0;
    for(; i < vertices.size(); i++)
        if(are_near(vertices[i].avg, p, tol)) return i;
    vertices.push_back(TopoGraph::Vertex(p));
    v_to_do.push_back(i);
    return i;
}

//takes a vector of T pointers, and returns a vector of T with copies
template<typename T>
std::vector<T> deref_vector(std::vector<boost::shared_ptr<T> > const &xs, unsigned from = 0) {
    std::vector<T> ret;
    ret.reserve(xs.size() - from);
    for(unsigned i = from; i < xs.size(); i++)
        ret.push_back(T(*xs[i]));
    return ret;
}

//used to create reversed sorting predicates
template<typename C>
struct ReverseAdapter {
    typedef typename C::second_argument_type first_argument_type;
    typedef typename C::first_argument_type second_argument_type;
    typedef typename C::result_type result_type;
    const C &comp;
    ReverseAdapter(const C &c) : comp(c) {}
    result_type operator()(const first_argument_type &a, const second_argument_type &b) const { return comp(b, a); }
};

//used to sort std::vector<Section*>
template<typename C>
struct DerefAdapter {
    typedef typename boost::shared_ptr<typename C::first_argument_type> first_argument_type;
    typedef typename boost::shared_ptr<typename C::second_argument_type> second_argument_type;
    typedef typename C::result_type result_type;
    const C &comp;
    DerefAdapter(const C &c) : comp(c) {}
    result_type operator()(const first_argument_type a, const second_argument_type b) const {
        if(!a) return false;
        if(!b) return true;
        return comp(*a, *b);
    }
};

#ifdef SWEEP_GRAPH_DEBUG
//used for debugging purposes - each element represents a subsequent iteration of the algorithm.
std::vector<std::vector<Section> > monoss;
std::vector<std::vector<Section> > chopss;
std::vector<std::vector<Section> > contexts;
#endif

//TODO: ensure that the outputted graph is planar
TopoGraph::TopoGraph(PathVector const &ps, Dim2 d, double tol) {
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
    std::vector<boost::shared_ptr<Section> > chops, context;
    std::sort(sections.begin(), sections.end(), SweepSorter(d));
    
    //context_vertex = each element corresponds to an element of the context, and gives its starting vertex
    //v_to_do = a list of vertices to be finalized
    std::vector<unsigned> context_vertex, v_to_do;
    
    vertices.reserve(sections.size());
    edges.reserve(sections.size()*2);
    
    //vector used to hold temporary windings vectors
    std::vector<int> windings(ps.size(), 0);
    
    //TODO: preprocess input segs, and remove everything that's too small?
    //  vertex finalization stuff relies on this a bit, probably erroneously.
    
    //TODO: figure out how to handle vertical / horizontal exits with vertex finalization
    
    //TODO: allocate one big block of memory for section data?
    
    unsigned mix = 0;
    while(mix < sections.size() || !chops.empty() || !v_to_do.empty()) {
        //mergepass of monos and chops
        boost::shared_ptr<Section> s;
        if(mix < sections.size() && (chops.empty() || lexo_point(sections[mix].fp, chops.back()->fp, d))) {
            s = boost::shared_ptr<Section>(new Section(sections[mix++]));
        } else if(!chops.empty()) {
            s = chops.back();
            chops.pop_back();
        }
        
        //represents our position in the sweep, which controls what we finalize
        //if we have no more to process, finish the rest by setting our position to infinity
        double lim = s.get() ? s->fp[d] : std::numeric_limits<double>::infinity();
        
        //process all of the vertices we're done with
        for(int i = v_to_do.size()-1; i >= 0; i--) {
            if(vertices[v_to_do[i]].avg[d] + tol <= lim) {
                // now that we have progressed far enough in the sweep to know the sections which
                // exit this vertex, and their order
                //TODO: make sure that vertical sections aren't finalized too early, and if they are,
                //       figure out how to account for this in a reasonable fashion
                for(unsigned j = 0; j < context_vertex.size(); j++) {
                    if(context_vertex[j] == v_to_do[i]) {
                        vertices[context_vertex[j]].exits.push_back(edges.size());
                        edges.push_back(TopoGraph::Edge(context[j], 0, 0));
                    }
                }
                //find the average of the endpoints of the vertex
                /*Point avg;
                for(unsigned j = 0; j < vertices[v_to_do[i]].enters.size(); j++)
                    avg += vertices[v_to_do[i]].enters[j].section->tp;
                for(unsigned j = 0; j < vertices[v_to_do[i]].exits.size(); j++)
                    avg += vertices[v_to_do[i]].exits[j].section->fp; */
                //TODO: verify that this doesn't allow funny situations to occur, eg, adding more to an already finalized vertex
                //avg /= vertices[to_do_i].degree();
                v_to_do.erase(v_to_do.begin() + i);
            }
        }
        
        //move all of the segments we're done with
        for(int i = context.size() - 1; i >= 0; i--) {
            if(context[i]->tp[d] < lim || are_near(context[i]->tp[d], lim)) {
                //figure out this section's winding
                std::fill(windings.begin(), windings.end(), 0);
                for(int j = 0; j < i; j++) {
                    boost::shared_ptr<Section> sec = context[j];
                    unsigned k = sec->curve.path;
                    if(k >= windings.size()) windings.resize(k+1);
                    if(sec->fp[d] == sec->tp[d]) continue;
                    if(sec->f < sec->t) windings[k]++;
                    if(sec->f > sec->t) windings[k]--;
                }
                
                context[i]->windings = windings;
        
                unsigned vert = get_vertex(vertices, v_to_do, context[i]->tp, tol);
                
                //TODO: erase section?
                //if(vert == vix[i]) continue;  // remove tiny things
                
                //add edges to vertices
                TopoGraph::Vertex &cv = vertices[context_vertex[i]];
                unsigned j = 0;
                for(; j < cv.exits.size(); j++) {
                    if(edges[cv.exits[j]].section == context[i]) {
                        edges[cv.exits[j]].other_vert = vert;
                        edges[cv.exits[j]].other_edge = edges.size();
                        break;
                    }
                }
                //TODO: assert? what if the above loop never finds a val?
                vertices[vert].enters.push_back(edges.size());
                edges.push_back(TopoGraph::Edge(context[i], cv.exits[j], context_vertex[i]));
                
                //remove it from the context
                context.erase(context.begin() + i);
                context_vertex.erase(context_vertex.begin() + i);
            }
        }
        
        
        if(s.get()) {
            //insert section into context, in the proper location
            unsigned context_ix = std::lower_bound(context.begin(), context.end(), s, s_sort) - context.begin();
            context.insert(context.begin() + context_ix, s);
            context_vertex.insert(context_vertex.begin() + context_ix, get_vertex(vertices, v_to_do, s->fp, tol));
            
            Interval si = Interval(s->fp[1-d], s->tp[1-d]);
            
            // Now we intersect with neighbors - do a sweep!
            std::vector<double> this_splits;
            for(unsigned i = 0; i < context.size(); i++) {
                if(i == context_ix) continue;
                
                boost::shared_ptr<Section> sec = context[i];
                if(si.intersects(Interval(sec->fp[1-d], sec->tp[1-d]))) {
                    std::vector<double> other_splits;
                    Crossings xs = mono_intersect(s->curve.get(ps), Interval(s->f, s->t),
                                                  sec->curve.get(ps), Interval(sec->f, sec->t));
                    
                    if(!xs.empty()) {
                        for(unsigned j = 0; j < xs.size(); j++) {
                            this_splits.push_back(xs[j].ta);
                            other_splits.push_back(xs[j].tb);
                        }
                        
                        merge(chops, split_section(sec, ps, other_splits, d), heap_sort);
                    }
                }
            }
            if(!this_splits.empty())
                merge(chops, split_section(context[context_ix], ps, this_splits, d), heap_sort);
            std::sort(chops.begin(), chops.end(), heap_sort);
        }
        
        #ifdef SWEEP_GRAPH_DEBUG
        std::vector<Section> rem;
        for(unsigned i = mix; i < sections.size(); i++) rem.push_back(Section(sections[i]));
        monoss.push_back(rem);
        chopss.push_back(deref_vector(chops));
        contexts.push_back(deref_vector(context));
        #endif
    }
}
/*
void trim_whiskers(TopoGraph &g) {
    std::vector<bool> keep(g.size(), true);
    std::vector<Vertex*> affected(g), new_affected;
    
    while(!affected.empty()) {
        //std::cout << affected.size() << std::endl;
        for(unsigned i = 0; i < affected.size(); i++) {
            Vertex *v = affected[i];
            if(v->degree() < 2) {
                std::vector<Vertex*>::const_iterator iter;
                for(unsigned j = 0; j < v->degree(); j++) {
                    Edge &e = v->lookup(j);
                    unsigned ix = e.other->find(e.section);
                    e.other->remove_edge(ix);
                    iter = std::find(new_affected.begin(), new_affected.end(), e.other);
                    if(iter == new_affected.end()) new_affected.push_back(e.other);
                }
                iter = std::find(g.begin(), g.end(), v);
                keep[iter - g.begin()] = false;
            }
        }

        affected = std::vector<Vertex*>(new_affected);
        new_affected.clear();
    }
    unsigned j = 0;
    for(unsigned i = 0; i < keep.size(); i++)
        if(keep[i]) g[j++] = g[i]; else delete(g[i]);
    g.resize(j);
}

void add_edge_at(Vertex *v, Section *s, Edge const &e, bool before = true) {
    for(unsigned i = 0; i < v->enters.size(); i++) {
        if(v->enters[i].section == s) {
            v->enters.insert(v->enters.begin() + (before ? i : i + 1), e);
            return;
        }
    }
    for(unsigned i = 0; i < v->exits.size(); i++) {
        if(v->exits[i].section == s) {
            v->exits.insert(v->exits.begin() + (before ? i : i + 1), e);
            return;
        }
    }
    assert(false);
}

void double_whiskers(TopoGraph &g) {
    for(unsigned i = 0; i < g.size(); i++) {
        if(g[i]->degree() == 1) {
            Vertex *v = g[i];
            unsigned ix = 0;
            Edge e = g[i]->lookup(ix);
            while(true) {
                ix = e.other->find(e.section) + 1;
                Edge next_edge = e.other->lookup(ix);
                Section *new_section = new Section(*e.section);
                add_edge_at(v, e.section, Edge(new_section, e.other), false);
                add_edge_at(e.other, e.section, Edge(new_section, v), true);
                if(e.other->degree() == 3) {
                    v = e.other;
                    e = next_edge;
                } else break;
            }
        }
    }
}

void remove_vestigial(TopoGraph &g) {
    for(unsigned i = 0; i < g.size(); i++) {
        Edge &e1 = g[i]->enters.front(),
             &e2 = g[i]->exits.front();
        if(g[i]->enters.size() == 1 && g[i]->exits.size() == 1 && e1.section->curve == e2.section->curve) {
            //vestigial vert
            Section *new_section = new Section(e1.section->curve,
                                     e1.section->f,  e2.section->t,
                                     e1.section->fp, e2.section->tp);
            Vertex *v1 = e1.other, *v2 = e2.other;
            v1->lookup_section(e1.section) = Edge(new_section, v2);
            v2->lookup_section(e2.section) = Edge(new_section, v1);
            g.erase(g.begin() + i);
        }
    }
}


//planar area finding
//linear on number of edges
Areas traverse_areas(TopoGraph const &g) {
    Areas ret;
    
    //stores which edges we've visited
    std::vector<std::vector<bool> > visited;
    for(unsigned i = 0; i < g.size(); i++) visited.push_back(std::vector<bool>(g[i]->degree(), false));
    
    for(unsigned vix = 0; vix < g.size(); vix++) {
        while(true) {
            //find an unvisited edge to start on
            
            unsigned e_ix = std::find(visited[vix].begin(), visited[vix].end(), false) - visited[vix].begin();
            if(e_ix == g[vix]->degree()) break;
            Edge e = g[vix]->lookup(e_ix);
            
            unsigned start = e_ix;
            unsigned cur = vix;
            
            Area area;
            //std::vector<std::vector<bool> > before(visited);
            while(cur < g.size() && !visited[cur][e_ix]) {
                visited[cur][e_ix] = true;
                
                area.push_back(e.section);
                
                //go to clockwise edge
                cur = std::find(g.begin(), g.end(), e.other) - g.begin();
                if(cur >= g.size()) break;
                e_ix = g[cur]->find(e.section);
                if(g[cur]->degree() == 1) {
                   visited[cur][e_ix] = true;
                   break;
                }
                //assert(e_ix != g[cur]->degree());
                if(e_ix == g[cur]->degree()) break;
                e = g[cur]->lookup(++e_ix);
                
                if(cur == vix && start == e_ix) break;
            }
            //if(vix == cur && start == e_ix) {
                ret.push_back(area);
            //} else visited = before;
        }
    }
    return ret;
}
*/

void remove_area_whiskers(Areas &areas) {
    for(int i = areas.size() - 1; i >= 0; i--)
        if(areas[i].size() == 2 && *areas[i][0] == *areas[i][1]) 
            areas.erase(areas.begin() + i);
}

Path area_to_path(PathVector const &ps, Area const &area) {
    Path ret;
    bool rev = (area.size() > 1) && are_near(area[0]->tp, area[1]->fp);
    for(int i = rev ? area.size() - 1 : 0; rev ? i >= 0 : i < (int)area.size(); i = rev ? i-1 : i+1) {
        Curve *curv = area[i]->get_portion(ps);
        ret.append(*curv, Path::STITCH_DISCONTINUOUS);
        delete curv;
    }
    return ret;
}

PathVector areas_to_paths(PathVector const &ps, Areas const &areas) {
    std::vector<Path> ret;
    ret.reserve(areas.size());
    for(unsigned i = 0; i < areas.size(); i++)
        ret.push_back(area_to_path(ps, areas[i]));
    return ret;
}

} // end namespace Geom
