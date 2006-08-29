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
    lineto,
    quadto,
    cubicto,
    ellipto
};

unsigned const SubPathOpHandles[] = {1, 2, 3, 4};

class SubPath{
public:
    bool closed; // should this shape be closed (join endpoints with a line)
    std::vector<Point> handles;
    std::vector<SubPathOp> cmd;

    class Elem{
    public:
        SubPathOp op;
        typedef std::vector<Point>::const_iterator const_iterator;
        const_iterator s, e;
        const_iterator begin() const {return s;}
        const_iterator end() const {return e;}
        Point first() const { return *s;}
        Point last() const { return e[-1];}
        size_t size() { return e - s; }

        Elem() {}
        Elem(SubPathOp op,
                 const_iterator s, const_iterator e) : op(op), s(s), e(e) {
        }
        
        Point operator[]( const int i) {return s[i];}
        Point operator[]( const int i) const {return s[i];}
        
        void point_tangent_acc_at(double t, Point &p, Point &t, Point &a) const;
        Point point_at(double t) const;
        //Point tangent_at(double t);
        bool nearest_location(Point p, double& dist, double& t) const;
    };

    class ConstIter {
    public:
        std::vector<SubPathOp>::const_iterator c;
        std::vector<Point>::const_iterator h;
        
        ConstIter() {}
        ConstIter(std::vector<SubPathOp>::const_iterator c,
                 std::vector<Point>::const_iterator h) :
            c(c), h(h) {}
        void operator++() {h+=SubPathOpHandles[*c]; c++;}
        void operator--() {c--; h-=SubPathOpHandles[*c];}
        Elem operator*() const {assert(*c >= 0); assert(*c <= ellipto);  return Elem(*c, h-1, h + SubPathOpHandles[*c]);}

        std::vector<Point>::const_iterator begin() const {return h;}
        std::vector<Point>::const_iterator end() const {return h + SubPathOpHandles[*c];}
        SubPathOp cmd() { return *c;}
    };

    typedef ConstIter const_iterator;

    class Location{
    public:
        ConstIter it;
        double t; // element specific meaning [0,1)
        Location(ConstIter it, double t) : it(it), t(t) {}
    };
    
    class HashCookie {
        unsigned long value;
    public:
        friend class SubPath;
        
        unsigned long get_value() { return value;}
    };

    ConstIter begin() const { return ConstIter(cmd.begin(), handles.begin()+1);}
    ConstIter end() const { return ConstIter(cmd.end(), handles.end());}
    
    bool empty() const { return cmd.empty(); }
    unsigned size() const { return cmd.size(); }

    Point initial_point() const { return handles.front(); }

    /** returns the point at the position given by walking along the path. */
    Location point_at_arc_length(double s);

    /** return the last nearest point on the path. */
    Location nearest_location(Point p, double& dist) const;

    /** return a new path over [begin, end). */
    SubPath subpath(ConstIter begin, ConstIter end);
    
    /** return a new path over [begin, end). */
    SubPath subpath(Location begin, Location end);

    /** compute the bounding box of this path. */
    Maybe<Rect> bbox() const;

    /** a new path with an extra node inserted at at without changing the curve. */
    SubPath insert_nodes(Location* b, Location* e);
    SubPath insert_node(Location at);

    /** coords of point on path. */
    Point point_at(Location at) const;

    void point_tangent_acc_at (Location at, Point & pos, Point & tgt, Point &acc) const;
    
    void push_back(Elem e);
    void insert(ConstIter before, ConstIter s, ConstIter e);

// mainly for debugging
public:
    ConstIter indexed_elem(int i) const {
        ConstIter it = begin();
        while(--i >= 0) ++it;
        return it;
    }
    
    operator HashCookie();
    
    unsigned total_segments() const;
};

inline bool operator==(SubPath::HashCookie a, SubPath::HashCookie b) {
    return a.get_value() == b.get_value();
}

class PathBuilder;

class Path {
public:
    typedef std::vector<SubPath>::const_iterator PathConstIter;
    typedef PathConstIter const_iterator;
    
    const_iterator begin() const { return _subpaths.begin(); }
    const_iterator end() const { return _subpaths.end(); }
    SubPath const &front() const { return _subpaths.front(); }
    SubPath const &back() const { return _subpaths.back(); }
    
    struct PathLocation {
        SubPath::ConstIter it;
        double t; // element specific meaning [0,1)
        PathLocation(SubPath::ConstIter it, double t) : it(it), t(t) {}
    };

    Path() {}
    Path(SubPath sp) { _subpaths.push_back(sp); }
    
    unsigned total_segments() const;

    template <typename F>
    Path map(F f) const {
        Path pr;
        for ( const_iterator it = begin() ; it != end() ; it++ ) {
            pr._subpaths.push_back(f(*it));
        }
        return pr;
    }

private:
    std::vector<SubPath> _subpaths;

    friend class PathBuilder;
};

inline bool operator!=(const SubPath::ConstIter &a, const SubPath::ConstIter &b) 
{ assert(a.c <= b.c);  assert(a.h <= b.h); return (a.c!=b.c) || (a.h != b.h);}

inline bool operator<(const SubPath::ConstIter &a, const SubPath::ConstIter &b) 
{ return (a.c < b.c) && (a.h < b.h);}

inline ptrdiff_t operator-(const SubPath::ConstIter &a, const SubPath::ConstIter &b) 
{ return a.c - b.c;}

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

template <class T> Path operator*(Path const &p, T const &m) {
    struct multiply_by {
        T const &_m;
        multiply_by(T const &m) : _m(m) {}
        SubPath operator()(SubPath const &sp) { return sp * _m; }
    };
    return p.map(multiply_by(m));
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
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

