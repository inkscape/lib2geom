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

enum SubPathOp{
    moveto,
    lineto,
    quadto,
    cubicto,
    ellipto,
    close
};

unsigned const SubPathOpHandles[] = {1, 1, 2, 3, 4, 0, 0};
int const SubPathOpTail[] = {0, -1, -1, -1, -1, 0, 0};

class SubPath{
public:
    std::vector<Point> handles;
    std::vector<SubPathOp> cmd;

    class SubPathElem{
    public:
        SubPathOp op;
        std::vector<Point>::const_iterator s, e;
        std::vector<Point>::const_iterator begin() const {return s;}
        std::vector<Point>::const_iterator end() const {return e;}
        Point first() { return *s;}
        Point last() { return e[-1];}

        SubPathElem() {}
        SubPathElem(SubPathOp op,
                 std::vector<Point>::const_iterator s,
                 std::vector<Point>::const_iterator e) : op(op), s(s), e(e) {
        }
        
        Point operator[](int i) {return s[i];}
        
        void point_tangent_acc_at(double t, Point &p, Point &t, Point &a);
        Point point_at(double t);
        //Point tangent_at(double t);
        bool nearest_location(Point p, double& dist, double& t);
    };

    class SubPathConstIter {
    public:
        std::vector<SubPathOp>::const_iterator c;
        std::vector<Point>::const_iterator h;
        
        SubPathConstIter() {}
        SubPathConstIter(std::vector<SubPathOp>::const_iterator c,
                 std::vector<Point>::const_iterator h) :
            c(c), h(h) {}
        void operator++() {h+=SubPathOpHandles[*c]; c++;}
        void operator--() {c--; h-=SubPathOpHandles[*c];}
        SubPathElem operator*() const {return SubPathElem(*c, h+SubPathOpTail[*c], h + SubPathOpHandles[*c]);}

        std::vector<Point>::const_iterator begin() const {return h;}
        std::vector<Point>::const_iterator end() const {return h + SubPathOpHandles[*c];}
        SubPathOp cmd() { return *c;}
    };

    typedef SubPathConstIter const_iterator;

    class SubPathLocation{
    public:
        SubPathConstIter it;
        double t; // element specific meaning [0,1)
        SubPathLocation(SubPathConstIter it, double t) : it(it), t(t) {}
    };

    SubPathConstIter begin() const { return SubPathConstIter(cmd.begin(), handles.begin());}
    SubPathConstIter end() const { return SubPathConstIter(cmd.end(), handles.end());}

    /** returns the point at the position given by walking along the path. */
    SubPathLocation point_at_arc_length(double s);

    /** return the last nearest point on the path. */
    SubPathLocation nearest_location(Point p, double& dist);

    /** return a new path over [begin, end). */
    SubPath subpath(SubPathConstIter begin, SubPathConstIter end);
    
    /** return a new path over [begin, end). */
    SubPath subpath(SubPathLocation begin, SubPathLocation end);

    /** compute the bounding box of this path. */
    Maybe<Rect> bbox() const;

    /** a new path with an extra node inserted at at without changing the curve. */
    SubPath insert_nodes(SubPathLocation* b, SubPathLocation* e);
    SubPath insert_node(SubPathLocation at);

    /** coords of point on path. */
    Point point_at(SubPathLocation at);

    void point_tangent_acc_at (SubPathLocation at, Point & pos, Point & tgt, Point &acc);
    
    void push_back(SubPathElem e);
    void insert(SubPathConstIter before, SubPathConstIter s, SubPathConstIter e);
    SubPathConstIter indexed_elem(int i) { // mainly for debugging
        SubPathConstIter it = begin();
        while(--i >= 0) ++it;
        return it;
    }
};

class Path{
public:
    std::vector<SubPath> subpaths;
    
    typedef std::vector<SubPath>::const_iterator PathConstIter;
    typedef PathConstIter const_iterator;

    class PathLocation{
    public:
        SubPath::SubPathConstIter it;
        double t; // element specific meaning [0,1)
        PathLocation(SubPath::SubPathConstIter it, double t) : it(it), t(t) {}
    };
    Path() {}
    Path(SubPath sp) {subpaths.push_back(sp);}
};

inline bool operator!=(const SubPath::SubPathConstIter &a, const SubPath::SubPathConstIter &b) 
{ return (a.c!=b.c) || (a.h != b.h);}

//SubPath operator * (SubPath, Matrix);

template <class T> SubPath operator*(SubPath const &p, T const &m) {
    SubPath pr;
    
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

