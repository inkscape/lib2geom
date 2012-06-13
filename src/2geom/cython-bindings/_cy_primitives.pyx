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

# --Generic Interval--
#TODO: move to better place -- ? 

cdef class cy_int_GenericInterval:
    cdef GenericInterval[int]* thisptr
    def __cinit__(self, int a, int b):
        self.thisptr = new GenericInterval[int](a, b)
    def __dealloc__(self):
        del self.thisptr
    def _type(self):
        return int
    def max(self):
        return self.thisptr.max()
    def min(self):
        return self.thisptr.min()
    def middle(self):
        return self.thisptr.middle()
    def extent(self):
        return self.thisptr.extent()
    def isSingular(self):
        return self.thisptr.isSingular()
    def contains(self, int c):
        return self.thisptr.contains(c)
    def containsInterval(self, cy_int_GenericInterval interval):
        return self.thisptr.contains(deref(interval.thisptr))
    def setMin(self, val):
        self.thisptr.setMin(val)
    def setMax(self, val):
        self.thisptr.setMax(val)
    def expandTo(self, val):
        self.thisptr.expandTo(val)
    def expandBy(self, val):
        self.thisptr.expandBy(val)
    def unionWith(self, cy_int_GenericInterval interval):
        self.thisptr.unionWith( deref(interval.thisptr) )
    def _add_number(self, int X):
        return wrap_int_GenericInterval(deref(self.thisptr)+X)
    def _sub_number(self, int X):
        return wrap_int_GenericInterval(deref(self.thisptr)-X)
    def _add_interval(self, cy_int_GenericInterval I):
        return wrap_int_GenericInterval(deref(self.thisptr)+deref(I.thisptr))
    def _sub_interval(self, cy_int_GenericInterval I):
        return wrap_int_GenericInterval(deref(self.thisptr)-deref(I.thisptr))
    def _or_interval(self, cy_int_GenericInterval I):
        return wrap_int_GenericInterval(deref(self.thisptr)|deref(I.thisptr))
    def _unary(self):
        return wrap_int_GenericInterval(-deref(self.thisptr))
    def comp(self, cy_int_GenericInterval other, op):
        if op==2:
            return deref(self.thisptr)==deref(other.thisptr)
        elif op==3:
            return deref(self.thisptr)!=deref(other.thisptr)

cdef cy_int_GenericInterval wrap_int_GenericInterval(GenericInterval[int] p):
    cdef GenericInterval[int] * retp = new GenericInterval[int]()
    retp[0] = p
    cdef cy_int_GenericInterval r = cy_int_GenericInterval.__new__(
                                        cy_int_GenericInterval, 0, 0)
    r.thisptr = retp
    return r
        
cdef class cy_double_GenericInterval:
    cdef GenericInterval[double]* thisptr
    def __cinit__(self, double a, double b):
        self.thisptr = new GenericInterval[double](a, b)
    def __dealloc__(self):
        del self.thisptr
    def _type(self):
        return float
    def max(self):
        return self.thisptr.max()
    def min(self):
        return self.thisptr.min()
    def middle(self):
        return self.thisptr.middle()
    def extent(self):
        return self.thisptr.extent()
    def isSingular(self):
        return self.thisptr.isSingular()
    def contains(self, double c):
        return self.thisptr.contains(c)
    def containsInterval(self, cy_double_GenericInterval interval):
        return self.thisptr.contains(deref(interval.thisptr))
    def setMin(self, val):
        self.thisptr.setMin(val)
    def setMax(self, val):
        self.thisptr.setMax(val)
    def expandTo(self, val):
        self.thisptr.expandTo(val)
    def expandBy(self, val):
        self.thisptr.expandBy(val)
    def unionWith(self, cy_double_GenericInterval interval):
        self.thisptr.unionWith( deref(interval.thisptr) )
    def _add_number(self, double X):
        return wrap_double_GenericInterval(deref(self.thisptr)+X)
    def _sub_number(self, double X):
        return wrap_double_GenericInterval(deref(self.thisptr)-X)
    def _add_interval(self, cy_double_GenericInterval I):
        return wrap_double_GenericInterval(deref(self.thisptr)+deref(I.thisptr))
    def _sub_interval(self, cy_double_GenericInterval I):
        return wrap_double_GenericInterval(deref(self.thisptr)-deref(I.thisptr))
    def _or_interval(self, cy_double_GenericInterval I):
        return wrap_double_GenericInterval(deref(self.thisptr)|deref(I.thisptr))
    def _unary(self):
        return wrap_double_GenericInterval(-deref(self.thisptr))
    def comp(self, cy_double_GenericInterval other, op):
        if op==2:
            return deref(self.thisptr)==deref(other.thisptr)
        elif op==3:
            return deref(self.thisptr)!=deref(other.thisptr)

cdef cy_double_GenericInterval wrap_double_GenericInterval(GenericInterval[double] p):
    cdef GenericInterval[double] * retp = new GenericInterval[double]()
    retp[0] = p
    cdef cy_double_GenericInterval r = cy_double_GenericInterval.__new__(
                                        cy_double_GenericInterval, 0, 0)
    r.thisptr = retp
    return r

class py_GenericInterval:
    def __init__(self, a = 0, b = None):
        self.cy_class = None
        #constructor for singular interval
        if b is None:
            if isinstance(a, int):
                self.cy_class = cy_int_GenericInterval(a, a)
                self.type = int
                self.cy_class_type = cy_int_GenericInterval
            elif isinstance(a, float):
                self.cy_class = cy_double_GenericInterval(a, a)
                self.type = float
                self.cy_class_type = cy_double_GenericInterval
            
        if isinstance(a, int) and isinstance(b, int):
            self.cy_class = cy_int_GenericInterval(a, b)
            self.type = int
            self.cy_class_type = cy_int_GenericInterval
        if isinstance(a, float):
            if isinstance(b, float) or isinstance(b, int):
                self.cy_class = cy_double_GenericInterval(a, b)
                self.type = float
                self.cy_class_type = cy_double_GenericInterval
        if isinstance(b, float):
            #first case can't happen
            if isinstance(a, float) or isinstance(a, int): 
                self.cy_class = cy_double_GenericInterval(a, b)
                self.type = float
                self.cy_class_type = cy_double_GenericInterval
        if self.cy_class is None:
            raise(TypeError)
    def __repr__(self):
        return "Interval [{0:.2f}; {1:.2f}]".format(
            self.min(), self.max())
    @classmethod
    def __from_cy_class(cls, cy_class):
        #alternative constructor from cy_class
        #this is not very nice, I am working on different way of handling templates
        ret = py_GenericInterval(0, 0)
        ret.type = cy_class._type()
        ret.cy_class = cy_class
        ret.cy_class_type = cy_class.__class__
        return ret
    @classmethod
    #TODO this is not done by the C++ method
    def unify(cls, interval1, interval2):
        if interval1.type == interval2.type:
            return interval1 | interval2
    @classmethod
    #TODO this is not done by the C++ method
    def from_list(cls, lst):
        if float in map(type, lst):
            ret = py_GenericInterval(float(lst[0]))
        else:
            #TODO maybe fail differently on empty list
            ret = py_GenericInterval(int(lst[0]))
        for i in lst[1:]:
            ret.expandTo(i)
        return ret
    def max(self):
        return self.cy_class.max()
    def min(self):
        return self.cy_class.min()
    def middle(self):
        return self.cy_class.middle()
    def extent(self):
        return self.cy_class.extent()
    def isSingular(self):
        return self.cy_class.isSingular()
    def contains(self, c):
        return self.cy_class.contains(c)
    def containsInterval(self, interval):
        return self.cy_class.containsInterval(interval.cy_class)

    def setMin(self, val):
        self.cy_class.setMin(val)
    def setMax(self, val):
        self.cy_class.setMax(val)
    def expandTo(self, val):
        self.cy_class.expandTo(val)
    def expandBy(self, val):
        self.cy_class.expandBy(val)
    def unionWith(self, interval):
        self.cy_class.unionWith( interval.cy_class )
    
    def __add__(self, other):
        if isinstance(other, self.type):
            return py_GenericInterval.__from_cy_class(
                self.cy_class._add_number(other))
        elif isinstance(other, py_GenericInterval) and\
         isinstance(other.cy_class, self.cy_class_type):
            return py_GenericInterval.__from_cy_class(
                self.cy_class._add_interval(other.cy_class))
        raise(TypeError)
    def __sub__(self, other):
        if isinstance(other, self.type):
            return py_GenericInterval.__from_cy_class(
                self.cy_class._sub_number(other))
        elif isinstance(other, py_GenericInterval) and\
         isinstance(other.cy_class, self.cy_class_type):
            return py_GenericInterval.__from_cy_class(
                self.cy_class._sub_interval(other.cy_class))
        raise(TypeError)
    def __or__(self, other):
        if isinstance(other, py_GenericInterval) and\
        isinstance(other.cy_class, self.cy_class_type):
            return py_GenericInterval.__from_cy_class(
                self.cy_class._or_interval(other.cy_class))
        raise(TypeError)
    def __neg__(self):
        return py_GenericInterval.__form_cy_class(self.cy_class._unary())
            
    def __eq__(self, other):
        if self.type != other.type:
            return NotImplemented
        return self.cy_class.comp(other.cy_class, 2)
        

    def __neq__(self, other):
        if self.type != other.type:
            return NotImplemented
        return self.cy_class.comp(other.cy_class, 3)
        
