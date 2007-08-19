#ifndef __2GEOM_REGION_H
#define __2GEOM_REGION_H

#include "path.h"
#include "path-intersection.h"

//for path_direction:
#include "sbasis-geometric.h"

namespace Geom {

//returns true for ccw, false for cw
inline bool path_direction(Path const &p) {
    Piecewise<D2<SBasis> > pw = p.toPwSb();
    double area;
    Point centre;
    Geom::centroid(pw, centre, area);
    return area > 0;
}

class Shape;

class Region {
    friend Crossings crossings(Region const &a, Region const &b);
    friend class Shape;
    friend Shape shape_boolean(bool rev, Shape const & a, Shape const & b, CrossingSet const & crs);

    Path boundary;
    boost::optional<Rect> box;
    bool fill;
  public:

    Region() {}
    Region(Path const &p) : boundary(p) { fill = path_direction(p); }
    Region(Path const &p, bool dir) : boundary(p), fill(dir) {}
    Region(Path const &p, boost::optional<Rect> const &b) : boundary(p), box(b) { fill = path_direction(p); }
    Region(Path const &p, boost::optional<Rect> const &b, bool dir) : boundary(p), box(b), fill(dir) {}
    
    bool isFill() const { return fill; }    
    Path getBoundary() const { return boundary; }
    Rect boundsFast() {
        if(!box) box = boost::optional<Rect>(boundary.boundsFast());
        return *box;
    }
    bool contains(Point const &p) const {
        bool temp = Geom::contains(boundary, p);
        return temp;
    }
    bool contains(Region const &other) const { return contains(other.getBoundary().initialPoint()); }
    
    Region inverse() const { return Region(boundary.reverse(), box, !fill); }
    
    Region operator*(Matrix const &m) const;
    
    bool invariants() const;
};

typedef std::vector<Region> Regions;

inline Crossings crossings(Region const &a, Region const &b) {
    return crossings(a.boundary, b.boundary);
}

unsigned outer_index(Regions const &ps);

//assumes they're already sanitized somewhat
inline Regions regions_from_paths(std::vector<Path> const &ps) {
    Regions res;
    for(unsigned i = 0; i < ps.size(); i++) {
        res.push_back(Region(ps[i]));
    }
    return res;
}

inline std::vector<Path> paths_from_regions(Regions const &rs) {
    std::vector<Path> res;
    for(unsigned i = 0; i < rs.size(); i++) {
        res.push_back(rs[i].getBoundary());
    }
    return res;
}

Regions region_boolean(bool rev, Region const & a, Region const & b, Crossings const &cr);
Regions region_boolean(bool rev, Region const & a, Region const & b, Crossings const & cr_a, Crossings const & cr_b);

inline Regions region_boolean(bool rev, Region const & a, Region const & b) {
    return region_boolean(rev, a, b, crossings(a.getBoundary(), b.getBoundary()));
}

Regions path_union(Region const & a, Region const & b, bool typ);
Regions path_subtract(Region const & a, Region const & b, bool typ);
Regions path_intersect(Region const & a, Region const & b, bool typ);
Regions path_exclude(Region const & a, Region const & b, bool typ);

}

#endif
