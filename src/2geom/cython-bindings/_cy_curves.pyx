from numbers import Number

from cython.operator cimport dereference as deref

from _cy_rectangle cimport cy_OptInterval, wrap_OptInterval, wrap_Rect, OptRect, wrap_OptRect
from _cy_rectangle cimport cy_Interval, wrap_Interval

from _cy_affine cimport cy_Translate, cy_Rotate, cy_Scale
from _cy_affine cimport cy_VShear, cy_HShear, cy_Zoom
from _cy_affine cimport cy_Affine, wrap_Affine, get_Affine, is_transform


cdef class cy_Curve:

    """Class representing generic curve.

    Curve maps unit interval to real plane. All curves should implement
    these methods.

    This class corresponds to Curve class in 2geom. It's children in cython
    aren't actually derived from it, it would make code more unreadable.
    """

    def __cinit__(self):
        """Create new Curve.

        You shouldn't create Curve this way, it usually wraps existing
        curves (f. e. in Path).
        """
        self.thisptr = <Curve *> new SBasisCurve(D2[SBasis]( SBasis(0), SBasis(0) ))

    def __call__(self, Coord t):
        """Get point at time value t."""
        return wrap_Point( deref(self.thisptr)(t) )

    def initial_point(self):
        """Get self(0)."""
        return wrap_Point(self.thisptr.initialPoint())

    def final_point(self):
        """Get self(1)."""
        return wrap_Point(self.thisptr.finalPoint())
    def is_degenerate(self):
        """Curve is degenerate if it's length is zero."""
        return self.thisptr.isDegenerate()

    def point_at(self, Coord t):
        """Equivalent to self(t)."""
        return wrap_Point(self.thisptr.pointAt(t))

    def value_at(self, Coord t, Dim2 d):
        """Equivalent to self(t)[d]."""
        return self.thisptr.valueAt(t, d)

    def point_and_derivatives(self, Coord t, unsigned int n):
        """Return point and at least first n derivatives at point t in list."""
        return wrap_vector_point(self.thisptr.pointAndDerivatives(t, n))

    def set_initial(self, cy_Point v):
        """Set initial point of curve."""
        self.thisptr.setInitial(deref( v.thisptr ))

    def set_final(self, cy_Point v):
        """Set final point of curve."""
        self.thisptr.setFinal(deref( v.thisptr ))

    def bounds_fast(self):
        """Return bounding rectangle for curve.

        This method is fast, but does not guarantee to give smallest
        rectangle.
        """
        return wrap_Rect(self.thisptr.boundsFast())

    def bounds_exact(self):
        """Return exact bounding rectangle for curve.

        This may take a while.
        """
        return wrap_Rect(self.thisptr.boundsExact())

    def bounds_local(self, cy_OptInterval i, unsigned int deg=0):
        """Return bounding rectangle to portion of curve."""
        return wrap_OptRect(self.thisptr.boundsLocal(deref( i.thisptr ), deg))

    #TODO rewrite all duplicates to copy."""
    def duplicate(self):
        """Duplicate the curve."""
        return wrap_Curve_p( self.thisptr.duplicate() )

    def transformed(self, m):
        """Transform curve by affine transform."""
        cdef Affine at
        if is_transform(m):
            at = get_Affine(m)
            return wrap_Curve_p( self.thisptr.transformed(at) )

    def portion(self, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return portion of curve, specified by endpoints or interval."""
        if interval is None:
            return wrap_Curve_p( self.thisptr.portion(deref( interval.thisptr )) )
        else:
            return wrap_Curve_p( self.thisptr.portion(fr, to) )

    def reverse(self):
        """Return curve with reversed time."""
        return wrap_Curve_p( self.thisptr.reverse() )

    def derivative(self):
        """Return curve's derivative."""
        return wrap_Curve_p( self.thisptr.derivative() )

    def nearest_point(self, cy_Point p, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return such t that |self(t) - point| is minimized."""
        if interval is None:
            return self.thisptr.nearestTime(deref( p.thisptr ), fr, to)
        else:
            return self.thisptr.nearestTime(deref( p.thisptr ), deref( interval.thisptr ))

    def all_nearest_points(self, cy_Point p, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return all values of t that |self(t) - point| is minimized."""
        if interval is None:
            return wrap_vector_double(self.thisptr.allNearestTimes(deref( p.thisptr ), fr, to))
        else:
            return wrap_vector_double(self.thisptr.allNearestTimes(deref( p.thisptr ),
                                                                    deref( interval.thisptr )))

    def length(self, Coord tolerance):
        """Return length of curve, within give tolerance."""
        return self.thisptr.length(tolerance)

    def roots(self, Coord v, Dim2 d):
        """Find time values where self(t)[d] == v."""
        return wrap_vector_double(self.thisptr.roots(v, d))

    def winding(self, cy_Point p):
        """Return winding number around specified point."""
        return self.thisptr.winding(deref( p.thisptr ))

    def unit_tangent_at(self, Coord t, unsigned int n):
        """Return tangent at self(t).

        Parameter n specifies how many derivatives to take into account."""
        return wrap_Point(self.thisptr.unitTangentAt(t, n))

    def to_SBasis(self):
        """Return tuple of SBasis functions."""
        cdef D2[SBasis] ret = self.thisptr.toSBasis()
        return ( wrap_SBasis(ret[0]), wrap_SBasis(ret[1]) )

    def degrees_of_freedom(self):
        """Return number of independent parameters needed to specify the curve."""
        return self.thisptr.degreesOfFreedom()
#~     def operator==(self, cy_Curve c):
#~         return deref( self.thisptr ) == deref( c.thisptr )

#~ cdef cy_Curve wrap_Curve(Curve & p):
#~     cdef Curve * retp = <Curve *> new SBasisCurve(D2[SBasis]( SBasis(), SBasis() ))
#~     retp[0] = p
#~     cdef cy_Curve r = cy_Curve.__new__(cy_Curve)
#~     r.thisptr = retp
#~     return r

cdef cy_Curve wrap_Curve_p(Curve * p):
    cdef cy_Curve r = cy_Curve.__new__(cy_Curve)
    r.thisptr = p
    return r

cdef class cy_Linear:
    """Function mapping linearly between two values.

    Corresponds to Linear class in 2geom.
    """

    cdef Linear* thisptr

    def __cinit__(self, aa = None, b = None):
        """Create new Linear from two end values.

        No arguments create zero constant, one value creats constant.
        """
        if aa is None:
            self.thisptr = new Linear()
        elif b is None:
            self.thisptr = new Linear(float(aa))
        else:
            self.thisptr = new Linear(float(aa), float(b))

    def __dealloc__(self):
        del self.thisptr

    def __call__(self, Coord t):
        """Get value at time value t."""
        return deref(self.thisptr)(t)

    def __getitem__(self, i):
        """Get end values."""
        return deref( self.thisptr ) [i]

    def __richcmp__(cy_Linear self, cy_Linear other, int op):
        if op == 2:
            return deref(self.thisptr) == deref(other.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(other.thisptr)


    def __neg__(cy_Linear self):
        """Negate all values of self."""
        return wrap_Linear( L_neg(deref(self.thisptr)) )


    def __add__(cy_Linear self, other):
        """Add number or other linear."""
        if isinstance(other, Number):
            return wrap_Linear( deref(self.thisptr) + float(other) )
        elif isinstance(other, cy_Linear):
            return wrap_Linear( deref(self.thisptr) + deref( (<cy_Linear> other).thisptr ) )

    def __sub__(cy_Linear self, other):
        """Substract number or other linear."""
        if isinstance(other, Number):
            return wrap_Linear( L_sub_Ld(deref(self.thisptr), float(other)) )
        elif isinstance(other, cy_Linear):
            return wrap_Linear( L_sub_LL(deref(self.thisptr), deref( (<cy_Linear> other).thisptr )) )


    def __mul__(cy_Linear self, double b):
        """Multiply linear by number."""
        return wrap_Linear(deref( self.thisptr ) * b)

    def __div__(cy_Linear self, double b):
        """Divide linear by value."""
        return wrap_Linear(deref( self.thisptr ) / b)

    def is_zero(self, double eps = EPSILON):
        """Test whether linear is zero within given tolerance."""
        return self.thisptr.isZero(eps)

    def is_constant(self, double eps = EPSILON):
        """Test whether linear is constant within given tolerance."""
        return self.thisptr.isConstant(eps)

    def is_finite(self):
        """Test whether linear is finite."""
        return self.thisptr.isFinite()

    def at0(self):
        """Equivalent to self(0)."""
        return self.thisptr.at0()

    def at1(self):
        """Equivalent to self(1)."""
        return self.thisptr.at1()

    def value_at(self, double t):
        """Equivalent to self(t)."""
        return self.thisptr.valueAt(t)

    def to_SBasis(self):
        """Convert to SBasis."""
        return wrap_SBasis(self.thisptr.toSBasis())

    def bounds_exact(self):
        """Return exact bounding interval

        This may take a while.
        """
        return wrap_OptInterval(self.thisptr.bounds_exact())

    def bounds_fast(self):
        """Return bounding interval

        This method is fast, but does not guarantee to give smallest
        interval.
        """
        return wrap_OptInterval(self.thisptr.bounds_fast())

    def bounds_local(self, double u, double v):
        """Return bounding interval to the portion of Linear."""
        return wrap_OptInterval(self.thisptr.bounds_local(u, v))

    def tri(self):
        """Return difference between end values."""
        return self.thisptr.tri()

    def hat(self):
        """Return value at (0.5)."""
        return self.thisptr.hat()

    @classmethod
    def sin(cls, cy_Linear bo, int k):
        """Return sine of linear."""
        return wrap_SBasis(sin(deref( bo.thisptr ), k))

    @classmethod
    def cos(cls, cy_Linear bo, int k):
        """Return cosine of linear."""
        return wrap_SBasis(cos(deref( bo.thisptr ), k))

    @classmethod
    def reciprocal(cls, cy_Linear a, int k):
        """Return reciprocical of linear."""
        return wrap_SBasis(reciprocal(deref( a.thisptr ), k))

    @classmethod
    def shift(cls, cy_Linear a, int sh):
        """Multiply by x**sh."""
        return wrap_SBasis(shift(deref( a.thisptr ), sh))

#leave these in cy2geom napespace?
def cy_lerp(double t, double a, double b):
    return lerp(t, a, b)

cdef cy_Linear wrap_Linear(Linear p):
    cdef Linear * retp = new Linear()
    retp[0] = p
    cdef cy_Linear r = cy_Linear.__new__(cy_Linear)
    r.thisptr = retp
    return r

cdef vector[Linear] make_vector_linear(object l):
    cdef vector[Linear] ret
    for i in l:
        ret.push_back( deref( (<cy_Linear> i).thisptr ) )
    return ret


cdef class cy_SBasis:

    """Class representing SBasis polynomial.

    Corresponds to SBasis class in 2geom."""

    def __cinit__(self, a=None, b=None):
        """Create new SBasis.

        This constructor only creates linear SBasis, specifying endpoints.
        """
        if a is None:
            self.thisptr = new SBasis()
        elif b is None:
                self.thisptr = new SBasis( float(a) )
        else:
            self.thisptr = new SBasis( float(a), float(b) )

    def __dealloc__(self):
        del self.thisptr

    @classmethod
    def from_linear(cls, cy_Linear l):
        """Create SBasis from Linear."""
        return wrap_SBasis( SBasis(deref( l.thisptr )) )

    @classmethod
    def from_linears(cls, lst):
        """Create SBasis from list of Linears."""
        return wrap_SBasis( SBasis( make_vector_linear(lst) ) )

    def size(self):
        """Return number of linears SBasis consists of."""
        return self.thisptr.size()

    def __call__(self, o):
        """Get point at time value t."""
        if isinstance(o, Number):
            return deref(self.thisptr)(float(o))
        elif isinstance(self, cy_SBasis):
            return wrap_SBasis(deref(self.thisptr)( deref( (<cy_SBasis> o).thisptr ) ))

    def __getitem__(self, unsigned int i):
        """Get Linear at i th position."""
        if i>=self.size():
            raise IndexError
        else:
            return wrap_Linear(deref( self.thisptr ) [i])

    def __neg__(self):
        """Return SBasis with negated values."""
        return wrap_SBasis( SB_neg(deref(self.thisptr)) )

    #cython doesn't use __rmul__, it switches the arguments instead
    def __add__(cy_SBasis self, other):
        """Add number or other SBasis to SBasis."""
        if isinstance(other, Number):
            return wrap_SBasis( deref(self.thisptr) + float(other) )
        elif isinstance(other, cy_SBasis):
            return wrap_SBasis( deref(self.thisptr) + deref( (<cy_SBasis> other).thisptr ) )

    def __sub__(cy_SBasis self, other):
        """Substract number or other SBasis from SBasis."""
        if isinstance(other, Number):
            return wrap_SBasis( SB_sub_Sd(deref(self.thisptr), float(other) ) )
        elif isinstance(other, cy_SBasis):
            return wrap_SBasis( SB_sub_SS(deref(self.thisptr), deref( (<cy_SBasis> other).thisptr ) ) )

    def __mul__(self, other):
        """Multiply SBasis by number or other SBasis."""
        if isinstance(other, Number):
            return wrap_SBasis( deref( (<cy_SBasis> self).thisptr ) * float(other) )
        elif isinstance(other, cy_SBasis):
            if isinstance(self, cy_SBasis):
                return wrap_SBasis( deref( (<cy_SBasis> self).thisptr ) * deref( (<cy_SBasis> other).thisptr ) )
            elif isinstance(self, Number):
                return wrap_SBasis( float(self) * deref( (<cy_SBasis> other).thisptr ) )

    def __div__(cy_SBasis self, double other):
        """Divide SBasis by number."""
        return wrap_SBasis( deref(self.thisptr)/other )


    def empty(self):
        """Test whether SBasis has no linears."""
        return self.thisptr.empty()

    def back(self):
        """Return last linear in SBasis."""
        return wrap_Linear(self.thisptr.back())

    def pop_back(self):
        """Remove last linear in SBasis."""
        self.thisptr.pop_back()

    def resize(self, unsigned int n, cy_Linear l = None):
        """Resize SBasis, optionally filling created slots with linear."""
        if l is None:
            self.thisptr.resize(n)
        else:
            self.thisptr.resize(n, deref( l.thisptr ))

#~     def reserve(self, unsigned int n):
#~         self.thisptr.reserve(n)

    def clear(self):
        """Make SBasis empty."""
        self.thisptr.clear()
#~     def insert(self, cy_::__gnu_cxx::__normal_iterator<Geom::Linear*, std::vector<Geom::Linear, std::allocator<Geom::Linear> > > before, cy_::__gnu_cxx::__normal_iterator<Geom::Linear const*, std::vector<Geom::Linear, std::allocator<Geom::Linear> > > src_begin, cy_::__gnu_cxx::__normal_iterator<Geom::Linear const*, std::vector<Geom::Linear, std::allocator<Geom::Linear> > > src_end):
#~         self.thisptr.insert(deref( before.thisptr ), deref( src_begin.thisptr ), deref( src_end.thisptr ))

    def at(self, unsigned int i):
        """Equivalent to self[i]."""
        return wrap_Linear(self.thisptr.at(i))

    def __richcmp__(cy_SBasis self, cy_SBasis B, int op):
        if op == 2:
            return deref( self.thisptr ) == deref( B.thisptr )
        elif op == 3:
            return deref( self.thisptr ) != deref( B.thisptr )

    def is_zero(self, double eps = EPSILON):
        """Test whether linear is zero within given tolerance."""
        return self.thisptr.isZero(eps)

    def is_constant(self, double eps = EPSILON):
        """Test whether linear is constant within given tolerance."""
        return self.thisptr.isConstant(eps)

    def is_finite(self):
        """Test whether linear is finite."""
        return self.thisptr.isFinite()

    def at0(self):
        """Equivalent to self(0)."""
        return self.thisptr.at0()

    def at1(self):
        """Equivalent to self(1)."""
        return self.thisptr.at1()

    def degrees_of_freedom(self):
        """Return number of independent parameters needed to specify the curve."""
        return self.thisptr.degreesOfFreedom()

    def value_at(self, double t):
        """Equivalent to self(t)[d]."""
        return self.thisptr.valueAt(t)

    def value_and_derivatives(self, double t, unsigned int n):
        """Return value and at least n derivatives at time t."""
        return wrap_vector_double (self.thisptr.valueAndDerivatives(t, n))

    def to_SBasis(self):
        """Just return self."""
        return wrap_SBasis(self.thisptr.toSBasis())

    def tail_error(self, unsigned int tail):
        """Return largest error after truncating linears from tail."""
        return self.thisptr.tailError(tail)

    def normalize(self):
        """Remove zero linears at the end."""
        self.thisptr.normalize()

    def truncate(self, unsigned int k):
        """Truncate SBasis to have k elements."""
        self.thisptr.truncate(k)

    @classmethod
    def sqrt(cls, cy_SBasis a, int k):
        """Return square root of SBasis.

        Use k to specify degree of resulting SBasis.
        """
        return wrap_SBasis(sqrt(deref( a.thisptr ), k))

    @classmethod
    def inverse(cls, cy_SBasis a, int k):
        """Return inverse function to SBasis.

        Passed SBasis must be function [1-1] -> [1-1] bijection.
        """
        return wrap_SBasis(inverse(deref( a.thisptr ), k))

    @classmethod
    def valuation(cls, cy_SBasis a, double tol = 0):
        """Return the degree of the first non zero coefficient."""
        return valuation(deref( a.thisptr ), tol)

    #call with level_set(SBasis(1, 5), 2, a = 0.2, b = 0.4, tol = 0.02)
    @classmethod
    def level_set(cls, cy_SBasis f, level, a = 0, b = 1, tol = 1e-5, vtol = 1e-5):
        """Return intervals where SBasis is in specified level.

        Specify range and tolerance in other arguments.
        """
        if isinstance(level, cy_Interval):
            return wrap_vector_interval(level_set(deref( f.thisptr ), deref( (<cy_Interval> level).thisptr ), a, b, tol)) #a, b, tol
        else:
            return wrap_vector_interval(level_set(deref( f.thisptr ), float(level), vtol, a, b, tol)) #vtol, a, b, tol

    @classmethod
    def shift(cls, cy_SBasis a, int sh):
        """Multiply by x**sh."""
        return wrap_SBasis(shift(deref( a.thisptr ), sh))

    @classmethod
    def compose(cls, cy_SBasis a, cy_SBasis b, k = None):
        """Compose two SBasis.

        Specify order of resulting SBasis by parameter k.
        """
        if k is None:
            return wrap_SBasis(compose(deref( a.thisptr ), deref( b.thisptr )))
        else:
            return wrap_SBasis(compose(deref( a.thisptr ), deref( b.thisptr ), int(k)))

    @classmethod
    def roots(cls, cy_SBasis s, cy_Interval inside = None):
        """Return time values where self equals 0.

        inside intervals specifies subset of domain.
        """
        if inside is None:
            return wrap_vector_double(roots(deref( s.thisptr )))
        else:
            return wrap_vector_double(roots(deref( s.thisptr ), deref( inside.thisptr )))

    @classmethod
    def multi_roots(cls, cy_SBasis f, levels, double htol = 1e-7, double vtol = 1e-7, double a = 0, double b = 1):
        """Return lists of roots for different levels."""
        cdef vector[double] l = make_vector_double(levels)
        cdef vector[ vector[double] ] r = multi_roots(deref( f.thisptr ), l, htol, vtol, a, b)
        lst = []
        for i in range(r.size()):
            lst.append( wrap_vector_double(r[i]) )
        return lst

    @classmethod
    def multiply_add(cls, cy_SBasis a, cy_SBasis b, cy_SBasis c):
        """Return a*b+c."""
        return wrap_SBasis(multiply_add(deref( a.thisptr ), deref( b.thisptr ), deref( c.thisptr )))

    @classmethod
    def divide(cls, cy_SBasis a, cy_SBasis b, int k):
        """Divide two SBasis functions.

        Use k to specify degree of resulting SBasis.
        """
        return wrap_SBasis(divide(deref( a.thisptr ), deref( b.thisptr ), k))

    @classmethod
    def compose_inverse(cls, cy_SBasis f, cy_SBasis g, unsigned int order, double tol):
        """Compose f with g's inverse.

        Requires g to be bijection g: [0, 1] -> [0, 1]
        """
        return wrap_SBasis(compose_inverse(deref( f.thisptr ), deref( g.thisptr ), order, tol))

    @classmethod
    def multiply(cls, cy_SBasis a, cy_SBasis b):
        """Multiply two SBasis functions."""
        return wrap_SBasis(multiply(deref( (<cy_SBasis> a).thisptr ), deref( (<cy_SBasis> b).thisptr )))

    @classmethod
    def derivative(cls, cy_SBasis a):
        """Return derivative os SBasis."""
        return wrap_SBasis(derivative(deref( (<cy_SBasis> a).thisptr )))

    @classmethod
    def integral(cls, a):
        """Return integral of SBasis."""
        return wrap_SBasis(integral(deref( (<cy_SBasis> a).thisptr )))

    @classmethod
    def portion(cls, cy_SBasis a, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return portion of SBasis, specified by endpoints or interval."""
        if interval is None:
            return wrap_SBasis( portion( deref( a.thisptr ), fr, to ) )
        else:
            return wrap_SBasis( portion( deref( a.thisptr ), deref( interval.thisptr ) ) )

    @classmethod
    def bounds_fast(cls, cy_SBasis a, int order = 0):
        """Return bounding interval

        This method is fast, but does not guarantee to give smallest
        interval.
        """
        return wrap_OptInterval(bounds_fast(deref( a.thisptr ), order))

    @classmethod
    def bounds_exact(cls, cy_SBasis a):
        """Return exact bounding interval

        This may take a while.
        """
        return wrap_OptInterval(bounds_exact(deref( a.thisptr )))

    @classmethod
    def bounds_local(cls, cy_SBasis a, cy_OptInterval t, int order = 0):
        """Return bounding interval to the portion of SBasis."""
        return wrap_OptInterval(bounds_local(deref( a.thisptr ), deref( t.thisptr ), order))

#~ def cy_level_sets(cy_SBasis f, vector[Interval] levels, double a, double b, double tol):
#~     return wrap_::std::vector<std::vector<Geom::Interval, std::allocator<Geom::Interval> >,std::allocator<std::vector<Geom::Interval, std::allocator<Geom::Interval> > > >(level_sets(deref( f.thisptr ), deref( levels.thisptr ), a, b, tol))
#~ def cy_level_sets(cy_SBasis f, vector[vector] levels, double a, double b, double vtol, double tol):
#~     return wrap_::std::vector<std::vector<Geom::Interval, std::allocator<Geom::Interval> >,std::allocator<std::vector<Geom::Interval, std::allocator<Geom::Interval> > > >(level_sets(deref( f.thisptr ), deref( levels.thisptr ), a, b, vtol, tol))

def cy_reverse(a):
    if isinstance(a, cy_Linear):
        return wrap_Linear( reverse(deref( (<cy_Linear> a).thisptr )))
    elif isinstance(a, cy_SBasis):
        return wrap_SBasis( reverse(deref( (<cy_SBasis> a).thisptr )))
    elif isinstance(a, cy_Bezier):
        return wrap_Bezier( reverse(deref( (<cy_Bezier> a).thisptr )))

#already implemented 
#~ def cy_truncate(cy_SBasis a, unsigned int terms):
#~     return wrap_SBasis(truncate(deref( a.thisptr ), terms))

cdef cy_SBasis wrap_SBasis(SBasis p):
    cdef SBasis * retp = new SBasis()
    retp[0] = p
    cdef cy_SBasis r = cy_SBasis.__new__(cy_SBasis, 0, 0)
    r.thisptr = retp
    return r


cdef class cy_SBasisCurve:

    """Curve mapping two SBasis functions to point (s1(t), s2(t)).

    Corresponds to SBasisCurve in 2geom.
    """

    cdef SBasisCurve* thisptr

#~     def __init__(self, cy_Curve other):
#~         self.thisptr = self.thisptr.SBasisCurve(deref( other.thisptr ))

    def __cinit__(self, cy_SBasis s1, cy_SBasis s2):
        """Create new SBasisCurve from two SBasis functions."""
        self.thisptr = new SBasisCurve( D2[SBasis](
            deref( s1.thisptr ),
            deref( s2.thisptr ) ) )

    def __dealloc__(self):
        del self.thisptr

    def __call__(self, double t):
        """Get point at time value t."""
        return wrap_Point(deref(self.thisptr)(t))

    def duplicate(self):
        """Duplicate the curve."""
        return wrap_SBasisCurve( <SBasisCurve> deref(self.thisptr.duplicate()) )

    def initial_point(self):
        """Get self(0)."""
        return wrap_Point(self.thisptr.initialPoint())

    def final_point(self):
        """Get self(1)."""
        return wrap_Point(self.thisptr.finalPoint())

    def is_degenerate(self):
        """Curve is degenerate if it's length is zero."""
        return self.thisptr.isDegenerate()

    def point_at(self, Coord t):
        """Equivalent to self(t)."""
        return wrap_Point(self.thisptr.pointAt(t))

    def point_and_derivatives(self, Coord t, unsigned int n):
        """Return point and at least first n derivatives at point t in list."""
        return wrap_vector_point(self.thisptr.pointAndDerivatives(t, n))

    def value_at(self, Coord t, Dim2 d):
        """Equivalent to self(t)[d]."""
        return self.thisptr.valueAt(t, d)

    def set_initial(self, cy_Point v):
        """Set initial point of curve."""
        self.thisptr.setInitial(deref( v.thisptr ))

    def set_final(self, cy_Point v):
        """Set final point of curve."""
        self.thisptr.setFinal(deref( v.thisptr ))

    def bounds_fast(self):
        """Return bounding rectangle for curve.

        This method is fast, but does not guarantee to give smallest
        rectangle.
        """
        return wrap_Rect(self.thisptr.boundsFast())

    def bounds_exact(self):
        """Return exact bounding rectangle for curve.

        This may take a while.
        """
        return wrap_Rect(self.thisptr.boundsExact())

    def bounds_local(self, cy_OptInterval i, unsigned int deg):
        """Return bounding rectangle to portion of curve."""
        return wrap_OptRect(self.thisptr.boundsLocal(deref( i.thisptr ), deg))

    def roots(self, Coord v, Dim2 d):
        """Find time values where self(t)[d] == v."""
        return wrap_vector_double( self.thisptr.roots(v, d) )

    def nearest_point(self, cy_Point p, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return such t that |self(t) - point| is minimized."""
        if interval is None:
            return self.thisptr.nearestTime(deref( p.thisptr ), fr, to)
        else:
            return (<Curve *> self.thisptr).nearestTime(deref( p.thisptr ), deref( interval.thisptr ) )

    def all_nearest_points(self, cy_Point p, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return all values of t that |self(t) - point| is minimized."""
        if interval is None:
            return wrap_vector_double(self.thisptr.allNearestTimes(deref( p.thisptr ), fr, to))
        else:
            return wrap_vector_double((<Curve *> self.thisptr).allNearestTimes(deref( p.thisptr ),
                                                                                deref( interval.thisptr ) ))

    def length(self, Coord tolerance = 0.01):
        """Return length of curve, within give tolerance."""
        return self.thisptr.length(tolerance)


    def portion(self, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return portion of curve, specified by endpoints or interval."""
        if interval is None:
            return wrap_SBasisCurve( <SBasisCurve> deref(self.thisptr.portion( fr, to ) ) )
        else:
            return wrap_SBasisCurve( <SBasisCurve>
                deref( (<Curve *> self.thisptr).portion( deref( interval.thisptr ))) )

    def transformed(self, t):
        """Transform curve by affine transform."""
        cdef Affine at
        if is_transform(t):
            at = get_Affine(t)
            return wrap_SBasisCurve( <SBasisCurve> deref(self.thisptr.transformed( at )))

    def reverse(self):
        """Return curve with reversed time."""
        return wrap_SBasisCurve( <SBasisCurve> deref( (<Curve *> self.thisptr).reverse() ) )

    def derivative(self):
        """Return curve's derivative."""
        return wrap_SBasisCurve( <SBasisCurve> deref(self.thisptr.derivative()) )


    def winding(self, cy_Point p):
        """Return winding number around specified point."""
        return (<Curve *> self.thisptr).winding(deref(p.thisptr))

    def unit_tangent_at(self, Coord t, int n = 3):
        """Return tangent at self(t).

        Parameter n specifies how many derivatives to take into account."""
        return wrap_Point((<Curve *> self.thisptr).unitTangentAt(t, n))

    def to_SBasis(self):
        """Return tuple containing it's SBasis functions."""
        return wrap_D2_SBasis(self.thisptr.toSBasis())

    def degrees_of_freedom(self):
        """Return number of independent parameters needed to specify the curve."""
        return self.thisptr.degreesOfFreedom()

cdef object wrap_D2_SBasis(D2[SBasis] p):
    return ( wrap_SBasis(p[0]), wrap_SBasis(p[1]) )

cdef cy_SBasisCurve wrap_SBasisCurve(SBasisCurve p):
    cdef SBasisCurve * retp = new SBasisCurve(D2[SBasis]( SBasis(), SBasis() ))
    retp[0] = p
    cdef cy_SBasisCurve r = cy_SBasisCurve.__new__(cy_SBasisCurve, cy_SBasis(), cy_SBasis())
    r.thisptr = retp
    return r


cdef class cy_Bezier:

    """Bezier polynomial.

    Corresponds to Bezier class in 2geom.
    """

    cdef Bezier* thisptr

    def __cinit__(self, *args):
        """Create Bezier polynomial specifying it's coeffincients

        This constructor takes up to four coefficients.
        """
        if len(args) == 0:
            #new Bezier() causes segfault
            self.thisptr = new Bezier(0)
        elif len(args) == 1:
            self.thisptr = new Bezier( float(args[0]) )
        elif len(args) == 2:
            self.thisptr = new Bezier( float(args[0]), float(args[1]) )
        elif len(args) == 3:
            self.thisptr = new Bezier( float(args[0]), float(args[1]), float(args[2]) )
        elif len(args) == 4:
            self.thisptr = new Bezier( float(args[0]), float(args[1]), float(args[2]), float(args[3]) )
        else:
            raise ValueError("Passed list has too many points")

    def __dealloc__(self):
        del self.thisptr

    def __call__(self, double t):
        """Get point at time value t."""
        return deref( self.thisptr ) (t)


    def __getitem__(self, unsigned int ix):
        """Get coefficient by accessing list."""
        if ix >= self.size():
            raise IndexError
        return deref( self.thisptr ) [ix]

    def order(self):
        """Return order of Bezier."""
        return self.thisptr.order()

    def size(self):
        """Return number of coefficents."""
        return self.thisptr.size()

    def __mul__( cy_Bezier self, double v):
        """Multiply Bezier by number."""
        return wrap_Bezier(deref( self.thisptr ) * v)

    def __add__( cy_Bezier self, double v):
        """Add number to Bezier."""
        return wrap_Bezier(deref( self.thisptr ) + v)

    def __sub__( cy_Bezier self, double v):
        """Substract number from Bezier."""
        return wrap_Bezier(deref( self.thisptr ) - v)

    def __div__( cy_Bezier self, double v):
        """Divide Bezier number."""
        return wrap_Bezier(deref( self.thisptr ) / v)


    def resize(self, unsigned int n, Coord v):
        """Change order of Bezier."""
        self.thisptr.resize(n, v)

    def clear(self):
        """Create empty Bezier."""
        self.thisptr.clear()

    def degree(self):
        """Return degree of Bezier polynomial."""
        return self.thisptr.degree()

    def is_zero(self, double eps = EPSILON):
        """Test whether linear is zero within given tolerance."""
        return self.thisptr.isZero(eps)

    def is_constant(self, double eps = EPSILON):
        """Test whether linear is constant within given tolerance."""
        return self.thisptr.isConstant(eps)

    def is_finite(self):
        """Test whether linear is finite."""
        return self.thisptr.isFinite()

    def at0(self):
        """Equivalent to self(0)."""
        return self.thisptr.at0()

    def at1(self):
        """Equivalent to self(1)."""
        return self.thisptr.at1()

    def value_at(self, double t):
        """Equivalent to self(t)."""
        return self.thisptr.valueAt(t)

    def to_SBasis(self):
        """Convert to SBasis."""
        return wrap_SBasis(self.thisptr.toSBasis())

    def set_point(self, unsigned int ix, double val):
        """Set self[ix] to val."""
        self.thisptr.setPoint(ix, val)

    def value_and_derivatives(self, Coord t, unsigned int n_derivs):
        """Return value and at least n derivatives at time t."""
        return wrap_vector_double(self.thisptr.valueAndDerivatives(t, n_derivs))

    def subdivide(self, Coord t):
        """Get two beziers, from 0 to t and from t to 1."""
        cdef pair[Bezier, Bezier] p = self.thisptr.subdivide(t)
        return ( wrap_Bezier(p.first), wrap_Bezier(p.second) )

    def roots(self, cy_Interval ivl = None):
        """Find time values where self(t)[d] == v."""
        if ivl is None:
            return wrap_vector_double(self.thisptr.roots())
        else:
            return wrap_vector_double(self.thisptr.roots(deref( ivl.thisptr )))

    def forward_difference(self, unsigned int k):
#TODO: ask someone what this function does.
#~         """Compute forward difference of degree k.
#~
#~         First forward difference of B is rougly function B'(t) = B(t+h)-B(t)
#~         for fixed step h"""
        return wrap_Bezier(self.thisptr.forward_difference(k))

    def elevate_degree(self):
        """Increase degree of Bezier by 1."""
        return wrap_Bezier(self.thisptr.elevate_degree())

    def reduce_degree(self):
        """Decrease degree of Bezier by 1."""
        return wrap_Bezier(self.thisptr.reduce_degree())

    def elevate_to_degree(self, unsigned int new_degree):
        """Incerase degree of Bezier to new_degree."""
        return wrap_Bezier(self.thisptr.elevate_to_degree(new_degree))

    def deflate(self):
#TODO: ask someone what this function does.
        #It looks like integral(self)*self.size()
        return wrap_Bezier(self.thisptr.deflate())

    @classmethod
    def bezier_points(cls, cy_Bezier a, cy_Bezier b):
        """Return control points of BezierCurve consiting of two beziers.

        Passed bezier must have same degree."""
        return wrap_vector_point(bezier_points( D2[Bezier]( deref(a.thisptr), deref(b.thisptr) ) ))

    @classmethod
    def multiply(cls, cy_Bezier a, cy_Bezier b):
        """Multiply two Bezier functions."""
        return wrap_Bezier(multiply(deref( (<cy_Bezier> a).thisptr ), 
                                    deref( (<cy_Bezier> b).thisptr )))

    @classmethod
    def portion(cls, cy_Bezier a, Coord fr=0, Coord to=1, interval=None):
        """Return portion of bezier, specified by endpoints or interval."""
        if interval is None:
            return wrap_Bezier(portion(deref( a.thisptr ), fr, to))
        else:
            return wrap_Bezier(portion(deref( a.thisptr ), float(interval.min()),
                                                           float(interval.max()) ))

    @classmethod
    def derivative(cls, cy_Bezier a):
        """Return derivative of a bezier."""
        return wrap_Bezier(derivative(deref( a.thisptr )))
            
    @classmethod
    def integral(cls, cy_Bezier a):
        """Return derivative of a bezier."""
        return wrap_Bezier(integral(deref( a.thisptr )))
        
    @classmethod
    def bounds_fast(cls, cy_Bezier a):
        """Return bounding interval

        This method is fast, but does not guarantee to give smallest
        interval.
        """
        return wrap_OptInterval(bounds_fast(deref( a.thisptr )))

    @classmethod
    def bounds_exact(cls, cy_Bezier a):
        """Return exact bounding interval

        This may take a while.
        """
        return wrap_OptInterval(bounds_exact(deref( a.thisptr )))

    @classmethod
    def bounds_local(cls, cy_Bezier a, cy_OptInterval t):
        """Return bounding interval to the portion of bezier."""
        return wrap_OptInterval(bounds_local(deref( a.thisptr ), deref( t.thisptr )))

#This is the same as bz.to_SBasis()
#~ def cy_bezier_to_sbasis(cy_SBasis sb, cy_Bezier bz):
#~     bezier_to_sbasis(deref( sb.thisptr ), deref( bz.thisptr ))

#These are look like internal functions.
#~ def cy_casteljau_subdivision(Coord t, cy_Coord * v, cy_Coord * left, cy_Coord * right, unsigned int order):
#~     return subdivideArr(t, v.thisptr, left.thisptr, right.thisptr, order)
#~ def cy_bernsteinValueAt(double t, cy_double * c_, unsigned int n):
#~     return bernsteinValueAt(t, c_.thisptr, n)

cdef cy_Bezier wrap_Bezier(Bezier p):
    cdef Bezier * retp = new Bezier()
    retp[0] = p
    cdef cy_Bezier r = cy_Bezier.__new__(cy_Bezier)
    r.thisptr = retp
    return r


cdef class cy_BezierCurve:

    """Bezier curve, consisting of two Bezier functions.

    Corresponds to BezierCurve class in 2geom.
    """

    #This flag is due to this class children
    def __cinit__(self, *args, **kwargs):
        """Don't use this constructor, use create instead."""
        pass

    def __dealloc__(self):
        del self.thisptr

    def __call__(self, Coord t):
        """Get point at time value t."""
        return wrap_Point(deref( <Curve *> self.thisptr )(t))

    def __getitem__(self, unsigned int ix):
        """Get control point by list access."""
        return wrap_Point(deref( self.thisptr ) [ix])

    @classmethod
    def create(cls,  pts):
        """Create new BezierCurve from control points."""
        return wrap_BezierCurve( deref( create( make_vector_point(pts) ) ) )

    def order(self):
        """Get order of curve."""
        return self.thisptr.order()

    def control_points(self):
        """Get control points."""
        return wrap_vector_point(self.thisptr.controlPoints())

    def set_point(self, unsigned int ix, cy_Point v):
        """Set control point."""
        self.thisptr.setPoint(ix, deref( v.thisptr ))

    def set_points(self, ps):
        """Set control points"""
        self.thisptr.setPoints( make_vector_point(ps) )

    def initial_point(self):
        """Get self(0)."""
        return wrap_Point(self.thisptr.initialPoint())

    def final_point(self):
        """Get self(1)."""
        return wrap_Point(self.thisptr.finalPoint())

    def is_degenerate(self):
        """Curve is degenerate if it's length is zero."""
        return self.thisptr.isDegenerate()

    def set_initial(self, cy_Point v):
        """Set initial point of curve."""
        self.thisptr.setInitial(deref( v.thisptr ))

    def set_final(self, cy_Point v):
        """Set final point of curve."""
        self.thisptr.setFinal(deref( v.thisptr ))

    def bounds_fast(self):
        """Return bounding rectangle for curve.

        This method is fast, but does not guarantee to give smallest
        rectangle.
        """
        return wrap_Rect(self.thisptr.boundsFast())

    def bounds_exact(self):
        """Return exact bounding rectangle for curve.

        This may take a while.
        """
        return wrap_Rect(self.thisptr.boundsExact())

    def bounds_local(cy_BezierCurve self, cy_OptInterval i, unsigned int deg):
        """Return bounding rectangle to portion of curve."""
        return wrap_OptRect(self.thisptr.boundsLocal(deref( i.thisptr ), deg))

    def nearest_point(self, cy_Point p, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return such t that |self(t) - point| is minimized."""
        if interval is None:
            return (<Curve *> self.thisptr).nearestTime(deref( p.thisptr ), fr, to)
        else:
            return (<Curve *> self.thisptr).nearestTime(deref( p.thisptr ), deref( interval.thisptr ) )

    def all_nearest_points(self, cy_Point p, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return all values of t that |self(t) - point| is minimized."""
        if interval is None:
            return wrap_vector_double((<Curve *> self.thisptr).allNearestTimes(deref( p.thisptr ), fr, to))
        else:
            return wrap_vector_double((<Curve *> self.thisptr).allNearestTimes(deref( p.thisptr ),
                                                                                deref( interval.thisptr ) ))

    def portion(self, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return portion of curve, specified by endpoints or interval."""
        if interval is None:
            return wrap_BezierCurve( <BezierCurve> deref(<BezierCurve *>
                (<Curve *> self.thisptr).portion( fr, to )
                ) )
        else:
            return wrap_BezierCurve( <BezierCurve> deref(<BezierCurve *>
                (<Curve *> self.thisptr).portion(deref( interval.thisptr ))
                ) )

    def duplicate(self):
        """Duplicate the curve."""
        return wrap_BezierCurve( deref( <BezierCurve *>  self.thisptr.duplicate()))

    def reverse(self):
        """Return curve with reversed time."""
        return wrap_BezierCurve( deref( <BezierCurve *> self.thisptr.reverse()))

    def transformed(self, t):
        """Transform curve by affine transform."""
        cdef Affine at
        if is_transform(t):
            at = get_Affine(t)
            return wrap_BezierCurve( deref( <BezierCurve *> self.thisptr.transformed( at )))

    def derivative(self):
        """Return curve's derivative."""
        return wrap_BezierCurve( deref( <BezierCurve *> self.thisptr.derivative()))

    def degrees_of_freedom(self):
        """Return number of independent parameters needed to specify the curve."""
        return self.thisptr.degreesOfFreedom()

    def roots(self, Coord v, Dim2 d):
        """Find time values where self(t)[d] == v."""
        return wrap_vector_double(self.thisptr.roots(v, d))

    def length(self, Coord tolerance = 0.01):
        """Return length of curve, within give tolerance."""
        return self.thisptr.length(tolerance)

    def point_at(self, Coord t):
        """Equivalent to self(t)."""
        return wrap_Point(self.thisptr.pointAt(t))

    def point_and_derivatives(self, Coord t, unsigned int n):
        """Return point and at least first n derivatives at point t in list."""
        return wrap_vector_point(self.thisptr.pointAndDerivatives(t, n))

    def value_at(self, Coord t, Dim2 d):
        """Equivalent to self(t)[d]."""
        return self.thisptr.valueAt(t, d)

    def to_SBasis(self):
        """Convert self to pair of SBasis functions."""
        return wrap_D2_SBasis(self.thisptr.toSBasis())

    def winding(self, cy_Point p):
        """Return winding number around specified point."""
        return (<Curve *> self.thisptr).winding(deref(p.thisptr))

    def unit_tangent_at(self, Coord t, int n = 3):
        """Return tangent at self(t).

        Parameter n specifies how many derivatives to take into account."""
        return wrap_Point((<Curve *> self.thisptr).unitTangentAt(t, n))

cdef cy_BezierCurve wrap_BezierCurve(BezierCurve p):
    cdef vector[Point] points = make_vector_point([cy_Point(), cy_Point()])
    cdef BezierCurve * retp = create(p.controlPoints())
    cdef cy_BezierCurve r = cy_BezierCurve.__new__(cy_BezierCurve, [cy_Point(), cy_Point()])
    r.thisptr = retp
    return r


cdef class cy_LineSegment(cy_BezierCurve):

    """Bezier curve with fixed order 1.

    This class inheriths from BezierCurve.

    Corresponds to LineSegment in 2geom. BezierCurveN is not wrapped.
    """

    def __cinit__(self, cy_Point p0=None,
                        cy_Point p1=cy_Point()):
        """Create new LineSegment from it's endpoints."""
        if p0 is None:
            self.thisptr = <BezierCurve *> new LineSegment()
        else:
            self.thisptr = <BezierCurve *> new LineSegment( deref(p0.thisptr),
                                                            deref(p1.thisptr))

    @classmethod
    def from_beziers(cls, cy_Bezier b0, cy_Bezier b1):
        """Create LineSegment from two linear beziers."""
        return wrap_LineSegment( LineSegment(deref(b0.thisptr), deref(b1.thisptr)) )

    def subdivide(self, Coord t):
        """Get two LineSegments, from 0 to t and from t to 1."""
        cdef pair[LineSegment, LineSegment] p = (<LineSegment *> self.thisptr).subdivide(t)
        return ( wrap_LineSegment(p.first), wrap_LineSegment(p.second) )

    def duplicate(self):
        """Duplicate the curve."""
        return wrap_LineSegment( deref( <LineSegment *>  self.thisptr.duplicate()))

    def portion(self, double fr=0, double to=1, cy_Interval interval=None):
        """Return portion of curve, specified by endpoints or interval."""
        if interval is None:
            return wrap_LineSegment( deref( <LineSegment *> self.thisptr.portion( fr, to ) ) )
        else:
            return wrap_LineSegment( deref( <LineSegment *>
                (<Curve *> self.thisptr).portion( deref( interval.thisptr ))
                ) )

    def reverse(self):
        """Return curve with reversed time."""
        return wrap_LineSegment( deref( <LineSegment *> self.thisptr.reverse()))

    def transformed(self, t):
        """Transform curve by affine transform."""
        cdef Affine at
        if is_transform(t):
            at = get_Affine(t)
            return wrap_LineSegment( deref( <LineSegment *> self.thisptr.transformed( at )))

    def derivative(self):
        """Return curve's derivative."""
        return wrap_LineSegment( deref( <LineSegment *> self.thisptr.derivative()))

cdef cy_LineSegment wrap_LineSegment(LineSegment p):
    cdef LineSegment * retp = new LineSegment()
    retp[0] = p
    cdef cy_LineSegment r = cy_LineSegment.__new__(cy_LineSegment)
    r.thisptr = <BezierCurve* > retp
    return r


cdef class cy_QuadraticBezier(cy_BezierCurve):

    """Bezier curve with fixed order 2.

    This class inheriths from BezierCurve.

    Corresponds to QuadraticBezier in 2geom. BezierCurveN is not wrapped.
    """

    def __cinit__(self, cy_Point p0=None,
                        cy_Point p1=cy_Point(),
                        cy_Point p2=cy_Point()):
        """Create new QuadraticBezier from three control points."""
        if p0 is None:
            self.thisptr = <BezierCurve *> new QuadraticBezier()
        else:
            self.thisptr = <BezierCurve *> new QuadraticBezier( deref( p0.thisptr ),
                                                                deref( p1.thisptr ),
                                                                deref( p2.thisptr ) )

    @classmethod
    def from_beziers(cls, cy_Bezier b0, cy_Bezier b1):
        """Create QuadraticBezier from two quadratic bezier functions."""
        return wrap_QuadraticBezier( QuadraticBezier(deref(b0.thisptr), deref(b1.thisptr)) )

    def subdivide(self, Coord t):
        """Get two QuadraticBeziers, from 0 to t and from t to 1."""
        cdef pair[QuadraticBezier, QuadraticBezier] p = (<QuadraticBezier *> self.thisptr).subdivide(t)
        return ( wrap_QuadraticBezier(p.first), wrap_QuadraticBezier(p.second) )

    def duplicate(self):
        """Duplicate the curve."""
        return wrap_QuadraticBezier( deref( <QuadraticBezier *>  self.thisptr.duplicate()))

    def portion(self, double fr=0, double to=1, cy_Interval interval=None):
        """Return portion of curve, specified by endpoints or interval."""
        if interval is None:
            return wrap_QuadraticBezier( deref( <QuadraticBezier *> self.thisptr.portion( fr, to ) ) )
        else:
            return wrap_QuadraticBezier( deref( <QuadraticBezier *>
                (<Curve *> self.thisptr).portion( deref( interval.thisptr ))
                ) )

    def reverse(self):
        """Return curve with reversed time."""
        return wrap_QuadraticBezier( deref( <QuadraticBezier *> self.thisptr.reverse()))

    def transformed(self, t):
        """Transform curve by affine transform."""
        cdef Affine at
        if is_transform(t):
            at = get_Affine(t)
            return wrap_QuadraticBezier( deref( <QuadraticBezier *> self.thisptr.transformed( at )))

    def derivative(self):
        """Return curve's derivative."""
        return wrap_LineSegment( deref( <LineSegment *> self.thisptr.derivative()))

cdef cy_QuadraticBezier wrap_QuadraticBezier(QuadraticBezier p):
    cdef QuadraticBezier * retp = new QuadraticBezier()
    retp[0] = p
    cdef cy_QuadraticBezier r = cy_QuadraticBezier.__new__(cy_QuadraticBezier)
    r.thisptr = <BezierCurve* > retp
    return r

cdef class cy_CubicBezier(cy_BezierCurve):

    """Bezier curve with fixed order 2.

    This class inheriths from BezierCurve.

    Corresponds to QuadraticBezier in 2geom. BezierCurveN is not wrapped.
    """

    def __cinit__(self, cy_Point p0=None,
                        cy_Point p1=cy_Point(),
                        cy_Point p2=cy_Point(),
                        cy_Point p3=cy_Point()):
        """Create new CubicBezier from four control points."""
        if p0 is None:
            self.thisptr = <BezierCurve *> new CubicBezier()
        else:
            self.thisptr = <BezierCurve *> new CubicBezier( deref( p0.thisptr ),
                                                            deref( p1.thisptr ),
                                                            deref( p2.thisptr ),
                                                            deref( p3.thisptr ) )

    @classmethod
    def from_beziers(cls, cy_Bezier b0, cy_Bezier b1):
        """Create CubicBezier from two cubic bezier functions."""
        return wrap_CubicBezier( CubicBezier(deref(b0.thisptr), deref(b1.thisptr)) )

    def subdivide(self, Coord t):
        """Get two CubicBeziers, from 0 to t and from t to 1."""
        cdef pair[CubicBezier, CubicBezier] p = (<CubicBezier *> self.thisptr).subdivide(t)
        return ( wrap_CubicBezier(p.first), wrap_CubicBezier(p.second) )

    def duplicate(self):
        """Duplicate the curve."""
        return wrap_CubicBezier( deref( <CubicBezier *>  self.thisptr.duplicate()))

    def portion(self, double fr=0, double to=1, cy_Interval interval=None):
        """Return portion of curve, specified by endpoints or interval."""
        if interval is None:
            return wrap_CubicBezier( deref( <CubicBezier *> self.thisptr.portion( fr, to ) ) )
        else:
            return wrap_CubicBezier( deref( <CubicBezier *>
                (<Curve *> self.thisptr).portion( deref( interval.thisptr ))
                ) )

    def reverse(self):
        """Return curve with reversed time."""
        return wrap_CubicBezier( deref( <CubicBezier *> self.thisptr.reverse()))

    def transformed(self, t):
        """Transform curve by affine transform."""
        cdef Affine at
        if is_transform(t):
            at = get_Affine(t)
            return wrap_CubicBezier( deref( <CubicBezier *> self.thisptr.transformed( at )))

    def derivative(self):
        """Return curve's derivative."""
        return wrap_QuadraticBezier( deref( <QuadraticBezier *> self.thisptr.derivative()))

cdef cy_CubicBezier wrap_CubicBezier(CubicBezier p):
    cdef CubicBezier * retp = new CubicBezier()
    retp[0] = p
    cdef cy_CubicBezier r = cy_CubicBezier.__new__(cy_CubicBezier)
    r.thisptr = <BezierCurve* > retp
    return r

#~ cdef class cy_BezierCurveN(cy_BezierCurve):


cdef class cy_HLineSegment(cy_LineSegment):

    """Horizontal line segment.

    This class corresponds to HLineSegment in 2geom.
    """

    def __cinit__(self, cy_Point p0=None, cy_Point p1=cy_Point()):
        """Create HLineSegment from it's endpoints."""
        if p0 is None:
            self.thisptr = <BezierCurve *> new HLineSegment()
        else:
            self.thisptr = <BezierCurve *> new HLineSegment( deref( p0.thisptr ), deref( p1.thisptr ) )

    @classmethod
    def from_points(cls, cy_Point p0, cy_Point p1):
        """Create HLineSegment from it's endpoints."""
        return wrap_HLineSegment( HLineSegment( deref(p0.thisptr),
                                                deref(p1.thisptr)) )

    @classmethod
    def from_point_length(cls, cy_Point p, Coord length):
        return wrap_HLineSegment( HLineSegment( deref( p.thisptr ), length ) )

    def set_initial(self, cy_Point p):
        """Set initial point of curve."""
        (<AxisLineSegment_X *> self.thisptr).setInitial( deref(p.thisptr) )

    def set_final(self, cy_Point p):
        """Set final point of curve."""
        (<AxisLineSegment_X *> self.thisptr).setFinal( deref(p.thisptr) )

    def bounds_fast(self):
        """Return bounding rectangle for curve.

        This method is fast, but does not guarantee to give smallest
        rectangle.
        """
        return wrap_Rect( (<AxisLineSegment_X *> self.thisptr).boundsFast() )

    def bounds_exact(self):
        """Return exact bounding rectangle for curve.

        This may take a while.
        """
        return wrap_Rect( (<AxisLineSegment_X *> self.thisptr).boundsExact() )

    def degrees_of_freedom(self):
        """Return number of independent parameters needed to specify the curve."""
        return (<AxisLineSegment_X *> self.thisptr).degreesOfFreedom()

    def roots(self, Coord v, Dim2 d):
        """Find time values where self(t)[d] == v."""
        return wrap_vector_double( (<AxisLineSegment_X *> self.thisptr).roots(v, d) )

    def nearest_point(self, cy_Point p, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return such t that |self(t) - point| is minimized."""
        if interval is None:
            return (<AxisLineSegment_X *> self.thisptr).nearestTime(deref( p.thisptr ), fr, to)
        else:
            return (<Curve *> self.thisptr).nearestTime(deref( p.thisptr ),
                                                         deref( ( interval.thisptr ) ) )

    def point_at(self, Coord t):
        """Equivalent to self(t)."""
        return wrap_Point((<AxisLineSegment_X *> self.thisptr).pointAt(t))

    def value_at(self, Coord t, Dim2 d):
        """Equivalent to self(t)[d]."""
        return (<AxisLineSegment_X *> self.thisptr).valueAt(t, d)

    def point_and_derivatives(self, Coord t, unsigned n):
        """Return point and at least first n derivatives at point t in list."""
        return wrap_vector_point( (<AxisLineSegment_X *> self.thisptr).pointAndDerivatives(t, n) )

    def get_Y(self):
        """Get distance of self from y-axis."""
        return (<HLineSegment *> self.thisptr).getY()

    def set_initial_X(self, Coord x):
        """Set initial point's X coordinate."""
        (<HLineSegment *> self.thisptr).setInitialX(x)

    def set_final_X(self, Coord x):
        """Set final point's X coordinate."""
        (<HLineSegment *> self.thisptr).setFinalX(x)

    def set_Y(self, Coord y):
        """Set Y coordinate of points."""
        (<HLineSegment *> self.thisptr).setY(y)

    def subdivide(self, Coord t):
        """Return two HLineSegments subdivided at t."""
        cdef pair[HLineSegment, HLineSegment] p = (<HLineSegment *> self.thisptr).subdivide(t)
        return (wrap_HLineSegment(p.first), wrap_HLineSegment(p.second))

    def duplicate(self):
        """Duplicate the curve."""
        return wrap_HLineSegment( deref(<HLineSegment *> self.thisptr.duplicate()) )

    def portion(self, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return portion of curve, specified by endpoints or interval."""
        if interval is None:
            return wrap_HLineSegment( deref( <HLineSegment *> self.thisptr.portion( fr, to ) ) )
        else:
            return wrap_HLineSegment( deref( <HLineSegment *>
                (<Curve *> self.thisptr).portion( deref( interval.thisptr ) )
                ) )

    def reverse(self):
        """Return curve with reversed time."""
        return wrap_HLineSegment( deref(<HLineSegment *> self.thisptr.reverse()) )

    def transformed(self, t):
        """Transform curve by affine transform."""
        cdef Affine at
        if is_transform(t):
            at = get_Affine(t)
            return wrap_LineSegment( deref(<LineSegment *> self.thisptr.transformed( at )) )

    def derivative(self):
        """Return curve's derivative."""
        return wrap_HLineSegment( deref(<HLineSegment *> self.thisptr.derivative()) )

cdef cy_HLineSegment wrap_HLineSegment(HLineSegment p):
    cdef HLineSegment * retp = new HLineSegment()
    retp[0] = p
    cdef cy_HLineSegment r = cy_HLineSegment.__new__(cy_HLineSegment)
    r.thisptr = <BezierCurve *> retp
    return r

cdef class cy_VLineSegment(cy_LineSegment):

    """Vertical line segment.

    This class corresponds to HLineSegment in 2geom.
    """

    def __cinit__(self, cy_Point p0=None, cy_Point p1=cy_Point()):
        """Create VLineSegment from it's endpoints."""
        if p0 is None:
            self.thisptr = <BezierCurve *> new VLineSegment()
        else:
            self.thisptr = <BezierCurve *> new VLineSegment( deref( p0.thisptr ), deref( p1.thisptr ) )

    @classmethod
    def from_points(cls, cy_Point p0, cy_Point p1):
        """Create VLineSegment from it's endpoints."""
        return wrap_VLineSegment( VLineSegment( deref(p0.thisptr),
                                                deref(p1.thisptr)) )

    @classmethod
    def from_point_length(cls, cy_Point p, Coord length):
        return wrap_VLineSegment( VLineSegment( deref( p.thisptr ), length ) )

    def set_initial(self, cy_Point p):
        """Set initial point of curve."""
        (<AxisLineSegment_Y *> self.thisptr).setInitial( deref(p.thisptr) )

    def set_final(self, cy_Point p):
        """Set final point of curve."""
        (<AxisLineSegment_Y *> self.thisptr).setFinal( deref(p.thisptr) )

    def bounds_fast(self):
        """Return bounding rectangle for curve.

        This method is fast, but does not guarantee to give smallest
        rectangle.
        """
        return wrap_Rect( (<AxisLineSegment_Y *> self.thisptr).boundsFast() )

    def bounds_exact(self):
        """Return exact bounding rectangle for curve.

        This may take a while.
        """
        return wrap_Rect( (<AxisLineSegment_Y *> self.thisptr).boundsExact() )

    def degrees_of_freedom(self):
        """Return number of independent parameters needed to specify the curve."""
        return (<AxisLineSegment_Y *> self.thisptr).degreesOfFreedom()

    def roots(self, Coord v, Dim2 d):
        """Find time values where self(t)[d] == v."""
        return wrap_vector_double( (<AxisLineSegment_Y *> self.thisptr).roots(v, d) )

    def nearest_point(self, cy_Point p, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return such t that |self(t) - point| is minimized."""
        if interval is None:
            return (<AxisLineSegment_Y *> self.thisptr).nearestTime(deref( p.thisptr ), fr, to)
        else:
            return (<Curve *> self.thisptr).nearestTime(deref( p.thisptr ),
                                                         deref( ( interval.thisptr ) ) )

    def point_at(self, Coord t):
        """Equivalent to self(t)."""
        return wrap_Point((<AxisLineSegment_Y *> self.thisptr).pointAt(t))

    def value_at(self, Coord t, Dim2 d):
        """Equivalent to self(t)[d]."""
        return (<AxisLineSegment_Y *> self.thisptr).valueAt(t, d)

    def point_and_derivatives(self, Coord t, unsigned n):
        """Return point and at least first n derivatives at point t in list."""
        return wrap_vector_point( (<AxisLineSegment_Y *> self.thisptr).pointAndDerivatives(t, n) )

    def get_X(self):
        return (<VLineSegment *> self.thisptr).getX()

    def set_initial_Y(self, Coord y):
        (<VLineSegment *> self.thisptr).setInitialY(y)

    def set_final_Y(self, Coord y):
        (<VLineSegment *> self.thisptr).setFinalY(y)

    def set_X(self, Coord x):
        (<VLineSegment *> self.thisptr).setX(x)

    def subdivide(self, Coord t):
        """Return two HLineSegments subdivided at t."""
        cdef pair[VLineSegment, VLineSegment] p = (<VLineSegment *> self.thisptr).subdivide(t)
        return (wrap_VLineSegment(p.first), wrap_VLineSegment(p.second))

    def duplicate(self):
        """Duplicate the curve."""
        return wrap_VLineSegment( deref(<VLineSegment *> self.thisptr.duplicate()) )

    def portion(self, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return portion of curve, specified by endpoints or interval."""
        if interval is None:
            return wrap_VLineSegment( deref( <VLineSegment *> self.thisptr.portion( fr, to ) ) )
        else:
            return wrap_VLineSegment( deref( <VLineSegment *>
                (<Curve *> self.thisptr).portion( deref( interval.thisptr ) )
                ) )

    def reverse(self):
        """Return curve with reversed time."""
        return wrap_VLineSegment( deref(<VLineSegment *> self.thisptr.reverse()) )

    def transformed(self, t):
        """Transform curve by affine transform."""
        cdef Affine at
        if is_transform(t):
            at = get_Affine(t)
            return wrap_LineSegment( deref(<LineSegment *> self.thisptr.transformed( at )) )

    def derivative(self):
        """Return curve's derivative."""
        return wrap_VLineSegment( deref(<VLineSegment *> self.thisptr.derivative()) )

cdef cy_VLineSegment wrap_VLineSegment(VLineSegment p):
    cdef VLineSegment * retp = new VLineSegment()
    retp[0] = p
    cdef cy_VLineSegment r = cy_VLineSegment.__new__(cy_VLineSegment)
    r.thisptr = <BezierCurve *> retp
    return r

cdef class cy_EllipticalArc:

    """Elliptical arc.

    Corresponds to EllipticalArc class in 2geom.
    """

    def __cinit__(self, cy_Point ip = cy_Point(0, 0),
                        Coord rx = 0,
                        Coord ry = 0,
                        Coord rot_angle = 0,
                        bint large_arc = True,
                        bint sweep = True,
                        cy_Point fp = cy_Point(0, 0)):
        """Create Elliptical arc from it's major axis and rays."""
        self.thisptr = new EllipticalArc(deref( ip.thisptr ), rx, ry, rot_angle, large_arc, sweep, deref( fp.thisptr ))

    def __dealloc__(self):
        del self.thisptr

    def __call__(self, Coord t):
        """Get point at time value t."""
        return wrap_Point( deref(<Curve *> self.thisptr)(t) )
    #Curve methods

    def length(self, Coord tolerance = 0.01):
        """Return length of curve, within give tolerance."""
        return (<Curve *> self.thisptr).length(tolerance)

    #AngleInterval methods

    def initial_angle(self):
        """Get initial Angle of arc."""
        return wrap_Angle((<AngleInterval *> self.thisptr).initialAngle())

    def final_angle(self):
        """Get final Angle of arc."""
        return wrap_Angle((<AngleInterval *> self.thisptr).finalAngle())

    def angle_at(self, Coord t):
        """Get Angle from time value."""
        return wrap_Angle((<AngleInterval *> self.thisptr).angleAt(t))

    def contains(self, cy_Angle a):
        """Test whether arc contains angle."""
        return (<AngleInterval *> self.thisptr).contains(deref( a.thisptr ))

    def extent(self):
        """Get extent of angle interval."""
        return (<AngleInterval *> self.thisptr).extent()

    def angle_interval(self):
        """Get underlying angle Interval."""
        return wrap_Interval(self.thisptr.angleInterval())

    def rotation_angle(self):
        """Return rotation angle of major axis."""
        return wrap_Angle(self.thisptr.rotationAngle())

    def ray(self, Dim2 d):
        """Access rays with X or Y."""
        return self.thisptr.ray(d)

    def rays(self):
        """Get rays as a point."""
        return wrap_Point(self.thisptr.rays())

    def large_arc(self):
        """Check if large arc flag is set."""
        return self.thisptr.largeArc()

    def sweep(self):
        """Check if sweep flag is set."""
        return self.thisptr.sweep()

    def chord(self):
        """Return chord of arc."""
        return wrap_LineSegment(self.thisptr.chord())

    def set(self, cy_Point ip, double rx, double ry, double rot_angle, bint large_arc, bint sweep, cy_Point fp):
        """Set arc's properties."""
        self.thisptr.set(deref( ip.thisptr ), rx, ry, rot_angle, large_arc, sweep, deref( fp.thisptr ))

    def set_extremes(self, cy_Point ip, cy_Point fp):
        """Set endpoints of arc."""
        self.thisptr.setExtremes(deref( ip.thisptr ), deref( fp.thisptr ))

    def center(self, coordinate=None):
        """Return center of ellipse, or it's coordinate."""
        if coordinate is None:
            return wrap_Point(self.thisptr.center())
        else:
            return self.thisptr.center(int(coordinate))

    def sweep_angle(self):
        """Equivalent to self.extent()"""
        return self.thisptr.sweepAngle()

    def contains_angle(self, Coord angle):
        """Test whether arc contains angle.

        Equivalent to self.contains(Angle(a))
        """
        return self.thisptr.containsAngle(angle)

    def point_at_angle(self, Coord a):
        """Get point of arc at specified angle."""
        return wrap_Point(self.thisptr.pointAtAngle(a))

    def value_at_angle(self, Coord a, Dim2 d):
        """Equivalent to self.point_at_angle(a)[d]"""
        return self.thisptr.valueAtAngle(a, d)

    def unit_circle_transform(self):
        """Get Affine transform needed to transform unit circle to ellipse."""
        return wrap_Affine(self.thisptr.unitCircleTransform())

    def is_SVG_compliant(self):
        """Check whether arc is SVG compliant

        SVG has special specification for degenerated ellipse."""
        return self.thisptr.isSVGCompliant()

    def subdivide(self, Coord t):
        """Return two arcs, subdivided at time t."""
        cdef pair[EllipticalArc, EllipticalArc] r = self.thisptr.subdivide(t)
        return (wrap_EllipticalArc(r.first), wrap_EllipticalArc(r.second))

    def initial_point(self):
        """Get self(0)."""
        return wrap_Point(self.thisptr.initialPoint())

    def final_point(self):
        """Get self(1)."""
        return wrap_Point(self.thisptr.finalPoint())

    def duplicate(self):
        """Duplicate the curve."""
        return wrap_EllipticalArc( deref(<EllipticalArc *> self.thisptr.duplicate()) )

    def set_initial(self, cy_Point p):
        """Set initial point of curve."""
        self.thisptr.setInitial(deref( p.thisptr ))

    def set_final(self, cy_Point p):
        """Set final point of curve."""
        self.thisptr.setFinal(deref( p.thisptr ))

    def is_degenerate(self):
        """Curve is degenerate if it's length is zero."""
        return self.thisptr.isDegenerate()

    def bounds_fast(self):
        """Return bounding rectangle for curve.

        This method is fast, but does not guarantee to give smallest
        rectangle.
        """
        return wrap_Rect(self.thisptr.boundsFast())

    def bounds_exact(self):
        """Return exact bounding rectangle for curve.

        This may take a while.
        """
        return wrap_Rect(self.thisptr.boundsExact())

    def bounds_local(self, cy_OptInterval i, unsigned int deg):
        """Return bounding rectangle to portion of curve."""
        return wrap_OptRect(self.thisptr.boundsLocal(deref( i.thisptr ), deg))

    def roots(self, double v, Dim2 d):
        """Find time values where self(t)[d] == v."""
        return wrap_vector_double(self.thisptr.roots(v, d))

    def nearest_point(self, cy_Point p, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return such t that |self(t) - point| is minimized."""
        if interval is None:
            return self.thisptr.nearestTime(deref( p.thisptr ), fr, to)
        else:
            return (<Curve *> self.thisptr).nearestTime(deref( p.thisptr ),
                                                         deref( interval.thisptr ) )

    def all_nearest_points(self, cy_Point p, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return all values of t that |self(t) - point| is minimized."""
        if interval is None:
            return wrap_vector_double( (<Curve *> self.thisptr).allNearestTimes(deref( p.thisptr ), fr, to))
        else:
            return wrap_vector_double( (<Curve *> self.thisptr).allNearestTimes(deref( p.thisptr ), deref( interval.thisptr ) ))

    def degrees_of_freedom(self):
        """Return number of independent parameters needed to specify the curve."""
        return self.thisptr.degreesOfFreedom()

    def derivative(self):
        """Return curve's derivative."""
        return wrap_EllipticalArc( deref(<EllipticalArc *> self.thisptr.derivative()) )

    def transformed(self, cy_Affine m):
        """Transform curve by affine transform."""
        return wrap_EllipticalArc( deref(<EllipticalArc *> self.thisptr.transformed(deref( m.thisptr ))) )

    def point_and_derivatives(self, Coord t, unsigned int n):
        """Return point and at least first n derivatives at point t in list."""
        return wrap_vector_point(self.thisptr.pointAndDerivatives(t, n))

    def to_SBasis(self):
        """Convert to pair of SBasis polynomials."""
        cdef D2[SBasis] r = self.thisptr.toSBasis()
        return ( wrap_SBasis(r[0]), wrap_SBasis(r[1]) )

    def value_at(self, Coord t, Dim2 d):
        """Equivalent to self(t)[d]."""
        return self.thisptr.valueAt(t, d)

    def point_at(self, Coord t):
        """Equivalent to self(t)."""
        return wrap_Point(self.thisptr.pointAt(t))


    def portion(self, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return portion of curve, specified by endpoints or interval."""
        if interval is None:
            return wrap_EllipticalArc( deref( <EllipticalArc *> self.thisptr.portion( fr, to ) ) )
        else:
            return wrap_EllipticalArc( deref( <EllipticalArc *> (<Curve *> self.thisptr).portion( deref( interval.thisptr ) ) ) )

    def reverse(self):
        """Return curve with reversed time."""
        return wrap_EllipticalArc( deref(<EllipticalArc *> self.thisptr.reverse()) )

    def winding(self, cy_Point p):
        """Return winding number around specified point."""
        return (<Curve *> self.thisptr).winding(deref(p.thisptr))

    def unit_tangent_at(self, Coord t, int n = 3):
        """Return tangent at self(t).

        Parameter n specifies how many derivatives to take into account."""
        return wrap_Point((<Curve *> self.thisptr).unitTangentAt(t, n))

cdef cy_EllipticalArc wrap_EllipticalArc(EllipticalArc p):
    cdef EllipticalArc * retp = new EllipticalArc()
    retp[0] = p
    cdef cy_EllipticalArc r = cy_EllipticalArc.__new__(cy_EllipticalArc)
    r.thisptr = retp
    return r


cdef class cy_SVGEllipticalArc(cy_EllipticalArc):

    """SVG compliant Elliptical arc.

    This class inherits from EllipticalArc, with only exception being
    degenerate case, where SVGEllipticalArc reduces to line segment.

    Corresponds to EllipticalArc class in 2geom.
    """

    def __cinit__(self, cy_Point ip = cy_Point(0, 0),
                        Coord rx = 0,
                        Coord ry = 0,
                        Coord rot_angle = 0,
                        bint large_arc = True,
                        bint sweep = True,
                        cy_Point fp = cy_Point(0, 0)):
        """Create SVGEllipticalArc from it's major axis and rays."""
        self.thisptr = <EllipticalArc *> new SVGEllipticalArc(deref( ip.thisptr ), rx, ry, rot_angle, large_arc, sweep, deref( fp.thisptr ))

    def __call__(self, Coord t):
        """Get point at time value t."""
        return self.point_at(t)

    def value_at(self, Coord t, Dim2 d):
        """Equivalent to self(t)[d]."""
        return (<SVGEllipticalArc *> self.thisptr).valueAt(t, d)

    def point_at(self, Coord t):
        """Equivalent to self(t)."""
        return wrap_Point((<SVGEllipticalArc *> self.thisptr).pointAt(t))

    def point_and_derivatives(self, Coord t, unsigned int n):
        """Return point and at least first n derivatives at point t in list."""
        return wrap_vector_point((<SVGEllipticalArc *> self.thisptr).pointAndDerivatives(t, n))

    def bounds_exact(self):
        """Return exact bounding rectangle for curve.

        This may take a while.
        """
        return wrap_Rect((<SVGEllipticalArc *> self.thisptr).boundsExact())

    def bounds_local(self, cy_OptInterval i, unsigned int deg):
        """Return bounding rectangle to portion of curve."""
        return wrap_OptRect((<SVGEllipticalArc *> self.thisptr).boundsLocal(deref( i.thisptr ), deg))

    def derivative(self):
        """Return curve's derivative."""
        if self.is_degenerate():
            return wrap_LineSegment( deref(<LineSegment *> (<SVGEllipticalArc *> self.thisptr).derivative())  )
        return wrap_EllipticalArc( deref(<EllipticalArc *> (<SVGEllipticalArc *> self.thisptr).derivative()) )

    def duplicate(self):
        """Duplicate the curve."""
        return wrap_SVGEllipticalArc( deref(<SVGEllipticalArc *> (<SVGEllipticalArc *> self.thisptr).duplicate()) )

    def roots(self, double v, Dim2 d):
        """Find time values where self(t)[d] == v."""
        return wrap_vector_double((<SVGEllipticalArc *> self.thisptr).roots(v, d))

    def all_nearest_points(self, cy_Point p, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return all values of t that |self(t) - point| is minimized."""
        if interval is None:
            return wrap_vector_double( (<SVGEllipticalArc *> self.thisptr).allNearestTimes(deref( p.thisptr ), fr, to))
        else:
            return wrap_vector_double((<Curve *> self.thisptr).allNearestTimes(deref( p.thisptr ),
                                                                                deref( interval.thisptr ) ))

    def to_SBasis(self):
        """Convert to pair of SBasis polynomials."""
        cdef D2[SBasis] r = (<SVGEllipticalArc *> self.thisptr).toSBasis()
        return ( wrap_SBasis(r[0]), wrap_SBasis(r[1]) )

    def is_SVG_compliant(self):
        """Check whether arc is SVG compliant

        SVG has special specification for degenerated ellipse."""
        return (<SVGEllipticalArc *> self.thisptr).isSVGCompliant()

cdef cy_SVGEllipticalArc wrap_SVGEllipticalArc(SVGEllipticalArc p):
    cdef SVGEllipticalArc * retp = new SVGEllipticalArc()
    retp[0] = p
    cdef cy_SVGEllipticalArc r = cy_SVGEllipticalArc.__new__(cy_SVGEllipticalArc)
    r.thisptr = <EllipticalArc *> retp
    return r

#TODO move somewhere else

cdef object wrap_vector_interval(vector[Interval] v):
    r = []
    cdef unsigned int i
    for i in range(v.size()):
        r.append( wrap_Interval(v[i]))
    return r


cdef bint is_Curve(object c):
    return any([
        isinstance(c, cy_Curve),
        isinstance(c, cy_SBasisCurve),
        isinstance(c, cy_BezierCurve),
        isinstance(c, cy_EllipticalArc)])

cdef Curve * get_Curve_p(object c):
    if isinstance(c, cy_Curve):
        return (<cy_Curve> c).thisptr
    elif isinstance(c, cy_SBasisCurve):
        return <Curve *> (<cy_SBasisCurve> c).thisptr
    elif isinstance(c, cy_BezierCurve):
        return <Curve *> (<cy_BezierCurve> c).thisptr
    elif isinstance(c, cy_EllipticalArc):
        return <Curve *> (<cy_EllipticalArc> c).thisptr
    return NULL

