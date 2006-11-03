#ifndef _BEZIER_TO_SBASIS
#define _BEZIER_TO_SBASIS

#include "multidim-sbasis.h"

template <typename T, unsigned D, unsigned order>
struct bezier_to_sbasis_impl {
    static inline multidim_sbasis<D> compute(T const &handles) {
        return multiply(BezOrd(1, 0), bezier_to_sbasis_impl<T, D, order-1>::compute(handles)) +
            multiply(BezOrd(0, 1), bezier_to_sbasis_impl<T, D, order-1>::compute(handles+1));
    }
};

template <typename T, unsigned D>
struct bezier_to_sbasis_impl<T, D, 1> {
    static inline multidim_sbasis<D> compute(T const &handles) {
        multidim_sbasis<D> mdsb;
        for(unsigned d = 0 ; d < D; d++) {
            mdsb[d] = BezOrd(handles[0][d], handles[1][d]);
        }
        return mdsb;
    }
};

template <typename T, unsigned D>
struct bezier_to_sbasis_impl<T, D, 0> {
    static inline multidim_sbasis<D> compute(T const &handles) {
        multidim_sbasis<D> mdsb;
        for(unsigned d = 0 ; d < D; d++) {
            mdsb[d] = BezOrd(handles[0][d], handles[0][d]);
        }
        return mdsb;
    }
};

template <unsigned D, unsigned order, typename T>
inline multidim_sbasis<D>
bezier_to_sbasis(T const &handles) {
    return bezier_to_sbasis_impl<T, D, order>::compute(handles);
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
