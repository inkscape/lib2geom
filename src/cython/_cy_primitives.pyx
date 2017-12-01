from numbers import Number

from _common_decl cimport *
from cython.operator cimport dereference as deref

from _cy_affine cimport cy_Translate, cy_Rotate, cy_Scale
from _cy_affine cimport cy_VShear, cy_HShear, cy_Zoom
from _cy_affine cimport cy_Affine, get_Affine, is_transform

from _cy_curves cimport cy_Curve, wrap_Curve_p
from _cy_curves cimport cy_LineSegment, wrap_LineSegment

cdef class cy_Angle:

    """Class representig angle.

    Angles can be tested for equality, but they are not ordered.

    Corresponds to Angle class in 2geom. Most members are direct
    calls to Angle methods, otherwise C++ call is specified.
    """

    def __cinit__(self, double x):
        """Create new angle from value in radians."""
        self.thisptr = new Angle(x)

    def __repr__(self):
        """repr(self)"""
        return "Angle({0:2.f})".format(self.radians())

    def __str__(self):
        """str(self)"""
        return "{0:.2f} radians".format(self.radians())

    def __dealloc__(self):
        del self.thisptr

    @classmethod
    def from_radians(cls, rad):
        """Construnct angle from radians."""
        return wrap_Angle(from_radians(rad))

    @classmethod
    def from_degrees(cls, d):
        """Construnct angle from degrees."""
        return wrap_Angle(from_degrees(d))

    @classmethod
    def from_degrees_clock(cls, d):
        """Construnct angle from degrees in clock convention."""
        return wrap_Angle(from_degrees_clock(d))

    @classmethod
    def from_Point(cls, cy_Point p):
        """Construct angle from Point. Calls Angle(Point) in 2geom."""
        cdef Point * pp = p.thisptr
        return wrap_Angle( Angle( deref(p.thisptr) ))

    def __float__(self):
        """float(self)"""
        return <Coord> deref(self.thisptr)

    def __add__(cy_Angle self, cy_Angle other):
        """alpha + beta"""
        return wrap_Angle(deref(other.thisptr) + deref(self.thisptr))

    def __sub__(cy_Angle self, cy_Angle other):
        """alpha - beta"""
        return wrap_Angle(deref(other.thisptr) - deref(self.thisptr))

    def __richcmp__(cy_Angle self, cy_Angle other, int op):
        """Test equality of angles. Note: angles are not ordered."""
        if op == 2:
            return deref(other.thisptr) == deref(self.thisptr)
        elif op == 3:
            return deref(other.thisptr) != deref(self.thisptr)
        return NotImplemented

    def radians(self):
        """Return the angle in radians."""
        return self.thisptr.radians()

    def radians0(self):
        """Return the angle in positive radians."""
        return self.thisptr.radians0()

    def degrees(self):
        """Return the angle in degrees."""
        return self.thisptr.degrees()

    def degrees_clock(self):
        """Return the angle in clock convention. Calls degreesClock() in 2geom."""
        return self.thisptr.degreesClock()

    @classmethod
    def rad_from_deg(cls, deg):
        """Convert degrees to radians."""
        return rad_from_deg(deg)

    @classmethod
    def deg_from_rad(cls, rad):
        """Convert radians to degrees."""
        return deg_from_rad(rad)

cdef cy_Angle wrap_Angle(Angle p):
    cdef Angle * retp = new Angle()
    retp[0] = p
    cdef cy_Angle r = cy_Angle.__new__(cy_Angle, 0)
    r.thisptr = retp
    return r


cdef class cy_AngleInterval:

    """ Class representing interval of angles.

    Corresponds to AngleInterval class in 2geom. Most members are direct
    calls to AngleInterval methods, otherwise C++ call is specified.
    """

    cdef AngleInterval* thisptr

    def __cinit__(self, start, end, bint cw=False):
        """Create AngleInterval from starting and ending value.

        Optional argument cw specifies direction - counter-clockwise
        is default.
        """
        self.thisptr = new AngleInterval(float(start), float(end), cw)

    def __call__(self, Coord t):
        """A(t), maps unit interval to Angle."""
        return wrap_Angle(deref( self.thisptr ) (t))

    def initial_angle(self):
        """Return initial angle as Angle instance."""
        return wrap_Angle(self.thisptr.initialAngle())

    def final_angle(self):
        """Return final angle as Angle instance."""
        return wrap_Angle(self.thisptr.finalAngle())

    def is_degenerate(self):
        """Test for empty interval."""
        return self.thisptr.isDegenerate()

    def angle_at(self, Coord t):
        """A.angle_at(t) <==> A(t)"""
        return wrap_Angle(self.thisptr.angleAt(t))

    def contains(self, cy_Angle a):
        """Test whether interval contains Angle."""
        return self.thisptr.contains(deref( a.thisptr ))

    def extent(self):
        """Calculate interval's extent."""
        return self.thisptr.extent()

cdef cy_AngleInterval wrap_AngleInterval(AngleInterval p):
    cdef AngleInterval * retp = new AngleInterval(0, 0, 0)
    retp[0] = p
    cdef cy_AngleInterval r = cy_AngleInterval.__new__(cy_AngleInterval)
    r.thisptr = retp
    return r


cdef class cy_Point:

    """Represents point or vector in 2D plane.

    Points are ordered lexicographically, with y coordinate being
    more significant.

    Corresponds to Point class in 2geom. Most members are direct
    calls to Point methods, otherwise C++ call is specified.
    """

    def __cinit__(self, double x=0.0, double y=0.0):
        """Create Point from it's cartesian coordinates."""
        self.thisptr = new Point(x, y)

    def __repr__(self):
        """repr(self)"""
        return "Point ({0:.3f}, {1:.3f})".format(self[0], self[1])

    def __str__(self):
        """str(self)"""
        return "[{0:.3f}, {1:.3f}]".format(self[0], self[1])

    def __dealloc__(self):
        del self.thisptr

    @classmethod
    def polar(cls, Coord angle, Coord radius = 1.0):
        """Create Point from polar coordinates."""
        return wrap_Point(polar(angle, radius))

    def length(self):
        """Return distance from origin or length of vector."""
        return self.thisptr.length()

    def ccw(self):
        """Return point rotated counter-clockwise."""
        return wrap_Point(self.thisptr.ccw())

    def cw(self):
        """Return point rotated clockwise."""
        return wrap_Point(self.thisptr.cw())

    def __getitem__(self, key):
        """Access coordinates of point."""
        return deref(self.thisptr)[key]

    @property
    def x(self):
        """First coordinate of point."""
        return self.thisptr.x()

    @property
    def y(self):
        """Second coordinate of point."""
        return self.thisptr.y()

    def round(self):
        """Create IntPoint rounding coordinates."""
        return wrap_IntPoint(self.thisptr.round())

    def floor(self):
        """Create IntPoint flooring coordinates."""
        return wrap_IntPoint(self.thisptr.floor())

    def ceil(self):
        """Create IntPoint ceiling coordinates."""
        return wrap_IntPoint(self.thisptr.ceil())

    def __neg__(self):
        """-P"""
        return(wrap_Point(-deref(self.thisptr)))

    def __abs__(self):
        """abs(P)"""
        return self.length()

    def __add__(cy_Point self, cy_Point other):
        """P + Q"""
        return wrap_Point(deref(self.thisptr) + deref(other.thisptr))

    def __sub__(cy_Point self, cy_Point other):
        """P - Q"""
        return wrap_Point(deref(self.thisptr) - deref(other.thisptr))

    #TODO exceptions
    def __mul__(cy_Point self, s):
        """Multiply point by number or Affine transform."""
        if isinstance(s, Number):
            return wrap_Point(deref(self.thisptr)* (<Coord> float(s)))
        elif isinstance(s, cy_Affine):
            return wrap_Point( deref(self.thisptr) * <Affine &> deref( (<cy_Affine> s).thisptr ) )
        elif isinstance(s, cy_Translate):
            return wrap_Point( deref(self.thisptr) * <Translate &> deref( (<cy_Translate> s).thisptr ) )
        elif isinstance(s, cy_Scale):
            return wrap_Point( deref(self.thisptr) * <Scale &> deref( (<cy_Scale> s).thisptr ) )
        elif isinstance(s, cy_Rotate):
            return wrap_Point( deref(self.thisptr) * <Rotate &> deref( (<cy_Rotate> s).thisptr ) )
        elif isinstance(s, cy_HShear):
            return wrap_Point( deref(self.thisptr) * <HShear &> deref( (<cy_HShear> s).thisptr ) )
        elif isinstance(s, cy_VShear):
            return wrap_Point( deref(self.thisptr) * <VShear &> deref( (<cy_VShear> s).thisptr ) )
        elif isinstance(s, cy_Zoom):
            return wrap_Point( deref(self.thisptr) * <Zoom &> deref( (<cy_Zoom> s).thisptr ) )
        return NotImplemented

    def __div__(cy_Point self, Coord s):
        """P / s"""
        return wrap_Point(deref(self.thisptr)/s)

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
        """Test whether point is finite."""
        return self.thisptr.isFinite()

    def isZero(self):
        """Test whether point is origin"""
        return self.thisptr.isZero()

    def isNormalized(self, eps=EPSILON):
        """Test whether point's norm is close to 1."""
        return self.thisptr.isNormalized(eps)

    @classmethod
    def L2(cls, cy_Point p):
        """Compute L2 (Euclidean) norm of point.

        L2(P) = sqrt( P.x**2 + P.y**2 )
        """
        return L2(deref(p.thisptr))

    @classmethod
    def L2sq(cls, cy_Point p):
        """Compute square of L2 (Euclidean) norm."""
        return L2sq(deref(p.thisptr))

    @classmethod
    def are_near(cls, cy_Point a, cy_Point b, double eps=EPSILON):
        """Test if two points are close."""
        return are_near(deref(a.thisptr), deref(b.thisptr), eps)

    @classmethod
    def middle_point(cls, cy_Point a, cy_Point b):
        """Return point between two points."""
        return wrap_Point(middle_point(deref(a.thisptr), deref(b.thisptr)))

    @classmethod
    def rot90(cls, cy_Point a):
        """Rotate point by 90 degrees."""
        return wrap_Point(rot90(deref(a.thisptr)))

    @classmethod
    def lerp(cls, double t, cy_Point a, cy_Point b):
        """Linearly interpolate between too points."""
        return wrap_Point(lerp(t, deref(a.thisptr), deref(b.thisptr)))

    @classmethod
    def dot(cls, cy_Point a, cy_Point b):
        """Calculate dot product of two points."""
        return dot(deref(a.thisptr), deref(b.thisptr))

    @classmethod
    def cross(cls, cy_Point a, cy_Point b):
        """Calculate (z-coordinate of) cross product of two points."""
        return cross(deref(a.thisptr), deref(b.thisptr))

    @classmethod
    def distance(cls, cy_Point a, cy_Point b):
        """Compute distance between two points."""
        return distance(deref(a.thisptr), deref(b.thisptr))

    @classmethod
    def distanceSq(cls, cy_Point a, cy_Point b):
        """Compute square of distance between two points."""
        return distanceSq(deref(a.thisptr), deref(b.thisptr))

    @classmethod
    def unit_vector(cls, cy_Point p):
        """Normalise point."""
        return wrap_Point(unit_vector(deref(p.thisptr)))

    @classmethod
    def L1(cls, cy_Point p):
        """Compute L1 (Manhattan) norm of a point.

        L1(P) = |P.x| + |P.y|
        """
        return L1(deref(p.thisptr))

    @classmethod
    def LInfty(cls, cy_Point p):
        """Compute Infinity norm of a point.

        LInfty(P) = max(|P.x|, |P.y|)
        """
        return LInfty(deref(p.thisptr))

    @classmethod
    def is_zero(cls, cy_Point p):
        """Test whether point is origin."""
        return is_zero(deref(p.thisptr))

    @classmethod
    def is_unit_vector(cls, cy_Point p):
        """Test whether point's length equal 1."""
        return is_unit_vector(deref(p.thisptr))

    @classmethod
    def atan2(cls, cy_Point p):
        """Return angle between point and x-axis."""
        return atan2(deref(p.thisptr))

    @classmethod
    def angle_between(cls, cy_Point a, cy_Point b):
        """Return angle between two point."""
        return angle_between(deref(a.thisptr), deref(b.thisptr))

    @classmethod
    def abs(cls, cy_Point p):
        """Return length of a point."""
        return wrap_Point(abs(deref(p.thisptr)))

    @classmethod
    def constrain_angle(cls, cy_Point a, cy_Point b, unsigned int n, cy_Point direction):
        """Rotate B around A to have specified angle wrt. direction."""
        return wrap_Point(constrain_angle(deref(a.thisptr), deref(b.thisptr), n, deref(direction.thisptr)))

cdef cy_Point wrap_Point(Point p):
    cdef Point * retp = new Point()
    retp[0] = p
    cdef cy_Point r = cy_Point.__new__(cy_Point)
    r.thisptr = retp
    return r

cdef object wrap_vector_point(vector[Point] v):
    r = []
    cdef unsigned int i
    for i in range(v.size()):
        r.append( wrap_Point(v[i]) )
    return r

cdef vector[Point] make_vector_point(object l):
    cdef vector[Point] ret
    for i in l:
        ret.push_back( deref( (<cy_Point> i).thisptr ) )
    return ret


cdef class cy_IntPoint:

    """Represents point with integer coordinates

    IntPoints are ordered lexicographically, with y coordinate being
    more significant.

    Corresponds to IntPoint class in 2geom. Most members are direct
    calls to IntPoint methods, otherwise C++ call is specified.
    """

    def __init__(self, IntCoord x = 0, IntCoord y = 0):
        """Create new IntPoint from it's cartesian coordinates."""
        self.thisptr =  new IntPoint(x ,y)

    def __getitem__(self, key):
        """Get coordinates of IntPoint."""
        return deref(self.thisptr)[key]

    def __repr__(self):
        """repr(self)"""
        return "IntPoint ({0}, {1})".format(self[0], self[1])

    def __str__(self):
        """str(self)"""
        return "[{0}, {1}]".format(self[0], self[1])

    def __dealloc__(self):
        del self.thisptr

    @property
    def x(self):
        """First coordinate of IntPoint."""
        return self.thisptr.x()

    @property
    def y(self):
        """Second coordinate of IntPoint."""
        return self.thisptr.y()

    def __add__(cy_IntPoint self, cy_IntPoint o):
        """P + Q"""
        return wrap_IntPoint(deref(self.thisptr)+deref( o.thisptr ))

    def __sub__(cy_IntPoint self, cy_IntPoint o):
        """P - Q"""
        return wrap_IntPoint(deref(self.thisptr)-deref( o.thisptr ))

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


cdef class cy_Line:

    """Class representing line in plane.

    Corresponds to Line class in 2geom. Most members are direct
    calls to Line methods, otherwise C++ call is specified.
    """

    cdef Line* thisptr

    def __cinit__(self, cy_Point cp = None, double x = 0):
        """Create Line from point and angle to x-axis.

        Constructor with no arguments calls Line() in 2geom, otherwise
        Line(Point &, double) is called.
        """
        if cp is None:
            self.thisptr = new Line()
        else:
            self.thisptr = new Line( deref(cp.thisptr), x)

    def __repr__(self):
        """repr(self)."""
        return "Line({0}, {1:3f})".format(repr(self.origin()), self.angle())

    def __str__(self):
        """str(self)"""
        return repr(self)

    def __dealloc__(self):
        del self.thisptr

    @classmethod
    def from_points(cls, cy_Point cp, cy_Point cq):
        """Create Line passing through two points.

        Calls Line(Point &, Point &) in 2geom.
        """
        return wrap_Line( Line( deref(cp.thisptr), deref(cq.thisptr) ) )

    @classmethod
    def from_origin_and_versor(cls, cy_Point o, cy_Point v):
        """Create Line passing through point with specified versor."""
        return wrap_Line( from_origin_and_versor(deref(o.thisptr), deref(v.thisptr)) )

    @classmethod
    def from_normal_distance(cls, cy_Point normal, double dist):
        """Create Line from it's normal and distance from origin."""
        return wrap_Line( from_normal_distance( deref(normal.thisptr), dist ) )

    @classmethod
    def from_LineSegment(cls, cy_LineSegment LS):
        """Create Line from LineSegment.

        Calls Line(LineSegment &) in 2geom.
        """
        return wrap_Line( Line( deref(<LineSegment *> LS.thisptr) ) )

    @classmethod
    def from_Ray(cls, cy_Ray R):
        """Create Line from Ray.

        Calls Line(Ray &) in 2geom.
        """
        return wrap_Line( Line( deref(R.thisptr) ) )

    #maybe implement as properties.

    def origin(self):
        """Return origin of line."""
        return wrap_Point(self.thisptr.origin())

    def versor(self):
        """Return versor of line."""
        return wrap_Point(self.thisptr.versor())

    def angle(self):
        """Return angle between line and x-axis."""
        return self.thisptr.angle()

    def set_origin(self, cy_Point origin):
        """Set origin."""
        self.thisptr.setOrigin( deref(origin.thisptr) )

    def set_versor(self, cy_Point versor):
        """Set versor."""
        self.thisptr.setVersor( deref(versor.thisptr) )

    def set_angle(self, Coord a):
        """Set angle."""
        self.thisptr.setAngle(a)

    def set_points(self, cy_Point cp, cy_Point cq):
        """Set two points line passes through."""
        self.thisptr.setPoints( deref(cp.thisptr), deref(cq.thisptr) )

    def set_coefficients(self, a, b, c):
        """Set coefficients in parametric equation of line."""
        self.thisptr.setCoefficients(a, b, c)

    def is_degenerate(self):
        """Test whether line's versor is zero vector."""
        return self.thisptr.isDegenerate()

    def point_at(self, t):
        """origin + t*versor"""
        return wrap_Point(self.thisptr.pointAt(t))

    def value_at(self, t, Dim2 d):
        """Coordinates of point_at(t)."""
        return self.thisptr.valueAt(t, d)

    def time_at(self, cy_Point cp):
        """Find time value corresponding to point on line."""
        return self.thisptr.timeAt( deref(cp.thisptr) )

    def time_at_projection(self, cy_Point cp):
        """Find time value corresponding to orthogonal projection of point."""
        return self.thisptr.timeAtProjection( deref(cp.thisptr) )

    def nearest_time(self, cy_Point cp):
        """Alias for time_at_projection."""
        return self.thisptr.nearestTime( deref(cp.thisptr) )

    def roots(self, Coord v, Dim2 d):
        """Return time values where self.value_at(t, dim) == v."""
        return wrap_vector_double( self.thisptr.roots(v, d) )

    def reverse(self):
        """Reverse line."""
        return wrap_Line( self.thisptr.reverse() )

    def derivative(self):
        """Take line's derivative."""
        return wrap_Line( self.thisptr.derivative() )

    def normal(self):
        """Return line's normal."""
        return wrap_Point( self.thisptr.normal() )

    def normal_and_dist(self):
        """return tuple containing normal vector and distance from origin.

        Calls normal_and_dist(x) and return it's result and x as a tuple.
        """
        cdef double x = 0
        cdef Point p = self.thisptr.normalAndDist(x)
        return (wrap_Point(p), x)

    def portion(self, Coord f, Coord t):
        """Return Curve corresponding to portion of line."""
        return wrap_Curve_p( self.thisptr.portion(f, t) )

    def ray(self, Coord t):
        """Return Ray continuing from time value t."""
        return wrap_Ray( self.thisptr.ray(t) )

    def segment(self, Coord f, Coord t):
        """Return LineSegment corresponding to portion of line."""
        return wrap_LineSegment( self.thisptr.segment(f, t) )

    def transformed(self, t):
        """Return line transformed by transform."""
        #doing this because transformed(t) takes reference
        cdef Affine at
        if is_transform(t):
            at = get_Affine(t)
            return wrap_Line(self.thisptr.transformed( at ))

    @classmethod
    def distance(cls, cy_Point cp, cy_Line cl):
        """Calculate distance between point and line."""
        return distance( deref(cp.thisptr), deref(cl.thisptr))

    @classmethod
    def are_near(cls, cy_Point cp, cy_Line cl, double eps=EPSILON):
        """Test if point is near line."""
        return are_near( deref(cp.thisptr), deref(cl.thisptr), eps)

    @classmethod
    def are_parallel(cls, cy_Line cl, cy_Line ck, eps=EPSILON):
        """Test if lines are almost parallel."""
        return are_parallel( deref(cl.thisptr), deref(ck.thisptr), eps)

    @classmethod
    def are_same(cls, cy_Line cl, cy_Line ck, double eps=EPSILON):
        """Test if lines represent the same line."""
        return are_same( deref(cl.thisptr), deref(ck.thisptr), eps)

    @classmethod
    def are_orthogonal(cls, cy_Line cl, cy_Line ck, eps=EPSILON):
        """Test two lines for orthogonality."""
        return are_orthogonal( deref(cl.thisptr), deref(ck.thisptr), eps)

    @classmethod
    def are_collinear(cls, cy_Point cp, cy_Point cq, cy_Point cr, eps=EPSILON):
        """Test for collinearity of vectors (cq-cp) and (cr-cp)"""
        return are_collinear( deref(cp.thisptr), deref(cq.thisptr), deref(cr.thisptr), eps)

    @classmethod
    def angle_between(cls, cy_Line cl, cy_Line ck):
        """Calculate angle between two lines"""
        return angle_between( deref(cl.thisptr), deref(ck.thisptr) )

cdef cy_Line wrap_Line(Line p):
    cdef Line * retp = new Line()
    retp[0] = p
    cdef cy_Line r = cy_Line.__new__(cy_Line)
    r.thisptr = retp
    return r

#-- Ray --

cdef class cy_Ray:

    """Ray represents half of line, starting at origin and going to
    infinity.

    Corresponds to Ray class in 2geom. Most members are direct
    calls to Ray methods, otherwise C++ call is specified.
    """

    cdef Ray* thisptr

    def __cinit__(self, cy_Point cp = None, double x = 0):
        """Create Ray from origin and angle with x-axis.

        Empty constructor calls Ray() in 2geom.
        """
        if cp is None:
            self.thisptr = new Ray()
        else:
            self.thisptr = new Ray( deref(cp.thisptr), x)

    def __repr__(self):
        """repr(self)."""
        return "Ray({0}, {1:3f})".format(repr(self.origin()), self.angle())

    def __str__(self):
        """str(self)"""
        return repr(self)

    def __dealloc__(self):
        del self.thisptr

    @classmethod
    def from_points(cls, cy_Point cp, cy_Point cq):
        """Create ray passing through two points, starting at first one."""
        return wrap_Ray( (Ray( deref(cp.thisptr), deref(cq.thisptr) )) )

    def origin(self):
        """Return origin."""
        return wrap_Point(self.thisptr.origin())

    def versor(self):
        """Return versor."""
        return wrap_Point(self.thisptr.versor())

    def angle(self):
        """Return angle between ray and x-axis."""
        return self.thisptr.angle()

    def set_origin(self, cy_Point cp):
        """Set origin."""
        self.thisptr.setOrigin( deref(cp.thisptr) )

    def set_versor(self, cy_Point cp):
        """Set versor."""
        self.thisptr.setVersor( deref(cp.thisptr) )

    def set_angle(self, Coord a):
        """Set angle."""
        self.thisptr.setAngle(a)

    def set_points(self, cy_Point cp, cy_Point cq):
        """Set origin and second point of ray."""
        self.thisptr.setPoints( deref(cp.thisptr), deref(cq.thisptr) )

    def is_degenerate(self):
        """Check for zero versor."""
        return self.thisptr.isDegenerate()

    def point_at(self, t):
        """origin + t * versor"""
        return wrap_Point(self.thisptr.pointAt(t))
    def value_at(self, t, Dim2 d):
        """Access coordinates of point_at(t)."""
        return self.thisptr.valueAt(t, d)

    def nearest_time(self, cy_Point cp):
        """Get time value of nearest point of ray."""
        return self.thisptr.nearestTime( deref(cp.thisptr) )
    def reverse(self):
        """Reverse the ray."""
        return wrap_Ray( self.thisptr.reverse() )

    def roots(self, Coord v, Dim2 d):
        """Return time values for which self.value_at(t, d) == v."""
        return wrap_vector_double( self.thisptr.roots(v, d) )

    def transformed(self, t):
        """Return ray transformed by affine transform."""
        cdef Affine at
        if is_transform(t):
            at = get_Affine(t)
            return wrap_Ray(self.thisptr.transformed( at ))

    def portion(self, Coord f, Coord t):
        """Return Curve corresponding to portion of ray."""
        return wrap_Curve_p( self.thisptr.portion(f, t) )

    def segment(self, Coord f, Coord t):
        """Return LineSegment corresponding to portion of ray."""
        return wrap_LineSegment( self.thisptr.segment(f, t) )

    @classmethod
    def distance(cls, cy_Point cp, cy_Ray cl):
        """Compute distance between point and ray."""
        return distance( deref(cp.thisptr), deref(cl.thisptr))

    @classmethod
    def are_near(cls, cy_Point cp, cy_Ray cl, double eps=EPSILON):
        """Check if distance between point and ray is small."""
        return are_near( deref(cp.thisptr), deref(cl.thisptr), eps)

    @classmethod
    def are_same(cls, cy_Ray cl, cy_Ray ck, double eps=EPSILON):
        """Check if two ray are same."""
        return are_same( deref(cl.thisptr), deref(ck.thisptr), eps)

    @classmethod
    def angle_between(cls, cy_Ray cl, cy_Ray ck, bint cw=True):
        """Compute angle between two rays.

        Can specify direction using parameter cw.
        """
        return angle_between( deref(cl.thisptr), deref(ck.thisptr), cw)

    @classmethod
    def make_angle_bisector_ray(cls, cy_Ray cl, cy_Ray ck):
        """Make ray bisecting smaller angle formed by two rays."""
        return wrap_Ray( make_angle_bisector_ray(deref(cl.thisptr), deref(ck.thisptr) ))

cdef cy_Ray wrap_Ray(Ray p):
    cdef Ray * retp = new Ray()
    retp[0] = p
    cdef cy_Ray r = cy_Ray.__new__(cy_Ray)
    r.thisptr = retp
    return r
