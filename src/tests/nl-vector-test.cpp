/** @file
 * @brief Unit tests for Vector, VectorView
 *//*
 * Authors:
 *   Olof Bjarnason <olof.bjarnason@gmail.com>
 * 
 * Copyright 2015 Authors
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
 */

#include <gtest/gtest.h>

#include <2geom/numeric/vector.h>

namespace Geom {

////
// Test fixture used in many tests.
// v1 = [0, 1, ..., 8, 9]
////
class CountingVectorFixture : public ::testing::Test {
public:
	CountingVectorFixture() : v1(10) { }

protected:
  virtual void SetUp() {
	for (unsigned int i = 0; i < this->v1.size(); ++i)
		this->v1[i] = i;
  }

  NL::Vector v1;
};

// These types are only here to differentiate
// between categories of tests - they both use
// the same v1 test fixture variable.
class VectorTest : public CountingVectorFixture { };
class VectorViewTest : public CountingVectorFixture { };

////
// Helper method to write simple tests
////
NL::Vector V3(double a, double b, double c) {
	NL::Vector v(3);
	v[0] = a;
	v[1] = b;
	v[2] = c;
	return v;
}

////
// ** Vector examples **
////

TEST_F(VectorTest, VectorStringRepresentation) {
	EXPECT_EQ(v1.str(), "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9]");
}

TEST_F(VectorTest, VectorConstructFromAnother) {
	NL::Vector v2(v1);
	EXPECT_EQ(v1.str(), v2.str());
}

TEST_F(VectorTest, OperatorEqualIsDefined) {
	EXPECT_TRUE(v1 == v1);
	NL::Vector v2(v1);
	EXPECT_TRUE(v1 == v2);
	// TODO: This operation compares doubles
	// with operator ==. Should it use a distance
	// threshold instead?
}

TEST_F(VectorTest, OperatorNotEqualIsntDefined) {
	SUCCEED();
	//NL::Vector v3(4);
	//EXPECT_TRUE(v1 != v3); // Not expressible in C++;
	// gives compile time error
}

TEST_F(VectorTest, VectorAssignment) {
	NL::Vector v2(v1.size());
	v2 = v1;
	EXPECT_EQ(v1, v2);
}

TEST_F(VectorTest, AssignedVectorMustBeSameSize) {
	NL::Vector v2(5);
	// On Linux, the assertion message is:
	// Assertion ... failed ...
	// On OSX, it is:
	// Assertion failed: (...), function ..., file ..., line ...
	// Thus we just look for the word "Assertion".
	EXPECT_DEATH({v2 = v1;}, "Assertion");
}

TEST_F(VectorTest, VectorScalesInplace) {
	v1.scale(2);
	EXPECT_EQ(v1.str(), "[0, 2, 4, 6, 8, 10, 12, 14, 16, 18]");
}

TEST_F(VectorTest, VectorTranslatesInplace) {
	v1.translate(1);
	EXPECT_EQ(v1.str(), "[1, 2, 3, 4, 5, 6, 7, 8, 9, 10]");
}

TEST_F(VectorTest, ScaleAndTranslateUsesFluentSyntax) {
	NL::VectorView vv(v1, 3);
	EXPECT_EQ(vv.translate(5).scale(10).str(), "[50, 60, 70]");
}

TEST_F(VectorTest, AddAssignment) {
	NL::Vector v2(v1);
	v2 += v1;
	EXPECT_EQ(v2.str(), "[0, 2, 4, 6, 8, 10, 12, 14, 16, 18]");
}

TEST_F(VectorTest, SubtractAssignment) {
	NL::Vector v2(v1);
	v2 -= v1;
	EXPECT_EQ(v2.str(), "[0, 0, 0, 0, 0, 0, 0, 0, 0, 0]");
}

TEST_F(VectorTest, SwappingElements) {
	v1.swap_elements(0, 9);
	EXPECT_EQ(v1.str(), "[9, 1, 2, 3, 4, 5, 6, 7, 8, 0]");
}

TEST_F(VectorTest, Reverse) {
	v1.reverse();
	EXPECT_EQ(v1.str(), "[9, 8, 7, 6, 5, 4, 3, 2, 1, 0]");
}

TEST(Vector, IsPositive) {
	EXPECT_TRUE(V3(1, 1, 1).is_positive());
	EXPECT_FALSE(V3(0, 0, 0).is_positive());
	EXPECT_FALSE(V3(-1, 0, 1).is_positive());
}

TEST_F(VectorTest, IsZero) {
	EXPECT_FALSE(v1.is_zero());
	EXPECT_TRUE(V3(0, 0, 0).is_zero());
}

TEST_F(VectorTest, IsNonNegative) {
	EXPECT_TRUE(V3(1, 1, 1).is_non_negative());
	EXPECT_TRUE(V3(0, 0, 0).is_non_negative());
	EXPECT_FALSE(V3(-1, 1, 1).is_non_negative());
}

TEST(Vector, Max) {
	EXPECT_EQ(V3(1, 5, 3).max(), 5);
}

TEST(Vector, MaxIndex) {
	EXPECT_EQ(V3(1, 5, 3).max_index(), 1u);
}

TEST(Vector, Min) {
	EXPECT_EQ(V3(1, -5, -300).min(), -300);
}

TEST(Vector, MinIndex) {
	EXPECT_EQ(V3(1, 5, 3).min_index(), 0u);
}

TEST_F(VectorTest, SetAll) {
	v1.set_all(5);
	EXPECT_EQ(v1.str(), "[5, 5, 5, 5, 5, 5, 5, 5, 5, 5]");
}

TEST_F(VectorTest, SetBasis) {
	v1.set_basis(1);
	EXPECT_EQ(v1.str(), "[0, 1, 0, 0, 0, 0, 0, 0, 0, 0]");
}

TEST(Vector, SwappingVectors) {
	NL::Vector a(V3(1, 2, 3));
	NL::Vector b(V3(7, 7, 7));
	NL::swap(a, b);
	EXPECT_EQ(V3(7, 7, 7), a);
	EXPECT_EQ(V3(1, 2, 3), b);
}

////
// ** VectorView tests **
////

// Construction examples

TEST_F(VectorViewTest, ViewCountOnly) {
	// VectorView(vector, showCount)
	EXPECT_EQ(NL::VectorView(v1, 5).str(), "[0, 1, 2, 3, 4]");
}

TEST_F(VectorViewTest, SkipSomeInitialElements) {
	// VectorView(vector, showCount, startIndex)
	EXPECT_EQ(NL::VectorView(v1, 5, 3).str(), "[3, 4, 5, 6, 7]");
}

TEST_F(VectorViewTest, SparseViewConstruction) {
	// VectorView(vector, showCount, startIndex, step)
	EXPECT_EQ(NL::VectorView(v1, 5, 0, 2).str(), "[0, 2, 4, 6, 8]");
	EXPECT_EQ(NL::VectorView(v1, 5, 1, 2).str(), "[1, 3, 5, 7, 9]");
}

TEST_F(VectorViewTest, ConstructFromAnotherView) {
	// VectorView(vectorview, showCount, startIndex, step)
	NL::VectorView vv(v1, 5, 1, 2);
	NL::VectorView view(vv, 3, 0, 2);
	EXPECT_EQ(view.str(), "[1, 5, 9]");
}

// Operations modify source vectors

TEST_F(VectorViewTest, PartialSourceModification) {
	NL::VectorView vv(v1, 3);
	vv.translate(10);
	EXPECT_EQ(v1.str(),
			  "[10, 11, 12, 3, 4, 5, 6, 7, 8, 9]");
	EXPECT_EQ(vv.str(),
			  "[10, 11, 12]");
}

// Scale and translate examples

TEST_F(VectorViewTest, ViewScalesInplace) {
	v1.scale(10);
	EXPECT_EQ(NL::VectorView(v1, 3).str(), "[0, 10, 20]");
}

TEST_F(VectorViewTest, ViewScaleAndTranslateUsesFluentSyntax) {
	EXPECT_EQ(NL::VectorView(v1, 3).scale(10).translate(1).str(),
		      "[1, 11, 21]");
}

// Assignment

TEST_F(VectorViewTest, AssignmentFromVectorAvailableForViews) {
	NL::VectorView vv(v1, v1.size());
	vv = v1;
	EXPECT_EQ(vv.str(), v1.str());
}

TEST_F(VectorViewTest, AssignmentFromVectorMustBeSameSize) {
	NL::VectorView vv(v1, 5);
	EXPECT_DEATH({vv = v1;}, "Assertion");
}

TEST_F(VectorViewTest, AssignmentFromViewAvailableForViews) {
	NL::VectorView view1(v1, v1.size());
	view1 = v1;
	NL::VectorView view2(view1);
	view2 = view1;
	EXPECT_EQ(view1.str(), view2.str());
}

TEST_F(VectorViewTest, AssignmentFromViewMustBeSameSize) {
	NL::VectorView view1(v1, v1.size());
	NL::VectorView view2(view1, view1.size() - 1);
	EXPECT_DEATH({view2 = view1;}, "Assertion");
}

// Add- and subtract assignment

TEST_F(VectorViewTest, AddAssignAvailableForViews) {
	NL::VectorView v2(v1);
	v1 += v2;
	EXPECT_EQ(v1.str(), "[0, 2, 4, 6, 8, 10, 12, 14, 16, 18]");
}

TEST_F(VectorViewTest, SubtractAssignAvailableForViews) {
	NL::Vector v2(v1);
	v1 -= v2;
	EXPECT_TRUE(v1.is_zero());
}

// View swapping

TEST_F(VectorViewTest, SwappingFromSameSourceVectorDoesNotModifySource) {
	NL::VectorView vv1(v1, 2, 0);
	NL::VectorView vv2(v1, 2, 8);
	NL::swap_view(vv1, vv2);
	EXPECT_EQ(v1.str(), "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9]");
}

TEST_F(VectorViewTest, SwappingFromSameSourceVectorModifiesViews) {
	NL::VectorView viewStart(v1, 2, 0);
	NL::VectorView viewEnd(v1, 2, 8);
	EXPECT_EQ(viewStart.str(), "[0, 1]");
	EXPECT_EQ(viewEnd.str(), "[8, 9]");
	NL::swap_view(viewStart, viewEnd);
	EXPECT_EQ(viewStart.str(), "[8, 9]");
	EXPECT_EQ(viewEnd.str(), "[0, 1]");
}

TEST_F(VectorViewTest, SwappingDifferentLengthViewFails) {
	NL::VectorView vv1(v1, 4);
	NL::VectorView vv2(v1, 3);
	EXPECT_DEATH({NL::swap_view(vv1, vv2);}, "Assertion");
}


} // namespace Geom
