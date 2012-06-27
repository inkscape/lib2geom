from cython.operator cimport dereference as deref
from _common_decl cimport *

#-- Angle --

cdef class cy_Angle:
    #TODO add constructor from Point -- done as class method (sufficient?)
    cdef Angle* thisptr

    def __cinit__(self, double x):
        self.thisptr = new Angle(x)
    def __repr__(self):
        return "Angle, {0:.2f} radians, {1:.1f} degrees".format(
            self.radians(), self.degrees())
    def __dealloc__(self):
        del self.thisptr

    def radians(self):
        return self.thisptr.radians()
    def radians0(self):
        return self.thisptr.radians0()
    def degrees(self):
        return self.thisptr.degrees()
    def degreesClock(self):
        return self.thisptr.degreesClock()
    #TODO unify naming convenctions: _underscore vs mixedCase
    @classmethod
    def from_radians(cls, rad):
        #there might be more idiomatic way to do this
        return wrap_Angle(from_radians(rad))
    @classmethod
    def from_degrees(cls, d):
        return wrap_Angle(from_degrees(d))
    @classmethod
    def from_degrees_clock(cls, d):
        return wrap_Angle(from_degrees_clock(d))
    @classmethod
    def from_Point(cls, cy_Point p):
        cdef Point * pp = p.thisptr
        return wrap_Angle( Angle( deref(p.thisptr) ))

    def __add__(cy_Angle self, cy_Angle other):
        return wrap_Angle(deref(other.thisptr) + deref(self.thisptr))
    def __sub__(cy_Angle self, cy_Angle other):
        return wrap_Angle(deref(other.thisptr) - deref(self.thisptr))

    def __richcmp__(cy_Angle self, cy_Angle other, int op):
        #angles are not ordered
        if op == 2:
            return deref(other.thisptr) == deref(self.thisptr)
        elif op == 3:
            return deref(other.thisptr) != deref(self.thisptr)
        return NotImplemented

    @classmethod
    def deg_to_rad(cls, deg):
        return deg_to_rad(deg)
    @classmethod
    def rad_to_deg(cls, rad):
        return rad_to_deg(rad)
    @classmethod
    def map_circular_arc_on_unit_interval(cls, angle, start_angle, end_angle, cw = True):
        return map_circular_arc_on_unit_interval(angle, start_angle, end_angle, cw)
    @classmethod
    def map_unit_interval_on_circular_arc(cls, t, start_angle, end_angle, cw = True):
        return map_unit_interval_on_circular_arc(t, start_angle, end_angle, cw)
    @classmethod
    def arc_contains(cls, angle, start_angle, inside_angle, end_angle):
        return arc_contains(angle, start_angle, inside_angle, end_angle)

cdef cy_Angle wrap_Angle(Angle p):
    cdef Angle * retp = new Angle()
    retp[0] = p
    cdef cy_Angle r = cy_Angle.__new__(cy_Angle, 0)
    r.thisptr = retp
    return r

#-- Point --

cdef class cy_Point:
    cdef Point* thisptr

    def __cinit__(self, double x=0.0, double y=0.0):
        self.thisptr = new Point(x, y)
    def __repr__(self):
        return "Point ({0:.2f}, {1:.2f})".format(self[0], self[1])
    def __dealloc__(self):
        del self.thisptr


    def length(self):
        return self.thisptr.length()
    def ccw(self):
        return wrap_Point(self.thisptr.ccw())
    def cw(self):
        return wrap_Point(self.thisptr.cw())
    def __getitem__(self, key):
        return deref(self.thisptr)[key]
    @property
    def x(self):
        return self.thisptr.x()
    @property
    def y(self):
        return self.thisptr.y()
        
    def round(self):
        return wrap_IntPoint(self.thisptr.round())
    def floor(self):
        return wrap_IntPoint(self.thisptr.floor())
    def ceil(self):
        return wrap_IntPoint(self.thisptr.ceil())

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
        return self.thisptr.isZero()
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
    def are_near(cls, cy_Point a, cy_Point b, double eps = EPSILON):
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

#-- IntPoint --

cdef class cy_IntPoint:
    cdef IntPoint* thisptr

    def __init__(self, IntCoord x = 0, IntCoord y = 0):
        self.thisptr =  new IntPoint(x ,y)

    def __getitem__(self, key):
        return deref(self.thisptr)[key]

    @property
    def x(self):
        return self.thisptr.x()
    @property
    def y(self):
        return self.thisptr.y()
    def __add__(cy_IntPoint self, cy_IntPoint o):
        return wrap_IntPoint(deref(self.thisptr)+deref( o.thisptr ))
    def __sub__(cy_IntPoint self, cy_IntPoint o):
        return wrap_IntPoint(deref(self.thisptr)-deref( o.thisptr ))

    #lexicographic ordering, y coordinate is more significant
    def __richcmp__(cy_IntPoint self, cy_IntPoint other, int op):
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

cdef cy_IntPoint wrap_IntPoint(IntPoint p):
    cdef IntPoint * retp = new IntPoint()
    retp[0] = p
    cdef cy_IntPoint r = cy_IntPoint.__new__(cy_IntPoint)
    r.thisptr = retp
    return r

#-- Line --

cdef class cy_Line:
    cdef Line* thisptr
    def __cinit__(self, cy_Point cp = None, double x = 0):
        if cp is None:
            self.thisptr = new Line()
        else:
            self.thisptr = new Line( deref(cp.thisptr), x)
    def __dealloc__(self):
        del self.thisptr
    @classmethod
    def fromPoints(cls, cy_Point cp, cy_Point cq):
        return wrap_Line( (Line( deref(cp.thisptr), deref(cq.thisptr) )) )

    def origin(self):
        return wrap_Point(self.thisptr.origin())
    def versor(self):
        return wrap_Point(self.thisptr.versor())
    def angle(self):
        return self.thisptr.angle()

    def setPoints(self, cy_Point cp, cy_Point cq):
        self.thisptr.setPoints( deref(cp.thisptr), deref(cq.thisptr) )
    def setCoefficients(self, a, b, c):
        self.thisptr.setCoefficients(a, b, c)
    def isDegenerate(self):
        return self.thisptr.isDegenerate()
    def pointAt(self, t):
        return wrap_Point(self.thisptr.pointAt(t))
    def valueAt(self, t, Dim2 d):
        return self.thisptr.valueAt(t, d)
    def timeAt(self, cy_Point cp):
        return self.thisptr.timeAt( deref(cp.thisptr) )
    def timeAtProjection(self, cy_Point cp):
        return self.thisptr.timeAtProjection( deref(cp.thisptr) )
    def nearestPoint(self, cy_Point cp):
        return self.thisptr.nearestPoint( deref(cp.thisptr) )
    def reverse(self):
        return wrap_Line( self.thisptr.reverse() )
    def derivative(self):
        return wrap_Line( self.thisptr.derivative() )
    def normal(self):
        return wrap_Point( self.thisptr.normal() )
    def normalAndDist(self, x):
        return wrap_Point( self.thisptr.normalAndDist(x) )
    @classmethod
    def distance(cls, cy_Point cp, cy_Line cl):
        return distance( deref(cp.thisptr), deref(cl.thisptr))
    @classmethod
    def are_near(cls, cy_Point cp, cy_Line cl, double eps = EPSILON):
        return are_near( deref(cp.thisptr), deref(cl.thisptr), eps)
    @classmethod
    def are_parallel(cls, cy_Line cl, cy_Line ck, eps = EPSILON):
        return are_parallel( deref(cl.thisptr), deref(ck.thisptr), eps)
    @classmethod
    def are_same(cls, cy_Line cl, cy_Line ck, double eps = EPSILON):
        return are_same( deref(cl.thisptr), deref(ck.thisptr), eps)
    @classmethod
    def are_orthogonal(cls, cy_Line cl, cy_Line ck, eps = EPSILON):
        return are_orthogonal( deref(cl.thisptr), deref(ck.thisptr), eps)
    @classmethod
    def are_collinear(cls, cy_Point cp, cy_Point cq, cy_Point cr, eps = EPSILON):
        return are_collinear( deref(cp.thisptr), deref(cq.thisptr), deref(cr.thisptr), eps)
    @classmethod
    def angle_between(cls, cy_Line cl, cy_Line ck):
        return angle_between( deref(cl.thisptr), deref(ck.thisptr) )
    #TODO rest of line methods
cdef cy_Line wrap_Line(Line p):
    cdef Line * retp = new Line()
    retp[0] = p
    cdef cy_Line r = cy_Line.__new__(cy_Line)
    r.thisptr = retp
    return r

#-- Ray --

cdef class cy_Ray:
    cdef Ray* thisptr
    def __cinit__(self, cy_Point cp = None, double x = 0):
        if cp is None:
            self.thisptr = new Ray()
        else:
            self.thisptr = new Ray( deref(cp.thisptr), x)
    def __dealloc__(self):
        del self.thisptr
    @classmethod
    def fromPoints(cls, cy_Point cp, cy_Point cq):
        return wrap_Ray( (Ray( deref(cp.thisptr), deref(cq.thisptr) )) )

    def origin(self):
        return wrap_Point(self.thisptr.origin())
    def versor(self):
        return wrap_Point(self.thisptr.versor())
    def angle(self):
        return self.thisptr.angle()

    def setOrigin(self, cy_Point cp):
        self.thisptr.setOrigin( deref(cp.thisptr) )
    def setVersor(self, cy_Point cp):
        self.thisptr.setVersor( deref(cp.thisptr) )
    def setPoints(self, cy_Point cp, cy_Point cq):
        self.thisptr.setPoints( deref(cp.thisptr), deref(cq.thisptr) )
    def isDegenerate(self):
        return self.thisptr.isDegenerate()
    def pointAt(self, t):
        return wrap_Point(self.thisptr.pointAt(t))
    def valueAt(self, t, Dim2 d):
        return self.thisptr.valueAt(t, d)
    def nearestPoint(self, cy_Point cp):
        return self.thisptr.nearestPoint( deref(cp.thisptr) )
    def reverse(self):
        return wrap_Ray( self.thisptr.reverse() )
    @classmethod
    def distance(cls, cy_Point cp, cy_Ray cl):
        return distance( deref(cp.thisptr), deref(cl.thisptr))
    @classmethod
    def are_near(cls, cy_Point cp, cy_Ray cl, double eps = EPSILON):
        return are_near( deref(cp.thisptr), deref(cl.thisptr), eps)
    @classmethod
    def are_same(cls, cy_Ray cl, cy_Ray ck, double eps = EPSILON):
        return are_same( deref(cl.thisptr), deref(ck.thisptr), eps)
    @classmethod
    def angle_between(cls, cy_Ray cl, cy_Ray ck, bint cw = True):
        return angle_between( deref(cl.thisptr), deref(ck.thisptr), cw)
    @classmethod
    def make_angle_bisector_ray(cls, cy_Ray cl, cy_Ray ck):
        return wrap_Ray( make_angle_bisector_ray(deref(cl.thisptr), deref(ck.thisptr) ))

cdef cy_Ray wrap_Ray(Ray p):
    cdef Ray * retp = new Ray()
    retp[0] = p
    cdef cy_Ray r = cy_Ray.__new__(cy_Ray)
    r.thisptr = retp
    return r
