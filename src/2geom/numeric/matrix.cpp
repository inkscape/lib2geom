#include <numeric/matrix.h>

namespace Geom { namespace NL {

Vector operator*( detail::BaseMatrixImpl const& A, 
                  detail::BaseVectorImpl const& v )
{
    assert(A.columns() == v.size());
    
    Vector result(A.rows(), 0.0);
    for (size_t i = 0; i < A.rows(); ++i)
        for (size_t j = 0; j < A.columns(); ++j)
            result[i] += A(i,j) * v[j];
    
    return result;
}

Matrix operator*( detail::BaseMatrixImpl const& A, 
                  detail::BaseMatrixImpl const& B )
{
    assert(A.columns() == B.rows());
    
    Matrix C(A.rows(), B.columns(), 0.0);
    for (size_t i = 0; i < C.rows(); ++i)
        for (size_t j = 0; j < C.columns(); ++j)
            for (size_t k = 0; k < A.columns(); ++k)
                C(i,j) += A(i,k) * B(k, j);
 
    return C;
}

Matrix pseudo_inverse(detail::BaseMatrixImpl const& A)
{
    
    Matrix U(A);
    Matrix V(A.columns(), A.columns());
    Vector s(A.columns());  
    gsl_vector* work = gsl_vector_alloc(A.columns());
    
    gsl_linalg_SV_decomp( U.get_gsl_matrix(), 
                          V.get_gsl_matrix(), 
                          s.get_gsl_vector(), 
                          work );
    
    Matrix P(A.columns(), A.rows(), 0.0);
    
    int sz = s.size();
    while ( sz-- > 0 && s[sz] == 0 ) {}
    ++sz;
    if (sz == 0) return P;
    VectorView sv(s, sz);
    
    for (size_t i = 0; i < sv.size(); ++i)
    {
        VectorView v = V.column_view(i);
        v.scale(1/sv[i]);
        for (size_t h = 0; h < P.rows(); ++h)
            for (size_t k = 0; k < P.columns(); ++k)
                P(h,k) += V(h,i) * U(k,i);
    }
    
    return P;
}

};
}
