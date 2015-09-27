

#include "numeric/vector.h"
#include "numeric/matrix.h"

#include <iostream>

using namespace Geom;


template< class charT >
inline
std::basic_ostream<charT> & 
operator<< (std::basic_ostream<charT> & os, const std::pair<size_t, size_t>& index_pair)
{
	os << "{" << index_pair.first << "," << index_pair.second << "}";
	return os;
}

template< typename T, typename U>
void check_test( const char* description, T output, U expected )
{
	bool result = ( output == expected );
	std::cout << "# " << description << " : ";
	if ( result )
		std::cout << "success!" << std::endl;
	else
		std::cout << "fail!" << std::endl
		          << "    output:    " << output << std::endl
		          << "    expected:  " << expected << std::endl;
}


void vector_test()
{
	// Deprecated. Replaced by ** Vector examples **
	// in nl-vector-test.cpp
	/*
	NL::Vector v1(10), v2(10), v3(5);
	for (unsigned int i = 0; i < v1.size(); ++i)
	{
		v1[i] = i;
	}
	std::cout << "v1: " << v1 << std::endl;
	v2 = v1;
	std::cout << "v2 = v1 : " << v2 << std::endl;
	bool value = (v1 == v2);
	std::cout << "(v1 == v2) : " << value << std::endl;
	v2.scale(10);
	std::cout << "v2.scale(10) : " << v2 << std::endl;
	value = (v1 == v2);
	std::cout << "(v1 == v2) : " << value << std::endl;
	v2.translate(20);
	std::cout << "v2.translate(20) : " << v2 << std::endl;
	v2 += v1;
	std::cout << "v2 += v1 : " << v2 << std::endl;
	v2.swap_elements(3, 9);
	std::cout << "v2.swap_elements(3, 9) : " << v2 << std::endl;
	v2.reverse();
	std::cout << "v2.reverse() : " << v2 << std::endl;
	value = v2.is_positive();
	std::cout << "v2.is_positive() : " << value << std::endl;
	v2 -= v1;
	std::cout << "v2 -= v1 : " << v2 << std::endl;
	double bound = v2.max();
	std::cout << "v2.max() : " << bound << std::endl;
	bound = v2.min();
	std::cout << "v2.min() : " << bound << std::endl;
	unsigned int index = v2.max_index();
	std::cout << "v2.max_index() : " << index << std::endl;
	index = v2.min_index();
	std::cout << "v2.min_index() : " << index << std::endl;
	v2.set_basis(4);
	std::cout << "v2.set_basis(4) : " << v2 << std::endl;
	value = v2.is_non_negative();
	std::cout << "v2.is_non_negative() : " << value << std::endl;
	v2.set_all(0);
	std::cout << "v2.set_all(0) : " << v2 << std::endl;
	value = v2.is_zero();
	std::cout << "v2.is_zero() : " << value << std::endl;
	NL::swap(v1, v2);
	std::cout << "swap(v1, v2) : v1: " << v1 << " v2: " << v2 << std::endl;
	*/
}


void const_vector_view_test()
{
	NL::Vector v1(10);
	for (unsigned int i = 0; i < v1.size(); ++i)
		v1[i] = i;
	NL::VectorView vv1(v1, 5, 1, 2);
	vv1.scale(10);
	std::cout << "v1 = " << v1 << std::endl;
	
	NL::ConstVectorView cvv1(v1, 6, 1);
	check_test( "cvv1(v1, 6, 1)", cvv1.str(), "[10, 2, 30, 4, 50, 6]");
	NL::ConstVectorView cvv2(v1, 3, 1, 3);
	check_test( "cvv2(v1, 3, 1, 3)", cvv2.str(), "[10, 4, 70]");
	NL::ConstVectorView cvv3(vv1, 3, 0, 2);
	std::cout << "vv1 = " << vv1 << std::endl;
	check_test( "cvv3(vv1, 3, 0, 2)", cvv3.str(), "[10, 50, 90]");
	NL::ConstVectorView cvv4(cvv1, 3, 1, 2);
	check_test( "cvv4(cvv1, 3, 1, 2)", cvv4.str(), "[2, 4, 6]");
	bool value = (cvv2 == cvv4);
	check_test( "(cvv2 == cvv4)", value, false);
	
	value = cvv2.is_zero();
	check_test( "cvv2.is_zero()", value, false);
	value = cvv2.is_negative();
	check_test( "cvv2.is_negative()", value, false);
	value = cvv2.is_positive();
	check_test( "cvv2.is_positive()", value, true);
	value = cvv2.is_non_negative();
	check_test( "cvv2.is_non_negative()", value, true);

	NL::VectorView vv2(v1, 3, 1, 3);
	vv2.scale(-1);
	value = cvv2.is_zero();
	std::cout << "v1 = " << v1 << std::endl;
	check_test( "cvv2.is_zero()", value, false);
	value = cvv2.is_negative();
	check_test( "cvv2.is_negative()", value, true);
	value = cvv2.is_positive();
	check_test( "cvv2.is_positive()", value, false);
	value = cvv2.is_non_negative();
	check_test( "cvv2.is_non_negative()", value, false);
	
	vv2.set_all(0);
	std::cout << "v1 = " << v1 << std::endl;
	value = cvv2.is_zero();
	check_test( "cvv2.is_zero()", value, true);	
	value = cvv2.is_negative();
	check_test( "cvv2.is_negative()", value, false);
	value = cvv2.is_positive();
	check_test( "cvv2.is_positive()", value, false);
	value = cvv2.is_non_negative();
	check_test( "cvv2.is_non_negative()", value, true);
	
	vv1.reverse();
	vv2[0] = -1;
	std::cout << "v1 = " << v1 << std::endl;
	value = cvv2.is_zero();
	check_test( "cvv2.is_zero()", value, false);	
	value = cvv2.is_negative();
	check_test( "cvv2.is_negative()", value, false);
	value = cvv2.is_positive();
	check_test( "cvv2.is_positive()", value, false);
	value = cvv2.is_non_negative();
	check_test( "cvv2.is_non_negative()", value, false);
	
	vv2 = cvv2;
	value = (vv2 == cvv2);
	std::cout << "vv2 = " << vv2 << std::endl;
	check_test( "(vv2 == cvv2)", value, true);
	NL::Vector v2(cvv2.size());
	v2 = cvv4;
	value = (v2 == cvv2);
	std::cout << "v2 = " << v2 << std::endl;
	check_test( "(v2 == cvv2)", value, false);
	const NL::Vector v3(cvv2.size());
	NL::ConstVectorView cvv5(v3, v3.size());
	check_test( "cvv5(v3, v3.size())", cvv4.str(), "[2, 0, 6]");

}

void vector_view_test()
{
	// Deprecated. Replaced by ** VectorView examples **
	// in nl-vector-test.cpp
	/*
	NL::Vector v1(10);
	for (unsigned int i = 0; i < v1.size(); ++i)
		v1[i] = i;
	NL::VectorView vv1(v1, 5), vv2(v1, 5, 3), vv3(v1, 5, 0, 2), vv4(v1, 5, 1, 2);
	std::cout << "v1 = " << v1 << std::endl;
	check_test( "vv1(v1, 5)", vv1.str(), "[0, 1, 2, 3, 4]");
	check_test( "vv2(v1, 5, 3)", vv2.str(), "[3, 4, 5, 6, 7]");
	check_test( "vv3(v1, 5, 0, 2)", vv3.str(), "[0, 2, 4, 6, 8]");
	check_test( "vv4(v1, 5, 1, 2)", vv4.str(), "[1, 3, 5, 7, 9]");
	
	NL::VectorView vv5(vv4, 3, 0, 2);
	std::cout << "vv4 = " << vv4 << std::endl;
	check_test( "vv5(vv4, 3, 0, 2)", vv5.str(), "[1, 5, 9]");
	vv5.scale(10);
	check_test( "vv5.scale(10) : vv5", vv5.str(), "[10, 50, 90]");
	check_test( "              : v1", v1.str(), "[0, 10, 2, 3, 4, 50, 6, 7, 8, 90]");
	vv5.translate(20);
	check_test( "vv5.translate(20) : vv5", vv5.str(), "[30, 70, 110]");
	check_test( "                  : v1", v1.str(), "[0, 30, 2, 3, 4, 70, 6, 7, 8, 110]");
	vv1 += vv2;
	check_test("vv1 += vv2", vv1.str(), "[3, 34, 72, 9, 11]");
	vv1 -= vv2;
	check_test("vv1 -= vv2", vv1.str(), "[-6, 23, 2, 3, 4]");
	NL::ConstVectorView cvv1(vv3, 3);
	vv5 = cvv1;
	check_test("vv5 = cvv1", vv5.str(), "[-6, 2, 4]");
	vv5 += cvv1;
	check_test("vv5 += cvv1", vv5.str(), "[-12, 4, 8]");
	vv5 -= cvv1;
	check_test("vv5 -= cvv1", vv5.str(), "[-6, 2, 4]");
	NL::Vector v2(vv1);
	std::cout << "v2 = " << v2 << std::endl;
	vv1 = v2;
	check_test( "vv1 = v2", vv1.str(), "[-6, -6, 2, 3, 4]");
	vv1 += v2;
	check_test( "vv1 += v2", vv1.str(), "[-12, -12, 4, 6, 8]");
	vv1 -= v2;
	check_test( "vv1 -= v2", vv1.str(), "[-6, -6, 2, 3, 4]");
	NL::swap_view(vv1, vv4);
	check_test( "swap_view(vv1, vv4)", v1.str(), "[-6, -6, 2, 3, 4, 2, 6, 7, 8, 4]");
	*/
}


void const_matrix_view_test()
{
	NL::Matrix m0(8,4);
	for (size_t i = 0; i < m0.rows(); ++i)
	{
		for (size_t j = 0; j < m0.columns(); ++j)
		{
			m0(i,j) = 10 * i + j;
		}
	}
	std::cout << "m0 = " << m0 << std::endl;
	
	// constructor test
	NL::Matrix m1(m0);
	NL::ConstMatrixView cmv1(m1, 2, 1, 4, 2);
	check_test("cmv1(m1, 2, 1, 4, 2)", cmv1.str(), "[[21, 22], [31, 32], [41, 42], [51, 52]]");
	NL::MatrixView mv1(m1, 2, 0, 4, 4);
	NL::ConstMatrixView cmv2(mv1, 2, 1, 2, 2);
	check_test("cmv2(mv1, 2, 1, 2, 2)", cmv2.str(), "[[41, 42], [51, 52]]");
	NL::ConstMatrixView cmv3(cmv1, 1, 1, 3, 1);
	check_test("cmv3(cmv1, 1, 1, 2, 1)", cmv3.str(), "[[32], [42], [52]]");
	const NL::Matrix & m2 = m1;
	NL::ConstMatrixView cmv4(m2, 2, 1, 4, 2);
	check_test("cmv4(m2, 2, 1, 4, 2)", cmv4.str(), "[[21, 22], [31, 32], [41, 42], [51, 52]]");
	const NL::MatrixView & mv2 = mv1;
	NL::ConstMatrixView cmv5(mv2, 2, 1, 2, 2);
	check_test("cmv5(mv2, 2, 1, 2, 2)", cmv5.str(), "[[41, 42], [51, 52]]");
	
	// row and column view test
	NL::ConstVectorView cvv1 = cmv1.row_const_view(2);
	check_test("cvv1 = cmv1.row_const_view(2)", cvv1.str(), "[41, 42]");
	NL::ConstVectorView cvv2 = cmv1.column_const_view(0);
	check_test("cvv2 = cmv1.column_const_view(0)", cvv2.str(), "[21, 31, 41, 51]");
	
	// property test
	bool value = cmv1.is_negative();
	check_test("cmv1.is_negative()", value, false);
	value = cmv1.is_non_negative();
	check_test("cmv1.is_non_negative()", value, true);
	value = cmv1.is_positive();
	check_test("cmv1.is_positive()", value, true);
	value = cmv1.is_zero();
	check_test("cmv1.is_zero()", value, false);
	
	m1.scale(-1);
	value = cmv1.is_negative();
	check_test("cmv1.is_negative()", value, true);
	value = cmv1.is_non_negative();
	check_test("cmv1.is_non_negative()", value, false);
	value = cmv1.is_positive();
	check_test("cmv1.is_positive()", value, false);
	value = cmv1.is_zero();
	check_test("cmv1.is_zero()", value, false);
	
	m1.translate(35);
	value = cmv1.is_negative();
	check_test("cmv1.is_negative()", value, false);
	value = cmv1.is_non_negative();
	check_test("cmv1.is_non_negative()", value, false);
	value = cmv1.is_positive();
	check_test("cmv1.is_positive()", value, false);
	value = cmv1.is_zero();
	check_test("cmv1.is_zero()", value, false);
	
	m1.set_all(0);
	value = cmv1.is_negative();
	check_test("cmv1.is_negative()", value, false);
	value = cmv1.is_non_negative();
	check_test("cmv1.is_non_negative()", value, true);
	value = cmv1.is_positive();
	check_test("cmv1.is_positive()", value, false);
	value = cmv1.is_zero();
	check_test("cmv1.is_zero()", value, true);
	
	m1.set_identity();
	value = cmv1.is_negative();
	check_test("cmv1.is_negative()", value, false);
	value = cmv1.is_non_negative();
	check_test("cmv1.is_non_negative()", value, true);
	value = cmv1.is_positive();
	check_test("cmv1.is_positive()", value, false);
	value = cmv1.is_zero();
	check_test("cmv1.is_zero()", value, false);
	
	// max, min test
	m1 = m0;
	std::cout << "cmv1 = " << cmv1 << std::endl;
	std::pair<size_t, size_t> out_elem = cmv1.max_index();
	std::pair<size_t, size_t> exp_elem(3,1);
	check_test("cmv1.max_index()", out_elem, exp_elem);
	double bound = cmv1.max();
	check_test("cmv1.max()", bound, cmv1(exp_elem.first, exp_elem.second));
	out_elem = cmv1.min_index();
	exp_elem.first = 0; exp_elem.second = 0;
	check_test("cmv1.min_index()", out_elem, exp_elem);
	bound = cmv1.min();
	check_test("cmv1.min()", bound, cmv1(exp_elem.first, exp_elem.second));
	
}


void matrix_view_test()
{
	NL::Matrix m0(8,4);
	for (size_t i = 0; i < m0.rows(); ++i)
	{
		for (size_t j = 0; j < m0.columns(); ++j)
		{
			m0(i,j) = 10 * i + j;
		}
	}
	std::cout << "m0 = " << m0 << std::endl;
	
	// constructor test
	NL::Matrix m1(m0);
	NL::MatrixView mv1(m1, 2, 1, 4, 2);
	check_test("mv1(m1, 2, 1, 4, 2)", mv1.str(), "[[21, 22], [31, 32], [41, 42], [51, 52]]");
	NL::MatrixView mv2(mv1, 2, 1, 2, 1);
	check_test("mv2(mv1, 2, 1, 2, 1)", mv2.str(), "[[42], [52]]");

	// operator = test
	NL::Matrix m2(4,2);
	m2.set_all(0);
	mv1 = m2;
	check_test("mv1 = m2", m1.str(), "[[0, 1, 2, 3], [10, 11, 12, 13], [20, 0, 0, 23], [30, 0, 0, 33], [40, 0, 0, 43], [50, 0, 0, 53], [60, 61, 62, 63], [70, 71, 72, 73]]");
	m1 = m0;
	NL::MatrixView mv3(m2, 0, 0, 4, 2);
	mv1 = mv3;
	check_test("mv1 = mv3", m1.str(), "[[0, 1, 2, 3], [10, 11, 12, 13], [20, 0, 0, 23], [30, 0, 0, 33], [40, 0, 0, 43], [50, 0, 0, 53], [60, 61, 62, 63], [70, 71, 72, 73]]");
	m1 = m0;
	NL::ConstMatrixView cmv1(m2, 0, 0, 4, 2);
	mv1 = cmv1;
	check_test("mv1 = cmv1", m1.str(), "[[0, 1, 2, 3], [10, 11, 12, 13], [20, 0, 0, 23], [30, 0, 0, 33], [40, 0, 0, 43], [50, 0, 0, 53], [60, 61, 62, 63], [70, 71, 72, 73]]");
	
	// operator == test
	m2.set_identity();
	mv1 = m2;
	bool value =  (mv1 == m2);
	check_test("(mv1 == m2)", value, true);
	value = (mv1 == mv3);
	check_test("(mv1 == mv3)", value, true);
	value = (mv1 == cmv1);
	check_test("(mv1 == cmv1)", value, true);
	
	// row and column view test
	m1 = m0;
	NL::ConstVectorView cvv1 = mv1.row_const_view(2);
	check_test("cvv1 = mv1.row_const_view(2)", cvv1.str(), "[41, 42]");
	NL::ConstVectorView cvv2 = mv1.column_const_view(0);
	check_test("cvv2 = mv1.column_const_view(0)", cvv2.str(), "[21, 31, 41, 51]");
	NL::VectorView vv1 = mv1.row_view(2);
	check_test("vv1 = mv1.row_view(2)", vv1.str(), "[41, 42]");
	NL::VectorView vv2 = mv1.column_view(0);
	check_test("vv2 = mv1.column_view(0)", vv2.str(), "[21, 31, 41, 51]");
	
	// swap_view test
	m1 = m0;
	swap_view(mv1, mv3);
	check_test("swap_view(mv1, mv3) : mv1", mv1.str(), "[[1, 0], [0, 1], [0, 0], [0, 0]]");
	check_test("                    : m1", m1.str(), m0.str());
	check_test("                    : mv3", mv3.str(), "[[21, 22], [31, 32], [41, 42], [51, 52]]");
	check_test("                    : m2", m2.str(), "[[1, 0], [0, 1], [0, 0], [0, 0]]");
	swap_view(mv1, mv3);
	
	// modifying operations test
	m1 = m0;
	m2.set_all(10);
	mv1 += m2;
	check_test("mv1 += m2", m1.str(), "[[0, 1, 2, 3], [10, 11, 12, 13], [20, 31, 32, 23], [30, 41, 42, 33], [40, 51, 52, 43], [50, 61, 62, 53], [60, 61, 62, 63], [70, 71, 72, 73]]");
	mv1 -= m2;
	check_test("mv1 -= m2", m1.str(), m0.str());
	mv1 += mv3;
	check_test("mv1 += mv3", m1.str(), "[[0, 1, 2, 3], [10, 11, 12, 13], [20, 31, 32, 23], [30, 41, 42, 33], [40, 51, 52, 43], [50, 61, 62, 53], [60, 61, 62, 63], [70, 71, 72, 73]]");
	mv1 -= mv3;
	check_test("mv1 -= mv3", m1.str(), m0.str());
	mv1 += cmv1;
	check_test("mv1 += cmv1", m1.str(), "[[0, 1, 2, 3], [10, 11, 12, 13], [20, 31, 32, 23], [30, 41, 42, 33], [40, 51, 52, 43], [50, 61, 62, 53], [60, 61, 62, 63], [70, 71, 72, 73]]");
	mv1 -= cmv1;
	check_test("mv1 -= cmv1", m1.str(), m0.str());
	
	m1 = m0;
	mv1.swap_rows(0,3);
	check_test("mv1.swap_rows(0,3)", m1.str(), "[[0, 1, 2, 3], [10, 11, 12, 13], [20, 51, 52, 23], [30, 31, 32, 33], [40, 41, 42, 43], [50, 21, 22, 53], [60, 61, 62, 63], [70, 71, 72, 73]]");
	m1 = m0;
	mv1.swap_columns(0,1);
	check_test("mv1.swap_columns(0,3)", m1.str(), "[[0, 1, 2, 3], [10, 11, 12, 13], [20, 22, 21, 23], [30, 32, 31, 33], [40, 42, 41, 43], [50, 52, 51, 53], [60, 61, 62, 63], [70, 71, 72, 73]]");
	
	m1 = m0;
	NL::MatrixView mv4(m1, 0, 0, 4, 4);
	mv4.transpose();
	check_test("mv4.transpose()", m1.str(), "[[0, 10, 20, 30], [1, 11, 21, 31], [2, 12, 22, 32], [3, 13, 23, 33], [40, 41, 42, 43], [50, 51, 52, 53], [60, 61, 62, 63], [70, 71, 72, 73]]");
	
}

void matrix_test()
{
	NL::Matrix m0(8,4);
	for (size_t i = 0; i < m0.rows(); ++i)
	{
		for (size_t j = 0; j < m0.columns(); ++j)
		{
			m0(i,j) = 10 * i + j;
		}
	}
	std::cout << "m0 = " << m0 << std::endl;
	
	// constructor test
	NL::Matrix m1(m0);
	check_test("m1(m0)", m1.str(), m0.str());
	NL::MatrixView mv1(m0, 2, 1, 4, 2);
	NL::Matrix m2(mv1);
	check_test("m2(mv1)", m2.str(), mv1.str());
	NL::MatrixView cmv1(m0, 2, 1, 4, 2);
	NL::Matrix m3(cmv1);
	check_test("m3(cmv1)", m3.str(), cmv1.str());
	
	// operator = and operator == test
	m1.set_all(0);
	m1 = m0;
	check_test("m1 = m0", m1.str(), m0.str());
	bool value = (m1 == m0);
	check_test("m1 == m0", value, true);
	m2.set_all(0);
	m2 = mv1;
	check_test("m2 = mv1", m2.str(), mv1.str());
	value = (m2 == mv1);
	check_test("m2 == mv1", value, true);
	m2.set_all(0);
	m2 = cmv1;
	check_test("m2 = cmv1", m2.str(), cmv1.str());
	value = (m2 == cmv1);
	check_test("m2 == cmv1", value, true);

	// row and column view test
	NL::ConstVectorView cvv1 = m2.row_const_view(2);
	check_test("cvv1 = m2.row_const_view(2)", cvv1.str(), "[41, 42]");
	NL::ConstVectorView cvv2 = m2.column_const_view(0);
	check_test("cvv2 = m2.column_const_view(0)", cvv2.str(), "[21, 31, 41, 51]");
	NL::VectorView vv1 = m2.row_view(2);
	check_test("vv1 = m2.row_view(2)", vv1.str(), "[41, 42]");
	NL::VectorView vv2 = m2.column_view(0);
	check_test("vv2 = m2.column_view(0)", vv2.str(), "[21, 31, 41, 51]");
	
	
	// modifying operations test
	NL::Matrix m4(8,4);
	m4.set_all(0);
	m1.set_all(0);
	m1 += m0;
	check_test("m1 += m0", m1.str(), m0.str());
	m1 -= m0;
	check_test("m1 -= m0", m1.str(), m4.str());
	NL::Matrix m5(4,2);
	m5.set_all(0);
	m2.set_all(0);
	m2 += mv1;
	check_test("m2 += mv1", m2.str(), mv1.str());
	m2 -= mv1;
	check_test("m2 -= mv1", m2.str(), m5.str());
	m2.set_all(0);
	m2 += cmv1;
	check_test("m2 += cmv1", m2.str(), cmv1.str());
	m2 -= cmv1;
	check_test("m2 -= cmv1", m2.str(), m5.str());
	
	// swap test	
	m3.set_identity();
	m5.set_identity();
	m1 = m0;
	m2 = mv1;
	swap(m2, m3);
	check_test("swap(m2, m3) : m2", m2.str(), m5.str());
	check_test("             : m3", m3.str(), mv1.str());

}

int main(int argc, char **argv) 
{	
    //const_vector_view_test();
    //vector_view_test();
	const_matrix_view_test();
	//matrix_view_test();
	//matrix_test();
    return 0;
}


