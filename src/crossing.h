#ifndef __GEOM_CROSSING_H
#define __GEOM_CROSSING_H

#include <vector>
#include <set>

namespace Geom {

struct Crossing {
    bool dir; //True: along a, a becomes outside.
    double ta, tb;  //time on a and b of crossing
    unsigned a, b;  //storage of indices
    Crossing() : dir(false), ta(0), tb(1), a(0), b(1) {}
    Crossing(double t_a, double t_b, bool direction) : dir(direction), ta(t_a), tb(t_b), a(0), b(1) {}
    Crossing(double t_a, double t_b, unsigned ai, unsigned bi, bool direction) : dir(direction), ta(t_a), tb(t_b), a(ai), b(bi) {}
    bool operator==(const Crossing & other) const { return a == other.a && b == other.b && dir == other.dir && ta == other.ta && tb == other.tb; }
    bool operator!=(const Crossing & other) const { return !(*this == other); }
    unsigned getOther(unsigned cur) { return a == cur ? b : a; }
};


/*inline bool near(Crossing a, Crossing b) {
    return near(a.ta, b.ta) && near(a.tb, b.tb);
}

struct NearF { bool operator()(Crossing a, Crossing b) { return near(a, b); } };
*/

struct CrossingOrder {
    unsigned ix;
    CrossingOrder(unsigned i) : ix(i) {}
    bool operator()(Crossing a, Crossing b) {
        return (ix == a.a ? a.ta < b.ta : a.tb < b.tb);
    }
};

typedef std::vector<Crossing> Crossings;
typedef std::vector<Crossings> CrossingSet;

inline void sort_crossings(Crossings &cr, unsigned ix) { std::sort(cr.begin(), cr.end(), CrossingOrder(ix)); }

inline Crossings reverse_ta(Crossings const &cr, std::vector<double> max) {
    Crossings ret;
    for(Crossings::const_iterator i = cr.begin(); i != cr.end(); ++i) {
        double mx = max[i->a];
        ret.push_back(Crossing(i->ta > mx ? (1 - (i->ta - mx) + mx) : mx - i->ta,
                               i->tb, !i->dir));
    }
    return ret;
}

inline Crossings reverse_tb(Crossings const &cr, unsigned split, std::vector<double> max) {
    Crossings ret;
    for(Crossings::const_iterator i = cr.begin(); i != cr.end(); ++i) {
        double mx = max[i->b - split];
        ret.push_back(Crossing(i->ta, i->tb > mx ? (1 - (i->tb - mx) + mx) : mx - i->tb,
                               !i->dir));
    }
    return ret;
}

inline CrossingSet reverse_ta(CrossingSet const &cr, unsigned split, std::vector<double> max) {
    CrossingSet ret;
    for(unsigned i = 0; i < cr.size(); i++) {
        Crossings res = reverse_ta(cr[i], max);
        if(i < split) std::reverse(res.begin(), res.end());
        ret.push_back(res);
    }
    return ret;
}

inline CrossingSet reverse_tb(CrossingSet const &cr, unsigned split, std::vector<double> max) {
    CrossingSet ret;
    for(unsigned i = 0; i < cr.size(); i++) {
        Crossings res = reverse_tb(cr[i], split, max);
        if(i >= split) std::reverse(res.begin(), res.end());
        ret.push_back(res);
    }
    return ret;
}

/*
inline void clean(Crossings &cr_a, Crossings &cr_b) {
    if(cr_a.empty()) return;
    
    //Remove anything with dupes
    
    for(Eraser<Crossings> i(&cr_a); !i.ended(); i++) {
        const Crossing cur = *i;
        Eraser<Crossings> next(i);
        next++;
        if(near(cur, *next)) {
            cr_b.erase(std::find(cr_b.begin(), cr_b.end(), cur));
            for(i = next; near(*i, cur); i++) {
                cr_b.erase(std::find(cr_b.begin(), cr_b.end(), *i));
            }
            continue;
        }
    }
}
*/

}

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
