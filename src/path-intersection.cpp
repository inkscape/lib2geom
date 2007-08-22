#include "path-intersection.h"

#include "basic-intersection.h"
#include "bezier-to-sbasis.h"
#include "ord.h"
#include "sweep.h"

//for path_direction:
#include "sbasis-geometric.h"

namespace Geom {

int winding(Path const &path, Point p) {
  //start on a segment which is not a horizontal line with y = p[y]
  Path::const_iterator start;
  for(Path::const_iterator iter = path.begin(); ; ++iter) {
    if(iter == path.end_closed()) { return 0; }
    if(iter->initialPoint()[Y]!=p[Y])  { start = iter; break; }
    if(iter->finalPoint()[Y]!=p[Y])    { start = iter; break; }
    if(iter->boundsFast().height()!=0.){ start = iter; break; }
  }
  int wind = 0;
  bool temp = true;
  for (Path::const_iterator iter = start; iter != start || temp
       ; ++iter, iter = (iter == path.end_closed()) ? path.begin() : iter )
  {
    temp = false;
    Rect bounds = iter->boundsFast();
    Coord x = p[X], y = p[Y];
    if(x > bounds.right() || !bounds[Y].contains(y)) continue;
    Point final = iter->finalPoint();
    Point initial = iter->initialPoint();
    Cmp final_to_ray = cmp(final[Y], y);
    Cmp initial_to_ray = cmp(initial[Y], y);
    // if y is included, these will have opposite values, giving order.
    Cmp c = cmp(final_to_ray, initial_to_ray); 
    if(x < bounds.left()) {    //ray goes through bbox
        // winding determined by position of endpoints
        if(final_to_ray != EQUAL_TO) {
            wind += int(c); // GT = counter-clockwise = 1; LT = clockwise = -1; EQ = not-included = 0
            //std::cout << int(c) << " ";
            goto cont;
        }
    } else {
        //inside bbox, use custom per-curve winding thingie
        int delt = iter->winding(p);
        wind += delt;
        //std::cout << "n" << delt << " ";
    }
    //Handling the special case of an endpoint on the ray:
    if(final[Y] == y) {
        //Traverse segments until it breaks away from y
        //99.9% of the time this will happen the first go
        Path::const_iterator next = iter;
        next++;
        for(; ; next++) {
            if(next == path.end_closed()) next = path.begin();
            Rect bnds = next->boundsFast();
            //TODO: X considerations
            if(bnds.height() > 0) {
                //It has diverged
                if(bnds.contains(p)) {
                    const double fudge = 0.01;
                    if(cmp(y, next->valueAt(fudge, Y)) == initial_to_ray) {
                        wind += int(c);
                        std::cout << "!!!!!" << int(c) << " ";
                    }
                    iter = next; // No increment, as the rest of the thing hasn't been counted.
                } else {
                    Coord ny = next->initialPoint()[Y];
                    if(cmp(y, ny) == initial_to_ray) {
                        //Is a continuation through the ray, so counts windingwise
                        wind += int(c);
                        std::cout << "!!!!!" << int(c) << " ";
                    }
                    iter = ++next;
                }
                goto cont;
            }
            if(next==start) return wind;
        }
        //Looks like it looped, which means everything's flat
        return 0;
    }
    
    cont:(void)0;
  }
  return wind;
}

//returns true for fill, false for hole
bool path_direction(Path const &p) {
    //could probably be more efficient, but this is a quick job
    double y = p.initialPoint()[Y];
    double x = p.initialPoint()[X];
    Cmp res = EQUAL_TO;
    goto doh;
    for(unsigned i = 1; i < p.size(); i++) {
        Cmp final_to_ray = cmp(p[i].finalPoint()[Y], y);
        Cmp initial_to_ray = cmp(p[i].initialPoint()[Y], y);
        // if y is included, these will have opposite values, giving order.
        Cmp c = cmp(final_to_ray, initial_to_ray);
        if(c != EQUAL_TO) {
            std::vector<double> rs = p[i].roots(y, Y);
            for(unsigned j = 0; j < rs.size(); j++) {
                double nx = p[i].valueAt(rs[j], X);
                if(nx > x) {
                    x = nx;
                    res = c;
                }
            }
        } else if(final_to_ray == EQUAL_TO) {
            goto doh;
        }
    }
    return res > 0;
    
    doh:
        //Otherwise fallback on area
        Piecewise<D2<SBasis> > pw = p.toPwSb();
        double area;
        Point centre;
        Geom::centroid(pw, centre, area);
        return area > 0;
}

/*
int path_direction(Path p) {
    int wind = 0, max = 0;
    for (Path::const_iterator iter = p.begin(); iter != p.end(); ++iter) {
        iter->initialPoint()
        
    }
}*/

/*
//Finds intervals larger than a particular width, which contain a crossing
std::vector<std::pair<Interval, Interval> > break_down(Curve const & a, Curve const & b, double width) {
    std::vector<std::pair<Interval, Interval> > res;
    int prev = b.winding(a.initialPoint());
    for(double t = width; t <= 1; t += width) {
        int val = b.winding(a.valueAt(t)
        if(val != prev) res.push_back(std::pair<Interval, Interval>(Interval(t-width,t),
                                     Interval();
    }
}
*/
/*
Crossings crossings_recurse(Curve const &a, Curve const &b,
                            std::vector<Interval> a_i, std::vector<Interval> b_i) {
    Crossings ret;
    a_bounds = a.boundsFast(); 
    b_bounds = b.boundsFast();
    if(a_bounds.intersects(b_bounds)) {
        
    }
    return ret;
}
*/

//pair intersect code based on njh's pair-intersect


template<typename T>
void append(T &a, T const &b) {
    a.insert(a.end(), b.begin(), b.end());
}

bool
linear_intersect(Point A0, Point A1, Point B0, Point B1,
                 double &tA, double &tB, double &det) {
    // kramers rule as cross products
    Point Ad = A1 - A0,
          Bd = B1 - B0,
           d = B0 - A0;
    det = cross(Ad, Bd);
    if( 1.0 + det == 1.0 )
        return false;
    else
    {
        double detinv = 1.0 / det;
        tA = cross(d, Bd) * detinv;
        tB = cross(d, Ad) * detinv;
        return tA > 0. && tA < 1. && tB > 0. && tB < 1.;
    }
}

void pair_intersect(std::vector<double> &Asects,
                    std::vector<double> &Bsects,
                    Curve const & A, double Al, double Ah, 
                    Curve const & B, double Bl, double Bh, unsigned depth=0) {
   // std::cout << depth << "(" << Al << ", " << Ah << ")\n";
    Rect Ar = SBasisCurve(A).boundsLocal(Interval(Al, Ah),0);
    if(Ar.isEmpty()) return;

    Rect Br = SBasisCurve(B).boundsLocal(Interval(Bl, Bh),0);
    if(Br.isEmpty()) return;
    
    if((depth > 12) || Ar.intersects(Br)) {
        if((depth > 12)) {
            double tA, tB, c;
            if(linear_intersect(A.pointAt(Al), A.pointAt(Ah), 
                                     B.pointAt(Bl), B.pointAt(Bh), 
                                     tA, tB, c)) {
                tA = tA * (Ah - Al) + Al;
                tB = tB * (Bh - Bl) + Bl;
                //std::cout << "intersects " << tA << ", " << tB << "\n";
                Asects.push_back(tA);
                Bsects.push_back(tB);
            }
            
        } else {
            double mid = (Al + Ah)/2;
            pair_intersect(Bsects, Asects,
                           B, Bl, Bh,
                           A, Al, mid, depth+1);
            pair_intersect(Bsects, Asects,
                           B, Bl, Bh,
                           A, mid, Ah, depth+1);
        }
    }
}
/*

void pair_intersect(std::vector<double> &Asects,
                    std::vector<double> &Bsects,
                    Curve const & A, double Al, double Ah, 
                    Curve const & B, double Bl, double Bh, unsigned depth=0) {
    std::cout << depth << "(" << Al << ", " << Ah << ")\n";

    Rect Ar = A.boundsLocal(Interval(Al, Ah));
    if(Ar.isEmpty()) return;
    Rect Br = B.boundsLocal(Interval(Bl, Bh));
    if(Br.isEmpty()) return;
    if(!Ar.intersects(Br)) return;
    
    if(depth <= 12 && Ah - Al > 0.1 && Bh - Bl > 0.1) {
        // We'll split B, and swap every recursion
        pair_intersect(Bsects, Asects,
                    B, Bl, (Bl+Bh)/2,
                    A, Al, Ah, depth+1);
        pair_intersect(Bsects, Asects,
                    B, (Bl+Bh)/2, Bh,
                    A, Al, Ah, depth+1);

    } else {
        double tA, tB;
        if(linear_pair_intersect(A, Al, Ah, 
                        B, Bl, Bh, 
                        tA, tB)) {
            std::cout << "Intersected\n";
            Asects.push_back(tA);
            Bsects.push_back(tB);
        }
    }
  
    //Make a bit of a guess for a split point in B
    double mid;
    Dim2 dim = Br.height() > Br.width() ? Y : X;
    double weight = (B.valueAt(Bh,dim) - B.valueAt(Bl,dim)) / Br[dim].extent();
    double ma = Ar[dim].middle(), mb = Br[dim].middle();
    if(!Br[dim].contains(ma)) {
        if(fabs(Ar[dim].min() - mb) < fabs(Ar[dim].max() - mb)) {
            if(Br[dim].contains(Ar[dim].min())) {
                ma = Ar[dim].min();
            } else {
                mid = 0.5; goto skipper;
            }
        } else {
            if(Br[dim].contains(Ar[dim].max())) {
                ma = Ar[dim].max();
            } else {
                mid = 0.5; goto skipper;
            }
        }
    }
    mid = ((ma - Br[dim].min()) / Br[dim].extent() * weight + .5) / (1 + fabs(weight));    mid = 0.5;
    mid = (Bh - Bl) * mid + Bl;
    std::cout << "weight: " << weight << " mid: " << mid << "\n";
} */

Crossings to_crossings(std::vector<std::pair<double, double> > ts, Curve const &a, Curve const &b) { 
    Crossings ret;
    for(unsigned i = 0; i < ts.size(); i++) {
        double at = ts[i].first, bt = ts[i].second;
        ret.push_back(Crossing(at, bt, cross(a.pointAt(at) - a.pointAt(at + .01),
                                             b.pointAt(bt) - b.pointAt(bt + .01)) > 0));
    }
    return ret;
}

template<typename A, typename B>
std::vector<std::pair<A, B> > zip(std::vector<A> const &a, std::vector<B> &b) {
    std::vector<std::pair<A, B> > ret;
    for(unsigned i = 0; i < a.size() && i < b.size(); i++)
        ret.push_back(std::pair<A, B>(a[i], b[i]));
    return ret;
}

Crossings SBCrosser::operator()(Curve const &a, Curve const &b) {
    std::vector<double> asects, bsects;
    pair_intersect(asects, bsects, a, 0, 1, b, 0, 1);
    return to_crossings(zip(asects, bsects), a, b);
}

int cnt;

void mono_pair(Curve const &A, double Al, double Ah,
               Curve const &B, double Bl, double Bh,
               unsigned depth, Crossings &ret, double tol) {
    if(depth >13 || Al > Ah || Bl > Bh) return;
    //std::cout << depth << "[" << Al << ", " << Ah << "]" << "[" << Bl << ", " << Bh << "]\n";
    cnt++;
    //std::cout << cnt << "\n";
    //Split B
    Point A0 = A.pointAt(Al), A1 = A.pointAt(Ah),
          B0 = B.pointAt(Bl), B1 = B.pointAt(Bh);
    //inline code that this implies? (without rect/interval construction)
    if(!Rect(A0, A1).intersects(Rect(B0, B1))) return;
    
    double tA, tB, c;

    if(linear_intersect(A0, A1, B0, B1, tA, tB, c)) {
        tA = tA * (Ah - Al) + Al;
        tB = tB * (Bh - Bl) + Bl;
        double dist = LInfty(A.pointAt(tA) - A.pointAt(tB));
        std::cout << dist;
        if(depth >= 12 || dist <= tol) {
            ret.push_back(Crossing(tA, tB, c > 0));
            return;
        }
        double hwidth = 0.1;
        if(tB - hwidth*2 <= 0) tB += hwidth; else
        if(tB + hwidth*2 >= 1) tB -= hwidth; else {
            mono_pair(B, Bl, tB - hwidth,
                      A, Al, Ah,
                      depth+1, ret, tol);
            mono_pair(B, tB - hwidth, tB + hwidth,
                      A, Al, Ah,
                      depth+1, ret, tol);
            mono_pair(B, tB + hwidth, Bh,
                      A, Al, Ah,
                      depth+1, ret, tol);
            return;
        }
        
    } //else
     tB = (Bl + Bh) / 2;
    mono_pair(B, Bl, tB,
              A, Al, Ah,
              depth+1, ret, tol);
    mono_pair(B, tB, Bh,
              A, Al, Ah,
              depth+1, ret, tol);
}

std::vector<double> mono_splits(Curve const &d) {
    std::vector<double> rs = d.roots(0, X);
    append(rs, d.roots(0, Y));
    rs.push_back(0); rs.push_back(1);
    std::sort(rs.begin(), rs.end());
    return rs;
}

Crossings NewCrosser::operator()(Curve const &a, Curve const &b) {
    Crossings ret;
    //Curve *da = a.derivative(), *db = b.derivative();
    //std::vector<double> sa = mono_splits(*da), sb = mono_splits(*db);
    std::vector<double> sa,sb; sa.push_back(1); sb.push_back(1);
    //delete da; delete db;
    for(unsigned i = 0; i < sa.size(); i++) {
        for(unsigned j = 0; j < sb.size(); j++) {
            //ret.push_back(Crossing(sa[i], sb[j], false));
            mono_pair(a, sa[i-1], sa[i],
                      b, sb[i-1], sb[i],
                      0, ret, .01);
        }
    }
    return ret;
}

std::vector<Rect> curve_bounds(Path const &x) {
    std::vector<Rect> ret;
    for(Path::const_iterator it = x.begin(); it != x.end_closed(); ++it)
        ret.push_back(it->boundsFast());
    return ret;
}

Crossings crossings(Path const &a, Path const &b, Crosser &c) {
    Crossings ret;
    std::vector<Rect> bounds_a = curve_bounds(a), bounds_b = curve_bounds(b);
    //TODO: pass bounds into sweeper
    std::vector<std::vector<unsigned> > ixs = fake_cull(a.size(), b.size());
    for(unsigned i = 0; i < a.size(); i++) {
        for(std::vector<unsigned>::iterator jp = ixs[i].begin(); jp != ixs[i].end(); jp++) {
            if(bounds_a[i].intersects(bounds_b[*jp])) {
                Crossings cc = bounds_a[i].area() < bounds_b[*jp].area() ? c(a[i], b[*jp]) : c(b[*jp], a[i]);
                //TODO: remove this loop and pass in some indicators
                for(Crossings::iterator it = cc.begin(); it != cc.end(); it++) {
                    ret.push_back(Crossing(it->ta + i, it->tb + *jp, it->dir));
                }
            }
        }
    }
    return ret;
}


Crossings self_crossings(Path const &a, Crosser &c) {
    Crossings ret;
    
    //TODO: sweep
    std::vector<Rect> bounds = curve_bounds(a);
    for(unsigned i = 0; i < bounds.size(); i++) {
        Crossings cc = to_crossings(find_self_intersections(a[i].toSBasis()), a[i], a[i]);
        for(Crossings::iterator it = cc.begin(); it != cc.end(); it++) {
            ret.push_back(Crossing(it->ta + i, it->tb + i, it->dir));
        }
        for(unsigned j = i+1; j < bounds.size(); j++) {
            if(bounds[i].intersects(bounds[j])) {
                cc = bounds[i].area() < bounds[j].area() ? c(a[i], a[j]) : c(a[j], a[i]);
                for(Crossings::iterator it = cc.begin(); it != cc.end(); it++) {
                    ret.push_back(Crossing(it->ta + i, it->tb + j, it->dir));
                }
            }
        }
    }
    return ret;
}   

CrossingSet crossings_among(std::vector<Path> const &ps, Crosser &c) {
    CrossingSet results(ps.size(), Crossings());
    
    //TODO: sweep
    for(unsigned i = 0; i < ps.size(); i++) {
        Crossings cr = self_crossings(ps[i], c);
        for(unsigned k = 0; k < cr.size(); k++) cr[k].a = cr[k].b = i;
        //Sort & add crossings
        sort_crossings(cr, i);
        Crossings n(results[i].size() + cr.size());
        std::merge(results[i].begin(), results[i].end(), cr.begin(), cr.end(), n.begin(), CrossingOrder(i));
        results[i] = n;
        for(unsigned j = i+1; j < ps.size(); j++) {
            cr = crossings(ps[i], ps[j]);
            for(unsigned k = 0; k < cr.size(); k++) { cr[k].a = i; cr[k].b = j; }
            //Sort & add I crossings
            sort_crossings(cr, i);
            n.resize(results[i].size() + cr.size());
            std::merge(results[i].begin(), results[i].end(), cr.begin(), cr.end(), n.begin(), CrossingOrder(i));
            results[i] = n;
            //Sort & add J crossings
            sort_crossings(cr, j);
            n.resize(results[j].size() + cr.size());
            std::merge(results[j].begin(), results[j].end(), cr.begin(), cr.end(), n.begin(), CrossingOrder(j));
            results[j] = n;
        }
    }
    return results;
}

}
