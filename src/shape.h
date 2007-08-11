#ifndef __2GEOM_SHAPE_H
#define __2GEOM_SHAPE_H

#include <vector>
#include <set>

#include "path.h"
#include "path-intersection.h"

//for path_fill:
#include "sbasis-geometric.h"
//TODO: BBOX optimizations

namespace Geom {

class Shape;

//returns true for ccw, false for cw
inline bool path_direction(Path const &p) {
    Piecewise<D2<SBasis> > pw = p.toPwSb();
    double area;
    Point centre;
    Geom::centroid(pw, centre, area);
    return area > 0;
}

class Region {
    friend Shape path_boolean(bool rev, Region const &, Region const &,
                                        Crossings const &, Crossings const &);
    Path _boundary;
    boost::optional<Rect> box;
    bool _fill;
  public:

    Region() {}
    Region(Path const &p) : _boundary(p) { _fill = path_direction(p); }
    Region(Path const &p, bool dir) : _fill(dir), _boundary(p) {}
    Region(Path const &p, boost::optional<Rect> const &b) : _boundary(p), box(b) {}
    Region(Path const &p, boost::optional<Rect> const &b, bool dir) : _fill(dir), _boundary(p), box(b) {}

    bool fill() const { return _fill; }    
    Path boundary() const { return _boundary; }
    Rect boundsFast() {
        if(!box) box = boost::optional<Rect>(boundary().boundsFast());
        return *box;
    }
    bool contains(Point const &p) const {
        return Geom::contains(boundary(), p);
    }
    
    Region operator*(Matrix const &m) const;
};

typedef std::vector<Region> Regions;

//assumes they're already sanitized somewhat
inline Regions regions_from_paths(std::vector<Path> const &ps) {
    Regions res;
    for(unsigned i = 0; i < ps.size(); i++) {
        res.push_back(Region(ps[i]));
    }
    return res;
}

class Shape {
    Regions content;
    friend Shape shape_region_boolean(bool rev, Shape const & a, Region const & b);
    friend Shape shape_union(Shape const &, Shape const &);
    friend Shape shape_subtract(Shape const &, Shape const &);
    friend Shape shape_intersect(Shape const &, Shape const &);
    friend Shape path_boolean(bool rev, Region const &, Region const &,
                                           Crossings const &, Crossings const &);
    
  public:
    Regions getContent() const { return content; }
    
    void mergeWith(Shape const &other) {
        content.insert(content.end(), other.content.begin(), other.content.end());
    }
    
    Shape() {}
    explicit Shape(Region const & r) { content.push_back(r); }
    explicit Shape(Regions const & c) : content(c) {}
    
    bool isEmpty() const { return content.empty(); }
    
    Shape operator*(Matrix const &m) const;
};

unsigned outer_index(Regions const &ps);

Shape path_boolean(bool rev, Region const & a, Region const & b, Crossings const &cr);
Shape path_boolean(bool rev, Region const & a, Region const & b, Crossings const & cr_a, Crossings const & cr_b);

inline Shape path_boolean(bool rev, Region const & a, Region const & b) {
    return path_boolean(rev, a, b, crossings(a.boundary(), b.boundary()));
}

}

#endif
