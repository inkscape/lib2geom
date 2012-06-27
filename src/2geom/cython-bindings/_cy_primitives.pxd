from _common_decl cimport Coord, IntCoord, Dim2, EPSILON
#from cython.libcpp cimport vector


cdef extern from "2geom/angle.h" namespace "Geom":
    cdef cppclass Angle:
        Angle()
        Angle(Coord)
        Angle(Point)
        Coord radians()
        Coord radians0()
        Coord degrees()
        Coord degreesClock()

        Angle &operator+(Angle &)
        Angle &operator-(Angle &)
        bint operator==(Angle &)
        bint operator!=(Angle &)

    Coord deg_to_rad(Coord)
    Coord rad_to_deg(Coord)
    double map_circular_arc_on_unit_interval(double, double, double, bint)
    Coord map_unit_interval_on_circular_arc(Coord, double, double, bint)
    bint arc_contains (double, double, double, double)



cdef extern from "2geom/angle.h" namespace "Geom::Angle":
    Angle from_radians(Coord d)
    Angle from_degrees(Coord d)
    Angle from_degrees_clock(Coord d)


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

        #TODO add rounding methods in case of wrapping IntPoint

        Coord &operator[](int)
        Point operator-()
        Point &operator+(Point &)
        Point &operator-(Point &)
        Point &operator*(Coord)
        Point &operator/(Coord)

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

cdef extern from "2geom/int-point.h" namespace "Geom":
    cdef cppclass IntPoint:
        IntPoint()
        IntPoint(IntCoord, IntCoord)
        IntPoint(IntPoint &)
        IntCoord operator[](unsigned int)
        #IntCoord & operator[](unsigned int)
        #IntCoord operator[](Dim2)
        #IntCoord & operator[](Dim2)
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

cdef extern from "2geom/line.h" namespace "Geom":
    cdef cppclass Line:
        Line()
        Line(Point &, Coord)
        Line(Point &, Point &)

        #Line(LineSegment &)
        #Line(Ray &)
        #Line from_normal_distance(Point, double)
        #Line from_origin_and_versor(Point, Point)
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
        Coord nearestPoint(Point &)
        #vector[Coord] roots(Coord, int)
        Line reverse()
        #Curve* portion(Coord, Coord)
        #LineSegment segment(Coord, Coord)
        #Ray ray(Coord)
        Line derivative()
        #Line transformed(Affine &)
        Point normal()
        Point normalAndDist(double &)

    double distance(Point &, Line &)
    bint are_near(Point &, Line &, double)
    bint are_parallel(Line &, Line &, double)
    bint are_same(Line &, Line &, double)
    bint are_orthogonal(Line &, Line &, double)
    bint are_collinear(Point &, Point &, Point &, double)

    double angle_between(Line &, Line &)
    #double distance(Point &, LineSegment &)

cdef extern from "2geom/ray.h" namespace "Geom":
    cdef cppclass Ray:
        Ray()
        Ray(Point &, Coord)
        Ray(Point&, Point &)
        Point origin()
        Point versor()
        void setOrigin(Point &)
        void setVersor(Point &)
        Coord angle()
        void setPoints(Point &, Point &)
        bint isDegenerate()
        Point pointAt(Coord)
        Coord valueAt(Coord, Dim2)
        #vector[Coord] roots(Coord, Dim2)#TODO
        Coord nearestPoint(Point &)
        Ray reverse()
        #Curve *portion(Coord, Coord)
        #LineSegment segment(Coord, Coord)
        #Ray transformed(Affine &)
    double distance(Point &, Ray &)
    bint are_near(Point &, Ray &, double)
    bint are_same(Ray&, Ray &, double)
    double angle_between(Ray &, Ray &, bint)
    Ray make_angle_bisector_ray(Ray &, Ray&)    
