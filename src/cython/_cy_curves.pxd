from _common_decl cimport *

from libcpp.vector cimport vector
from libcpp.pair cimport pair

from _cy_rectangle cimport Interval, OptInterval, Rect, OptRect
from _cy_affine cimport Affine
from _cy_primitives cimport Point, cy_Point, wrap_Point, wrap_vector_point, make_vector_point
from _cy_primitives cimport Angle, cy_Angle, wrap_Angle
from _cy_primitives cimport AngleInterval


cdef extern from "2geom/d2.h" namespace "Geom":
    cdef cppclass D2[T]:
        D2()
        D2(T &, T &)
        T& operator[](unsigned i)

cdef extern from "2geom/curve.h" namespace "Geom":
    cdef cppclass Curve:
        Curve()
        Point initialPoint()
        Point finalPoint()
        bint isDegenerate()
        Point pointAt(Coord)
        Coord valueAt(Coord, Dim2)
        Point operator()(Coord)
        vector[Point] pointAndDerivatives(Coord, unsigned int)
        void setInitial(Point &)
        void setFinal(Point &)
        Rect boundsFast()
        Rect boundsExact()
        OptRect boundsLocal(OptInterval &, unsigned int)
        OptRect boundsLocal(OptInterval &)
        Curve * duplicate()
        Curve * transformed(Affine &)
        Curve * portion(Coord, Coord)
        Curve * portion(Interval &)
        Curve * reverse()
        Curve * derivative()
        Coord nearestTime(Point &, Coord, Coord)
        Coord nearestTime(Point &, Interval &)
        vector[double] allNearestTimes(Point &, Coord, Coord)
        vector[double] allNearestTimes(Point &, Interval &)
        Coord length(Coord)
        vector[double] roots(Coord, Dim2)
        int winding(Point &)
        Point unitTangentAt(Coord, unsigned int)
        D2[SBasis] toSBasis()
        int degreesOfFreedom()
        bint operator==(Curve &)

cdef class cy_Curve:
    cdef Curve* thisptr

#~ cdef cy_Curve wrap_Curve(Curve & p)
cdef cy_Curve wrap_Curve_p(Curve * p)


cdef extern from "2geom/linear.h" namespace "Geom":
    cdef cppclass Linear:
#~         Linear(Linear &)
        Linear()
        Linear(double, double)
        Linear(double)
        double operator[](int const)
        bint isZero(double)
        bint isConstant(double)
        bint isFinite()
        double at0()
        double at1()
        double valueAt(double)
        double operator()(double)
        SBasis toSBasis()
        OptInterval bounds_exact()
        OptInterval bounds_fast()
        OptInterval bounds_local(double, double)
        double tri()
        double hat()

    bint operator==(Linear &, Linear &)
    bint operator!=(Linear &, Linear &)
    Linear operator*(Linear &, double)
    Linear operator+(Linear &, double)
    Linear operator+(Linear &, Linear &)
    #cython has trouble resolving these
    Linear L_neg "operator-" (Linear &)
    Linear L_sub_Ld "operator-"(Linear &, double)
    Linear L_sub_LL "operator-"(Linear &, Linear &)
    Linear operator/(Linear &, double)

    double lerp(double, double, double)
    Linear reverse(Linear &)


cdef extern from "2geom/sbasis.h" namespace "Geom":
    cdef cppclass SBasis:

        SBasis()
        SBasis(double)
        SBasis(double, double)
        SBasis(SBasis &)
        SBasis(vector[Linear] &)
        SBasis(Linear &)

        size_t size()
        Linear operator[](unsigned int)
        bint empty()
        Linear & back()
        void pop_back()
        void resize(unsigned int)
        void resize(unsigned int, Linear &)
        void reserve(unsigned int)
        void clear()
#~         void insert(::__gnu_cxx::__normal_iterator<Geom::Linear*, std::vector<Geom::Linear, std::allocator<Geom::Linear> > >, ::__gnu_cxx::__normal_iterator<Geom::Linear const*, std::vector<Geom::Linear, std::allocator<Geom::Linear> > >, ::__gnu_cxx::__normal_iterator<Geom::Linear const*, std::vector<Geom::Linear, std::allocator<Geom::Linear> > >)
        Linear & at(unsigned int)
        bint operator==(SBasis &)
        bint operator!=(SBasis &)

        bint isZero(double)
        bint isConstant(double)
        bint isFinite()
        double at0()
        double at1()
        int degreesOfFreedom()
        double valueAt(double)
        double operator()(double)
        vector[double] valueAndDerivatives(double, unsigned int)
        SBasis toSBasis()
        double tailError(unsigned int)
        SBasis operator()(SBasis &)
        void normalize()
        void truncate(unsigned int)

    SBasis operator*(SBasis &, SBasis &)
    SBasis operator*(double, SBasis &)
    SBasis operator*(SBasis &, double)
    SBasis operator+(SBasis &, double)
    SBasis operator+(SBasis &, SBasis &)
    SBasis SB_sub_Sd "operator-" (SBasis &, double)
    SBasis SB_sub_SS "operator-" (SBasis &, SBasis &)
    SBasis SB_neg "operator-" (SBasis &)
    SBasis operator/(SBasis &, double)

    SBasis sin(Linear, int)
    SBasis cos(Linear, int)
    SBasis sqrt(SBasis &, int)
    SBasis reciprocal(Linear &, int)
    SBasis shift(Linear &, int)
    SBasis shift(SBasis &, int)
    SBasis inverse(SBasis, int)

    SBasis portion(SBasis &, Interval)
    SBasis portion(SBasis &, double, double)
    SBasis compose(SBasis &, SBasis &, unsigned int)
    SBasis compose(SBasis &, SBasis &)
    SBasis truncate(SBasis &, unsigned int)

    unsigned int valuation(SBasis &, double)

    vector[ vector[double] ] multi_roots(SBasis &, vector[double] &, double, double, double, double) #TODO
    vector[Interval] level_set(SBasis &, Interval &, double, double, double)
    vector[Interval] level_set(SBasis &, double, double, double, double, double)
    vector[double] roots(SBasis &, Interval const)
    vector[double] roots(SBasis &)
    vector[ vector[Interval] ] level_sets(SBasis &, vector[Interval] &, double, double, double) #TODO
    vector[ vector[Interval] ] level_sets(SBasis &, vector[double] &, double, double, double, double) #TODO

    SBasis reverse(SBasis &)
    SBasis derivative(SBasis &)
    SBasis integral(SBasis &)
    SBasis divide(SBasis &, SBasis &, int)
    SBasis compose_inverse(SBasis &, SBasis &, unsigned int, double)
    SBasis multiply(SBasis &, SBasis &)
    SBasis multiply_add(SBasis &, SBasis &, SBasis)

    OptInterval bounds_exact(SBasis &)
    OptInterval bounds_local(SBasis &, OptInterval &, int)
    OptInterval bounds_fast(SBasis &, int)

cdef class cy_SBasis:
    cdef SBasis* thisptr

cdef extern from "2geom/sbasis-curve.h" namespace "Geom":
    cdef cppclass SBasisCurve:
        SBasisCurve(D2[SBasis] &)
        SBasisCurve(Curve &)
        Curve * duplicate()
        Point initialPoint()
        Point finalPoint()
        bint isDegenerate()
        Point pointAt(Coord)
        Point operator()(double)
        vector[Point] pointAndDerivatives(Coord, unsigned int)
        Coord valueAt(Coord, Dim2)
        void setInitial(Point &)
        void setFinal(Point &)
        Rect boundsFast()
        Rect boundsExact()
        OptRect boundsLocal(OptInterval &, unsigned int)
        vector[double] roots(Coord, Dim2)
        Coord nearestTime(Point &, Coord, Coord)
        vector[double] allNearestTimes(Point &, Coord, Coord)
        Coord length(Coord)
        Curve * portion(Coord, Coord)
        Curve * transformed(Affine &)
        Curve * derivative()
        D2[SBasis] toSBasis()
        int degreesOfFreedom()


cdef extern from "2geom/bezier.h" namespace "Geom":
    cdef cppclass Bezier:
#~         struct Order
#~             pass
        unsigned int order()
        unsigned int size()
        Bezier()
#~         Bezier(Bezier.Order)
        Bezier(Coord)
        Bezier(Coord, Coord)
        Bezier(Coord, Coord, Coord)
        Bezier(Coord, Coord, Coord, Coord)
        void resize(unsigned int, Coord)
        void clear()
        unsigned int degree()
        bint isZero(double)
        bint isConstant(double)
        bint isFinite()
        Coord at0()
        Coord at1()
        Coord valueAt(double)
        Coord operator()(double)
        SBasis toSBasis()
        Coord & operator[](unsigned int)
        void setPoint(unsigned int, double)
        vector[double] valueAndDerivatives(Coord, unsigned int)
        pair[Bezier, Bezier] subdivide(Coord)
        vector[double] roots()
        vector[double] roots(Interval const)
        Bezier forward_difference(unsigned int)
        Bezier elevate_degree()
        Bezier reduce_degree()
        Bezier elevate_to_degree(unsigned int)
        Bezier deflate()
        Bezier(Coord *, unsigned int)

    Bezier operator*(Bezier &, double)
    Bezier operator+(Bezier &, double)
    Bezier operator-(Bezier &, double)
    Bezier operator/(Bezier &, double)

    Bezier portion(Bezier &, double, double)
    OptInterval bounds_local(Bezier &, OptInterval)
    Coord casteljau_subidivision(Coord, Coord *, Coord *, Coord *, unsigned int)
    void bezier_to_sbasis(SBasis &, Bezier &)
    Bezier integral(Bezier &)
    Bezier derivative(Bezier &)
    OptInterval bounds_exact(Bezier &)
    double bernsteinValueAt(double, double *, unsigned int)
    vector[Point] bezier_points(D2[Bezier] &)
    OptInterval bounds_fast(Bezier &)
    Bezier multiply(Bezier &, Bezier &)
    Bezier reverse(Bezier &)

#This is ugly workaround around cython's lack of support for integer template parameters
cdef extern from *:
    ctypedef int n_0 "0"
    ctypedef int n_1 "1"

cdef extern from "2geom/bezier-curve.h" namespace "Geom":
    cdef cppclass BezierCurve:
        unsigned int order()
        vector[Point] controlPoints()
        void setPoint(unsigned int, Point)
        void setPoints(vector[Point] &)
        Point operator[](unsigned int)
        Point initialPoint()
        Point finalPoint()
        bint isDegenerate()
        void setInitial(Point &)
        void setFinal(Point &)
        Rect boundsFast()
        Rect boundsExact()
        OptRect boundsLocal(OptInterval &, unsigned int)
        Curve * duplicate()
        Curve * portion(Coord, Coord)
        Curve * reverse()
        Curve * transformed(Affine &)
        Curve * derivative()
        int degreesOfFreedom()
        vector[double] roots(Coord, Dim2)
        Coord length(Coord)
        Point pointAt(Coord)
        vector[Point] pointAndDerivatives(Coord, unsigned int)
        Coord valueAt(Coord, Dim2)
        D2[SBasis] toSBasis()
        #protected:
#~         BezierCurve()
#~         BezierCurve(D2[Bezier] &)
#~         BezierCurve(Bezier &, Bezier &)
#~         BezierCurve(vector[Point] &)
    #~ Point middle_point(LineSegment &)
    #~ Coord length(LineSegment &)
    Coord bezier_length(Point, Point, Point, Point, Coord)
    Coord bezier_length(Point, Point, Point, Coord)
    Coord bezier_length(vector[Point] &, Coord)

    cdef cppclass LineSegment:
        LineSegment()
        LineSegment(D2[Bezier] &)
        LineSegment(Bezier, Bezier)
        LineSegment(vector[Point] &)
        LineSegment(Point, Point)
        pair[LineSegment, LineSegment] subdivide(Coord)

        Curve * duplicate()
        Curve * reverse()
        Curve * transformed(Affine &)
        Curve * derivative()

    cdef cppclass QuadraticBezier:
        QuadraticBezier()
        QuadraticBezier(D2[Bezier] &)
        QuadraticBezier(Bezier, Bezier)
        QuadraticBezier(vector[Point] &)
        QuadraticBezier(Point, Point, Point)
        pair[QuadraticBezier, QuadraticBezier] subdivide(Coord)

        Curve * duplicate()
        Curve * reverse()
        Curve * transformed(Affine &)
        Curve * derivative()


    cdef cppclass CubicBezier:
        CubicBezier()
        CubicBezier(D2[Bezier] &)
        CubicBezier(Bezier, Bezier)
        CubicBezier(vector[Point] &)
        CubicBezier(Point, Point, Point, Point)
        pair[CubicBezier, CubicBezier] subdivide(Coord)

        Curve * duplicate()
        Curve * reverse()
        Curve * transformed(Affine &)
        Curve * derivative()

#~     cdef cppclass BezierCurveN[n_1]:
#~         BezierCurveN ()
#~         BezierCurveN(Bezier &, Bezier &)
#~         #BezierCurveN(Point &, Point &)
#~         BezierCurveN(vector[Point] &)
#~         pair[BezierCurveN, BezierCurveN] subdivide(Coord)
#~
#~         Curve * duplicate()
#~         Curve * reverse()
#~         Curve * transformed(Affine &)
#~         Curve * derivative()

cdef class cy_BezierCurve:
    cdef BezierCurve* thisptr

cdef class cy_LineSegment(cy_BezierCurve):
    pass

cdef cy_LineSegment wrap_LineSegment(LineSegment p)

cdef extern from "2geom/bezier-curve.h" namespace "Geom::BezierCurve":
    BezierCurve * create(vector[Point] &)

cdef extern from "2geom/elliptical-arc.h" namespace "Geom":
    cdef cppclass EllipticalArc:
        EllipticalArc(EllipticalArc &)
        EllipticalArc()
        EllipticalArc(Point, Coord, Coord, Coord, bint, bint, Point)
        Interval angleInterval()
        Angle rotationAngle()
        Coord ray(Dim2)
        Point rays()
        bint largeArc()
        bint sweep()
        LineSegment chord()
        void set(Point &, double, double, double, bint, bint, Point &)
        void setExtremes(Point &, Point &)
        Coord center(Dim2)
        Point center()
        Coord sweepAngle()
        bint containsAngle(Coord)
        Point pointAtAngle(Coord)
        Coord valueAtAngle(Coord, Dim2)
        Affine unitCircleTransform()
        bint isSVGCompliant()
        pair[EllipticalArc, EllipticalArc] subdivide(Coord)
        Point initialPoint()
        Point finalPoint()
        Curve * duplicate()
        void setInitial(Point &)
        void setFinal(Point &)
        bint isDegenerate()
        Rect boundsFast()
        Rect boundsExact()
        OptRect boundsLocal(OptInterval &, unsigned int)
        vector[double] roots(double, Dim2)
        double nearestTime(Point &, double, double)
        int degreesOfFreedom()
        Curve * derivative()
        Curve * transformed(Affine &)
        vector[Point] pointAndDerivatives(Coord, unsigned int)
        D2[SBasis] toSBasis()
        double valueAt(Coord, Dim2)
        Point pointAt(Coord)
        Curve * portion(double, double)
        Curve * reverse()

cdef class cy_EllipticalArc:
    cdef EllipticalArc* thisptr
cdef cy_EllipticalArc wrap_EllipticalArc(EllipticalArc p)

cdef Curve * get_Curve_p(object c)
cdef bint is_Curve(object c)
