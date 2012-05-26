from _common_decl cimport *
from cython.operator cimport dereference as deref

cdef extern from "2geom/point.h" namespace "Geom":
    cdef cppclass Point:
        Point()
        Point(Coord, Coord)
        Coord length()
        Point ccw()
        Point cw()
        
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

cdef class cy_Point:
    cdef Point* thisptr     
    
    def __cinit__(self, double x=0.0, double y=0.0):
        self.thisptr = new Point(x, y)
    def __repr__(self):
        return "Point ({0:.2f}, {1:.2f})".format(self[0], self[1])
    
    def length(self):
        return self.thisptr.length()
    def ccw(self):
        return wrap_Point(self.thisptr.ccw())
    def cw(self):
        return wrap_Point(self.thisptr.cw())
    def __getitem__(self, key):
        return deref(self.thisptr)[key]

    def __neg__(self):
        return(wrap_Point(-deref(self.thisptr)))
    def __add__(cy_Point self, cy_Point other):
        return wrap_Point(deref(self.thisptr) + deref(other.thisptr))
    def __sub__(cy_Point self, cy_Point other):
        return wrap_Point(deref(self.thisptr) - deref(other.thisptr))
    #TODO rewrite to accept transforms
    #TODO exceptions
    def __mul__(cy_Point self, Coord s):
        return wrap_Point(deref(self.thisptr)*s)
    def __div__(cy_Point self, Coord s):
        return wrap_Point(deref(self.thisptr)/s)
    #lexicographic ordering, y coordinate is more significant
    def __richcmp__(cy_Point self, cy_Point other, int op):
        if op == 0:
            return deref(self.thisptr) < deref(other.thisptr)
        if op == 1:
            return deref(self.thisptr) <= deref(other.thisptr)
        if op == 2:
            return deref(self.thisptr) == deref(other.thisptr)
        if op == 3:
            return deref(self.thisptr) != deref(other.thisptr)
        if op == 4:
            return deref(self.thisptr) > deref(other.thisptr)
        if op == 5:
            return deref(self.thisptr) >= deref(other.thisptr)
    
    def isFinite(self):
        return self.thisptr.isFinite()
    def isZero(self):
        return self.thistpr.isZero()
    def isNormalized(self, eps = EPSILON):
        return self.thisptr.isNormalized(eps)

    @classmethod
    def polar(cy_Point cls, Coord angle, Coord radius = 1.0):
        return wrap_Point(polar(angle, radius))
    
    @classmethod
    def L2(cls, cy_Point p):
        return L2(deref(p.thisptr))
    @classmethod
    def L2sq(cls, cy_Point p):
        return L2sq(deref(p.thisptr))
    @classmethod
    def are_near(cls, cy_Point a, cy_Point b, eps = EPSILON):
        return are_near(deref(a.thisptr), deref(b.thisptr), eps)
    @classmethod
    def middle_point(cls, cy_Point a, cy_Point b):
        return wrap_Point(middle_point(deref(a.thisptr), deref(b.thisptr)))
    @classmethod
    def rot90(cls, cy_Point a):
        return wrap_Point(rot90(deref(a.thisptr)))
    @classmethod
    def lerp(cls, double t, cy_Point a, cy_Point b):
        return wrap_Point(lerp(t, deref(a.thisptr), deref(b.thisptr)))
    @classmethod
    def dot(cls, cy_Point a, cy_Point b):
        return dot(deref(a.thisptr), deref(b.thisptr))
    @classmethod
    def cross(cls, cy_Point a, cy_Point b):
        return cross(deref(a.thisptr), deref(b.thisptr))
    @classmethod
    def distance(cls, cy_Point a, cy_Point b):
        return distance(deref(a.thisptr), deref(b.thisptr))
    @classmethod
    def distanceSq(cls, cy_Point a, cy_Point b):
        return distanceSq(deref(a.thisptr), deref(b.thisptr))
    
    @classmethod
    def unit_vector(cls, cy_Point p):
        return wrap_Point(unit_vector(deref(p.thisptr)))
    @classmethod
    def L1(cls, cy_Point p):
        return L1(deref(p.thisptr))
    @classmethod
    def LInfty(cls, cy_Point p):
        return LInfty(deref(p.thisptr))
    @classmethod
    def is_zero(cls, cy_Point p):
        return is_zero(deref(p.thisptr))
    @classmethod
    def is_unit_vector(cls, cy_Point p):
        return is_unit_vector(deref(p.thisptr))
    @classmethod
    def atan2(cls, cy_Point p):
        return atan2(deref(p.thisptr))
    @classmethod
    def angle_between(cls, cy_Point a, cy_Point b):
        return angle_between(deref(a.thisptr), deref(b.thisptr))
    @classmethod
    def abs(cls, cy_Point p):
        return wrap_Point(abs(deref(p.thisptr)))
    @classmethod
    def constrain_angle(cls, cy_Point a, cy_Point b, unsigned int n, cy_Point p):
        return wrap_Point(constrain_angle(deref(a.thisptr), deref(b.thisptr), n, deref(p.thisptr))) 

cdef cy_Point wrap_Point(Point p):
    cdef Point * retp = new Point()
    retp[0] = p
    cdef cy_Point r = cy_Point.__new__(cy_Point)
    r.thisptr = retp
    return r

