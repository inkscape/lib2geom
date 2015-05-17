#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/svg-path-parser.h>
#include <2geom/utils.h>
#include <cstdlib>
#include <2geom/crossing.h>
#include <2geom/path-intersection.h>
#include <2geom/transforms.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/pathvector.h>

using namespace Geom;

struct EndPoint {
  public:
    Point point, norm;
    double time;
    EndPoint() : time(0) { }
    EndPoint(Point p, Point n, double t) : point(p), norm(n), time(t) { }
};

struct Edge {
  public:
    EndPoint from, to;
    int ix;
    bool cw;
    Edge() { }
    Edge(EndPoint f, EndPoint t, int i, bool c) : from(f), to(t), ix(i), cw(c) { }
    bool operator==(Edge const &other) { return from.time == other.from.time && to.time == other.to.time; }
};

typedef std::vector<Edge> Edges;

Edges edges(Path const &p, Crossings const &cr, unsigned ix) {
    Edges ret = Edges();
    EndPoint prev;
    for(unsigned i = 0; i <= cr.size(); i++) {
        double t = cr[i == cr.size() ? 0 : i].getTime(ix);
        Point pnt = p.pointAt(t);
        Point normal = p.pointAt(t+0.01) - pnt;
        normal.normalize();
        std::cout << pnt << "\n";
        EndPoint cur(pnt, normal, t);
        if(i == 0) { prev = cur; continue; }
        ret.push_back(Edge(prev, cur, ix, false));
        ret.push_back(Edge(prev, cur, ix, true));
        prev = cur;
    }
    return ret;
}

template<class T>
void append(std::vector<T> &vec, std::vector<T> const &other) {
    vec.insert(vec.end(),other.begin(), other.end());
}

Edges edges(PathVector const &ps, CrossingSet const &crs) {
    Edges ret = Edges();
    for(unsigned i = 0; i < crs.size(); i++) {
        Edges temp = edges(ps[i], crs[i], i);
        append(ret, temp); 
    }
    return ret;
}

PathVector edges_to_paths(Edges const &es, PathVector const &ps) {
    PathVector ret;
    for(unsigned i = 0; i < es.size(); i++) {
        ret.push_back(ps[es[i].ix].portion(es[i].from.time, es[i].to.time));
    }
    return ret;
}

void draw_cell(cairo_t *cr, Edges const &es, PathVector const &ps) {
    cairo_set_source_rgba(cr, uniform(), uniform(), uniform(), 0.5);
    cairo_set_line_width(cr, uniform() * 10);
    PathVector paths = edges_to_paths(es, ps);
    Piecewise<D2<SBasis> > pw = paths_to_pw(paths);
    double area;
    Point centre;
    Geom::centroid(pw, centre, area);
    cairo_path(cr, paths); //* (Translate(-centre) * Scale(0.2) * Translate(centre*2)));
    cairo_stroke(cr);
}

//Only works for normal
double ang(Point n1, Point n2) {
    return (dot(n1, n2)+1) * (cross(n1, n2) < 0 ? -1 : 1);
}

template<class T>
void remove(std::vector<T> &vec, T const &val) {
    for (typename std::vector<T>::iterator it = vec.begin(); it != vec.end(); ++it) {
        if(*it == val) {
            vec.erase(it);
            return;
        }
    }
}

std::vector<Edges> cells(cairo_t */*cr*/, PathVector const &ps) {
    CrossingSet crs = crossings_among(ps);
    Edges es = edges(ps, crs);
    std::vector<Edges> ret = std::vector<Edges>();

    while(!es.empty()) {
        std::cout << "hello!\n";
        Edge start = es.front();
        Path p = Path();
        Edge cur = start;
        bool rev = false;
        Edges cell = Edges();
        do {
            std::cout << rev << " " << cur.from.time << ", " << cur.to.time << "\n";
            double a = 0;
            Edge was = cur;
            EndPoint curpnt = rev ? cur.from : cur.to;
            Point norm = rev ? -curpnt.norm : curpnt.norm;
            //Point to = curpnt.point + norm *20;
            
            //std::cout << norm;
            for(unsigned i = 0; i < es.size(); i++) {
                if(es[i] == was || es[i].cw != start.cw) continue;
                if((!are_near(curpnt.time, es[i].from.time)) &&
                    are_near(curpnt.point, es[i].from.point, 0.1)) {
                    double v = ang(norm, es[i].from.norm);
                    //draw_line_seg(cr, curpnt.point, to);
                    //draw_line_seg(cr, to, es[i].from.point + es[i].from.norm*30); 
                    //std::cout << v << "\n";
                    if(start.cw ? v < a : v > a ) {
                        a = v;
                        cur = es[i];
                        rev = false;
                    }
                }
                if((!are_near(curpnt.time, es[i].to.time)) &&
                    are_near(curpnt.point, es[i].to.point, 0.1)) {
                    double v = ang(norm, -es[i].to.norm);
                    if(start.cw ? v < a : v > a) {
                        a = v;
                        cur = es[i];
                        rev = true; 
                    }
                }
            }
            cell.push_back(cur);
            remove(es, cur);
            if(cur == was) break;
        } while(!(cur == start));
        if(are_near(start.to.point, rev ? cur.from.point : cur.to.point)) {
            ret.push_back(cell);
        }
    }
    return ret;
}

int cellWinding(Edges const &/*es*/, PathVector const &/*ps*/) {
    return 0;
}

class Sanitize: public Toy {
    PathVector paths;
    std::vector<Edges> es;
    PointSetHandle angh;
    PointSetHandle pathix;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save,
                      std::ostringstream *timer_stream)
    {
        int ix = pathix.pts[0][X] / 10;
        es = cells(cr, paths);
        draw_cell(cr, es[ix], paths);

        cairo_set_source_rgba(cr, 0, 0, 0, 1);
        //cairo_path(cr, paths);
        //cairo_stroke(cr);

        Point ap = angh.pts[1] - angh.pts[0], bp = angh.pts[2] - angh.pts[0];
        ap.normalize(); bp.normalize();
        *notify << ang(ap, bp);
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
    
    public:
    Sanitize () {}
    void first_time(int argc, char** argv) {
        const char *path_name="sanitize_examples.svgd";
        if(argc > 1)
            path_name = argv[1];
        paths = read_svgd(path_name);
        
        handles.push_back(&angh); handles.push_back(&pathix);
        angh.push_back(100, 100);
        angh.push_back(80, 100);
        angh.push_back(100, 80);
        pathix.push_back(30, 200);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new Sanitize());
    return 0;
} 
