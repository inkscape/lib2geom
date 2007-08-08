#ifndef __GEOM_CROSSING_H
#define __GEOM_CROSSING_H

#include <vector>
#include <set>

namespace Geom {

struct Crossing {
    bool dir; //True: along a, a becomes outside.
    double ta, tb;  //time on a and b of crossing
    Crossing() : dir(false), ta(0), tb(0) {}
    Crossing(double t_a, double t_b, bool direction) : dir(direction), ta(t_a), tb(t_b) {}
    bool operator==(const Crossing & other) const { return dir == other.dir && ta == other.ta && tb == other.tb; }
    bool operator!=(const Crossing & other) const { return !(*this == other); }
};

inline bool near(Crossing a, Crossing b) {
    return near(a.ta, b.ta) && near(a.tb, b.tb);
}

struct NearF { bool operator()(Crossing a, Crossing b) { return near(a, b); } };

struct OrderA { bool operator()(Crossing a, Crossing b) { return a.ta < b. ta; } };
struct OrderB { bool operator()(Crossing a, Crossing b) { return a.tb < b. tb; } };

typedef std::vector<Crossing> Crossings;

inline void sortA(Crossings &cr) {
    OrderA f;
    std::sort(cr.begin(), cr.end(), f);
}

inline void sortB(Crossings &cr) {
    OrderB f;
    std::sort(cr.begin(), cr.end(), f);
}

template<typename T>
struct Eraser {
    T *x;
    typename T::iterator i, o;
    bool skip;
    
    Eraser(T *t) : x(t), skip(false) { i = o = t->begin(); }
    ~Eraser() { finish(); }
    
    bool operator==(Eraser const &other) { return other.i == i; }
    bool operator!=(Eraser const &other) { return other.i != i; }
    bool operator==(typename T::iterator const &it) { return it == i; }
    bool operator!=(typename T::iterator const &it) { return it != i; }
    
    typedef typename T::iterator::value_type value_type;
    typedef typename T::iterator::pointer pointer;
    
    value_type const operator*() const { return *i; }
    pointer operator->() const { return &(*i); }
    
    Eraser &operator++() {
        if(skip != true) {
            if(o != i) *o = *i;
            ++o;
        }
        ++i; 
        return *this;
    }
    
    Eraser operator++(int) {
        Eraser old=*this;
        ++(*this);
        return old;
    }
    
    void erase() { skip = true; }
    
    template<class InputIterator>
    void replace(InputIterator first, InputIterator last) {
        skip = true;
        const int siz = last - first;
        unsigned io = i - x->begin(), oo = o - x->begin();
        x->resize(x->size() + siz);
        i = x->begin() + io;
        o = x->begin() + oo;
        std::copy(first, last, o);
        o += siz;
    }
    void replace(T const &a) { replace(a.begin(), a.end()); }
    void replace(value_type const &a) {
        skip = true;
        *o = a;
        o++;
    }
    
    void finish() { x->resize(o - x->begin()); }
};

inline void clean(Crossings &cr_a, Crossings &cr_b) {
    if(cr_a.empty()) return;
    
    //Remove anything with dupes
    
    for(Eraser<Crossings> i(&cr_a); i != cr_a.end(); i++) {
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

}

#endif
