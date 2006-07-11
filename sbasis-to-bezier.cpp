/* From Sanchez-Reyes 1997
   W_{j,k} = W_{n0j, n-k} = choose(n-2k-1, j-k)choose(2k+1,k)/choose(n,j)
     for k=0,...,q-1; j = k, ...,n-k-1
   W_{q,q} = 1 (n even)

This is wrong, it should read
   W_{j,k} = W_{n0j, n-k} = choose(n-2k-1, j-k)/choose(n,j)
     for k=0,...,q-1; j = k, ...,n-k-1
   W_{q,q} = 1 (n even)

*/
#include "sbasis-to-bezier.h"
#include "choose.h"

double W(unsigned n, unsigned j, unsigned k) {
    unsigned q = (n+1)/2;
    if((n & 1) == 0 && j == q && k == q)
        return 1;
    if(k > n-k) return W(n, n-j, n-k);
    assert(!(k < 0));
    if(k < 0) return 0;
    assert(!(k >= q));
    if(k >= q) return 0;
    //assert(!(j >= n-k));
    if(j >= n-k) return 0;
    //assert(!(j < k));
    if(j < k) return 0;
    return choose<double>(n-2*k-1, j-k) /
        choose<double>(n,j);
}

// this produces a degree k bezier from a degree k sbasis
std::vector<double>
sbasis_to_bezier(SBasis const &B, unsigned q) {
    std::vector<double> result;
    if(q > B.size())
        q = B.size();
    unsigned n = q*2;
    result.resize(n, 0);
    n--;
    for(int k = 0; k < q; k++) {
        for(int j = 0; j <= n-k; j++) {
            result[j] += (W(n, j, k)*B[k][0] +
                          W(n, n-j, k)*B[k][1]);
        }
    }
    return result;
}

// this produces a degree k bezier from a degree k sbasis
std::vector<Geom::Point>
sbasis_to_bezier(multidim_sbasis<2> const &B, unsigned q) {
    std::vector<Geom::Point> result;
    if(q > B.size())
        q = B.size();
    unsigned n = q*2;
    result.resize(n, Geom::Point(0,0));
    n--;
    for(int dim = 0; dim < 2; dim++) {
        for(int k = 0; k < q; k++) {
            for(int j = 0; j <= n-k; j++) {
                result[j][dim] += (W(n, j, k)*B[dim][k][0] +
                             W(n, n-j, k)*B[dim][k][1]);
                }
        }
    }
    return result;
}

std::vector<Geom::Point>
sbasis2_to_bezier(multidim_sbasis<2> const &B, unsigned q) {
    if(B.size() != 2)
	    return sbasis_to_bezier(B, B.size());
    std::vector<Geom::Point> result;
    const double third = 1./3;
    result.resize(4, Geom::Point(0,0));
    for(int dim = 0; dim < 2; dim++) {
        const SBasis &Bd(B[dim]);
        result[0][dim] = Bd[0][0];
        result[1][dim] = third*(2*Bd[0][0] +   Bd[0][1] + Bd[1][0]);
        result[2][dim] = third*(  Bd[0][0] + 2*Bd[0][1] + Bd[1][1]);
        result[3][dim] = Bd[0][1];
    }
    return result;
}


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
