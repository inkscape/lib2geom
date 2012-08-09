from cython.operator cimport dereference as deref
from numbers import Number
from _cy_rectangle cimport cy_OptInterval, wrap_OptInterval, wrap_Rect, OptRect, wrap_OptRect
from _cy_rectangle cimport cy_Interval, wrap_Interval   

from _cy_affine cimport cy_Translate, cy_Rotate, cy_Scale
from _cy_affine cimport cy_VShear, cy_HShear, cy_Zoom
from _cy_affine cimport cy_Affine, wrap_Affine, get_Affine, is_transform
#~ TODO:
#~     Curve class is used to work nicely with C++'s static typing, it, however
#~     becomes unneccessary in python with it's dynamin (or even duck) typing.
#~     So far I am not wrapping Curve. Methods returning Curve will cast to 
#~     underlying type, and it will be possible to convert between different 
#~     curve types. 
#~     TODO? Discuss this.

cdef class cy_Curve:
#    cdef Curve* thisptr
#~     def operator=(self, cy_Curve arg0):
#~         return wrap_Curve(deref( self.thisptr ) = deref( arg0.thisptr ))
#~     def __init__(self, cy_Curve arg0):
#~         self.thisptr = self.thisptr.Curve(deref( arg0.thisptr ))
    def __cinit__(self):
        self.thisptr = <Curve *> new SBasisCurve(D2[SBasis]( SBasis(), SBasis() ))
    def __call__(self, Coord t):
        return wrap_Point( deref(self.thisptr)(t) )
    def initialPoint(self):
        return wrap_Point(self.thisptr.initialPoint())
    def finalPoint(self):
        return wrap_Point(self.thisptr.finalPoint())
    def isDegenerate(self):
        return self.thisptr.isDegenerate()
    def pointAt(self, Coord t):
        return wrap_Point(self.thisptr.pointAt(t))
    def valueAt(self, Coord t, Dim2 d):
        return self.thisptr.valueAt(t ,d)
#~     def operator()(self, Coord t):
#~         return wrap_Point(deref( self.thisptr ) () t)
#~     def pointAndDerivatives(self, Coord t, unsigned int n):
#~         return wrap_::std::vector<Geom::Point, std::allocator<Geom::Point> >(self.thisptr.pointAndDerivatives(t ,n))
    def setInitial(self, cy_Point v):
        self.thisptr.setInitial(deref( v.thisptr ))
    def setFinal(self, cy_Point v):
        self.thisptr.setFinal(deref( v.thisptr ))
    def boundsFast(self):
        return wrap_Rect(self.thisptr.boundsFast())
    def boundsExact(self):
        return wrap_Rect(self.thisptr.boundsExact())
    def boundsLocal(self, cy_OptInterval i, unsigned int deg):
        return wrap_OptRect(self.thisptr.boundsLocal(deref( i.thisptr ) ,deg))
    def boundsLocal(self, cy_OptInterval a):
        return wrap_OptRect(self.thisptr.boundsLocal(deref( a.thisptr )))
    def duplicate(self):
        return wrap_Curve_p( self.thisptr.duplicate() ) 
#~     def transformed(self, cy_Affine m):
#~         return wrap_Curve( deref(self.thisptr.transformed(deref( m.thisptr ))) )
#~     def portion(self, Coord a, Coord b):
#~         return wrap_Curve( deref(self.thisptr.portion(a ,b)) )
#~     def portion(self, cy_Interval i):
#~         return wrap_Curve( deref(self.thisptr.portion(deref( i.thisptr ))) )
    def reverse(self):
        return wrap_Curve_p( self.thisptr.reverse() )
    def derivative(self):
        return wrap_Curve_p( self.thisptr.derivative() )
    def nearestPoint(self, cy_Point p, Coord a, Coord b):
        return self.thisptr.nearestPoint(deref( p.thisptr ) ,a ,b)
    def nearestPoint(self, cy_Point p, cy_Interval i):
        return self.thisptr.nearestPoint(deref( p.thisptr ) ,deref( i.thisptr ))
#~     def allNearestPoints(self, cy_Point p, Coord from, Coord to):
#~         return wrap_::std::vector<double, std::allocator<double> >(self.thisptr.allNearestPoints(deref( p.thisptr ) ,from ,to))
#~     def allNearestPoints(self, cy_Point p, cy_Interval i):
#~         return wrap_::std::vector<double, std::allocator<double> >(self.thisptr.allNearestPoints(deref( p.thisptr ) ,deref( i.thisptr )))
    def length(self, Coord tolerance):
        return self.thisptr.length(tolerance)
#~     def roots(self, Coord v, Dim2 d):
#~         return wrap_::std::vector<double, std::allocator<double> >(self.thisptr.roots(v ,d))
    def winding(self, cy_Point p):
        return self.thisptr.winding(deref( p.thisptr ))
    def unitTangentAt(self, Coord t, unsigned int n):
        return wrap_Point(self.thisptr.unitTangentAt(t ,n))
#~     def toSBasis(self):
#~         return wrap_D2<Geom::SBasis>(self.thisptr.toSBasis())
    def degreesOfFreedom(self):
        return self.thisptr.degreesOfFreedom()
#~     def operator==(self, cy_Curve c):
#~         return deref( self.thisptr ) == deref( c.thisptr )

cdef cy_Curve wrap_Curve(Curve & p):
    cdef Curve * retp = <Curve *> new SBasisCurve(D2[SBasis]( SBasis(), SBasis() ))
    retp[0] = p
    cdef cy_Curve r = cy_Curve.__new__(cy_Curve)
    r.thisptr = retp
    return r

cdef cy_Curve wrap_Curve_p(Curve * p):
#~     cdef Curve * retp = <Curve *> SBasisCurve(D2[SBasis]( SBasis(), SBasis() ))
#~     retp[0] = p
    cdef cy_Curve r = cy_Curve.__new__(cy_Curve)
    r.thisptr = p
    return r

cdef class cy_Linear:
    cdef Linear* thisptr
    def __cinit__(self, aa = None, b = None):
        if aa is None:
            self.thisptr = new Linear()
        elif b is None:
            self.thisptr = new Linear(float(aa))
        else:
            self.thisptr = new Linear(float(aa), float(b))
    def __call__(self, Coord t):
        return deref(self.thisptr)(t)
    def __getitem__(self, i):
        return deref( self.thisptr ) [i]
    def __richcmp__(cy_Linear self, cy_Linear other, int op):
        if op == 2:
            return deref(self.thisptr) == deref(other.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(other.thisptr)
            
    def __neg__(cy_Linear self):
        return wrap_Linear( L_neg(deref(self.thisptr)) )
    
    def __add__(cy_Linear self, other):
        if isinstance(other, Number):
            return wrap_Linear( deref(self.thisptr) + float(other) )
        elif isinstance(other, cy_Linear):
            return wrap_Linear( deref(self.thisptr) + deref( (<cy_Linear> other).thisptr ) )
    def __sub__(cy_Linear self, other):
        if isinstance(other, Number):
            return wrap_Linear( L_sub_Ld(deref(self.thisptr), float(other)) )
        elif isinstance(other, cy_Linear):
            return wrap_Linear( L_sub_LL(deref(self.thisptr), deref( (<cy_Linear> other).thisptr )) )

    def __mul__(cy_Linear self, double b):
        return wrap_Linear(deref( self.thisptr ) * b)
    def __div__(cy_Linear self, double b):
        return wrap_Linear(deref( self.thisptr ) / b)
        
    def isZero(self, double eps = EPSILON):
        return self.thisptr.isZero(eps)
    def isConstant(self, double eps = EPSILON):
        return self.thisptr.isConstant(eps)
    def isFinite(self):
        return self.thisptr.isFinite()
    def at0(self):
        return self.thisptr.at0()
    def at1(self):
        return self.thisptr.at1()
    def valueAt(self, double t):
        return self.thisptr.valueAt(t)
    def toSBasis(self):
        return wrap_SBasis(self.thisptr.toSBasis())
    def bounds_exact(self):
        return wrap_OptInterval(self.thisptr.bounds_exact())
    def bounds_fast(self):
        return wrap_OptInterval(self.thisptr.bounds_fast())
    def bounds_local(self, double u, double v):
        return wrap_OptInterval(self.thisptr.bounds_local(u ,v))
    def tri(self):
        return self.thisptr.tri()
    def hat(self):
        return self.thisptr.hat()

#leave these in cy2geom napespace?
def cy_lerp(double t, double a, double b):
    return lerp(t, a, b)
    

cdef cy_Linear wrap_Linear(Linear p):
    cdef Linear * retp = new Linear()
    retp[0] = p
    cdef cy_Linear r = cy_Linear.__new__(cy_Linear)
    r.thisptr = retp
    return r



cdef class cy_SBasis:
#    cdef SBasis* thisptr
    def __cinit__(self, *args):
        if len(args) == 0:
            self.thisptr = new SBasis()
        elif len(args) == 1 :
            if isinstance(args[0], Number):
                self.thisptr = new SBasis( float(args[0]) )
            elif isinstance(args[0], cy_Linear):
                self.thisptr = new SBasis( deref( (<cy_Linear> args[0]).thisptr ) )
            elif isinstance(args[0], cy_SBasis):
                self.thisptr = new SBasis( deref( (<cy_SBasis> args[0]).thisptr ) )
        elif len(args) == 2 :
            self.thisptr = new SBasis( float(args[0]), float(args[1]) )
    def size(self):
        return self.thisptr.size()
    def __call__(self, o):
        if isinstance(o, Number):
            return deref(self.thisptr)(float(o))
        elif isinstance(self, cy_SBasis):
            return wrap_SBasis(deref(self.thisptr)( deref( (<cy_SBasis> o).thisptr ) ))
    def __getitem__(self, unsigned int i):
        return wrap_Linear(deref( self.thisptr ) [i])
#~     def begin(self):
#~         return wrap_::__gnu_cxx::__normal_iterator<Geom::Linear const*, std::vector<Geom::Linear, std::allocator<Geom::Linear> > >(self.thisptr.begin())
#~     def end(self):
#~         return wrap_::__gnu_cxx::__normal_iterator<Geom::Linear const*, std::vector<Geom::Linear, std::allocator<Geom::Linear> > >(self.thisptr.end())
#~     def begin(self):
#~         return wrap_::__gnu_cxx::__normal_iterator<Geom::Linear*, std::vector<Geom::Linear, std::allocator<Geom::Linear> > >(self.thisptr.begin())
#~     def end(self):
#~         return wrap_::__gnu_cxx::__normal_iterator<Geom::Linear*, std::vector<Geom::Linear, std::allocator<Geom::Linear> > >(self.thisptr.end())
    #cython doesn't use __rmul__, it switches the arguments instead
    def __neg__(self):
        return wrap_SBasis( SB_neg(deref(self.thisptr)) )
    def __add__(cy_SBasis self, other):
        if isinstance(other, Number):
            return wrap_SBasis( deref(self.thisptr) + float(other) )
        elif isinstance(other, cy_SBasis):
            return wrap_SBasis( deref(self.thisptr) + deref( (<cy_SBasis> other).thisptr ) )
    def __sub__(cy_SBasis self, other):
        if isinstance(other, Number):
            return wrap_SBasis( SB_sub_Sd(deref(self.thisptr), float(other) ) )
        elif isinstance(other, cy_SBasis):
            return wrap_SBasis( SB_sub_SS(deref(self.thisptr), deref( (<cy_SBasis> other).thisptr ) ) )
    def __mul__(self, other):
        if isinstance(other, Number):
            return wrap_SBasis( deref( (<cy_SBasis> self).thisptr ) * float(other) )
        elif isinstance(other, cy_SBasis):
            if isinstance(self, cy_SBasis):
                return wrap_SBasis( deref( (<cy_SBasis> self).thisptr ) * deref( (<cy_SBasis> other).thisptr ) )
            elif isinstance(self, Number):
                return wrap_SBasis( float(self) * deref( (<cy_SBasis> other).thisptr ) )
    def __div__(cy_SBasis self, double other):
        return wrap_SBasis( deref(self.thisptr)/other )

    def empty(self):
        return self.thisptr.empty()
    def back(self):
        return wrap_Linear(self.thisptr.back())
    def back(self):
        return wrap_Linear(self.thisptr.back())
    def pop_back(self):
        self.thisptr.pop_back()
    def resize(self, unsigned int n, cy_Linear l = None):
        if l is None:
            self.thisptr.resize(n)
        else:
            self.thisptr.resize(n ,deref( l.thisptr ))
    def reserve(self, unsigned int n):
        self.thisptr.reserve(n)
    def clear(self):
        self.thisptr.clear()
#~     def insert(self, cy_::__gnu_cxx::__normal_iterator<Geom::Linear*, std::vector<Geom::Linear, std::allocator<Geom::Linear> > > before, cy_::__gnu_cxx::__normal_iterator<Geom::Linear const*, std::vector<Geom::Linear, std::allocator<Geom::Linear> > > src_begin, cy_::__gnu_cxx::__normal_iterator<Geom::Linear const*, std::vector<Geom::Linear, std::allocator<Geom::Linear> > > src_end):
#~         self.thisptr.insert(deref( before.thisptr ) ,deref( src_begin.thisptr ) ,deref( src_end.thisptr ))
    def at(self, unsigned int i):
        return wrap_Linear(self.thisptr.at(i))
    def __richcmp__(cy_SBasis self, cy_SBasis B, int op):
        if op == 2:
            return deref( self.thisptr ) == deref( B.thisptr )
        elif op == 3:
            return deref( self.thisptr ) != deref( B.thisptr )
#~     def __init__(self, cy_SBasis a):
#~         self.thisptr = self.thisptr.SBasis(deref( a.thisptr ))
#~     def __init__(self, cy_::std::vector<Geom::Linear, std::allocator<Geom::Linear> > ls):
#~         self.thisptr = self.thisptr.SBasis(deref( ls.thisptr ))
#~     def __init__(self, cy_::size_t n, cy_Linear l):
#~         self.thisptr = self.thisptr.SBasis(deref( n.thisptr ) ,deref( l.thisptr ))
    def isZero(self, double eps = EPSILON):
        return self.thisptr.isZero(eps)
    def isConstant(self, double eps = EPSILON):
        return self.thisptr.isConstant(eps)
    def isFinite(self):
        return self.thisptr.isFinite()
    def at0(self):
        return self.thisptr.at0()
    def at1(self):
        return self.thisptr.at1()
    def degreesOfFreedom(self):
        return self.thisptr.degreesOfFreedom()
    def valueAt(self, double t):
        return self.thisptr.valueAt(t)
    def valueAndDerivatives(self, double t, unsigned int n):
        return wrap_vector_double (self.thisptr.valueAndDerivatives(t ,n))
    def toSBasis(self):
        return wrap_SBasis(self.thisptr.toSBasis())
    def tailError(self, unsigned int tail):
        return self.thisptr.tailError(tail)
    def normalize(self):
        self.thisptr.normalize()
    def truncate(self, unsigned int k):
        self.thisptr.truncate(k)

def cy_sin(cy_Linear bo, int k):
    return wrap_SBasis(sin(deref( bo.thisptr ), k))    
def cy_cos(cy_Linear bo, int k):
    return wrap_SBasis(cos(deref( bo.thisptr ), k))
def cy_sqrt(cy_SBasis a, int k):
    return wrap_SBasis(sqrt(deref( a.thisptr ), k))
def cy_reciprocal(cy_Linear a, int k):
    return wrap_SBasis(reciprocal(deref( a.thisptr ), k))
def cy_shift(a, int sh):
    if isinstance(a, cy_Linear):
        return wrap_SBasis(shift(deref( (<cy_Linear> a).thisptr ), sh))
    elif isinstance(a, cy_SBasis):
        return wrap_SBasis(shift(deref( (<cy_SBasis> a).thisptr ), sh))
def cy_inverse(cy_SBasis a, int k):
    return wrap_SBasis(inverse(deref( a.thisptr ), k))
def cy_valuation(cy_SBasis a, double tol = 0):
    return valuation(deref( a.thisptr ), tol)

def cy_compose(cy_SBasis a, cy_SBasis b, k = None):
    if k is None:
        return wrap_SBasis(compose(deref( a.thisptr ), deref( b.thisptr )))
    else:
        return wrap_SBasis(compose(deref( a.thisptr ), deref( b.thisptr ), int(k)))
def cy_portion(t, *args):
    if isinstance(t, cy_SBasis):
        if len(args) == 1:
            if isinstance(args[0], cy_Interval):
                return wrap_SBasis( portion( deref( (<cy_SBasis> t).thisptr ), deref( (<cy_Interval> args[0]).thisptr ) ) )
        elif len(args) == 2:
            return wrap_SBasis( portion( deref( (<cy_SBasis> t).thisptr ), float(args[0]), float(args[1]) ) )
    elif isinstance(t, cy_Bezier):
        return wrap_Bezier(portion(deref( (<cy_Bezier> t).thisptr ), float(args[0]), float(args[1])))        
def cy_truncate(cy_SBasis a, unsigned int terms):
    return wrap_SBasis(truncate(deref( a.thisptr ), terms))

def cy_reverse(a):
    if isinstance(a, cy_Linear):
        return wrap_Linear( reverse(deref( (<cy_Linear> a).thisptr )))
    elif isinstance(a, cy_SBasis):
        return wrap_SBasis( reverse(deref( (<cy_SBasis> a).thisptr )))
    elif isinstance(a, cy_Bezier):
        return wrap_Bezier( reverse(deref( (<cy_Bezier> a).thisptr )))

def cy_multiply(a, b):
    if isinstance(a, cy_SBasis):
        return wrap_SBasis(multiply(deref( (<cy_SBasis> a).thisptr ), deref( (<cy_SBasis> b).thisptr )))
    elif isinstance(a, cy_Bezier):
        return wrap_Bezier(multiply(deref( (<cy_Bezier> a).thisptr ), deref( (<cy_Bezier> b).thisptr )))
def cy_multiply_add(cy_SBasis a, cy_SBasis b, cy_SBasis c):
    return wrap_SBasis(multiply_add(deref( a.thisptr ), deref( b.thisptr ), deref( c.thisptr )))
def cy_divide(cy_SBasis a, cy_SBasis b, int k):
    return wrap_SBasis(divide(deref( a.thisptr ), deref( b.thisptr ), k))

def cy_compose_inverse(cy_SBasis f, cy_SBasis g, unsigned int order, double tol):
    return wrap_SBasis(compose_inverse(deref( f.thisptr ), deref( g.thisptr ), order, tol))


def cy_derivative(a):
    if isinstance(a, cy_SBasis):
        return wrap_SBasis(derivative(deref( (<cy_SBasis> a).thisptr )))
    elif isinstance(a, cy_Bezier):
        return wrap_Bezier(derivative(deref( (<cy_Bezier> a).thisptr )))

def cy_integral(a):
    if isinstance(a, cy_SBasis):
        return wrap_SBasis(integral(deref( (<cy_SBasis> a).thisptr )))
    elif isinstance(a, cy_Bezier):
        return wrap_Bezier(integral(deref( (<cy_Bezier> a).thisptr )))

def cy_roots(cy_SBasis s, cy_Interval inside = None):
    if inside is None:
        return wrap_vector_double(roots(deref( s.thisptr )))
    else:
        return wrap_vector_double(roots(deref( s.thisptr ), deref( inside.thisptr )))

def cy_multi_roots(cy_SBasis f, levels, double htol = 1e-7, double vtol = 1e-7, double a = 0, double b = 1):
    cdef vector[double] l = make_vector_double(levels)
    cdef vector[ vector[double] ] r = multi_roots(deref( f.thisptr ), l, htol, vtol, a, b)
    lst = []
    for i in range(r.size()):
        lst.append( wrap_vector_double(r[i]) )
    return lst

#call with level_set(SBasis(1, 5), 2, a = 0.2, b = 0.4, tol = 0.02) 
def cy_level_set(cy_SBasis f, level, a = 0, b = 1, tol = 1e-5, vtol = 1e-5):
    if isinstance(level, cy_Interval):
        return wrap_vector_interval(level_set(deref( f.thisptr ), deref( (<cy_Interval> level).thisptr ), a, b, tol)) #a, b, tol
    else:
        return wrap_vector_interval(level_set(deref( f.thisptr ), float(level), vtol, a, b, tol)) #vtol, a, b, tol

#~ def cy_level_sets(cy_SBasis f, cy_::std::vector<Geom::Interval,std::allocator<Geom::Interval> > levels, double a, double b, double tol):
#~     return wrap_::std::vector<std::vector<Geom::Interval, std::allocator<Geom::Interval> >,std::allocator<std::vector<Geom::Interval, std::allocator<Geom::Interval> > > >(level_sets(deref( f.thisptr ), deref( levels.thisptr ), a, b, tol))
#~ def cy_level_sets(cy_SBasis f, cy_::std::vector<double,std::allocator<double> > levels, double a, double b, double vtol, double tol):
#~     return wrap_::std::vector<std::vector<Geom::Interval, std::allocator<Geom::Interval> >,std::allocator<std::vector<Geom::Interval, std::allocator<Geom::Interval> > > >(level_sets(deref( f.thisptr ), deref( levels.thisptr ), a, b, vtol, tol))


def cy_bounds_fast(a, int order = 0):
    if isinstance(a, cy_SBasis):
        return wrap_OptInterval(bounds_fast(deref( (<cy_SBasis> a).thisptr ), order))
    elif isinstance(a, cy_Bezier):
        return wrap_OptInterval(bounds_fast(deref( (<cy_Bezier> a).thisptr )))
    
def cy_bounds_exact(a):
    if isinstance(a, cy_SBasis):
        return wrap_OptInterval(bounds_exact(deref( (<cy_SBasis> a).thisptr )))
    elif isinstance(a, cy_Bezier):
        return wrap_OptInterval(bounds_exact(deref( (<cy_Bezier> a).thisptr )))

def cy_bounds_local(a, cy_OptInterval t, int order = 0):
    if isinstance(a, cy_SBasis):
        return wrap_OptInterval(bounds_local(deref( (<cy_SBasis> a).thisptr ), deref( t.thisptr ), order))
    if isinstance(a, cy_Bezier):
        return wrap_OptInterval(bounds_local(deref( (<cy_Bezier> a).thisptr ), deref( t.thisptr )))
    



cdef cy_SBasis wrap_SBasis(SBasis p):
    cdef SBasis * retp = new SBasis()
    retp[0] = p
    cdef cy_SBasis r = cy_SBasis.__new__(cy_SBasis)
    r.thisptr = retp
    return r

cdef class cy_SBasisCurve:
    cdef SBasisCurve* thisptr
#~     def __init__(self, cy_SBasisCurve arg0):
#~         self.thisptr = self.thisptr.SBasisCurve(deref( arg0.thisptr ))
#~     def __init__(self, cy_D2<Geom::SBasis> sb):
#~         self.thisptr = self.thisptr.SBasisCurve(deref( sb.thisptr ))
#~     def __init__(self, cy_Curve other):
#~         self.thisptr = self.thisptr.SBasisCurve(deref( other.thisptr ))
    def __cinit__(self, *args):
        if len(args) == 1:
            pass
#~             if isinstance(args[0], cy_Curve):
#~                 self.thisptr = new SBasisCurve( deref( (<cy_Curve> args[0]).thisptr ) )
        elif len(args) == 2:
            if isinstance(args[0], cy_SBasis) and isinstance(args[1], cy_SBasis):
                self.thisptr = new SBasisCurve( D2[SBasis]( 
                    deref( (<cy_SBasis> args[0]).thisptr ), 
                    deref( (<cy_SBasis> args[1]).thisptr ) ) )
    def __call__(self, double t):
        return wrap_Point(deref(self.thisptr)(t))
    def duplicate(self):
        return wrap_SBasisCurve( <SBasisCurve> deref(self.thisptr.duplicate()) )
    def initialPoint(self):
        return wrap_Point(self.thisptr.initialPoint())
    def finalPoint(self):
        return wrap_Point(self.thisptr.finalPoint())
    def isDegenerate(self):
        return self.thisptr.isDegenerate()
    def pointAt(self, Coord t):
        return wrap_Point(self.thisptr.pointAt(t))
    def pointAndDerivatives(self, Coord t, unsigned int n):
        return wrap_vector_point(self.thisptr.pointAndDerivatives(t ,n))
    def valueAt(self, Coord t, Dim2 d):
        return self.thisptr.valueAt(t ,d)
    def setInitial(self, cy_Point v):
        self.thisptr.setInitial(deref( v.thisptr ))
    def setFinal(self, cy_Point v):
        self.thisptr.setFinal(deref( v.thisptr ))
    def boundsFast(self):
        return wrap_Rect(self.thisptr.boundsFast())
    def boundsExact(self):
        return wrap_Rect(self.thisptr.boundsExact())
    def boundsLocal(self, cy_OptInterval i, unsigned int deg):
        return wrap_OptRect(self.thisptr.boundsLocal(deref( i.thisptr ) ,deg))
    def roots(self, Coord v, Dim2 d):
        return wrap_vector_double( self.thisptr.roots(v, d) )
    def nearestPoint(self, cy_Point p, *args):
        if len(args) == 0:
            return self.thisptr.nearestPoint(deref( p.thisptr ), 0, 1)
        elif len(args) == 1:
            if isinstance(args[0], cy_Interval):
                return (<Curve *> self.thisptr).nearestPoint(deref( p.thisptr ), deref( (<cy_Interval> args[0]).thisptr ) )
        elif len(args) == 2:
            return self.thisptr.nearestPoint(deref( p.thisptr ), float(args[0]), float(args[1]))
    def allNearestPoints(self, cy_Point p, *args):
        if len(args) == 0:
            return wrap_vector_double(self.thisptr.allNearestPoints(deref( p.thisptr ), 0, 1))
        elif len(args) == 1:
            if isinstance(args[0], cy_Interval):
                return wrap_vector_double((<Curve *> self.thisptr).allNearestPoints(deref( p.thisptr ), deref( (<cy_Interval> args[0]).thisptr ) ))
        elif len(args) == 2:
            return wrap_vector_double(self.thisptr.allNearestPoints(deref( p.thisptr ), float(args[0]), float(args[1])))

    def length(self, Coord tolerance = 0.01):
        return self.thisptr.length(tolerance)

    def portion(self, *args):
        if len(args) == 1:
            if isinstance(args[0], cy_Interval):
                #TODO: break this down somehow
                return wrap_SBasisCurve( <SBasisCurve> deref( (<Curve *> self.thisptr).portion( deref( (<cy_Interval> args[0]).thisptr ))) )
        elif len(args) == 2:
            return wrap_SBasisCurve( <SBasisCurve> deref(self.thisptr.portion( float(args[0]), float(args[1]) ) ) ) 
    def transformed(self, t):
        cdef Affine at
        if is_transform(t): 
            at = get_Affine(t)
            return wrap_SBasisCurve( <SBasisCurve> deref(self.thisptr.transformed( at )))
    def reverse(self):
        return wrap_SBasisCurve( <SBasisCurve> deref( (<Curve *> self.thisptr).reverse() ) )
    def derivative(self):
        return wrap_SBasisCurve( <SBasisCurve> deref(self.thisptr.derivative()) )

    def winding(self, cy_Point p):
        return (<Curve *> self.thisptr).winding(deref(p.thisptr))
    def unitTangentAt(self, Coord t, int n = 3):
        return wrap_Point((<Curve *> self.thisptr).unitTangentAt(t, n))
    def toSBasis(self):
        return wrap_D2_SBasis(self.thisptr.toSBasis())
    def degreesOfFreedom(self):
        return self.thisptr.degreesOfFreedom()

cdef object wrap_D2_SBasis(D2[SBasis] p):
    return ( wrap_SBasis(p[0]), wrap_SBasis(p[1]) )

cdef cy_SBasisCurve wrap_SBasisCurve(SBasisCurve p):
    cdef SBasisCurve * retp = new SBasisCurve(D2[SBasis]( SBasis(), SBasis() ))
    retp[0] = p
    cdef cy_SBasisCurve r = cy_SBasisCurve.__new__(cy_SBasisCurve)
    r.thisptr = retp
    return r



cdef class cy_Bezier:
    cdef Bezier* thisptr
    
    def __cinit__(self, *args):
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

    def __call__(self, double t):
        return deref( self.thisptr ) (t) 

    def __getitem__(self, unsigned int ix):
        return deref( self.thisptr ) [ix]
        
    def order(self):
        return self.thisptr.order()
    def size(self):
        return self.thisptr.size()
#~     def __init__(self, cy_Bezier b):
#~         self.thisptr = new Bezier(deref( b.thisptr ))
#~     def __init__(self, cy_Bezier::Order ord):
#~         self.thisptr = new Bezier(deref( ord.thisptr ))

    def __mul__( cy_Bezier self, double v):
        return wrap_Bezier(deref( self.thisptr ) * v)
    def __add__( cy_Bezier self, double v):
        return wrap_Bezier(deref( self.thisptr ) + v)
    def __sub__( cy_Bezier self, double v):
        return wrap_Bezier(deref( self.thisptr ) - v)
    def __div__( cy_Bezier self, double v):
        return wrap_Bezier(deref( self.thisptr ) / v)

    def resize(self, unsigned int n, Coord v):
        self.thisptr.resize(n, v)
    def clear(self):
        self.thisptr.clear()
    def degree(self):
        return self.thisptr.degree()
    def isZero(self, double eps = EPSILON):
        return self.thisptr.isZero(eps)
    def isConstant(self, double eps = EPSILON):
        return self.thisptr.isConstant(eps)
    def isFinite(self):
        return self.thisptr.isFinite()
    def at0(self):
        return self.thisptr.at0()
    def at1(self):
        return self.thisptr.at1()
    def valueAt(self, double t):
        return self.thisptr.valueAt(t)

    def toSBasis(self):
        return wrap_SBasis(self.thisptr.toSBasis())

    def setPoint(self, unsigned int ix, double val):
        self.thisptr.setPoint(ix, val)
    def valueAndDerivatives(self, Coord t, unsigned int n_derivs):
        return wrap_vector_double(self.thisptr.valueAndDerivatives(t, n_derivs))
    def subdivide(self, Coord t):
        cdef pair[Bezier, Bezier] p = self.thisptr.subdivide(t)
        return ( wrap_Bezier(p.first), wrap_Bezier(p.second) )
    def roots(self, cy_Interval ivl = None):
        if ivl is None:
            return wrap_vector_double(self.thisptr.roots())
        else:
            return wrap_vector_double(self.thisptr.roots(deref( ivl.thisptr )))            
    def forward_difference(self, unsigned int k):
        return wrap_Bezier(self.thisptr.forward_difference(k))
    def elevate_degree(self):
        return wrap_Bezier(self.thisptr.elevate_degree())
    def reduce_degree(self):
        return wrap_Bezier(self.thisptr.reduce_degree())
    def elevate_to_degree(self, unsigned int newDegree):
        return wrap_Bezier(self.thisptr.elevate_to_degree(newDegree))
    def deflate(self):
        return wrap_Bezier(self.thisptr.deflate())
#~     def __init__(self, cy_Coord * c, unsigned int ord):
#~         self.thisptr = new Bezier(c.thisptr ,ord)
#free functions:
#~ def cy_subdivideArr(Coord t, cy_Coord * v, cy_Coord * left, cy_Coord * right, unsigned int order):
#~     return subdivideArr(t, v.thisptr, left.thisptr, right.thisptr, order)
def cy_bezier_to_sbasis(cy_SBasis sb, cy_Bezier bz):
    bezier_to_sbasis(deref( sb.thisptr ), deref( bz.thisptr ))
#~ def cy_bernsteinValueAt(double t, cy_double * c_, unsigned int n):
#~     return bernsteinValueAt(t, c_.thisptr, n)
def cy_bezier_points(cy_Bezier a, cy_Bezier b):
    return wrap_vector_point(bezier_points( D2[Bezier]( deref(a.thisptr), deref(b.thisptr) ) ))

cdef cy_Bezier wrap_Bezier(Bezier p):
    cdef Bezier * retp = new Bezier()
    retp[0] = p
    cdef cy_Bezier r = cy_Bezier.__new__(cy_Bezier)
    r.thisptr = retp
    return r


cdef class cy_BezierCurve:
    cdef BezierCurve* thisptr
    def __cinit__(self, *args):
#~         if len(args) == 0:
#~             self.thisptr = new BezierCurve(0)
        if len(args) == 1:
            if all( map(lambda x: isinstance(x, cy_Point), args[0]) ):
                self.thisptr = create(make_vector_point(args[0]))
#~         elif len(args) == 2:
#~             if isinstance(args[0], cy_Bezier) and isinstance(args[1], cy_Bezier):
#~                 self.thisptr = new BezierCurve( D2[Bezier](deref( (<cy_Bezier> args[0]).thisptr ), deref( (<cy_Bezier> args[1]).thisptr ) ) )
    def __call__(self, Coord t):
        return wrap_Point(deref( <Curve *> self.thisptr )(t))
    def order(self):
        return self.thisptr.order()
    def points(self):
        return wrap_vector_point(self.thisptr.points())
    def setPoint(self, unsigned int ix, cy_Point v):
        self.thisptr.setPoint(ix, deref( v.thisptr ))
    def setPoints(self,  ps):
        self.thisptr.setPoints( make_vector_point(ps) )
    def __getitem__(self, unsigned int ix):
        return wrap_Point(deref( self.thisptr ) [ix])
    @classmethod
    def create(cls,  pts):
        return wrap_BezierCurve( deref( create( make_vector_point(pts) ) ) )
    def initialPoint(self):
        return wrap_Point(self.thisptr.initialPoint())
    def finalPoint(self):
        return wrap_Point(self.thisptr.finalPoint())
    def isDegenerate(self):
        return self.thisptr.isDegenerate()
    def setInitial(self, cy_Point v):
        self.thisptr.setInitial(deref( v.thisptr ))
    def setFinal(self, cy_Point v):
        self.thisptr.setFinal(deref( v.thisptr ))
    def boundsFast(self):
        return wrap_Rect(self.thisptr.boundsFast())
    def boundsExact(self):
        return wrap_Rect(self.thisptr.boundsExact())
    def boundsLocal(cy_BezierCurve self, cy_OptInterval i, unsigned int deg):
        return wrap_OptRect(self.thisptr.boundsLocal(deref( i.thisptr ), deg))
    def nearestPoint(self, cy_Point p, *args):
        if len(args) == 0:
            return (<Curve *> self.thisptr).nearestPoint(deref( p.thisptr ), 0, 1)
        elif len(args) == 1:
            if isinstance(args[0], cy_Interval):
                return (<Curve *> self.thisptr).nearestPoint(deref( p.thisptr ), deref( (<cy_Interval> args[0]).thisptr ) )
        elif len(args) == 2:
            return (<Curve *> self.thisptr).nearestPoint(deref( p.thisptr ), float(args[0]), float(args[1]))
    def duplicate(self):
        return wrap_BezierCurve( deref( <BezierCurve *>  self.thisptr.duplicate()))
    def portion(self, *args):
        if len(args) == 1:
            if isinstance(args[0], cy_Interval):
                #TODO: break this down somehow
                return wrap_BezierCurve( deref( <BezierCurve *> (<Curve *> self.thisptr).portion( deref( (<cy_Interval> args[0]).thisptr ))) )
        elif len(args) == 2:
            return wrap_BezierCurve( deref( <BezierCurve *> self.thisptr.portion( float(args[0]), float(args[1]) ) ) ) 
    def allNearestPoints(self, cy_Point p, *args):
        if len(args) == 0:
            return wrap_vector_double((<Curve *> self.thisptr).allNearestPoints(deref( p.thisptr ), 0, 1))
        elif len(args) == 1:
            if isinstance(args[0], cy_Interval):
                return wrap_vector_double((<Curve *> self.thisptr).allNearestPoints(deref( p.thisptr ), deref( (<cy_Interval> args[0]).thisptr ) ))
        elif len(args) == 2:
            return wrap_vector_double( (<Curve *> self.thisptr).allNearestPoints(deref( p.thisptr ), float(args[0]), float(args[1])))


    def reverse(self):
        return wrap_BezierCurve( deref( <BezierCurve *> self.thisptr.reverse()))
    def transformed(self, t):
        cdef Affine at
        if is_transform(t): 
            at = get_Affine(t)
            return wrap_BezierCurve( deref( <BezierCurve *> self.thisptr.transformed( at )))
    def derivative(self):
        return wrap_BezierCurve( deref( <BezierCurve *> self.thisptr.derivative()))
    def degreesOfFreedom(self):
        return self.thisptr.degreesOfFreedom()
    def roots(self, Coord v, Dim2 d):
        return wrap_vector_double(self.thisptr.roots(v, d))
    def length(self, Coord tolerance = 0.01):
        return self.thisptr.length(tolerance)
    def pointAt(self, Coord t):
        return wrap_Point(self.thisptr.pointAt(t))
    def pointAndDerivatives(self, Coord t, unsigned int n):
        return wrap_vector_point(self.thisptr.pointAndDerivatives(t, n))
    def valueAt(self, Coord t, Dim2 d):
        return self.thisptr.valueAt(t, d)
    def toSBasis(self):
        return wrap_D2_SBasis(self.thisptr.toSBasis())
    def winding(self, cy_Point p):
        return (<Curve *> self.thisptr).winding(deref(p.thisptr))
    def unitTangentAt(self, Coord t, int n = 3):
        return wrap_Point((<Curve *> self.thisptr).unitTangentAt(t, n))

cdef cy_BezierCurve wrap_BezierCurve(BezierCurve p):
    cdef vector[Point] points = make_vector_point([cy_Point(), cy_Point()])
    cdef BezierCurve * retp = create(p.points())
#~     retp.setPoints(p.points())
#~     retp[0] = p
    cdef cy_BezierCurve r = cy_BezierCurve.__new__(cy_BezierCurve, [cy_Point(), cy_Point()])
    r.thisptr = retp
    return r
    
    
cdef class cy_LineSegment(cy_BezierCurve):
    def __cinit__(self, *args):
    #TODO: find out how this works
#~         del self.thisptr
        if len(args) == 0:
            self.thisptr = <BezierCurve *> new LineSegment()
        if len(args) == 2:
            if isinstance(args[0], cy_Bezier) and isinstance(args[1], cy_Bezier):
                #TODO: check for arguments order
                self.thisptr = <BezierCurve *> new LineSegment( deref( (<cy_Bezier> args[0]).thisptr ), 
                                                                deref( (<cy_Bezier> args[1]).thisptr ) )
            elif isinstance(args[0], cy_Point) and isinstance(args[1], cy_Point):
                self.thisptr = <BezierCurve *> new LineSegment( deref( (<cy_Point> args[0]).thisptr ), 
                                                                deref( (<cy_Point> args[1]).thisptr ) )
    def subdivide(self, Coord t):
        cdef pair[LineSegment, LineSegment] p = (<LineSegment *> self.thisptr).subdivide(t)
        return ( wrap_LineSegment(p.first), wrap_LineSegment(p.second) )#~ 

    def duplicate(self):
        return wrap_LineSegment( deref( <LineSegment *>  self.thisptr.duplicate()))
    def portion(self, *args):
        if len(args) == 1:
            if isinstance(args[0], cy_Interval):
                #TODO: break this down somehow
                return wrap_LineSegment( deref( <LineSegment *> (<Curve *> self.thisptr).portion( deref( (<cy_Interval> args[0]).thisptr ))) )
        elif len(args) == 2:
            return wrap_LineSegment( deref( <LineSegment *> self.thisptr.portion( float(args[0]), float(args[1]) ) ) ) 

    def reverse(self):
        return wrap_LineSegment( deref( <LineSegment *> self.thisptr.reverse()))
    def transformed(self, t):
        cdef Affine at
        if is_transform(t): 
            at = get_Affine(t)
            return wrap_LineSegment( deref( <LineSegment *> self.thisptr.transformed( at )))
    def derivative(self):
        return wrap_LineSegment( deref( <LineSegment *> self.thisptr.derivative()))

cdef cy_LineSegment wrap_LineSegment(LineSegment p):
#~     cdef vector[Point] points = make_vector_point()
    cdef LineSegment * retp = new LineSegment()
#~     retp.setPoints(p.points())
    retp[0] = p
    cdef cy_LineSegment r = cy_LineSegment.__new__(cy_LineSegment)
    r.thisptr = <BezierCurve* > retp
    return r
    
cdef class cy_QuadraticBezier(cy_BezierCurve):
    def __cinit__(self, *args):
    #TODO: find out how this works
#~         del self.thisptr
        if len(args) == 0:
            self.thisptr = <BezierCurve *> new QuadraticBezier()
        elif len(args) == 2:
            if isinstance(args[0], cy_Bezier) and isinstance(args[1], cy_Bezier):
                self.thisptr = <BezierCurve *> new QuadraticBezier( deref( (<cy_Bezier> args[0]).thisptr ), 
                                                                    deref( (<cy_Bezier> args[1]).thisptr ) )
        elif len(args) == 3:
            if isinstance(args[0], cy_Point) and isinstance(args[1], cy_Point) and isinstance(args[2], cy_Point):
                self.thisptr = <BezierCurve *> new QuadraticBezier( deref( (<cy_Point> args[0]).thisptr ), 
                                                                    deref( (<cy_Point> args[1]).thisptr ),
                                                                    deref( (<cy_Point> args[2]).thisptr ) )
    def subdivide(self, Coord t):
        cdef pair[QuadraticBezier, QuadraticBezier] p = (<QuadraticBezier *> self.thisptr).subdivide(t)
        return ( wrap_QuadraticBezier(p.first), wrap_QuadraticBezier(p.second) )#~ 

    def duplicate(self):
        return wrap_QuadraticBezier( deref( <QuadraticBezier *>  self.thisptr.duplicate()))
    def portion(self, *args):
        if len(args) == 1:
            if isinstance(args[0], cy_Interval):
                #TODO: break this down somehow
                return wrap_QuadraticBezier( deref( <QuadraticBezier *> (<Curve *> self.thisptr).portion( deref( (<cy_Interval> args[0]).thisptr ))) )
        elif len(args) == 2:
            return wrap_QuadraticBezier( deref( <QuadraticBezier *> self.thisptr.portion( float(args[0]), float(args[1]) ) ) ) 

    def reverse(self):
        return wrap_QuadraticBezier( deref( <QuadraticBezier *> self.thisptr.reverse()))
    def transformed(self, t):
        cdef Affine at
        if is_transform(t): 
            at = get_Affine(t)
            return wrap_QuadraticBezier( deref( <QuadraticBezier *> self.thisptr.transformed( at )))
    def derivative(self):
        return wrap_LineSegment( deref( <LineSegment *> self.thisptr.derivative()))

cdef cy_QuadraticBezier wrap_QuadraticBezier(QuadraticBezier p):
#~     cdef vector[Point] points = make_vector_point()
    cdef QuadraticBezier * retp = new QuadraticBezier()
#~     retp.setPoints(p.points())
    retp[0] = p
    cdef cy_QuadraticBezier r = cy_QuadraticBezier.__new__(cy_QuadraticBezier)
    r.thisptr = <BezierCurve* > retp
    return r
    
cdef class cy_CubicBezier(cy_BezierCurve):
    def __cinit__(self, *args):
    #TODO: find out how this works
#~         del self.thisptr
        if len(args) == 0:
            self.thisptr = <BezierCurve *> new CubicBezier()
        elif len(args) == 2:
            if isinstance(args[0], cy_Bezier) and isinstance(args[1], cy_Bezier):
                self.thisptr = <BezierCurve *> new CubicBezier( deref( (<cy_Bezier> args[0]).thisptr ), 
                                                                    deref( (<cy_Bezier> args[1]).thisptr ) )
        elif len(args) == 4:
            if isinstance(args[0], cy_Point) and isinstance(args[1], cy_Point) and isinstance(args[2], cy_Point) and isinstance(args[3], cy_Point):
                self.thisptr = <BezierCurve *> new CubicBezier( deref( (<cy_Point> args[0]).thisptr ), 
                                                                    deref( (<cy_Point> args[1]).thisptr ),
                                                                    deref( (<cy_Point> args[2]).thisptr ),
                                                                    deref( (<cy_Point> args[3]).thisptr ) )
    def subdivide(self, Coord t):
        cdef pair[CubicBezier, CubicBezier] p = (<CubicBezier *> self.thisptr).subdivide(t)
        return ( wrap_CubicBezier(p.first), wrap_CubicBezier(p.second) )#~ 

    def duplicate(self):
        return wrap_CubicBezier( deref( <CubicBezier *>  self.thisptr.duplicate()))
    def portion(self, *args):
        if len(args) == 1:
            if isinstance(args[0], cy_Interval):
                #TODO: break this down somehow
                return wrap_CubicBezier( deref( <CubicBezier *> (<Curve *> self.thisptr).portion( deref( (<cy_Interval> args[0]).thisptr ))) )
        elif len(args) == 2:
            return wrap_CubicBezier( deref( <CubicBezier *> self.thisptr.portion( float(args[0]), float(args[1]) ) ) ) 

    def reverse(self):
        return wrap_CubicBezier( deref( <CubicBezier *> self.thisptr.reverse()))
    def transformed(self, t):
        cdef Affine at
        if is_transform(t): 
            at = get_Affine(t)
            return wrap_CubicBezier( deref( <CubicBezier *> self.thisptr.transformed( at )))
    def derivative(self):
        return wrap_QuadraticBezier( deref( <QuadraticBezier *> self.thisptr.derivative()))

cdef cy_CubicBezier wrap_CubicBezier(CubicBezier p):
#~     cdef vector[Point] points = make_vector_point()
    cdef CubicBezier * retp = new CubicBezier()
#~     retp.setPoints(p.points())
    retp[0] = p
    cdef cy_CubicBezier r = cy_CubicBezier.__new__(cy_CubicBezier)
    r.thisptr = <BezierCurve* > retp
    return r
    
#~ cdef class cy_BezierCurveN(cy_BezierCurve):
#~     def __cinit__(self, *args):
#~         if len(args) == 2:
#~             if isinstance(args[0], cy_Bezier) and isinstance(args[1], cy_Bezier):
#~                 self.thisptr = <BezierCurve *> new BezierCurveN[n_1]( deref( (<cy_Bezier> args[0]).thisptr ), 
#~                                                                     deref( (<cy_Bezier> args[1]).thisptr ) )
#~             
#~ cdef class cy_BezierCurveN:
#~     cdef BezierCurveN * thisptr1



cdef class cy_HLineSegment(cy_LineSegment):
    def __cinit__(self, *args):
        if len(args) == 0:
            self.thisptr = <BezierCurve *> new HLineSegment()
        elif len(args) == 2:
            if isinstance(args[0], cy_Point) and isinstance(args[1], cy_Point):
                self.thisptr = <BezierCurve *> new HLineSegment( deref( (<cy_Point> args[0]).thisptr ),
                                                                 deref( (<cy_Point> args[1]).thisptr ) )
            elif isinstance(args[0], cy_Point) and isinstance(args[1], Number):
                self.thisptr = <BezierCurve *> new HLineSegment( deref( (<cy_Point> args[0]).thisptr ),
                                                                 float(args[1]) )
        elif len(args) == 3:
            self.thisptr = <BezierCurve *> new HLineSegment( float(args[0]), float(args[1]), float(args[2]) )

    def setInitial(self, cy_Point p):
        (<AxisLineSegment_X *> self.thisptr).setInitial( deref(p.thisptr) )
    def setFinal(self, cy_Point p):
        (<AxisLineSegment_X *> self.thisptr).setFinal( deref(p.thisptr) )
    def boundsFast(self):
        return wrap_Rect( (<AxisLineSegment_X *> self.thisptr).boundsFast() )
    def boundsExact(self):
        return wrap_Rect( (<AxisLineSegment_X *> self.thisptr).boundsExact() )
    def degreesOfFreedom(self):
        return (<AxisLineSegment_X *> self.thisptr).degreesOfFreedom()
    def roots(self, Coord v, Dim2 d):
        return wrap_vector_double( (<AxisLineSegment_X *> self.thisptr).roots(v, d) )
    def nearestPoint(self, cy_Point p, *args):
        if len(args) == 0:
            return (<AxisLineSegment_X *> self.thisptr).nearestPoint(deref( p.thisptr ), 0, 1)
        elif len(args) == 1:
            if isinstance(args[0], cy_Interval):
                return (<Curve *> self.thisptr).nearestPoint(deref( p.thisptr ), deref( (<cy_Interval> args[0]).thisptr ) )
        elif len(args) == 2:
            return (<AxisLineSegment_X *> self.thisptr).nearestPoint(deref( p.thisptr ), float(args[0]), float(args[1]))
    def pointAt(self, Coord t):
        return wrap_Point((<AxisLineSegment_X *> self.thisptr).pointAt(t))
    def valueAt(self, Coord t, Dim2 d):
        return (<AxisLineSegment_X *> self.thisptr).valueAt(t, d)
    def pointAndDerivatives(self, Coord t, unsigned n):
        return wrap_vector_point( (<AxisLineSegment_X *> self.thisptr).pointAndDerivatives(t, n) )
    
    def getY(self):
        return (<HLineSegment *> self.thisptr).getY()
    def setInitialX(self, Coord x):
        (<HLineSegment *> self.thisptr).setInitialX(x)
    def setFinalX(self, Coord x):
        (<HLineSegment *> self.thisptr).setFinalX(x)
    def setY(self, Coord y):
        (<HLineSegment *> self.thisptr).setY(y)
    def subdivide(self, Coord t):
        cdef pair[HLineSegment, HLineSegment] p = (<HLineSegment *> self.thisptr).subdivide(t)
        return (wrap_HLineSegment(p.first), wrap_HLineSegment(p.second))
    def duplicate(self):
        return wrap_HLineSegment( deref(<HLineSegment *> self.thisptr.duplicate()) )
    def portion(self, *args):
        if len(args) == 1:
            if isinstance(args[0], cy_Interval):
                return wrap_HLineSegment( deref( <HLineSegment *> (<Curve *> self.thisptr).portion( deref( (<cy_Interval> args[0]).thisptr ) ) ) )
        elif len(args) == 2:
            return wrap_HLineSegment( deref( <HLineSegment *> self.thisptr.portion( float(args[0]), float(args[1]) ) ) ) 
    def reverse(self):
        return wrap_HLineSegment( deref(<HLineSegment *> self.thisptr.reverse()) )
    def transformed(self, t):
        cdef Affine at
        if is_transform(t): 
            at = get_Affine(t)
            return wrap_LineSegment( deref(<LineSegment *> self.thisptr.transformed( at )) )
    def derivative(self):
        return wrap_HLineSegment( deref(<HLineSegment *> self.thisptr.derivative()) )

cdef cy_HLineSegment wrap_HLineSegment(HLineSegment p):
    cdef HLineSegment * retp = new HLineSegment()
    retp[0] = p
    cdef cy_HLineSegment r = cy_HLineSegment.__new__(cy_HLineSegment)
    r.thisptr = <BezierCurve *> retp
    return r

cdef class cy_VLineSegment(cy_LineSegment):
    def __cinit__(self, *args):
        if len(args) == 0:
            self.thisptr = <BezierCurve *> new VLineSegment()
        elif len(args) == 2:
            if isinstance(args[0], cy_Point) and isinstance(args[1], cy_Point):
                self.thisptr = <BezierCurve *> new VLineSegment( deref( (<cy_Point> args[0]).thisptr ),
                                                                 deref( (<cy_Point> args[1]).thisptr ) )
            elif isinstance(args[0], cy_Point) and isinstance(args[1], Number):
                self.thisptr = <BezierCurve *> new VLineSegment( deref( (<cy_Point> args[0]).thisptr ),
                                                                 float(args[1]) )
        elif len(args) == 3:
            self.thisptr = <BezierCurve *> new VLineSegment( float(args[0]), float(args[1]), float(args[2]) )

    def setInitial(self, cy_Point p):
        (<AxisLineSegment_Y *> self.thisptr).setInitial( deref(p.thisptr) )
    def setFinal(self, cy_Point p):
        (<AxisLineSegment_Y *> self.thisptr).setFinal( deref(p.thisptr) )
    def boundsFast(self):
        return wrap_Rect( (<AxisLineSegment_Y *> self.thisptr).boundsFast() )
    def boundsExact(self):
        return wrap_Rect( (<AxisLineSegment_Y *> self.thisptr).boundsExact() )
    def degreesOfFreedom(self):
        return (<AxisLineSegment_Y *> self.thisptr).degreesOfFreedom()
    def roots(self, Coord v, Dim2 d):
        return wrap_vector_double( (<AxisLineSegment_Y *> self.thisptr).roots(v, d) )
    def nearestPoint(self, cy_Point p, *args):
        if len(args) == 0:
            return (<AxisLineSegment_Y *> self.thisptr).nearestPoint(deref( p.thisptr ), 0, 1)
        elif len(args) == 1:
            if isinstance(args[0], cy_Interval):
                return (<Curve *> self.thisptr).nearestPoint(deref( p.thisptr ), deref( (<cy_Interval> args[0]).thisptr ) )
        elif len(args) == 2:
            return (<AxisLineSegment_Y *> self.thisptr).nearestPoint(deref( p.thisptr ), float(args[0]), float(args[1]))
    def pointAt(self, Coord t):
        return wrap_Point((<AxisLineSegment_Y *> self.thisptr).pointAt(t))
    def valueAt(self, Coord t, Dim2 d):
        return (<AxisLineSegment_Y *> self.thisptr).valueAt(t, d)
    def pointAndDerivatives(self, Coord t, unsigned n):
        return wrap_vector_point( (<AxisLineSegment_Y *> self.thisptr).pointAndDerivatives(t, n) )
    
    def getX(self):
        return (<VLineSegment *> self.thisptr).getX()
    def setInitialY(self, Coord y):
        (<VLineSegment *> self.thisptr).setInitialY(y)
    def setFinalY(self, Coord y):
        (<VLineSegment *> self.thisptr).setFinalY(y)
    def setX(self, Coord x):
        (<VLineSegment *> self.thisptr).setX(x)
    def subdivide(self, Coord t):
        cdef pair[VLineSegment, VLineSegment] p = (<VLineSegment *> self.thisptr).subdivide(t)
        return (wrap_VLineSegment(p.first), wrap_VLineSegment(p.second))
    def duplicate(self):
        return wrap_VLineSegment( deref(<VLineSegment *> self.thisptr.duplicate()) )
    def portion(self, *args):
        if len(args) == 1:
            if isinstance(args[0], cy_Interval):
                return wrap_VLineSegment( deref( <VLineSegment *> (<Curve *> self.thisptr).portion( deref( (<cy_Interval> args[0]).thisptr ) ) ) )
        elif len(args) == 2:
            return wrap_VLineSegment( deref( <VLineSegment *> self.thisptr.portion( float(args[0]), float(args[1]) ) ) ) 
    def reverse(self):
        return wrap_VLineSegment( deref(<VLineSegment *> self.thisptr.reverse()) )
    def transformed(self, t):
        cdef Affine at
        if is_transform(t): 
            at = get_Affine(t)
            return wrap_LineSegment( deref(<LineSegment *> self.thisptr.transformed( at )) )
    def derivative(self):
        return wrap_VLineSegment( deref(<VLineSegment *> self.thisptr.derivative()) )

cdef cy_VLineSegment wrap_VLineSegment(VLineSegment p):
    cdef VLineSegment * retp = new VLineSegment()
    retp[0] = p
    cdef cy_VLineSegment r = cy_VLineSegment.__new__(cy_VLineSegment)
    r.thisptr = <BezierCurve *> retp
    return r

cdef class cy_EllipticalArc:
    cdef EllipticalArc* thisptr
    def __cinit__(self, cy_Point ip = cy_Point(0, 0), 
                        Coord rx = 0, 
                        Coord ry = 0, 
                        Coord rot_angle = 0, 
                        bint large_arc = True, 
                        bint sweep = True, 
                        cy_Point fp = cy_Point(0, 0)):
        self.thisptr = new EllipticalArc(deref( ip.thisptr ) ,rx ,ry ,rot_angle ,large_arc ,sweep ,deref( fp.thisptr ))
    def __call__(self, Coord t):
        return wrap_Point( deref(<Curve *> self.thisptr)(t) )
    #Curve methods
    def length(self, Coord tolerance = 0.01):
        return (<Curve *> self.thisptr).length(tolerance)

    #AngleInterval methods
    def initialAngle(self):
        return wrap_Angle((<AngleInterval *> self.thisptr).initialAngle())
    def finalAngle(self):
        return wrap_Angle((<AngleInterval *> self.thisptr).finalAngle())
    def angleAt(self, Coord t):
        return wrap_Angle((<AngleInterval *> self.thisptr).angleAt(t))
    def contains(self, cy_Angle a):
        return (<AngleInterval *> self.thisptr).contains(deref( a.thisptr ))
    def extent(self):
        return (<AngleInterval *> self.thisptr).extent()

    def angleInterval(self):
        return wrap_Interval(self.thisptr.angleInterval())
    def rotationAngle(self):
        return wrap_Angle(self.thisptr.rotationAngle())
    def ray(self, Dim2 d):
        return self.thisptr.ray(d)
    def rays(self):
        return wrap_Point(self.thisptr.rays())
    def largeArc(self):
        return self.thisptr.largeArc()
    def sweep(self):
        return self.thisptr.sweep()
    def chord(self):
        return wrap_LineSegment(self.thisptr.chord())
    def set(self, cy_Point ip, double rx, double ry, double rot_angle, bint large_arc, bint sweep, cy_Point fp):
        self.thisptr.set(deref( ip.thisptr ), rx, ry, rot_angle, large_arc, sweep, deref( fp.thisptr ))
    def setExtremes(self, cy_Point ip, cy_Point fp):
        self.thisptr.setExtremes(deref( ip.thisptr ), deref( fp.thisptr ))
    def center(self, *args):
        if len(args) == 0:
            return wrap_Point(self.thisptr.center())
        else:
            return self.thisptr.center(int(args[0]))
    def sweepAngle(self):
        return self.thisptr.sweepAngle()
    def containsAngle(self, Coord angle):
        return self.thisptr.containsAngle(angle)
    def pointAtAngle(self, Coord t):
        return wrap_Point(self.thisptr.pointAtAngle(t))
    def valueAtAngle(self, Coord t, Dim2 d):
        return self.thisptr.valueAtAngle(t, d)
    def unitCircleTransform(self):
        return wrap_Affine(self.thisptr.unitCircleTransform())
    def isSVGCompliant(self):
        return self.thisptr.isSVGCompliant()
    def subdivide(self, Coord t):
        cdef pair[EllipticalArc, EllipticalArc] r = self.thisptr.subdivide(t)
        return (wrap_EllipticalArc(r.first), wrap_EllipticalArc(r.second))
    def initialPoint(self):
        return wrap_Point(self.thisptr.initialPoint())
    def finalPoint(self):
        return wrap_Point(self.thisptr.finalPoint())
    def duplicate(self):
        return wrap_EllipticalArc( deref(<EllipticalArc *> self.thisptr.duplicate()) )
    def setInitial(self, cy_Point p):
        self.thisptr.setInitial(deref( p.thisptr ))
    def setFinal(self, cy_Point p):
        self.thisptr.setFinal(deref( p.thisptr ))
    def isDegenerate(self):
        return self.thisptr.isDegenerate()
    def boundsFast(self):
        return wrap_Rect(self.thisptr.boundsFast())
    def boundsExact(self):
        return wrap_Rect(self.thisptr.boundsExact())
    def boundsLocal(self, cy_OptInterval i, unsigned int deg):
        return wrap_OptRect(self.thisptr.boundsLocal(deref( i.thisptr ), deg))
    def roots(self, double v, Dim2 d):
        return wrap_vector_double(self.thisptr.roots(v, d))
    def nearestPoint(self, cy_Point p, *args):
        if len(args) == 0:
            return self.thisptr.nearestPoint(deref( p.thisptr ), 0, 1)
        elif len(args) == 1:
            if isinstance(args[0], cy_Interval):
                return (<Curve *> self.thisptr).nearestPoint(deref( p.thisptr ), deref( (<cy_Interval> args[0]).thisptr ) )
        elif len(args) == 2:
            return self.thisptr.nearestPoint(deref( p.thisptr ), float(args[0]), float(args[1]))
    def allNearestPoints(self, cy_Point p, *args):
        if len(args) == 0:
            return wrap_vector_double((<Curve *> self.thisptr).allNearestPoints(deref( p.thisptr ), 0, 1))
        elif len(args) == 1:
            if isinstance(args[0], cy_Interval):
                return wrap_vector_double((<Curve *> self.thisptr).allNearestPoints(deref( p.thisptr ), deref( (<cy_Interval> args[0]).thisptr ) ))
        elif len(args) == 2:
            return wrap_vector_double( (<Curve *> self.thisptr).allNearestPoints(deref( p.thisptr ), float(args[0]), float(args[1])))
    def degreesOfFreedom(self):
        return self.thisptr.degreesOfFreedom()
    def derivative(self):
        return wrap_EllipticalArc( deref(<EllipticalArc *> self.thisptr.derivative()) )
    def transformed(self, cy_Affine m):
        return wrap_EllipticalArc( deref(<EllipticalArc *> self.thisptr.transformed(deref( m.thisptr ))) )
    def pointAndDerivatives(self, Coord t, unsigned int n):
        return wrap_vector_point(self.thisptr.pointAndDerivatives(t, n))
    def toSBasis(self):
        cdef D2[SBasis] r = self.thisptr.toSBasis()
        return ( wrap_SBasis(r[0]), wrap_SBasis(r[1]) )
    def valueAt(self, Coord t, Dim2 d):
        return self.thisptr.valueAt(t, d)
    def pointAt(self, Coord t):
        return wrap_Point(self.thisptr.pointAt(t))
    
    def portion(self, *args):
        if len(args) == 1:
            if isinstance(args[0], cy_Interval):
                return wrap_EllipticalArc( deref( <EllipticalArc *> (<Curve *> self.thisptr).portion( deref( (<cy_Interval> args[0]).thisptr ) ) ) )
        elif len(args) == 2:
            return wrap_EllipticalArc( deref( <EllipticalArc *> self.thisptr.portion( float(args[0]), float(args[1]) ) ) ) 
    
    def reverse(self):
        return wrap_EllipticalArc( deref(<EllipticalArc *> self.thisptr.reverse()) )
    def winding(self, cy_Point p):
        return (<Curve *> self.thisptr).winding(deref(p.thisptr))
    def unitTangentAt(self, Coord t, int n = 3):
        return wrap_Point((<Curve *> self.thisptr).unitTangentAt(t, n))

cdef cy_EllipticalArc wrap_EllipticalArc(EllipticalArc p):
    cdef EllipticalArc * retp = new EllipticalArc()
    retp[0] = p
    cdef cy_EllipticalArc r = cy_EllipticalArc.__new__(cy_EllipticalArc)
    r.thisptr = retp
    return r
    
    
cdef class cy_SVGEllipticalArc(cy_EllipticalArc):
    def __cinit__(self, cy_Point ip = cy_Point(0, 0), 
                        Coord rx = 0, 
                        Coord ry = 0, 
                        Coord rot_angle = 0, 
                        bint large_arc = True, 
                        bint sweep = True, 
                        cy_Point fp = cy_Point(0, 0)):
        self.thisptr = <EllipticalArc *> new SVGEllipticalArc(deref( ip.thisptr ), rx, ry, rot_angle, large_arc, sweep, deref( fp.thisptr ))
#~     def __call__(self, Coord t):
#~         return self.pointAt(t)
    def valueAt(self, Coord t, Dim2 d):
        return (<SVGEllipticalArc *> self.thisptr).valueAt(t, d)
    def pointAt(self, Coord t):
        return wrap_Point((<SVGEllipticalArc *> self.thisptr).pointAt(t))    
    def pointAndDerivatives(self, Coord t, unsigned int n):
        return wrap_vector_point((<SVGEllipticalArc *> self.thisptr).pointAndDerivatives(t, n))    
    def boundsExact(self):
        return wrap_Rect((<SVGEllipticalArc *> self.thisptr).boundsExact())
    def boundsLocal(self, cy_OptInterval i, unsigned int deg):
        return wrap_OptRect((<SVGEllipticalArc *> self.thisptr).boundsLocal(deref( i.thisptr ), deg))
    def derivative(self):
        if self.isDegenerate():
            return wrap_LineSegment( deref(<LineSegment *> (<SVGEllipticalArc *> self.thisptr).derivative())  )
        return wrap_EllipticalArc( deref(<EllipticalArc *> (<SVGEllipticalArc *> self.thisptr).derivative()) )
    def duplicate(self):
        return wrap_SVGEllipticalArc( deref(<SVGEllipticalArc *> (<SVGEllipticalArc *> self.thisptr).duplicate()) )
    def roots(self, double v, Dim2 d):
        return wrap_vector_double((<SVGEllipticalArc *> self.thisptr).roots(v, d))
    def allNearestPoints(self, cy_Point p, *args):
        if len(args) == 0:
            return wrap_vector_double((<SVGEllipticalArc *> self.thisptr).allNearestPoints(deref( p.thisptr ), 0, 1))
        elif len(args) == 1:
            if isinstance(args[0], cy_Interval):
                return wrap_vector_double((<Curve *> self.thisptr).allNearestPoints(deref( p.thisptr ), deref( (<cy_Interval> args[0]).thisptr ) ))
        elif len(args) == 2:
            return wrap_vector_double( (<SVGEllipticalArc *> self.thisptr).allNearestPoints(deref( p.thisptr ), float(args[0]), float(args[1])))
    def toSBasis(self):
        cdef D2[SBasis] r = (<SVGEllipticalArc *> self.thisptr).toSBasis()
        return ( wrap_SBasis(r[0]), wrap_SBasis(r[1]) ) 
    def isSVGCompliant(self):
        return (<SVGEllipticalArc *> self.thisptr).isSVGCompliant()
        
cdef cy_SVGEllipticalArc wrap_SVGEllipticalArc(SVGEllipticalArc p):
    cdef SVGEllipticalArc * retp = new SVGEllipticalArc()
    retp[0] = p
    cdef cy_SVGEllipticalArc r = cy_SVGEllipticalArc.__new__(cy_SVGEllipticalArc)
    r.thisptr = <EllipticalArc *> retp
    return r

#TODO move somewhere else

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
        
