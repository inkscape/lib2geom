#include <iostream>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/path-intersection.h>
#include <2geom/basic-intersection.h>
#include <2geom/pathvector.h>

#include <cstdlib>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>
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

void draw_bounds(cairo_t *cr, PathVector ps) {
    srand(0); 
    vector<Rect> bnds;
    for(auto & p : ps) {
        for(const auto & it : p) {
            Rect bounds = (it.boundsFast());
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
            for(unsigned int j : res[i]) {
                draw_line_seg(cr, bnds[i].midpoint(), bnds[j].midpoint());
                cairo_stroke(cr);
            }
        }
        cairo_restore(cr);
    }
}

void mark_verts(cairo_t *cr, PathVector ps) {
    for(auto & p : ps)
        for(const auto & it : p)
            draw_cross(cr, it.initialPoint());
}

int winding(PathVector ps, Point p) {
    int wind = 0;
    for(const auto & pt : ps)
        wind += winding(pt,p);
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
        int id;
    };
    class Crossing{
    public:
        vector<int> joins;
        //double crossing_product; // first cross(d^nA, d^nB) for n that is non-zero, or 0 if the two curves are the same
    };
    vector<Rect> rs;
    PathVector* pths;
    cairo_t* cr;
    std::vector<Piece> pieces;
    
    Uncross(PathVector &pt, cairo_t* cr):pths(&pt), cr(cr) {}
    
    void build() {
        cairo_save(cr);
        PathVector &ps(*pths);
        for(auto & p : ps) {
            for(const auto & it : p) {
                Rect bounds = (it.boundsExact());
                rs.push_back(bounds);
                //cairo_set_source_rgba(cr, uniform(), uniform(), uniform(), .5);
                //draw_rect(cr, bounds.min(), bounds.max());
                cairo_stroke(cr);
                pieces.emplace_back();
                pieces.back().bounds = bounds;
                pieces.back().curve = &it;
                pieces.back().parameters = Interval(0,1);
                pieces.back().sb = it.toSBasis();
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
        if(a->bounds[Y].max() < b->bounds[Y].min()) { // bounds are strictly ordered
            return -1;
        }
        if(a->bounds[Y].min() > b->bounds[Y].max()) { // bounds are strictly ordered
            return 1;
        }
        std::vector<std::pair<double, double> > xs;
        find_intersections(xs, a->sb, b->sb);
        if(!xs.empty()) {
            polish_intersections( xs, a->sb, b->sb);
            // must split around these points to make new Pieces
            for(auto & x : xs) {
                std::cout << "cross:" << x.first << " , " << x.second << "\n";
                
                cairo_save(cr); 
                draw_circ(cr, a->sb(x.first));
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
        return 0; // FIXME
    }
    
    
    static void
    draw_interval(cairo_t* cr, Interval I, Point origin, Point /*dir*/) {
        cairo_save(cr);
        cairo_set_line_width(cr, 0.5);

        cairo_move_to(cr, Point(I.min(), -3) + origin);
        cairo_line_to(cr, Point(I.min(), +3) + origin);
        cairo_move_to(cr, Point(I.max(), -3) + origin);
        cairo_line_to(cr, Point(I.max(), +3) + origin);

        cairo_move_to(cr, Point(I.min(), 0) + origin);
        cairo_line_to(cr, Point(I.min(), 0) + origin);
        cairo_stroke(cr);
        cairo_restore(cr);
    }
    void broke_sweep_bounds() {
        cairo_save(cr);

        cairo_set_source_rgb(cr, 1, 0, 0);
        events.reserve(rs.size()*2);
        std::vector<std::vector<unsigned> > pairs(rs.size());

        for(unsigned i = 0; i < rs.size(); i++) {
            events.emplace_back(rs[i].left(), i, false);
            events.emplace_back(rs[i].right(), i, true);
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
                for(unsigned int jx : open) {
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
            events.emplace_back(rs[i].left(), i, false);
            events.emplace_back(rs[i].right(), i, true);
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
                for(unsigned int jx : open) {
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
    PathVector path;
    PointHandle test_pt_handle;
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 0.5);
        cairo_save(cr);
        cairo_path(cr, path);
        cairo_set_source_rgb(cr, 1, 1, 0);
        cairo_stroke(cr);
        cairo_restore(cr);
        //mark_verts(cr, path);
        cairo_stroke(cr);
        cairo_set_line_width(cr, 1);
        //draw_bounds(cr, path);
        if(0) {
            Uncross uc(path, cr);
            uc.build();
            
            uc.sweep_bounds();
            
            //draw_bounds(cr, path); mark_verts(cr, path);
        }
        
        cairo_save(cr);
        PathVector &ps(path);
        vector<Rect> rs;
        std::vector<Uncross::Piece> pieces;
        std::vector<Uncross::Crossing> crosses;
        int id_counter = 0;
        for(auto & p : ps) {
            int piece_start = pieces.size();
            for(Path::iterator it = p.begin(); it != p.end(); ++it) {
                Rect bounds = (it->boundsExact());
                rs.push_back(bounds);
                /*cairo_set_source_rgba(cr, uniform(), uniform(), uniform(), .5);
                draw_rect(cr, bounds.min(), bounds.max());
                cairo_stroke(cr);*/
                pieces.emplace_back();
                pieces.back().bounds = bounds;
                pieces.back().curve = &*it;
                pieces.back().parameters = Interval(0,1);
                pieces.back().sb = it->toSBasis();
                pieces.back().mark = 0;
                pieces.back().id = id_counter++;
                if(it != p.begin() and !crosses.empty())
                    crosses.back().joins.push_back(pieces.back().id);
                crosses.emplace_back();
                crosses.back().joins.push_back(pieces.back().id);
            }
            crosses.back().joins.push_back(pieces[piece_start].id);
            //crosses[cross_start].joins.push_back(pieces.back().id);
        }
        cairo_restore(cr);
        std::vector<std::vector<unsigned> > prs = sweep_bounds(rs);
        
        cairo_save(cr);
        std::vector<Uncross::Piece> new_pieces;
        for(unsigned i = 0; i < prs.size(); i++) {
            int ix = i;
            Uncross::Piece& A = pieces[ix];
            for(int jx : prs[i]) {
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
                    int A_prec_id = A.id;
                    int B_prec_id = B.id;
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
                                new_pieces.emplace_back();
                                new_pieces.back().bounds = *Abnds;
                                new_pieces.back().curve = A.curve;
                                new_pieces.back().parameters = A_slice;
                                new_pieces.back().sb = Asb;
                                new_pieces.back().id = id_counter++;
                                crosses.emplace_back();
                                crosses.back().joins.push_back(A_prec_id);
                                crosses.back().joins.push_back(new_pieces.back().id);
                                A.mark = 1;
                                A_prec_id = new_pieces.back().id;
                            }

                            cout << "Bslice" <<B_slice << endl;
                            D2<SBasis> Bsb = portion(B.sb, B_slice);
                            OptRect Bbnds = bounds_exact(Bsb);
                            if(Bbnds) {
                                new_pieces.emplace_back();
                                new_pieces.back().bounds = *Bbnds;
                                new_pieces.back().curve = B.curve;
                                new_pieces.back().parameters = B_slice;
                                new_pieces.back().sb = Bsb;
                                new_pieces.back().id = id_counter++;
                                crosses.emplace_back();
                                crosses.back().joins.push_back(B_prec_id);
                                crosses.back().joins.push_back(new_pieces.back().id);
                                B.mark = 1;
                                B_prec_id = new_pieces.back().id;
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
        
        for(auto & new_piece : new_pieces) {
            cout << new_piece.parameters << ", " <<new_piece.id <<endl;
            cairo_save(cr);
            cairo_rectangle(cr, new_piece.bounds);
            cairo_set_source_rgba(cr, 0,1,0,0.1);
            cairo_fill(cr);
            cairo_set_source_rgba(cr, 0.3,0.3,0,0.1);
            cairo_rectangle(cr, new_piece.bounds);
            cairo_stroke(cr);
            cairo_restore(cr);
            cairo_d2_sb(cr, new_piece.sb);
            cairo_stroke(cr);
        }
        

        cout << "crossings:";
        for(auto & cr : crosses) {
            for(int join : cr.joins) {
                cout << join << ", ";
            }
            cout << endl;
        }
        
        std::streambuf* cout_buffer = std::cout.rdbuf();
        std::cout.rdbuf(notify->rdbuf());
        *notify << "\nwinding:" << winding(path, test_pt_handle.pos) << "\n";
        std::cout.rdbuf(cout_buffer);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    public:
    WindingTest () : test_pt_handle(300,300) {}
    void first_time(int argc, char** argv) override {
        const char *path_name="winding.svgd";
        if(argc > 1)
            path_name = argv[1];
        path = read_svgd(path_name);
        OptRect bounds = bounds_exact(path);
        if (bounds) {
            path *= Translate(Point(10,10) - bounds->min());
        }
        
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
