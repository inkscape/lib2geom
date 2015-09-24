from _common_decl cimport *


cdef extern from "2geom/affine.h" namespace "Geom":
    cdef cppclass Affine:
        pass
    cdef cppclass Translate
    cdef cppclass Scale
    cdef cppclass Rotate
    cdef cppclass VShear
    cdef cppclass HShear
    cdef cppclass Zoom

    
cdef extern from "2geom/angle.h" namespace "Geom":
    cdef cppclass Angle:
        Angle()
        Angle(Coord)
        Angle(Point)
        Coord radians()
        Coord radians0()
        Coord degrees()
        Coord degreesClock()

        Coord operator()
        Angle &operator+(Angle &)
        Angle &operator-(Angle &)
        bint operator==(Angle &)
        bint operator!=(Angle &)

    Coord rad_from_deg(Coord)
    Coord deg_from_rad(Coord)

cdef extern from "2geom/angle.h" namespace "Geom::Angle":
    Angle from_radians(Coord d)
    Angle from_degrees(Coord d)
    Angle from_degrees_clock(Coord d)

cdef class cy_Angle:
    cdef Angle* thisptr

cdef cy_Angle wrap_Angle(Angle)


cdef extern from "2geom/angle.h" namespace "Geom":
    cdef cppclass AngleInterval:
        AngleInterval(AngleInterval &)
        AngleInterval(Angle &, Angle &, bint)
        AngleInterval(double, double, bint)
        Angle & initialAngle()
        Angle & finalAngle()
        bint isDegenerate()
        Angle angleAt(Coord)
        Angle operator()(Coord)
        bint contains(Angle &)
        Coord extent()


cdef extern from "2geom/point.h" namespace "Geom":
    cdef cppclass Point:
        Point()
        Point(Coord, Coord)
        Coord length()
        Point ccw()
        Point cw()
        Coord x()
        Coord y()
        IntPoint round()
        IntPoint floor()
        IntPoint ceil()

        bint isFinite()
        bint isZero()
        bint isNormalized(Coord)

        bint operator==(Point &)
        bint operator!=(Point &)
        bint operator<(Point &)
        bint operator>(Point &)
        bint operator<=(Point &)
        bint operator>=(Point &)

        Coord &operator[](int)
        Point operator-()
        Point &operator+(Point &)
        Point &operator-(Point &)
        Point &operator*(Coord)
        Point &operator/(Coord)
        
        Point &operator*(Affine  &)
        Point &operator*(Translate  &)
        Point &operator*(Scale  &)
        Point &operator*(Rotate  &)
        Point &operator*(HShear  &)
        Point &operator*(VShear  &)
        Point &operator*(Zoom  &)
            
    Coord L2(Point &)
    Coord L2sq(Point &)

    bint are_near(Point &, Point &, double)

    Point middle_point(Point &, Point &)
    Point rot90(Point &)
    Point lerp(double, Point &, Point &)

    Coord dot(Point &, Point &)
    Coord cross(Point &, Point &)
    Coord distance (Point &, Point &)
    Coord distanceSq (Point &, Point &)

    Point unit_vector(Point &)
    Coord L1(Point &)
    Coord LInfty(Point &)
    bint is_zero(Point &)
    bint is_unit_vector(Point &)
    double atan2(Point &)
    double angle_between(Point &, Point &)
    Point abs(Point &)
    Point constrain_angle(Point &, Point &, unsigned int, Point &)

cdef extern from "2geom/point.h" namespace "Geom::Point":
    Point polar(Coord angle, Coord radius)

cdef class cy_Point:
    cdef Point* thisptr

cdef cy_Point wrap_Point(Point p)
cdef object wrap_vector_point(vector[Point] v)
cdef vector[Point] make_vector_point(object l)


cdef extern from "2geom/int-point.h" namespace "Geom":
    cdef cppclass IntPoint:
        IntPoint()
        IntPoint(IntCoord, IntCoord)
        IntPoint(IntPoint &)
        IntCoord operator[](unsigned int)
        IntCoord x()
        IntCoord y()
        #why doesn't IntPoint have unary -?
        IntPoint & operator+(IntPoint &)
        IntPoint & operator-(IntPoint &)
        bint operator==(IntPoint &)
        bint operator!=(IntPoint &)
        bint operator<=(IntPoint &)
        bint operator>=(IntPoint &)
        bint operator>(IntPoint &)
        bint operator<(IntPoint &)

cdef class cy_IntPoint:
    cdef IntPoint* thisptr

cdef cy_IntPoint wrap_IntPoint(IntPoint p)


cdef extern from "2geom/curve.h" namespace "Geom":
    cdef cppclass Curve

cdef extern from "2geom/bezier.h" namespace "Geom":
    cdef cppclass LineSegment

cdef extern from "2geom/line.h" namespace "Geom":
    cdef cppclass Line:
        Line()
        Line(Point &, Coord)
        Line(Point &, Point &)

        Line(LineSegment &)
        Line(Ray &)
        Line* duplicate()

        Point origin()
        Point versor()
        Coord angle()
        void setOrigin(Point &)
        void setVersor(Point &)
        void setAngle(Coord)
        void setPoints(Point &, Point &)
        void setCoefficients(double, double, double)
        bint isDegenerate()
        Point pointAt(Coord)
        Coord valueAt(Coord, Dim2)
        Coord timeAt(Point &)
        Coord timeAtProjection(Point &)
        Coord nearestTime(Point &)
        vector[Coord] roots(Coord, Dim2)
        Line reverse()
        Curve* portion(Coord, Coord)
        LineSegment segment(Coord, Coord)
        Ray ray(Coord)
        Line derivative()
        Line transformed(Affine &)
        Point normal()
        Point normalAndDist(double &)

    double distance(Point &, Line &)
    bint are_near(Point &, Line &, double)
    bint are_parallel(Line &, Line &, double)
    bint are_same(Line &, Line &, double)
    bint are_orthogonal(Line &, Line &, double)
    bint are_collinear(Point &, Point &, Point &, double)

    double angle_between(Line &, Line &)
    double distance(Point &, LineSegment &)

cdef extern from "2geom/line.h" namespace "Geom::Line":
    Line from_origin_and_versor(Point, Point)
    Line from_normal_distance(Point, double)


cdef extern from "2geom/ray.h" namespace "Geom":
    cdef cppclass Ray:
        Ray()
        Ray(Point &, Coord)
        Ray(Point&, Point &)
        Point origin()
        Point versor()
        void setOrigin(Point &)
        void setVersor(Point &)
        void setAngle(Coord)
        Coord angle()
        void setPoints(Point &, Point &)
        bint isDegenerate()
        Point pointAt(Coord)
        Coord valueAt(Coord, Dim2)
        vector[Coord] roots(Coord, Dim2)
        Coord nearestTime(Point &)
        Ray reverse()
        Curve *portion(Coord, Coord)
        LineSegment segment(Coord, Coord)
        Ray transformed(Affine &)
    double distance(Point &, Ray &)
    bint are_near(Point &, Ray &, double)
    bint are_same(Ray&, Ray &, double)
    double angle_between(Ray &, Ray &, bint)
    Ray make_angle_bisector_ray(Ray &, Ray&)    
