#include "gtest/gtest.h"
#include <vector>
#include <2geom/coord.h>
#include <2geom/interval.h>
#include <2geom/intersection.h>

// streams out a vector
template <class T>
std::ostream&
operator<< (std::ostream &out, const std::vector<T,
             std::allocator<T> > &v)
{
    typedef std::ostream_iterator<T, char,
    std::char_traits<char> > Iter;

    std::copy (v.begin (), v.end (), Iter (out, " "));

    return out;
}

template <typename T, unsigned xn>
std::vector<T> vector_from_array(const T (&x)[xn]) {
    std::vector<T> v;
    for(unsigned i = 0; i < xn; i++) {
        v.push_back(x[i]);
    }
    return v;
}

template <typename T, unsigned xn>
void expect_array(const T (&x)[xn], std::vector<T> y) {
    EXPECT_EQ(xn, y.size());
    for(unsigned i = 0; i < y.size(); i++) {
        EXPECT_EQ(x[i], y[i]);
    }
}

Geom::Interval bound_vector(std::vector<double> const &v) {
    double low = v[0];
    double high = v[0];
    for(unsigned i = 0; i < v.size(); i++) {
      low = std::min(v[i], low);
      high = std::max(v[i], high);
    }
    return Geom::Interval(low-1, high-1);
}


// Custom assertion formatting predicates

template <typename T>
::testing::AssertionResult ObjectNear(char const *l_expr,
                                      char const *r_expr,
                                      char const */*eps_expr*/,
                                      T const &l,
                                      T const &r,
                                      Geom::Coord eps)
{
    if (!Geom::are_near(l, r, eps)) {
        return ::testing::AssertionFailure() << "Objects are not near\n"
            << "First object: " << l_expr << "\n"
            << "Value: " << l << "\n"
            << "Second object: " << r_expr << "\n"
            << "Value: " << r << "\n"
            << "Threshold: " << Geom::format_coord_nice(eps) << std::endl;
    }
    return ::testing::AssertionSuccess();
}

template <typename T>
::testing::AssertionResult ObjectNotNear(char const *l_expr,
                                         char const *r_expr,
                                         char const */*eps_expr*/,
                                         T const &l,
                                         T const &r,
                                         Geom::Coord eps)
{
    if (Geom::are_near(l, r, eps)) {
        return ::testing::AssertionFailure() << "Objects are near\n"
            << "First object: " << l_expr << "\n"
            << "Value: " << l << "\n"
            << "Second object: " << r_expr << "\n"
            << "Value: " << r << "\n"
            << "Threshold: " << Geom::format_coord_nice(eps) << std::endl;
    }
    return ::testing::AssertionSuccess();
}

#define EXPECT_near(a, b, eps) EXPECT_PRED_FORMAT3(ObjectNear, a, b, eps)
#define EXPECT_not_near(a, b, eps) EXPECT_PRED_FORMAT3(ObjectNotNear, a, b, eps)



template <typename T>
::testing::AssertionResult VectorEqual(char const *l_expr,
                                       char const *r_expr,
                                       std::vector<T> const &l,
                                       std::vector<T> const &r)
{
    if (l.size() != r.size()) {
        return ::testing::AssertionFailure() << "Vectors differ in size\n"
            << l_expr << " has size " << l.size() << "\n"
            << r_expr << " has size " << r.size() << std::endl;
    }
    for (unsigned i = 0; i < l.size(); ++i) {
        if (!(l[i] == r[i])) {
            return ::testing::AssertionFailure() << "Vectors differ"
                << "\nVector: " << l_expr
                << "\nindex " << i << " contains: " << l[i]
                << "\nVector:" << r_expr
                << "\nindex " << i << " contains: " << r[i] << std::endl;
        }
    }
    return ::testing::AssertionSuccess();
}

template <typename T>
::testing::AssertionResult VectorNear(char const *l_expr,
                                      char const *r_expr,
                                      char const */*eps_expr*/,
                                      std::vector<T> const &l,
                                      std::vector<T> const &r,
                                      Geom::Coord eps)
{
    if (l.size() != r.size()) {
        return ::testing::AssertionFailure() << "Vectors differ in size\n"
            << l_expr << "has size " << l.size() << "\n"
            << r_expr << "has size " << r.size() << std::endl;
    }
    for (unsigned i = 0; i < l.size(); ++i) {
        if (!Geom::are_near(l[i], r[i], eps)) {
            return ::testing::AssertionFailure() << "Vectors differ by more than "
                << Geom::format_coord_nice(eps)
                << "\nVector: " << l_expr
                << "\nindex " << i << " contains: " << l[i]
                << "\nVector:" << r_expr
                << "\nindex " << i << " contains: " << r[i] << std::endl;
        }
    }
    return ::testing::AssertionSuccess();
}

#define EXPECT_vector_equal(a, b) EXPECT_PRED_FORMAT2(VectorEqual, a, b)
#define EXPECT_vector_near(a, b, eps) EXPECT_PRED_FORMAT3(VectorNear, a, b, eps)



template <typename TA, typename TB>
::testing::AssertionResult IntersectionsValid(
    char const *l_expr, char const *r_expr, const char */*xs_expr*/, const char */*eps_expr*/,
    TA const &shape_a, TB const &shape_b,
    std::vector<Geom::Intersection<typename Geom::ShapeTraits<TA>::TimeType,
                                   typename Geom::ShapeTraits<TB>::TimeType> > const &xs,
    Geom::Coord eps)
{
    std::ostringstream os;
    bool failed = false;

    for (unsigned i = 0; i < xs.size(); ++i) {
        Geom::Point pa = shape_a.pointAt(xs[i].first);
        Geom::Point pb = shape_b.pointAt(xs[i].second);
        if (!Geom::are_near(pa, xs[i].point(), eps) ||
            !Geom::are_near(pb, xs[i].point(), eps) ||
            !Geom::are_near(pb, pa, eps))
        {
            os << "Intersection " << i << " does not match\n"
               << Geom::format_coord_nice(xs[i].first) << " evaluates to " << pa << "\n"
               << Geom::format_coord_nice(xs[i].second) << " evaluates to " << pb << "\n"
               << "Reported intersection point is " << xs[i].point() << std::endl;
            failed = true;
        }
    }

    if (failed) {
        return ::testing::AssertionFailure()
            << "Intersections do not match\n"
            << "Shape A: " << l_expr << "\n"
            << "Shape B: " << r_expr << "\n"
            << os.str()
            << "Threshold: " << Geom::format_coord_nice(eps) << std::endl;
    }

    return ::testing::AssertionSuccess();
}

#define EXPECT_intersections_valid(a, b, xs, eps) EXPECT_PRED_FORMAT4(IntersectionsValid, a, b, xs, eps)
