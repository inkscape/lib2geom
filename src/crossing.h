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

inline void sort_crossings(Crossings &cr, unsigned ix) { std::sort(cr.begin(), cr.end(), CrossingOrder(ix)); }

template<typename T>
struct Eraser {
    T *x;
    unsigned i, o;
    bool skip;
    
    Eraser(T *t) : x(t), i(0), o(0), skip(false) {}
    ~Eraser() { finish(); }
    
    bool operator==(Eraser const &other) const { return other.i == i; }
    bool operator!=(Eraser const &other) const { return other.i != i; }
    /* bool operator==(typename T::iterator const &it) { return it == i; }
    bool operator!=(typename T::iterator const &it) { return it != i; }
    */
    
    bool ended() const { return i >= x->size(); }
    
    typedef typename T::iterator::value_type value_type;
    typedef typename T::iterator::pointer pointer;
    
    value_type const operator*() const { return (*x)[i]; }
    pointer operator->() const { return &(*x)[i]; }
    
    Eraser &operator++() {
        if(skip != true) {
            if(o != i) (*x)[o] = (*x)[i];
            ++o;
        } else skip = false;
        ++i;
        return *this;
    }
    
    Eraser operator++(int) {
        Eraser old=*this;
        ++(*this);
        return old;
    }
    
    void erase() { skip = true; }
    
    void replace(value_type const &a) {
        skip = true;
        (*x)[o] = a;
        ++o;
    }
    
    void finish() { x->resize(o); }
};

template<typename T>
struct Replacer {
    T *x;
    T o;
    unsigned i;
    bool skip;
    
    Replacer(T *t) : x(t), i(0), skip(false) { o = T(); }
    ~Replacer() { finish(); }
    
    bool ended() const { return i >= x->size(); }
    
    typedef typename T::iterator::value_type value_type;
    typedef typename T::iterator::pointer pointer;
    
    value_type const operator*() const { return (*x)[i]; }
    pointer operator->() const { return &(*x)[i]; }
    
    Replacer &operator++() {
        if(!skip) o.push_back((*x)[i]); else skip = false;
        ++i;
        return *this;
    }
    
    Replacer operator++(int) {
        Replacer old=*this;
        ++(*this);
        return old;
    }
    
    void erase() { skip = true; }
    
    template<class InputIterator>
    void replace(InputIterator first, InputIterator last) {
        skip = true;
        o.insert(o.end(), first, last);
    }
    void replace(T const &a) { replace(a.begin(), a.end()); }
    void replace(value_type const &a) {
        skip = true;
        o.push_back(a);
    }
    
    void finish() {
        x->resize(o.size());
        std::copy(o.begin(), o.end(), x->begin());
    }
};
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
