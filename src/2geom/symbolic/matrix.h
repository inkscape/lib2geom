/*
 * Matrix<CoeffT> class template
 *
 * Authors:
 *      Marco Cecchetti <mrcekets at gmail.com>
 *
 * Copyright 2008  authors
 *
 */


#ifndef _GEOM_SL_MATRIX_H_
#define _GEOM_SL_MATRIX_H_


#include <vector>
#include <map>
#include <2geom/symbolic/multipoly.h>




namespace Geom { namespace SL {

/*
 *  generic Matrix class template
 *  needed for building up a matrix with polynomial entries
 */
template< typename Coeff>
class Matrix
{
  public:
    typedef Coeff coeff_type;
    typedef std::vector<coeff_type> container_type;

    Matrix()
    {}

    Matrix(size_t m, size_t n)
        : m_data(m*n), m_rows(m), m_columns(n)
    {
    }

    void resize(size_t m, size_t n)
    {
        m_data.resize(m,n);
        m_rows = m;
        m_columns = n;
    }

    size_t rows() const
    {
        return m_rows;
    }

    size_t columns() const
    {
        return m_columns;
    }

    coeff_type const& operator() (size_t i, size_t j) const
    {
        return m_data[i * columns() + j];
    }

    coeff_type & operator() (size_t i, size_t j)
    {
        return m_data[i * columns() + j];
    }

  private:
    container_type m_data;
    size_t m_rows;
    size_t m_columns;
};


template< typename Coeff, typename charT >
inline
std::basic_ostream<charT> &
operator<< ( std::basic_ostream<charT> & os,
             const Matrix<Coeff> & _matrix )
{
    if (_matrix.rows() == 0 || _matrix.columns() == 0) return os;

    os << "{{" << _matrix(0,0);
    for (size_t j = 1; j < _matrix.columns(); ++j)
    {
        os << ", " << _matrix(0,j);
    }
    os << "}";

    for (size_t i = 1; i < _matrix.rows(); ++i)
    {
        os << ", {" << _matrix(i,0);
        for (size_t j = 1; j < _matrix.columns(); ++j)
        {
            os << ", " << _matrix(i,j);
        }
        os << "}";
    }
    os << "}";
    return os;
}


/*
template< typename Coeff>
class SymmetricSquareMatrix
{
  public:
    typedef Coeff coeff_type;
    typedef std::vector<coeff_type> container_type;

    SymmetricSquareMatrix(size_t n)
        : m_data((n*n)/2 + n), m_size(n)
    {

    }

    size_t rows() const
    {
        return m_size;
    }

    size_t columns() const
    {
        return m_size;
    }

    coeff_type const& operator() (size_t i, size_t j) const
    {
        return m_data[i * columns() + j];
    }

    coeff_type & operator() (size_t i, size_t j)
    {
        return m_data[i * columns() + j];
    }

    coeff_type det()
    {

    }

  private:
    container_type m_data;
    size_t m_size;
};
*/

/*
 * This is an adaptation of the LU algorithm used in the numerical case.
 * This algorithm is based on the article due to Bareiss:
 * "Sylvester's identity and multistep integer-preserving Gaussian elimination"
 */

/*
template< typename CoeffT >
CoeffT det(Matrix<CoeffT> const& M)
{
    assert(M.rows() == M.columns());

    Matrix<CoeffT> A(M);
    CoeffT n;
    CoeffT d = one<CoeffT>()();
    for (size_t k = 1; k < A.rows(); ++k)
    {
        for (size_t i = k; i < A.rows(); ++i)
        {
            for (size_t j = k; j < A.columns(); ++j)
            {
                n = A(i,j) * A(k-1,k-1) - A(k-1,j) * A(i,k-1);
//                std::cout << "k, i, j: "
//                          << k << ", " << i << ",  " << j << std::endl;
//                std::cout << "n = " << n << std::endl;
//                std::cout << "d = " << d << std::endl;
                A(i,j) = factor(n, d);
            }
        }
        d = A(k-1,k-1);
    }
    return A(A.rows()-1, A.columns()-1);
}
*/


/*
 *  GiNaC Copyright (C) 1999-2008 Johannes Gutenberg University Mainz, Germany
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


/*
 * determinant_minor
 * This routine has been taken from the ginac project
 * and adapted as needed; comments are the original ones.
 */

/** Recursive determinant for small matrices having at least one symbolic
 *  entry.  The basic algorithm, known as Laplace-expansion, is enhanced by
 *  some bookkeeping to avoid calculation of the same submatrices ("minors")
 *  more than once.  According to W.M.Gentleman and S.C.Johnson this algorithm
 *  is better than elimination schemes for matrices of sparse multivariate
 *  polynomials and also for matrices of dense univariate polynomials if the
 *  matrix' dimesion is larger than 7.
 *
 *  @return the determinant as a new expression (in expanded form)
 *  @see matrix::determinant() */

template< typename Coeff >
Coeff determinant_minor(Matrix<Coeff> const& M)
{
    assert(M.rows() == M.columns());
    // for small matrices the algorithm does not make any sense:
    const unsigned int n = M.columns();
    if (n == 1)
        return M(0,0);
    if (n == 2)
        return (M(0,0) * M(1,1) - M(0,1) * M(1,0));
    if (n == 3)
        return ( M(0,0)*M(1,1)*M(2,2) + M(0,2)*M(1,0)*M(2,1)
                + M(0,1)*M(1,2)*M(2,0) - M(0,2)*M(1,1)*M(2,0)
                - M(0,0)*M(1,2)*M(2,1) - M(0,1)*M(1,0)*M(2,2) );

    // This algorithm can best be understood by looking at a naive
    // implementation of Laplace-expansion, like this one:
    // ex det;
    // matrix minorM(this->rows()-1,this->cols()-1);
    // for (unsigned r1=0; r1<this->rows(); ++r1) {
    //     // shortcut if element(r1,0) vanishes
    //     if (m[r1*col].is_zero())
    //         continue;
    //     // assemble the minor matrix
    //     for (unsigned r=0; r<minorM.rows(); ++r) {
    //         for (unsigned c=0; c<minorM.cols(); ++c) {
    //             if (r<r1)
    //                 minorM(r,c) = m[r*col+c+1];
    //             else
    //                 minorM(r,c) = m[(r+1)*col+c+1];
    //         }
    //     }
    //     // recurse down and care for sign:
    //     if (r1%2)
    //         det -= m[r1*col] * minorM.determinant_minor();
    //     else
    //         det += m[r1*col] * minorM.determinant_minor();
    // }
    // return det.expand();
    // What happens is that while proceeding down many of the minors are
    // computed more than once.  In particular, there are binomial(n,k)
    // kxk minors and each one is computed factorial(n-k) times.  Therefore
    // it is reasonable to store the results of the minors.  We proceed from
    // right to left.  At each column c we only need to retrieve the minors
    // calculated in step c-1.  We therefore only have to store at most
    // 2*binomial(n,n/2) minors.

    // Unique flipper counter for partitioning into minors
    std::vector<unsigned int> Pkey;
    Pkey.reserve(n);
    // key for minor determinant (a subpartition of Pkey)
    std::vector<unsigned int> Mkey;
    Mkey.reserve(n-1);
    // we store our subminors in maps, keys being the rows they arise from
    typedef typename std::map<std::vector<unsigned>, Coeff> Rmap;
    typedef typename std::map<std::vector<unsigned>, Coeff>::value_type Rmap_value;
    Rmap A;
    Rmap B;
    Coeff det;
    // initialize A with last column:
    for (unsigned int r = 0; r < n; ++r)
    {
        Pkey.erase(Pkey.begin(),Pkey.end());
        Pkey.push_back(r);
        A.insert(Rmap_value(Pkey,M(r,n-1)));
    }
    // proceed from right to left through matrix
    for (int c = n-2; c >= 0; --c)
    {
        Pkey.erase(Pkey.begin(),Pkey.end());  // don't change capacity
        Mkey.erase(Mkey.begin(),Mkey.end());
        for (unsigned int i = 0; i < n-c; ++i)
            Pkey.push_back(i);
        unsigned int fc = 0;  // controls logic for our strange flipper counter
        do
        {
            det = Geom::SL::zero<Coeff>()();
            for (unsigned int r = 0; r < n-c; ++r)
            {
                // maybe there is nothing to do?
                if (M(Pkey[r], c).is_zero())
                    continue;
                // create the sorted key for all possible minors
                Mkey.erase(Mkey.begin(),Mkey.end());
                for (unsigned int i = 0; i < n-c; ++i)
                    if (i != r)
                        Mkey.push_back(Pkey[i]);
                // Fetch the minors and compute the new determinant
                if (r % 2)
                    det -= M(Pkey[r],c)*A[Mkey];
                else
                    det += M(Pkey[r],c)*A[Mkey];
            }
            // store the new determinant at its place in B:
            if (!det.is_zero())
                B.insert(Rmap_value(Pkey,det));
            // increment our strange flipper counter
            for (fc = n-c; fc > 0; --fc)
            {
                ++Pkey[fc-1];
                if (Pkey[fc-1]<fc+c)
                    break;
            }
            if (fc < n-c && fc > 0)
                for (unsigned int j = fc; j < n-c; ++j)
                    Pkey[j] = Pkey[j-1]+1;
        } while(fc);
        // next column, so change the role of A and B:
        A.swap(B);
        B.clear();
    }

    return det;
}

} /*end namespace Geom*/  } /*end namespace SL*/




#endif // _GEOM_SL_MATRIX_H_


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
