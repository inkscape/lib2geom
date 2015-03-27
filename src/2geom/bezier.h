/**
 * @file
 * @brief Bezier polynomial
 *//*
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Michael Sloan <mgsloan@gmail.com>
 *   Nathan Hurst <njh@njhurst.com>
 *
 * Copyright 2007 Authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 */

#ifndef LIB2GEOM_SEEN_BEZIER_H
#define LIB2GEOM_SEEN_BEZIER_H

#include <algorithm>
#include <valarray>
#include <boost/optional.hpp>
#include <2geom/choose.h>
#include <2geom/coord.h>
#include <2geom/d2.h>
#include <2geom/math-utils.h>
#include <2geom/solver.h>

namespace Geom {

/** @brief Compute the value of a Bernstein-Bezier polynomial.
 * This method uses a Horner-like fast evaluation scheme.
 * @param t Time value
 * @param c_ Pointer to coefficients
 * @param n Degree of the polynomial (number of coefficients minus one) */
template <typename T>
inline T bernstein_value_at(double t, T const *c_, unsigned n) {
    double u = 1.0 - t;
    double bc = 1;
    double tn = 1;
    T tmp = c_[0]*u;
    for(unsigned i=1; i<n; i++){
        tn = tn*t;
        bc = bc*(n-i+1)/i;
        tmp = (tmp + tn*bc*c_[i])*u;
    }
    return (tmp + tn*t*c_[n]);
}

/** @brief Perform Casteljau subdivision of a Bezier polynomial.
 * Given an array of coefficients and a time value, computes two new Bernstein-Bezier basis
 * polynomials corresponding to the \f$[0, t]\f$ and \f$[t, 1]\f$ intervals of the original one.
 * @param t Time value
 * @param v Array of input coordinates
 * @param left Output polynomial corresponding to \f$[0, t]\f$
 * @param right Output polynomial corresponding to \f$[t, 1]\f$
 * @param order Order of the input polynomial, equal to one less the number of coefficients
 * @return Value of the polynomial at @a t */
template <typename T>
inline T casteljau_subdivision(double t, T const *v, T *left, T *right, unsigned order) {
    // The Horner-like scheme gives very slightly different results, but we need
    // the result of subdivision to match exactly with Bezier's valueAt function.
    T val = bernstein_value_at(t, v, order);

    if (!left && !right) {
        return val;
    }

    if (!right) {
        if (left != v) {
            std::copy(v, v + order + 1, left);
        }
        for (std::size_t i = order; i > 0; --i) {
            for (std::size_t j = i; j <= order; ++j) {
                left[j] = lerp(t, left[j-1], left[j]);
            }
        }
        left[order] = val;
        return left[order];
    }

    if (right != v) {
        std::copy(v, v + order + 1, right);
    }
    for (std::size_t i = 1; i <= order; ++i) {
        if (left) {
            left[i-1] = right[0];
        }
        for (std::size_t j = i; j > 0; --j) {
            right[j-1] = lerp(t, right[j-1], right[j]);
        }
    }
    right[0] = val;
    if (left) {
        left[order] = right[0];
    }
    return right[0];
}

/**
 * @brief Polynomial in Bernstein-Bezier basis
 * @ingroup Fragments
 */
class Bezier
    : boost::arithmetic< Bezier, double
    , boost::additive< Bezier
      > >
{
private:
    std::valarray<Coord> c_;

    friend Bezier portion(const Bezier & a, Coord from, Coord to);
    friend OptInterval bounds_fast(Bezier const & b);
    friend Bezier derivative(const Bezier & a);
    friend class Bernstein;

    void
    find_bezier_roots(std::vector<double> & solutions,
                      double l, double r) const;

protected:
    Bezier(Coord const c[], unsigned ord)
        : c_(c, ord+1)
    {}

public:
    unsigned order() const { return c_.size()-1;}
    unsigned degree() const { return order(); }
    unsigned size() const { return c_.size();}

    Bezier() {}
    Bezier(const Bezier& b) :c_(b.c_) {}
    Bezier &operator=(Bezier const &other) {
        if ( c_.size() != other.c_.size() ) {
            c_.resize(other.c_.size());
        }
        c_ = other.c_;
        return *this;
    }

    struct Order {
        unsigned order;
        explicit Order(Bezier const &b) : order(b.order()) {}
        explicit Order(unsigned o) : order(o) {}
        operator unsigned() const { return order; }
    };

    //Construct an arbitrary order bezier
    Bezier(Order ord) : c_(0., ord.order+1) {
        assert(ord.order ==  order());
    }

    /// @name Construct Bezier polynomials from their control points
    /// @{
    explicit Bezier(Coord c0) : c_(0., 1) {
        c_[0] = c0;
    }
    Bezier(Coord c0, Coord c1) : c_(0., 2) {
        c_[0] = c0; c_[1] = c1;
    }
    Bezier(Coord c0, Coord c1, Coord c2) : c_(0., 3) {
        c_[0] = c0; c_[1] = c1; c_[2] = c2;
    }
    Bezier(Coord c0, Coord c1, Coord c2, Coord c3) : c_(0., 4) {
        c_[0] = c0; c_[1] = c1; c_[2] = c2; c_[3] = c3;
    }
    Bezier(Coord c0, Coord c1, Coord c2, Coord c3, Coord c4) : c_(0., 5) {
        c_[0] = c0; c_[1] = c1; c_[2] = c2; c_[3] = c3; c_[4] = c4;
    }
    Bezier(Coord c0, Coord c1, Coord c2, Coord c3, Coord c4,
           Coord c5) : c_(0., 6) {
        c_[0] = c0; c_[1] = c1; c_[2] = c2; c_[3] = c3; c_[4] = c4;
        c_[5] = c5;
    }
    Bezier(Coord c0, Coord c1, Coord c2, Coord c3, Coord c4,
           Coord c5, Coord c6) : c_(0., 7) {
        c_[0] = c0; c_[1] = c1; c_[2] = c2; c_[3] = c3; c_[4] = c4;
        c_[5] = c5; c_[6] = c6;
    }
    Bezier(Coord c0, Coord c1, Coord c2, Coord c3, Coord c4,
           Coord c5, Coord c6, Coord c7) : c_(0., 8) {
        c_[0] = c0; c_[1] = c1; c_[2] = c2; c_[3] = c3; c_[4] = c4;
        c_[5] = c5; c_[6] = c6; c_[7] = c7;
    }
    Bezier(Coord c0, Coord c1, Coord c2, Coord c3, Coord c4,
           Coord c5, Coord c6, Coord c7, Coord c8) : c_(0., 9) {
        c_[0] = c0; c_[1] = c1; c_[2] = c2; c_[3] = c3; c_[4] = c4;
        c_[5] = c5; c_[6] = c6; c_[7] = c7; c_[8] = c8;
    }
    Bezier(Coord c0, Coord c1, Coord c2, Coord c3, Coord c4,
           Coord c5, Coord c6, Coord c7, Coord c8, Coord c9) : c_(0., 10) {
        c_[0] = c0; c_[1] = c1; c_[2] = c2; c_[3] = c3; c_[4] = c4;
        c_[5] = c5; c_[6] = c6; c_[7] = c7; c_[8] = c8; c_[9] = c9;
    }

    template <typename Iter>
    Bezier(Iter first, Iter last) {
        c_.resize(std::distance(first, last));
        for (std::size_t i = 0; first != last; ++first, ++i) {
            c_[i] = *first;
        }
    }
    Bezier(std::vector<Coord> const &vec)
        : c_(&vec[0], vec.size())
    {}
    /// @}

    void resize (unsigned int n, Coord v = 0)
    {
        c_.resize (n, v);
    }

    void clear()
    {
        c_.resize(0);
    }

    //IMPL: FragmentConcept
    typedef Coord output_type;
    inline bool isZero(double eps=EPSILON) const {
        for(unsigned i = 0; i <= order(); i++) {
            if( ! are_near(c_[i], 0., eps) ) return false;
        }
        return true;
    }
    inline bool isConstant(double eps=EPSILON) const {
        for(unsigned i = 1; i <= order(); i++) {
            if( ! are_near(c_[i], c_[0], eps) ) return false;
        }
        return true;
    }
    inline bool isFinite() const {
        for(unsigned i = 0; i <= order(); i++) {
            if(!IS_FINITE(c_[i])) return false;
        }
        return true;
    }
    inline Coord at0() const { return c_[0]; }
    inline Coord &at0() { return c_[0]; }
    inline Coord at1() const { return c_[order()]; }
    inline Coord &at1() { return c_[order()]; }

    inline Coord valueAt(double t) const {
        return bernstein_value_at(t, &c_[0], order());
    }
    inline Coord operator()(double t) const { return valueAt(t); }

    SBasis toSBasis() const;

    inline Coord &operator[](unsigned ix) { return c_[ix]; }
    inline Coord const &operator[](unsigned ix) const { return const_cast<std::valarray<Coord>&>(c_)[ix]; }

    inline void setCoeff(unsigned ix, double val) { c_[ix] = val; }

    /**
    *  The size of the returned vector equals n_derivs+1.
    */
    std::vector<Coord> valueAndDerivatives(Coord t, unsigned n_derivs) const {
        /* This is inelegant, as it uses several extra stores.  I think there might be a way to
         * evaluate roughly in situ. */

         // initialize return vector with zeroes, such that we only need to replace the non-zero derivs
        std::vector<Coord> val_n_der(n_derivs + 1, Coord(0.0));

        // initialize temp storage variables
        std::valarray<Coord> d_(order()+1);
        for(unsigned i = 0; i < size(); i++) {
            d_[i] = c_[i];
        }

        unsigned nn = n_derivs + 1;
        if(n_derivs > order()) {
            nn = order()+1; // only calculate the non zero derivs
        }
        for(unsigned di = 0; di < nn; di++) {
            //val_n_der[di] = (casteljau_subdivision(t, &d_[0], NULL, NULL, order() - di));
            val_n_der[di] = bernstein_value_at(t, &d_[0], order() - di);
            for(unsigned i = 0; i < order() - di; i++) {
                d_[i] = (order()-di)*(d_[i+1] - d_[i]);
            }
        }

        return val_n_der;
    }

    void subdivide(Coord t, Bezier *left, Bezier *right) const {
		if (left) {
			left->c_.resize(size());
			if (right) {
				right->c_.resize(size());
				casteljau_subdivision<double>(t, &const_cast<std::valarray<Coord>&>(c_)[0],
					&left->c_[0], &right->c_[0], order());
			} else {
				casteljau_subdivision<double>(t, &const_cast<std::valarray<Coord>&>(c_)[0],
					&left->c_[0], NULL, order());
			}
		} else if (right) {
			right->c_.resize(size());
			casteljau_subdivision<double>(t, &const_cast<std::valarray<Coord>&>(c_)[0],
				NULL, &right->c_[0], order());
		}
	}
	std::pair<Bezier, Bezier > subdivide(Coord t) const {
        std::pair<Bezier, Bezier> ret;
        subdivide(t, &ret.first, &ret.second);
        return ret;
    }

    std::vector<double> roots() const {
        std::vector<double> solutions;
        find_bezier_roots(solutions, 0, 1);
        std::sort(solutions.begin(), solutions.end());
        return solutions;
    }
    std::vector<double> roots(Interval const &ivl) const {
        std::vector<double> solutions;
        find_bernstein_roots(&const_cast<std::valarray<Coord>&>(c_)[0], order(), solutions, 0, ivl.min(), ivl.max());
        std::sort(solutions.begin(), solutions.end());
        return solutions;
    }

    Bezier forward_difference(unsigned k) {
        Bezier fd(Order(order()-k));
        unsigned n = fd.size();
        
        for(unsigned i = 0; i < n; i++) {
            fd[i] = 0;
            for(unsigned j = i; j < n; j++) {
                fd[i] += (((j)&1)?-c_[j]:c_[j])*choose<double>(n, j-i);
            }
        }
        return fd;
    }

    Bezier elevate_degree() const {
        Bezier ed(Order(order()+1));
        unsigned n = size();
        ed[0] = c_[0];
        ed[n] = c_[n-1];
        for(unsigned i = 1; i < n; i++) {
            ed[i] = (i*c_[i-1] + (n - i)*c_[i])/(n);
        }
        return ed;
    }

    Bezier reduce_degree() const {
        if(order() == 0) return *this;
        Bezier ed(Order(order()-1));
        unsigned n = size();
        ed[0] = c_[0];
        ed[n-1] = c_[n]; // ensure exact endpoints
        unsigned middle = n/2;
        for(unsigned i = 1; i < middle; i++) {
            ed[i] = (n*c_[i] - i*ed[i-1])/(n-i);
        }
        for(unsigned i = n-1; i >= middle; i--) {
            ed[i] = (n*c_[i] - i*ed[n-i])/(i);
        }
        return ed;
    }

    Bezier elevate_to_degree(unsigned newDegree) const {
        Bezier ed = *this;
        for(unsigned i = degree(); i < newDegree; i++) {
            ed = ed.elevate_degree();
        }
        return ed;
    }

    Bezier deflate() const {
        if(order() == 0) return *this;
        unsigned n = order();
        Bezier b(Order(n-1));
        for(unsigned i = 0; i < n; i++) {
            b[i] = (n*c_[i+1])/(i+1);
        }
        return b;
    }

    // basic arithmetic operators
    Bezier &operator+=(double v) {
        c_ += v;
        return *this;
    }
    Bezier &operator-=(double v) {
        c_ -= v;
        return *this;
    }
    Bezier &operator*=(double v) {
        c_ *= v;
        return *this;
    }
    Bezier &operator/=(double v) {
        c_ /= v;
        return *this;
    }
    Bezier &operator+=(Bezier const &other) {
        if (c_.size() > other.size()) {
            c_ += other.elevate_to_degree(degree()).c_;
        } else if (c_.size() < other.size()) {
            *this = elevate_to_degree(other.degree());
            c_ += other.c_;
        } else {
            c_ += other.c_;
        }
        return *this;
    }
    Bezier &operator-=(Bezier const &other) {
        if (c_.size() > other.size()) {
            c_ -= other.elevate_to_degree(degree()).c_;
        } else if (c_.size() < other.size()) {
            *this = elevate_to_degree(other.degree());
            c_ -= other.c_;
        } else {
            c_ -= other.c_;
        }
        return *this;
    }
};


void bezier_to_sbasis (SBasis & sb, Bezier const& bz);

inline
Bezier multiply(Bezier const& f, Bezier const& g) {
    unsigned m = f.order();
    unsigned n = g.order();
    Bezier h(Bezier::Order(m+n));
    // h_k = sum_(i+j=k) (m i)f_i (n j)g_j / (m+n k)
    
    for(unsigned i = 0; i <= m; i++) {
        const double fi = choose<double>(m,i)*f[i];
        for(unsigned j = 0; j <= n; j++) {
            h[i+j] += fi * choose<double>(n,j)*g[j];
        }
    }
    for(unsigned k = 0; k <= m+n; k++) {
        h[k] /= choose<double>(m+n, k);
    }
    return h;
}

inline
SBasis Bezier::toSBasis() const {
    SBasis sb;
    bezier_to_sbasis(sb, (*this));
    return sb;
    //return bezier_to_sbasis(&c_[0], order());
}

inline Bezier reverse(const Bezier & a) {
    Bezier result = Bezier(Bezier::Order(a));
    for(unsigned i = 0; i <= a.order(); i++)
        result[i] = a[a.order() - i];
    return result;
}

inline Bezier portion(const Bezier & a, double from, double to) {
    Bezier ret(a);

    bool reverse_result = false;
    if (from > to) {
        std::swap(from, to);
        reverse_result = true;
    }

    do {
        if (from == 0) {
            if (to == 1) {
                break;
            }
            casteljau_subdivision<double>(to, &ret.c_[0], &ret.c_[0], NULL, ret.order());
            break; 
        }
        casteljau_subdivision<double>(from, &ret.c_[0], NULL, &ret.c_[0], ret.order());
        if (to == 1) break;
        casteljau_subdivision<double>((to - from) / (1 - from), &ret.c_[0], &ret.c_[0], NULL, ret.order());
        // to protect against numerical inaccuracy in the above expression, we manually set
        // the last coefficient to a value evaluated directly from the original polynomial
        ret.c_[ret.order()] = a.valueAt(to);
    } while(0);

    if (reverse_result) {
        std::reverse(&ret.c_[0], &ret.c_[0] + ret.c_.size());
    }
    return ret;
}

// XXX Todo: how to handle differing orders
inline std::vector<Point> bezier_points(const D2<Bezier > & a) {
    std::vector<Point> result;
    for(unsigned i = 0; i <= a[0].order(); i++) {
        Point p;
        for(unsigned d = 0; d < 2; d++) p[d] = a[d][i];
        result.push_back(p);
    }
    return result;
}

inline Bezier derivative(const Bezier & a) {
    //if(a.order() == 1) return Bezier(0.0);
    if(a.order() == 1) return Bezier(a.c_[1]-a.c_[0]);
    Bezier der(Bezier::Order(a.order()-1));

    for(unsigned i = 0; i < a.order(); i++) {
        der.c_[i] = a.order()*(a.c_[i+1] - a.c_[i]);
    }
    return der;
}

inline Bezier integral(const Bezier & a) {
    Bezier inte(Bezier::Order(a.order()+1));

    inte[0] = 0;
    for(unsigned i = 0; i < inte.order(); i++) {
        inte[i+1] = inte[i] + a[i]/(inte.order());
    }
    return inte;
}

inline OptInterval bounds_fast(Bezier const & b) {
    OptInterval ret = Interval::from_array(&const_cast<Bezier&>(b).c_[0], b.size());
    return ret;
}

inline OptInterval bounds_exact(Bezier const & b) {
    OptInterval ret(b.at0(), b.at1());
    std::vector<Coord> r = derivative(b).roots();
    for (unsigned i = 0; i < r.size(); ++i) {
        ret->expandTo(b.valueAt(r[i]));
    }
    return ret;
}

inline OptInterval bounds_local(Bezier const & b, OptInterval i) {
    //return bounds_local(b.toSBasis(), i);
    if (i) {
        return bounds_fast(portion(b, i->min(), i->max()));
    } else {
        return OptInterval();
    }
}

inline std::ostream &operator<< (std::ostream &os, const Bezier & b) {
    os << "Bezier(";
    for(unsigned i = 0; i < b.order(); i++) {
        os << format_coord_nice(b[i]) << ", ";
    }
    os << format_coord_nice(b[b.order()]) << ")";
    return os;
}

}
#endif // LIB2GEOM_SEEN_BEZIER_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
