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
using namespace Geom;
using namespace std;

void draw_rect(cairo_t *cr, Point tl, Point br) {
    cairo_move_to(cr, tl[X], tl[Y]);
    cairo_line_to(cr, br[X], tl[Y]);
    cairo_line_to(cr, br[X], br[Y]);
    cairo_line_to(cr, tl[X], br[Y]);
    cairo_close_path(cr);
}

void draw_bounds(cairo_t *cr, vector<Path> ps) {
    srand(0); 
    vector<Rect> bnds;
    for(unsigned i = 0; i < ps.size(); i++) {
        for(Path::iterator it = ps[i].begin(); it != ps[i].end(); it++) {
            Rect bounds = *(it->boundsFast());
            bnds.push_back(bounds);
            cairo_set_source_rgba(cr, uniform(), uniform(), uniform(), .5);
            //draw_rect(cr, bounds.min(), bounds.max());
            cairo_stroke(cr);
        }
    }
    {
        std::vector<std::vector<unsigned> > res = sweep_bounds(bnds);
        cairo_set_line_width(cr,0.5);
        cairo_save(cr);
        cairo_set_source_rgb(cr, 1, 0, 0);
        for(unsigned i = 0; i < res.size(); i++) {
            for(unsigned j = 0; j < res[i].size(); j++) {
                draw_line_seg(cr, bnds[i].midpoint(), bnds[res[i][j]].midpoint());
                cairo_stroke(cr);
            }
        }
        cairo_restore(cr);
    }
}

void mark_verts(cairo_t *cr, vector<Path> ps) {
    for(unsigned i = 0; i < ps.size(); i++)
        for(Path::iterator it = ps[i].begin(); it != ps[i].end(); it++)
            draw_cross(cr, it->initialPoint());
}

int winding(vector<Path> ps, Point p) {
    int wind = 0;
    for(unsigned i = 0; i < ps.size(); i++)
        wind += winding(ps[i],p);
    return wind;
}

template<typename T>
//std::vector<T>::iterator
T* insort(std::vector<T> &v, const T& val)
{
    T* iter = upper_bound(v.begin(), v.end(), val);
    
    unsigned offset = iter - v.begin();
    v.insert(iter, val);

    return offset + v.begin();
}


class Uncross{
public:
    class Piece{
    public:
        Rect bounds;
        Interval parameters;
        Curve const* curve;
        D2<SBasis> sb;
        int mark;
    };
    class Crossing{
    public:
        Piece *A, *B;
        double crossing_product; // first cross(d^nA, d^nB) for n that is non-zero, or 0 if the two curves are the same
    };
    vector<Rect> rs;
    vector<Path>* pths;
    cairo_t* cr;
    std::vector<Piece> pieces;
    
    Uncross(vector<Path> &pt, cairo_t* cr):pths(&pt), cr(cr) {}
    
    void build() {
        cairo_save(cr);
        vector<Path> &ps(*pths);
        for(unsigned i = 0; i < ps.size(); i++) {
            for(Path::iterator it = ps[i].begin(); it != ps[i].end(); it++) {
                Rect bounds = *(it->boundsExact());
                rs.push_back(bounds);
                //cairo_set_source_rgba(cr, uniform(), uniform(), uniform(), .5);
                //draw_rect(cr, bounds.min(), bounds.max());
                cairo_stroke(cr);
                pieces.push_back(Piece());
                pieces.back().bounds = bounds;
                pieces.back().curve = &*it;
                pieces.back().parameters = Interval(0,1);
                pieces.back().sb = it->toSBasis();
            }
        }
        cairo_restore(cr);
    }    

    struct Event {
        double x;
        unsigned ix;
        bool closing;
        Event(double pos, unsigned i, bool c) : x(pos), ix(i), closing(c) {}
// Lexicographic ordering by x then closing
        bool operator<(Event const &other) const {
            if(x < other.x) return true;
            if(x > other.x) return false;
            return closing < other.closing;
        }

    };

    std::vector<Event> events;
    std::vector<unsigned> open;
    
    int cmpy(Piece* a, Piece* b, Interval X) {
        if(a->bounds[1][1] < b->bounds[1][0]) { // bounds are strictly ordered
            return -1;
        }
        if(a->bounds[1][0] > b->bounds[1][1]) { // bounds are strictly ordered
            return 1;
        }
        std::vector<std::pair<double, double> > xs;
        find_intersections(xs, a->sb, b->sb);
        if(not xs.empty()) {
            polish_intersections( xs, a->sb, b->sb);
            // must split around these points to make new Pieces
            for(unsigned i = 0; i < xs.size(); i++) {
                std::cout << "cross:" << xs[i].first << " , " << xs[i].second << "\n";
                
                cairo_save(cr); 
                draw_circ(cr, a->sb(xs[i].first));
                cairo_stroke(cr);
                cairo_restore(cr);
                int ix = events[0].ix;
                if(0){
                pop_heap(events.begin(), events.end());
                events.pop_back();
                    std::vector<unsigned>::iterator iter = std::find(open.begin(), open.end(), ix);
                    open.erase(iter);
                    std::cout << "kill\n";
                }
                
            }
        } else { // any point gives an order
            vector<double> ar = roots(a->sb[0] - X.middle());
            vector<double> br = roots(b->sb[0] - X.middle());
            if ((ar.size() == 1) and (br.size() == 1))
                
                return a->sb[1](ar[0]) < b->sb[1](br[0]);
        }
    }
    
    
    static void
    draw_interval(cairo_t* cr, Interval I, Point origin, Point dir) {
        cairo_save(cr);
        cairo_set_line_width(cr, 0.5);
        for(int i = 0; i < 2; i++) {
            cairo_move_to(cr, Point(I[i], -3) + origin);
            cairo_line_to(cr, Point(I[i], +3) + origin);
        }
        cairo_move_to(cr, Point(I[0], 0) + origin);
        cairo_line_to(cr, Point(I[1], 0) + origin);
        cairo_stroke(cr);
        cairo_restore(cr);
    }
    void broke_sweep_bounds() {
        cairo_save(cr);

        cairo_set_source_rgb(cr, 1, 0, 0);
        events.reserve(rs.size()*2);
        std::vector<std::vector<unsigned> > pairs(rs.size());

        for(unsigned i = 0; i < rs.size(); i++) {
            events.push_back(Event(rs[i].left(), i, false));
            events.push_back(Event(rs[i].right(), i, true));
        }
        //std::sort(events.begin(), events.end());
        std::make_heap(events.begin(), events.end());

        //for(unsigned i = 0; i < events.size(); i++) {
        
        int i = 0;
        while(!events.empty()) {
            unsigned ix = events[0].ix;
            if(events[0].closing) {
                std::vector<unsigned>::iterator iter = std::find(open.begin(), open.end(), ix);
                if(iter != open.end()) {
                cairo_save(cr);
                cairo_set_source_rgb(cr, 0, 1, 0);
                cairo_set_line_width(cr, 0.25);
                cairo_rectangle(cr, rs[*iter]);
                cairo_stroke(cr);
                cairo_restore(cr);
                open.erase(iter);}
            } else {
                draw_interval(cr, rs[ix][0], Point(0,5*i+10), Point(0, 1));
                for(unsigned j = 0; j < open.size(); j++) {
                    unsigned jx = open[j];
                    OptInterval oiy = intersect(rs[ix][Y], rs[jx][Y]);
                    if(oiy) {
                        pairs[jx].push_back(ix);
                        std::cout << "oiy:" << *oiy << std::endl;
                        OptInterval oix = intersect(rs[ix][X], rs[jx][X]);
                        if(oix) {
                            std::cout << *oix;
                            std::cout << cmpy(&pieces[ix], &pieces[jx], *oix);
                            continue;
                        }
                        //draw_line_seg(cr, rs[ix].midpoint(), rs[jx].midpoint());
                        cairo_stroke(cr);
                    }
                }
                open.push_back(ix);
            }
            pop_heap(events.begin(), events.end());
            events.pop_back();
            i++;
        }
        //return pairs;
        cairo_restore(cr);
    }

    void sweep_bounds() {
        cairo_save(cr);

        cairo_set_source_rgb(cr, 1, 0, 0);
        events.reserve(rs.size()*2);
        std::vector<std::vector<unsigned> > pairs(rs.size());

        for(unsigned i = 0; i < rs.size(); i++) {
            events.push_back(Event(rs[i].left(), i, false));
            events.push_back(Event(rs[i].right(), i, true));
        }
        std::sort(events.begin(), events.end());
        
        for(unsigned i = 0; i < events.size(); i++) {
            unsigned ix = events[i].ix;
            if(events[i].closing) {
                std::vector<unsigned>::iterator iter = std::find(open.begin(), open.end(), ix);
                if(iter != open.end()) {
                    cairo_save(cr);
                    cairo_set_source_rgb(cr, 0, 1, 0);
                    cairo_set_line_width(cr, 0.25);
                    cairo_rectangle(cr, rs[*iter]);
                    cairo_stroke(cr);
                    cairo_restore(cr);
                    open.erase(iter);
                }
            } else {
                draw_interval(cr, rs[ix][0], Point(0,5*i+10), Point(0, 1));
                for(unsigned j = 0; j < open.size(); j++) {
                    unsigned jx = open[j];
                    OptInterval oiy = intersect(rs[ix][Y], rs[jx][Y]);
                    if(oiy) {
                        pairs[jx].push_back(ix);
                        std::cout << "oiy:" << *oiy << std::endl;
                        OptInterval oix = intersect(rs[ix][X], rs[jx][X]);
                        if(oix) {
                            std::cout << *oix;
                            std::cout << cmpy(&pieces[ix], &pieces[jx], *oix);
                            continue;
                        }
                        //draw_line_seg(cr, rs[ix].midpoint(), rs[jx].midpoint());
                        cairo_stroke(cr);
                    }
                }
                open.push_back(ix);
            }
        }

        cairo_restore(cr);
    }


};

class WindingTest: public Toy {
    vector<Path> path;
    PointHandle test_pt_handle;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 0.5);
        cairo_save(cr);
        cairo_path(cr, path);
        cairo_set_source_rgb(cr, 1, 1, 0);
        cairo_stroke(cr);
        cairo_restore(cr);
        mark_verts(cr, path);
        cairo_set_line_width(cr, 1);
        //draw_bounds(cr, path);
        if(0) {
            Uncross uc(path, cr);
            uc.build();
            
            uc.sweep_bounds();
            
            //draw_bounds(cr, path); mark_verts(cr, path);
        }
        
        cairo_save(cr);
        vector<Path> &ps(path);
        vector<Rect> rs;
        std::vector<Uncross::Piece> pieces;
        for(unsigned i = 0; i < ps.size(); i++) {
            for(Path::iterator it = ps[i].begin(); it != ps[i].end(); it++) {
                Rect bounds = *(it->boundsExact());
                rs.push_back(bounds);
                /*cairo_set_source_rgba(cr, uniform(), uniform(), uniform(), .5);
                draw_rect(cr, bounds.min(), bounds.max());
                cairo_stroke(cr);*/
                pieces.push_back(Uncross::Piece());
                pieces.back().bounds = bounds;
                pieces.back().curve = &*it;
                pieces.back().parameters = Interval(0,1);
                pieces.back().sb = it->toSBasis();
                pieces.back().mark = 0;
            }
        }
        cairo_restore(cr);
        std::vector<std::vector<unsigned> > prs = sweep_bounds(rs);
        
        cairo_save(cr);
        std::vector<Uncross::Piece> new_pieces;
        for(unsigned i = 0; i < prs.size(); i++) {
            int ix = i;
            Uncross::Piece& A = pieces[ix];
            for(unsigned j = 0; j < prs[i].size(); j++) {
                int jx = prs[i][j];
                Uncross::Piece& B = pieces[jx];
                cairo_set_source_rgb(cr, 0, 1, 0);
                draw_line_seg(cr, rs[ix].midpoint(), rs[jx].midpoint());
                cairo_stroke(cr);
                cout << ix << ", " << jx << endl;
                std::vector<std::pair<double, double> > xs;
                find_intersections(xs, A.sb, B.sb);
                if(not xs.empty()) {
                    polish_intersections( xs, A.sb, B.sb);
                    // must split around these points to make new Pieces
                    double A_t_prev = 0;
                    double B_t_prev = 0;
                    for(unsigned cv_idx = 0; cv_idx <= xs.size(); cv_idx++) {
                        double A_t = 1;
                        double B_t = 1;
                        if(cv_idx < xs.size()) {
                            A_t = xs[cv_idx].first;
                            B_t = xs[cv_idx].second;
                        }
                        
                        cairo_save(cr); 
                        draw_circ(cr, A.sb(xs[cv_idx].first));
                        cairo_stroke(cr);
                        cairo_restore(cr);
                        
                        Interval A_slice(A_t_prev, A_t);
                        Interval B_slice(B_t_prev, B_t);
                        if((A_slice.extent() > 0) or (B_slice.extent() > 0)) {
                            cout << "Aslice" <<A_slice << endl;
                            D2<SBasis> Asb = portion(A.sb, A_slice);
                            OptRect Abnds = bounds_exact(Asb);
                            if(Abnds) {
                                new_pieces.push_back(Uncross::Piece());
                                new_pieces.back().bounds = *Abnds;
                                new_pieces.back().curve = A.curve;
                                new_pieces.back().parameters = A_slice;
                                new_pieces.back().sb = Asb;
                                A.mark = 1;
                            }

                            cout << "Bslice" <<B_slice << endl;
                            D2<SBasis> Bsb = portion(B.sb, B_slice);
                            OptRect Bbnds = bounds_exact(Bsb);
                            if(Bbnds) {
                                new_pieces.push_back(Uncross::Piece());
                                new_pieces.back().bounds = *Bbnds;
                                new_pieces.back().curve = B.curve;
                                new_pieces.back().parameters = B_slice;
                                new_pieces.back().sb = Bsb;
                                B.mark = 1;
                            }
                        }
                        A_t_prev = A_t;
                        B_t_prev = B_t;
                    }
                }
            }
        }
        if(1)for(unsigned i = 0; i < prs.size(); i++) {
            if(not pieces[i].mark)
                new_pieces.push_back(pieces[i]);
        }
        cairo_restore(cr);
        
        for(unsigned i = 0; i < new_pieces.size(); i++) {
            cout << new_pieces[i].parameters << endl;
            cairo_save(cr);
            cairo_rectangle(cr, new_pieces[i].bounds);
            cairo_set_source_rgba(cr, 0,1,0,0.1);
            cairo_fill(cr);
            cairo_set_source_rgba(cr, 0.3,0.3,0,0.1);
            cairo_rectangle(cr, new_pieces[i].bounds);
            cairo_stroke(cr);
            cairo_restore(cr);
            cairo_d2_sb(cr, new_pieces[i].sb);
            cairo_stroke(cr);
        }
        
        std::streambuf* cout_buffer = std::cout.rdbuf();
        std::cout.rdbuf(notify->rdbuf());
        *notify << "\nwinding:" << winding(path, test_pt_handle.pos) << "\n";
        std::cout.rdbuf(cout_buffer);

        Toy::draw(cr, notify, width, height, save);
    }

    public:
    WindingTest () : test_pt_handle(300,300) {}
    void first_time(int argc, char** argv) {
        const char *path_name="winding.svgd";
        if(argc > 1)
            path_name = argv[1];
        path = read_svgd(path_name);
        OptRect bounds = bounds_exact(path);
        if(bounds)
            path += Point(10,10)-bounds->min();
        
        handles.push_back(&test_pt_handle);
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
