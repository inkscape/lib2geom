#ifndef SEEN_PATH_H
#define SEEN_PATH_H

#include "point.h"
#include "rect.h"
#include "maybe.h"
#include <vector>

/* Need some kind of efficient iterator / position reference. */

/** Ideas:
 * path consists of handles and commands
 * reversing the handles and commands reverses the path
 * affine transforming the handles affine transforms the path
 * paths are immutable
 * all methods are O(n) or better, and bounds are given for each method and function.
 * library is as self contained as possible
*/

/** To be solved:
 * How do we show elements through the iterator?
 */

namespace Geom{

enum PathOp{
    moveto,
    lineto,
    quadto,
    cubicto,
    ellipto,
    close
};

unsigned const PathOpHandles[] = {1, 1, 2, 3, 4, 0, 0};
int const PathOpTail[] = {0, -1, -1, -1, -1, 0, 0};

class Path{
public:
    std::vector<Point> handles;
    std::vector<PathOp> cmd;

    class PathElem{
    public:
        PathOp op;
        std::vector<Point>::const_iterator s, e;
        std::vector<Point>::const_iterator begin() const {return s;}
        std::vector<Point>::const_iterator end() const {return e;}
        Point first() { return *s;}
        Point last() { return e[-1];}

        PathElem() {}
        PathElem(PathOp op,
                 std::vector<Point>::const_iterator s,
                 std::vector<Point>::const_iterator e) : op(op), s(s), e(e) {
        }
        
        Point operator[](int i) {return s[i];}
        
        void point_tangent_acc_at(double t, Point &p, Point &t, Point &a);
        Point point_at(double t);
        //Point tangent_at(double t);
        bool nearest_location(Point p, double& dist, double& t);
    };

    class PathConstIter {
    public:
        std::vector<PathOp>::const_iterator c;
        std::vector<Point>::const_iterator h;
        
        PathConstIter() {}
        PathConstIter(std::vector<PathOp>::const_iterator c,
                 std::vector<Point>::const_iterator h) :
            c(c), h(h) {}
        void operator++() {h+=PathOpHandles[*c]; c++;}
        void operator--() {c--; h-=PathOpHandles[*c];}
        PathElem operator*() const {return PathElem(*c, h+PathOpTail[*c], h + PathOpHandles[*c]);}

        std::vector<Point>::const_iterator begin() const {return h;}
        std::vector<Point>::const_iterator end() const {return h + PathOpHandles[*c];}
        PathOp cmd() { return *c;}
    };

    typedef PathConstIter const_iterator;

    class PathLocation{
    public:
        PathConstIter it;
        double t; // element specific meaning [0,1)
        PathLocation(PathConstIter it, double t) : it(it), t(t) {}
    };

    PathConstIter begin() const { return PathConstIter(cmd.begin(), handles.begin());}
    PathConstIter end() const { return PathConstIter(cmd.end(), handles.end());}

    /** returns the point at the position given by walking along the path. */
    PathLocation point_at_arc_length(double s);

    /** return the last nearest point on the path. */
    PathLocation nearest_location(Point p, double& dist);

    /** return a new path over [begin, end). */
    Path subpath(PathConstIter begin, PathConstIter end);
    
    /** return a new path over [begin, end). */
    Path subpath(PathLocation begin, PathLocation end);

    /** compute the bounding box of this path. */
    Maybe<Rect> bbox() const;

    /** a new path with an extra node inserted at at without changing the curve. */
    Path insert_nodes(PathLocation* b, PathLocation* e);
    Path insert_node(PathLocation at);

    /** coords of point on path. */
    Point point_at(PathLocation at);

    void point_tangent_acc_at (PathLocation at, Point & pos, Point & tgt, Point &acc);
    
    void push_back(PathElem e);
    void insert(PathConstIter before, PathConstIter s, PathConstIter e);
    PathConstIter indexed_elem(int i) { // mainly for debugging
        PathConstIter it = begin();
        while(--i >= 0) ++it;
        return it;
    }
};

inline bool operator!=(const Path::PathConstIter &a, const Path::PathConstIter &b) 
{ return (a.c!=b.c) || (a.h != b.h);}

//Path operator * (Path, Matrix);

template <class T> Path operator*(Path const &p, T const &m) {
    Path pr;
    
    pr.cmd = p.cmd;
    pr.handles.reserve(p.handles.size());
    
    for(unsigned i = 0; i < p.handles.size(); i++) {
        pr.handles.push_back(p.handles[i]*m);
    }
    return pr;
}

template <typename Point, unsigned order>
struct BezImpl {
    static inline Point compute(double t, Point *b) {
        Point child[order];
        for ( unsigned i=0 ; i < order ; i++ ) {
            child[i] = BezImpl<Point, 1>(t, b + i);
        }
        return BezImpl<Point, order-1>::compute(t, child);
    }
};

template <typename Point>
struct BezImpl<Point, 1> {
    static inline Point compute(double t, Point *b) {
        return ( 1 - t ) * b[0] + t * b[1];
    }
};

template <typename Point>
struct BezImpl<Point, 0> {
    static inline Point compute(double t, Point *b) {
        return *b;
    }
};

template <unsigned order, typename Point>

inline Point bezier_at(double t, Point *b) {
    return BezImpl<Point, order>::compute(t, b);
}

}; // namespace Geom

#endif // SEEN_PATH_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

