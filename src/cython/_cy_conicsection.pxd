from _common_decl cimport *

from libcpp.vector cimport vector
from libcpp.pair cimport pair

from _cy_rectangle cimport Interval, OptInterval, Rect, OptRect
from _cy_rectangle cimport cy_OptRect
from _cy_affine cimport is_transform, get_Affine, Affine
from _cy_curves cimport Curve, cy_Curve,  wrap_Curve_p
from _cy_curves cimport SBasis, cy_SBasis
from _cy_curves cimport EllipticalArc, cy_EllipticalArc, wrap_EllipticalArc

from _cy_path cimport Path, cy_Path

from _cy_primitives cimport Point, cy_Point, wrap_Point, wrap_vector_point, make_vector_point


cdef extern from "2geom/circle.h" namespace "Geom":
    cdef cppclass Circle:
        Circle()
        Circle(double, double, double)
        Circle(Point, double)
        Circle(double, double, double, double)
        Circle(vector[Point] &)
        void setCenter(Point &)
        void setRadius(double)
        void set(double, double, double, double)
        void fit(vector[Point] &)
        EllipticalArc * arc(Point &, Point &, Point &, bint)
        Point center()
        Coord center(Dim2)
        Coord radius()

cdef extern from "2geom/ellipse.h" namespace "Geom":
    cdef cppclass Ellipse:
        Ellipse()
        Ellipse(double, double, double, double, double)
        Ellipse(double, double, double, double, double, double)
        Ellipse(vector[Point] &)
        Ellipse(Circle &)
        void set(double, double, double, double, double)
        void set(double, double, double, double, double, double)
        void set(vector[Point] &)
        EllipticalArc * arc(Point &, Point &, Point &, bint)
        Point center()
        Coord center(Dim2)
        Coord ray(Dim2)
        Coord rot_angle()
        vector[double] implicit_form_coefficients()
        Ellipse transformed(Affine &)
