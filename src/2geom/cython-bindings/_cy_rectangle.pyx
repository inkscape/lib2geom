from cython.operator cimport dereference as deref
from _common_decl cimport *

from _cy_affine cimport cy_Affine, get_Affine, is_transform

from numbers import Number

cdef class cy_GenericInterval:
    cdef GenericInterval[WrappedPyObject]* thisptr
    def __cinit__(self, u = 0, v = None):
        if v is None:
            self.thisptr = new GenericInterval[WrappedPyObject]( WrappedPyObject(u) )
        else:
            self.thisptr = new GenericInterval[WrappedPyObject]( WrappedPyObject(u), WrappedPyObject(v) )
    @classmethod
    def from_Interval(self, i):
        return cy_GenericInterval( i.min(), i.max() )
    @classmethod
    def from_list(self, lst):
        if len(lst) == 0:
            return cy_GenericInterval()
        ret = cy_GenericInterval(lst[0])
        for i in lst[1:]:
            ret.expandTo(i)
        return ret

    def __str__(self):
        return "[{}, {}]".format(self.min(), self.max())
    def __repr__(self):
        if self.isSingular():
            return "GenericInterval({})".format( str(self.min()) )
        return "GenericInterval({}, {})".format( str(self.min()) , str(self.max()) )

    def min(self):
        return self.thisptr.min().getObj()
    def max(self):
        return self.thisptr.max().getObj()
    def extent(self):
        return self.thisptr.extent().getObj()
    def middle(self):
        return self.thisptr.middle().getObj()

    def isSingular(self):
        return self.thisptr.isSingular()

    def setMin(self, val):
        self.thisptr.setMin( WrappedPyObject(val) )
    def setMax(self, val):
        self.thisptr.setMax( WrappedPyObject(val) )
    def expandTo(self, val):
        self.thisptr.expandTo( WrappedPyObject(val) )
    def expandBy(self, val):
        self.thisptr.expandBy( WrappedPyObject(val) )
    def unionWith(self, cy_GenericInterval interval):
        self.thisptr.unionWith( deref(interval.thisptr) )

    def contains(self, other):
        if isinstance(other, cy_GenericInterval):
            return self.thisptr.contains(  deref( (<cy_GenericInterval> other).thisptr) )
        else:
            return self.thisptr.contains( WrappedPyObject(other) )
    def intersects(self, cy_GenericInterval other):
        return self.thisptr.intersects(deref( other.thisptr ))

    def __neg__(self):
        return wrap_GenericInterval(-deref(self.thisptr))

    def _add_pyobj(self, X):
        return wrap_GenericInterval(deref(self.thisptr) + WrappedPyObject(X) )
    def _sub_pyobj(self, X):
        return wrap_GenericInterval(deref(self.thisptr) - WrappedPyObject(X) )

    def _add_interval(self, cy_GenericInterval I):
        return wrap_GenericInterval(deref(self.thisptr)+deref(I.thisptr))
    def _sub_interval(self, cy_GenericInterval I):
        return wrap_GenericInterval(deref(self.thisptr)-deref(I.thisptr))

    def __add__(cy_GenericInterval self, other):
        if isinstance(other, cy_GenericInterval):
            return self._add_interval(other)
        else:
            return self._add_pyobj(other)

    def __sub__(cy_GenericInterval self, other):
        if isinstance(other, cy_GenericInterval):
            return self._sub_interval(other)
        else:
            return self._sub_pyobj(other)

    def __or__(cy_GenericInterval self, cy_GenericInterval I):
        return wrap_GenericInterval(deref(self.thisptr)|deref(I.thisptr))

    def _eq(self, cy_GenericInterval other):
        return deref(self.thisptr)==deref(other.thisptr)
    def _neq(self, cy_GenericInterval other):
        return deref(self.thisptr)!=deref(other.thisptr)

    def __richcmp__(cy_GenericInterval self, other, op):
        if op == 2:
            return self._eq(other)
        elif op == 3:
            return self._neq(other)

cdef cy_GenericInterval wrap_GenericInterval(GenericInterval[WrappedPyObject] p):
    cdef GenericInterval[WrappedPyObject] * retp = new GenericInterval[WrappedPyObject](WrappedPyObject(0))
    retp[0] = p
    cdef cy_GenericInterval r = cy_GenericInterval.__new__(
                                        cy_GenericInterval, 0, 0)
    r.thisptr = retp
    return r

cdef class cy_GenericOptInterval:
    cdef GenericOptInterval[WrappedPyObject]* thisptr
    def __cinit__(self, u = None, v = None):
        if u is None:
            self.thisptr = new GenericOptInterval[WrappedPyObject]()
        elif v is None:
            self.thisptr = new GenericOptInterval[WrappedPyObject](WrappedPyObject(u))
        else:
            self.thisptr = new GenericOptInterval[WrappedPyObject](WrappedPyObject(u), WrappedPyObject(v) )
    @classmethod
    def from_Interval(self, i):
        if hasattr(i, "isEmpty"):
            if i.isEmpty():
                return cy_GenericOptInterval()
            else:
                return cy_GenericOptInterval.from_Interval(i.Interval)
        return cy_GenericOptInterval( i.min(), i.max() )

    @classmethod
    def from_list(self, lst):
        if len(lst) == 0:
            return cy_GenericInterval()
        ret = cy_GenericInterval(lst[0])
        for i in lst[1:]:
            ret.expandTo(i)
        return ret

    def __bool__(self):
        return not self.thisptr.isEmpty()
    def __str__(self):
        if not self:
            return "[]"
        return "[{}, {}]".format(self.Interval.min(), self.Interval.max())
    def __repr__(self):
        if not self:
            return "GenericOptInterval()"
        if self.Interval.isSingular():
            return "GenericOptInterval({})".format( str(self.Interval.min()) )
        return "GenericOptInterval({}, {})".format( str(self.Interval.min()) , str(self.Interval.max()) )

    #provides access to underlying interval
    #TODO shouldn't OptInterval actually provide Interval's methods?
    property Interval:
        def __get__(self):
            return wrap_GenericInterval(self.thisptr.get())

    def isEmpty(self):
        return self.thisptr.isEmpty()
    #This should also accept cy_Interval?
    def unionWith(self, cy_GenericOptInterval o):
        self.thisptr.unionWith( deref(o.thisptr) )
    def intersectWith(cy_GenericOptInterval self, cy_GenericOptInterval o):
        self.thisptr.intersectWith( deref(o.thisptr) )

    def __or__(cy_GenericOptInterval self, cy_GenericOptInterval o):
        return wrap_GenericOptInterval(deref(self.thisptr) | deref(o.thisptr))
    def __and__(cy_GenericOptInterval self, cy_GenericOptInterval o):
        return wrap_GenericOptInterval(deref(self.thisptr) & deref(o.thisptr))

    def __richcmp__(cy_GenericOptInterval self, cy_GenericOptInterval o, int op):
        if op == 2:
            return deref(self.thisptr) == deref(o.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(o.thisptr)

cdef cy_GenericOptInterval wrap_GenericOptInterval(GenericOptInterval[WrappedPyObject] p):
    cdef GenericOptInterval[WrappedPyObject] * retp = new GenericOptInterval[WrappedPyObject]()
    retp[0] = p
    cdef cy_GenericOptInterval r = cy_GenericOptInterval.__new__(cy_GenericOptInterval)
    r.thisptr = retp
    return r


cdef class cy_Interval:
#~     cdef Interval* thisptr
    def __cinit__(self, u = None, v = None):
        if u is None:
            self.thisptr = new Interval()
        elif v is None:
            self.thisptr = new Interval(<Coord>float(u))
        else:
            self.thisptr = new Interval(<Coord>float(u), <Coord>float(v))
    @classmethod
    def from_Interval(c, i):
        return cy_Interval( i.min(), i.max() )

    @classmethod
    #TODO this is not done by the C++ method
    def from_list(cls, lst):
        if len(lst) == 0:
            return cy_Interval()
        ret = cy_Interval(lst[0])
        for i in lst[1:]:
            ret.expandTo(i)
        return ret

    def __str__(self):
        return "[{}, {}]".format(self.min(), self.max())
    def __repr__(self):
        if self.isSingular():
            return "Interval({})".format( str(self.min()) )
        return "Interval({}, {})".format( str(self.min()) , str(self.max()) )


    def min(self):
        return self.thisptr.min()
    def max(self):
        return self.thisptr.max()
    def extent(self):
        return self.thisptr.extent()
    def middle(self):
        return self.thisptr.middle()

    def setMin(self, Coord val):
        self.thisptr.setMin(val)
    def setMax(self, Coord val):
        self.thisptr.setMax(val)
    def expandTo(self, Coord val):
        self.thisptr.expandTo(val)
    def expandBy(self, Coord amount):
        self.thisptr.expandBy(amount)
    def unionWith(self, cy_Interval a):
        self.thisptr.unionWith(deref( a.thisptr ))
    @classmethod
    def unify(cls, I, J):
        return I|J

#    Not exposing this - deprecated
#    def operator[](self, unsigned int i):
#        return self.thisptr.operator[](i)

    def isSingular(self):
        return self.thisptr.isSingular()
    def isFinite(self):
        return self.thisptr.isFinite()

    def contains(cy_Interval self, other):
        if isinstance(other, Number):
            return self.thisptr.contains(<Coord>float(other))
        else:
            return self.thisptr.contains( <Interval &>deref((<cy_Interval>other).thisptr) )
    def intersects(self, cy_Interval val):
        return self.thisptr.intersects(deref( val.thisptr ))

    def interiorContains(cy_Interval self, other):
        if isinstance(other, Number):
            return self.thisptr.interiorContains(<Coord>float(other))
        else:
            return self.thisptr.interiorContains( <Interval &>deref((<cy_Interval>other).thisptr) )
    def interiorIntersects(self, cy_Interval val):
        return self.thisptr.interiorIntersects(deref( val.thisptr ))

    def _cmp_Interval(cy_Interval self, cy_Interval other, op):
        if op == 2:
            return deref(self.thisptr) == deref(other.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(other.thisptr)
    def _cmp_IntInterval(cy_Interval self, cy_IntInterval other, op):
        if op == 2:
            return deref(self.thisptr) == deref(other.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(other.thisptr)
    def __richcmp__(cy_Interval self, other, op):
        if isinstance(other, cy_Interval):
            return self._cmp_Interval(other, op)
        elif isinstance(other, cy_IntInterval):
            return self._cmp_IntInterval(other, op)

    def __neg__(self):
        return wrap_Interval(-deref(self.thisptr))

    def _add_number(self, Coord X):
        return wrap_Interval(deref(self.thisptr)+X)
    def _sub_number(self, Coord X):
        return wrap_Interval(deref(self.thisptr)-X)
    def _mul_number(self, Coord X):
        return wrap_Interval(deref(self.thisptr)*X)

    def _add_interval(self, cy_Interval I):
        return wrap_Interval(deref(self.thisptr)+deref(I.thisptr))
    def _sub_interval(self, cy_Interval I):
        return wrap_Interval(deref(self.thisptr)-deref(I.thisptr))
    def _mul_interval(self, cy_Interval I):
        return wrap_Interval(deref(self.thisptr)*deref(I.thisptr))

    def __mul__(cy_Interval self, other):
        if isinstance(other, Number):
            return self._mul_number(float(other))
        else:
            return self._mul_interval(other)

    def __add__(cy_Interval self, other):
        if isinstance(other, Number):
            return self._add_number(float(other))
        else:
            return self._add_interval(other)

    def __sub__(cy_Interval self, other):
        if isinstance(other, Number):
            return self._sub_number(float(other))
        else:
            return self._sub_interval(other)

    def __div__(cy_Interval self, Coord s):
        return wrap_Interval(deref(self.thisptr)/s)

    def __or__(cy_Interval self, cy_Interval I):
        return wrap_Interval(deref(self.thisptr)|deref(I.thisptr))

    def roundOutwards(self):
        return wrap_IntInterval(self.thisptr.roundOutwards())
    def roundInwards(self):
        return wrap_OptIntInterval(self.thisptr.roundInwards())

cdef cy_Interval wrap_Interval(Interval p):
    cdef Interval * retp = new Interval()
    retp[0] = p
    cdef cy_Interval r = cy_Interval.__new__(cy_Interval)
    r.thisptr = retp
    return r

cdef class cy_OptInterval:
#~     cdef OptInterval* thisptr
    def __cinit__(self, u = None, v = None):
        if u is None:
            self.thisptr = new OptInterval()
        elif v is None:
            self.thisptr = new OptInterval(<Coord>float(u))
        else:
            self.thisptr = new OptInterval(<Coord>float(u), <Coord>float(v))
    @classmethod
    def from_Interval(cls, i):
        if hasattr(i, "isEmpty"):
            if i.isEmpty():
                return cy_OptInterval()
            else:
                return cy_OptInterval.from_Interval(i.Interval)
        return cy_OptInterval( i.min(), i.max() )
    def __bool__(self):
        return not self.thisptr.isEmpty()
    def __str__(self):
        if not self:
            return "[]"
        return "[{}, {}]".format(self.Interval.min(), self.Interval.max())
    def __repr__(self):
        if not self:
            return "OptInterval()"
        if self.Interval.isSingular():
            return "OptInterval({})".format( str(self.Interval.min()) )
        return "OptInterval({}, {})".format( str(self.Interval.min()) , str(self.Interval.max()) )


    #provides access to underlying interval
    #TODO shouldn't OptInterval actually provide Interval's methods?
    property Interval:
        def __get__(self):
            return wrap_Interval(self.thisptr.get())

    def isEmpty(self):
        return self.thisptr.isEmpty()
    #This should also accept cy_Interval?
    def unionWith(self, cy_OptInterval o):
        self.thisptr.unionWith( deref(o.thisptr) )
    def intersectWith(cy_OptInterval self, cy_OptInterval o):
        self.thisptr.intersectWith( deref(o.thisptr) )

    def __or__(cy_OptInterval self, cy_OptInterval o):
        return wrap_OptInterval(deref(self.thisptr) | deref(o.thisptr))
    def __and__(cy_OptInterval self, cy_OptInterval o):
        return wrap_OptInterval(deref(self.thisptr) & deref(o.thisptr))

cdef cy_OptInterval wrap_OptInterval(OptInterval p):
    cdef OptInterval * retp = new OptInterval()
    retp[0] = p
    cdef cy_OptInterval r = cy_OptInterval.__new__(cy_OptInterval)
    r.thisptr = retp
    return r


cdef class cy_IntInterval:

    cdef IntInterval* thisptr
    def __cinit__(self, u = None, v = None):
        if u is None:
            self.thisptr = new IntInterval()
        elif v is None:
            self.thisptr = new IntInterval(<IntCoord>int(u))
        else:
            self.thisptr = new IntInterval(<IntCoord>int(u), <IntCoord>int(v))
    @classmethod
    def from_IntInterval(cls, i):
        return cy_IntInterval( i.min(), i.max() )
    @classmethod
    #TODO this is not done by the C++ method
    def from_list(cls, lst):
        if len(lst) == 0:
            return cy_IntInterval()
        ret = cy_IntInterval(lst[0])
        for i in lst[1:]:
            ret.expandTo(i)
        return ret

    def __str__(self):
        return "[{}, {}]".format(self.min(), self.max())
    def __repr__(self):
        if self.isSingular():
            return "IntInterval({})".format( str(self.min()) )
        return "IntInterval({}, {})".format( str(self.min()) , str(self.max()) )


    def min(self):
        return self.thisptr.min()
    def max(self):
        return self.thisptr.max()
    def extent(self):
        return self.thisptr.extent()
    def middle(self):
        return self.thisptr.middle()

    def setMin(self, IntCoord val):
        self.thisptr.setMin(val)
    def setMax(self, IntCoord val):
        self.thisptr.setMax(val)
    def expandTo(self, IntCoord val):
        self.thisptr.expandTo(val)
    def expandBy(self, IntCoord amount):
        self.thisptr.expandBy(amount)
    def unionWith(self, cy_IntInterval a):
        self.thisptr.unionWith(deref( a.thisptr ))
    @classmethod
    def unify(cls, I, J):
        return I|J

#    Not exposing this - deprecated
#    def operator[](self, unsigned int i):
#        return self.thisptr.operator[](i)

    def isSingular(self):
        return self.thisptr.isSingular()

    def contains(cy_IntInterval self, other):
        if isinstance(other, Number):
            return self.thisptr.contains(<IntCoord>int(other))
        else:
            return self.thisptr.contains( <IntInterval &>deref((<cy_IntInterval>other).thisptr) )

    def intersects(self, cy_IntInterval val):
        return self.thisptr.intersects(deref( val.thisptr ))

    def __richcmp__(cy_IntInterval self, cy_IntInterval other, op):
        if op == 2:
            return deref(self.thisptr) == deref(other.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(other.thisptr)

    def __neg__(self):
        return wrap_IntInterval(-deref(self.thisptr))

    def _add_number(self, IntCoord X):
        return wrap_IntInterval(deref(self.thisptr)+X)
    def _sub_number(self, IntCoord X):
        return wrap_IntInterval(deref(self.thisptr)-X)

    def _add_interval(self, cy_IntInterval I):
        return wrap_IntInterval(deref(self.thisptr)+deref(I.thisptr))
    def _sub_interval(self, cy_IntInterval I):
        return wrap_IntInterval(deref(self.thisptr)-deref(I.thisptr))

    def __add__(cy_IntInterval self, other):
        if isinstance(other, Number):
            return self._add_number(int(other))
        else:
            return self._add_interval(other)

    def __sub__(cy_IntInterval self, other):
        if isinstance(other, Number):
            return self._sub_number(int(other))
        else:
            return self._sub_interval(other)

    def __or__(cy_IntInterval self, cy_IntInterval I):
        return wrap_IntInterval(deref(self.thisptr)|deref(I.thisptr))

cdef cy_IntInterval wrap_IntInterval(IntInterval p):
    cdef IntInterval * retp = new IntInterval()
    retp[0] = p
    cdef cy_IntInterval r = cy_IntInterval.__new__(cy_IntInterval)
    r.thisptr = retp
    return r

cdef class cy_OptIntInterval:

    cdef OptIntInterval* thisptr
    def __cinit__(self, u = None, v = None):
        if u is None:
            self.thisptr = new OptIntInterval()
        elif v is None:
            self.thisptr = new OptIntInterval(<IntCoord>int(u))
        else:
            self.thisptr = new OptIntInterval(<IntCoord>int(u), <IntCoord>int(v))
    @classmethod
    def from_Interval(self, i):
        if hasattr(i, "isEmpty"):
            if i.isEmpty():
                return cy_OptIntInterval()
            else:
                return cy_OptIntInterval.from_Interval(i.Interval)
        return cy_OptIntInterval( i.min(), i.max() )

    def __bool__(self):
        return not self.thisptr.isEmpty()
    def __str__(self):
        if not self:
            return "[]"
        return "[{}, {}]".format(self.Interval.min(), self.Interval.max())
    def __repr__(self):
        if not self:
            return "OptIntInterval()"
        if self.Interval.isSingular():
            return "OptIntInterval({})".format( str(self.Interval.min()) )
        return "OptIntInterval({}, {})".format( str(self.Interval.min()) , str(self.Interval.max()) )
    #provides access to underlying interval
    #TODO shouldn't OptInterval actually provide Interval's methods?
    property Interval:
        def __get__(self):
            return wrap_IntInterval(self.thisptr.get())

    def isEmpty(self):
        return self.thisptr.isEmpty()

    def unionWith(self, cy_OptIntInterval o):
        self.thisptr.unionWith( deref(o.thisptr) )

    def intersectWith(self, cy_OptIntInterval o):
        self.thisptr.intersectWith( deref(o.thisptr) )

    def __or__(cy_OptIntInterval self, cy_OptIntInterval o):
        return wrap_OptIntInterval(deref(self.thisptr) | deref(o.thisptr))
    def __and__(cy_OptIntInterval self, cy_OptIntInterval o):
        return wrap_OptIntInterval(deref(self.thisptr) & deref(o.thisptr))

    #TODO decide how to implement various combinations of comparisons!


cdef cy_OptIntInterval wrap_OptIntInterval(OptIntInterval p):
    cdef OptIntInterval * retp = new OptIntInterval()
    retp[0] = p
    cdef cy_OptIntInterval r = cy_OptIntInterval.__new__(cy_OptIntInterval)
    r.thisptr = retp
    return r

cdef class cy_GenericRect:
    cdef GenericRect[WrappedPyObject]* thisptr

    def __cinit__(self, *args):
        cdef WrappedPyObject zero = WrappedPyObject(0)
        if len(args) == 0:
            self.thisptr = new GenericRect[WrappedPyObject](zero, zero, zero, zero)
        elif len(args) == 2:
            if isinstance(args[0], cy_GenericInterval) and isinstance(args[1], cy_GenericInterval):
                self.thisptr = new GenericRect[WrappedPyObject](deref((<cy_GenericInterval> args[0]).thisptr),
                                                                deref((<cy_GenericInterval> args[1]).thisptr))
        elif len(args) == 4:
            self.thisptr = new GenericRect[WrappedPyObject](WrappedPyObject(args[0]),
                                                            WrappedPyObject(args[1]),
                                                            WrappedPyObject(args[2]),
                                                            WrappedPyObject(args[3]))


    @classmethod
    def from_list(cls, lst):
        ret = cy_GenericRect()
        for a in lst:
            ret.expandTo(a)
        return ret
    @classmethod
    def from_xywh(cls, x, y, w, h):
        return wrap_GenericRect( from_xywh(WrappedPyObject(x),
                                        WrappedPyObject(y),
                                        WrappedPyObject(w),
                                        WrappedPyObject(h)))
#    @classmethod
#    def from_xywh(cls, cy_Point xy, cy_Point wh):
#        return wrap_GenericRecti(cls.thisptr.from_xywh(deref( xy.thisptr ) ,deref( wh.thisptr )))
#    def infinite(self):
#        return wrap_GenericRecti(self.thisptr.infinite())

    def __getitem__(self, Dim2 d):
        return wrap_GenericInterval( deref(self.thisptr)[d] )

    def min(self):
        return wrap_PyPoint( self.thisptr.min() )
    def max(self):
        return wrap_PyPoint( self.thisptr.max() )
    def corner(self, unsigned int i):
        return wrap_PyPoint( self.thisptr.corner(i) )
    def top(self):
        return self.thisptr.top().getObj()
    def bottom(self):
        return self.thisptr.bottom().getObj()
    def left(self):
        return self.thisptr.left().getObj()
    def right(self):
        return self.thisptr.right().getObj()
    def width(self):
        return self.thisptr.width().getObj()
    def height(self):
        return self.thisptr.height().getObj()
#For some reason, Cpp aspectRatio returns Coord.
#    def aspectRatio(self):
#        return self.thisptr.aspectRatio().getObj()
    def dimensions(self):
        return wrap_PyPoint( self.thisptr.dimensions() )
    def midpoint(self):
        return wrap_PyPoint( self.thisptr.midpoint() )
    def area(self):
        return self.thisptr.area().getObj()
    def hasZeroArea(self):
        return self.thisptr.hasZeroArea()
    def maxExtent(self):
        return self.thisptr.maxExtent().getObj()
    def minExtent(self):
        return self.thisptr.minExtent().getObj()

    def intersects(self, cy_GenericRect r):
        return self.thisptr.intersects(deref( r.thisptr ))
    def contains(self, r):
        if isinstance(r, cy_GenericRect):
            return self.thisptr.contains( deref((<cy_GenericRect> r).thisptr) )
        elif isinstance(r, tuple):
            return self.thisptr.contains( make_PyPoint(r) )

    def setLeft(self, val):
        self.thisptr.setLeft( WrappedPyObject(val) )
    def setRight(self, val):
        self.thisptr.setRight( WrappedPyObject(val) )
    def setTop(self, val):
        self.thisptr.setTop( WrappedPyObject(val) )
    def setBottom(self, val):
        self.thisptr.setBottom( WrappedPyObject(val) )
    def setMin(self, p):
        self.thisptr.setMin(make_PyPoint(p))
    def setMax(self, p):
        self.thisptr.setMax(make_PyPoint(p))
    def expandTo(self, p):
        self.thisptr.expandTo(make_PyPoint(p))
    def unionWith(self, cy_GenericRect b):
        self.thisptr.unionWith(deref( b.thisptr ))
    def expandBy(self, x, y = None):
        if y is None:
            self.thisptr.expandBy(WrappedPyObject(x))
        else:
            self.thisptr.expandBy(WrappedPyObject(x),
                                  WrappedPyObject(y))

    def __add__(cy_GenericRect self, p):
        return wrap_GenericRect( deref(self.thisptr) + make_PyPoint(p) )
    def __sub__(cy_GenericRect self, p):
        return wrap_GenericRect( deref(self.thisptr) - make_PyPoint(p) )
    def __or__(cy_GenericRect self, cy_GenericRect o):
        return wrap_GenericRect( deref(self.thisptr) | deref( o.thisptr ))
    def __richcmp__(cy_GenericRect self, cy_GenericRect o, int op):
        if op == 2:
            return deref(self.thisptr) == deref(o.thisptr)
        if op == 3:
            return deref(self.thisptr) != deref(o.thisptr)

cdef PyPoint make_PyPoint(p):
    return PyPoint( WrappedPyObject(p[0]), WrappedPyObject(p[1]) )

#D2[WrappedPyObject] is converted to tuple
cdef wrap_PyPoint(PyPoint p):
    return (p[0].getObj(), p[1].getObj())

cdef cy_GenericRect wrap_GenericRect(GenericRect[WrappedPyObject] p):
    cdef WrappedPyObject zero = WrappedPyObject(0)
    cdef GenericRect[WrappedPyObject] * retp = new GenericRect[WrappedPyObject](zero, zero, zero, zero)
    retp[0] = p
    cdef cy_GenericRect r = cy_GenericRect.__new__(cy_GenericRect)
    r.thisptr = retp
    return r


cdef class cy_Rect:
    #cdef Rect* thisptr
    def __cinit__(self, *args):
        if len(args) == 0:
            self.thisptr = new Rect()
        elif len(args) == 2:
            if isinstance(args[0], cy_Interval) and isinstance(args[0], cy_Interval):
                self.thisptr = new Rect(deref((<cy_Interval> args[0]).thisptr),
                                        deref(  (<cy_Interval> args[1]).thisptr))
            elif isinstance(args[0], cy_Point) and isinstance(args[1], cy_Point):
                self.thisptr = new Rect( deref( (<cy_Point> args[0]).thisptr),
                                         deref( (<cy_Point> args[1]).thisptr) )
        elif len(args) == 4:
            self.thisptr = new Rect(<Coord>(args[0]),
                                    <Coord>(args[1]),
                                    <Coord>(args[2]),
                                    <Coord>(args[3]))
    @classmethod
    def from_list(cls, lst):
        if lst == []:
            return cy_Rect()
        if len(lst) == 1:
            return cy_Rect(lst[0], lst[0])
        ret = cy_Rect(lst[0], lst[1])
        for a in lst:
            ret.expandTo(a)
        return ret
    @classmethod
    def from_xywh(cls, x, y, w, h):
        return wrap_Rect( from_xywh(<Coord> x,
                                    <Coord> y,
                                    <Coord> w,
                                    <Coord> h) )
#    @classmethod
#    def from_xywh(cls, cy_Point xy, cy_Point wh):
#        return wrap_GenericRecti(cls.thisptr.from_xywh(deref( xy.thisptr ) ,deref( wh.thisptr )))
#    def infinite(self):
#        return wrap_GenericRecti(self.thisptr.infinite())

    def __str__(self):
        return "Rectangle with dimensions {}, topleft point {}".format(str(self.dimensions()), str(self.min()))
    def __repr__(self):
        return "Rect({}, {}, {}, {})".format( str(self.left()),
                                              str(self.top()),
                                              str(self.right()),
                                              str(self.bottom()))


    def __getitem__(self, Dim2 d):
        return wrap_Interval( deref(self.thisptr)[d] )

    def min(self):
        return wrap_Point( self.thisptr.min() )
    def max(self):
        return wrap_Point( self.thisptr.max() )
    def corner(self, unsigned int i):
        return wrap_Point( self.thisptr.corner(i) )
    def top(self):
        return self.thisptr.top()
    def bottom(self):
        return self.thisptr.bottom()
    def left(self):
        return self.thisptr.left()
    def right(self):
        return self.thisptr.right()
    def width(self):
        return self.thisptr.width()
    def height(self):
        return self.thisptr.height()
    def aspectRatio(self):
        return self.thisptr.aspectRatio()
    def dimensions(self):
        return wrap_Point( self.thisptr.dimensions() )
    def midpoint(self):
        return wrap_Point( self.thisptr.midpoint() )
    def area(self):
        return self.thisptr.area()
    def hasZeroArea(self, Coord eps = EPSILON):
        return self.thisptr.hasZeroArea(eps)
    def maxExtent(self):
        return self.thisptr.maxExtent()
    def minExtent(self):
        return self.thisptr.minExtent()

    def intersects(self, cy_Rect r):
        return self.thisptr.intersects(deref( r.thisptr ))
    def contains(cy_Rect self, r):
        if isinstance(r, cy_Rect):
            return self.thisptr.contains( deref( (<cy_Rect> r).thisptr ) )
        elif isinstance(r, cy_Point):
            return self.thisptr.contains( deref( (<cy_Point> r).thisptr ) )

#    def contains(self, cy_Point p):
#        return self.thisptr.contains(deref( p.thisptr ))

    def interiorIntersects(self, cy_Rect r):
        return self.thisptr.interiorIntersects(deref( r.thisptr ))

    def interiorContains(self, other):
        if isinstance(other, cy_Point):
            return self.thisptr.interiorContains( deref( (<cy_Point> other).thisptr ) )
        elif isinstance(other, cy_Rect):
            return self.thisptr.interiorContains( deref( (<cy_Rect> other).thisptr ) )
        elif isinstance(other, cy_OptRect):
            return self.thisptr.interiorContains( deref( (<cy_OptRect> other).thisptr ) )

    def setLeft(self, Coord val):
        self.thisptr.setLeft(val)
    def setRight(self, Coord val):
        self.thisptr.setRight(val)
    def setTop(self, Coord val):
        self.thisptr.setTop(val)
    def setBottom(self, Coord val):
        self.thisptr.setBottom(val)
    def setMin(self, cy_Point p):
        self.thisptr.setMin( deref( p.thisptr ) )
    def setMax(self, cy_Point p):
        self.thisptr.setMax( deref( p.thisptr ))
    def expandTo(self, cy_Point p):
        self.thisptr.expandTo( deref( p.thisptr ) )
    def unionWith(self, cy_Rect b):
        self.thisptr.unionWith(deref( b.thisptr ))
    def expandBy(cy_Rect self, x, y = None):
        if y is None:
            if isinstance(x, cy_Point):
                self.thisptr.expandBy( deref( (<cy_Point> x).thisptr ) )
            else:
                self.thisptr.expandBy( <Coord> x)
        else:
            self.thisptr.expandBy( <Coord> x,
                                   <Coord> y)

    def __add__(cy_Rect self, cy_Point p):
        return wrap_Rect( deref(self.thisptr) + deref( p.thisptr ) )
    def __sub__(cy_Rect self, cy_Point p):
        return wrap_Rect( deref(self.thisptr) - deref( p.thisptr ) )
    def __mul__(cy_Rect self, t):
        cdef Affine at
        if is_transform(t):
            at = get_Affine(t)
            return wrap_Rect( deref(self.thisptr) * at )
    def __or__(cy_Rect self, cy_Rect o):
        return wrap_Rect( deref(self.thisptr) | deref( o.thisptr ))
    def __richcmp__(cy_Rect self, o, int op):
        if op == 2:
            if isinstance(o, cy_Rect):
                return deref(self.thisptr) == deref( (<cy_Rect> o).thisptr)
            elif isinstance(o, cy_IntRect):
                return deref(self.thisptr) == deref( (<cy_IntRect> o).thisptr)
        if op == 3:
            if isinstance(o, cy_Rect):
                return deref(self.thisptr) != deref( (<cy_Rect> o).thisptr)
            elif isinstance(o, cy_IntRect):
                return deref(self.thisptr) != deref( (<cy_IntRect> o).thisptr)

    def roundInwards(self):
        return wrap_OptIntRect(self.thisptr.roundInwards())
    def roundOutwards(self):
        return wrap_IntRect(self.thisptr.roundOutwards())

    @classmethod
    def distanceSq(cls, cy_Point p, cy_Rect rect):
        return distanceSq( deref(p.thisptr), deref(rect.thisptr) )
    @classmethod
    def distance(cls, cy_Point p, cy_Rect rect):
        return distance( deref(p.thisptr), deref(rect.thisptr) )

cdef cy_Rect wrap_Rect(Rect p):
    cdef Rect* retp = new Rect()
    retp[0] = p
    cdef cy_Rect r = cy_Rect.__new__(cy_Rect)
    r.thisptr = retp
    return r

cdef class cy_OptRect:
#    cdef OptRect* thisptr
    def __cinit__(self, *args):
        if len(args) == 0:
            self.thisptr = new OptRect()
        elif len(args) == 1:
            if isinstance(args[0], cy_Rect):
                self.thisptr = new OptRect( deref( (<cy_Rect> args[0]).thisptr ) )
        elif len(args) == 2:
            if isinstance(args[0], cy_OptInterval) and isinstance(args[0], cy_OptInterval):
                self.thisptr = new OptRect(deref((<cy_OptInterval> args[0]).thisptr),
                                        deref(  (<cy_OptInterval> args[1]).thisptr))
            elif isinstance(args[0], cy_Point) and isinstance(args[1], cy_Point):
                self.thisptr = new OptRect( deref( (<cy_Point> args[0]).thisptr),
                                         deref( (<cy_Point> args[1]).thisptr) )
        elif len(args) == 4:
            self.thisptr = new OptRect(<Coord>(args[0]),
                                    <Coord>(args[1]),
                                    <Coord>(args[2]),
                                    <Coord>(args[3]))
    @classmethod
    def from_Rect(cls, r):
        if hasattr(r, "isEmpty"):
            if r.isEmpty():
                return cy_OptRect()
            else:
                return cy_OptRect(  r.Rect.min().x,
                                    r.Rect.min().y,
                                    r.Rect.max().x,
                                    r.Rect.max().y )
        return cy_OptRect(  r.min().x,
                            r.min().y,
                            r.max().x,
                            r.max().y )

    @classmethod
    def from_list(cls, lst):
        if lst == []:
            return cy_OptRect()
        if len(lst) == 1:
            return cy_OptRect(lst[0], lst[0])
        ret = cy_OptRect(lst[0], lst[1])
        for a in lst:
            ret.expandTo(a)
        return ret

    property Rect:
        def __get__(self):
            return wrap_Rect(self.thisptr.get())

    def __bool__(self):
        return not self.thisptr.isEmpty()

    def isEmpty(self):
        return self.thisptr.isEmpty()

    def intersects(self, other):
        if isinstance(other, cy_Rect):
            return self.thisptr.intersects( deref( (<cy_Rect> other).thisptr ) )
        elif isinstance(other, cy_OptRect):
            return self.thisptr.intersects( deref( (<cy_OptRect> other).thisptr ) )
    def contains(self, other):
        if isinstance(other, cy_Rect):
            return self.thisptr.contains( deref( (<cy_Rect> other).thisptr ) )
        elif isinstance(other, cy_OptRect):
            return self.thisptr.contains( deref( (<cy_OptRect> other).thisptr ) )
        elif isinstance(other, cy_Point):
            return self.thisptr.contains( deref( (<cy_Point> other).thisptr ) )
    def unionWith(self, other):
        if isinstance(other, cy_Rect):
            self.thisptr.unionWith( deref( (<cy_Rect> other).thisptr ) )
        elif isinstance(other, cy_OptRect):
            self.thisptr.unionWith( deref( (<cy_OptRect> other).thisptr ) )
    def intersectWith(self, other):
        if isinstance(other, cy_Rect):
            self.thisptr.intersectWith( deref( (<cy_Rect> other).thisptr ) )
        elif isinstance(other, cy_OptRect):
            self.thisptr.intersectWith( deref( (<cy_OptRect> other).thisptr ) )
    def expandTo(self, cy_Point p):
        self.thisptr.expandTo( deref(p.thisptr) )

    def __or__(cy_OptRect self, cy_OptRect other):
        return wrap_OptRect( deref(self.thisptr) | deref(other.thisptr) )
    def __and__(cy_OptRect self, other):
        if isinstance(other, cy_Rect):
            return wrap_OptRect( deref(self.thisptr) & deref( (<cy_Rect> other).thisptr) )
        elif isinstance(other, cy_OptRect):
            return wrap_OptRect( deref(self.thisptr) & deref( (<cy_OptRect> other).thisptr) )
    def __richcmp__(cy_OptRect self, other, op):
        if op == 2:
            if isinstance(other, cy_Rect):
                return deref(self.thisptr) == deref( (<cy_Rect> other).thisptr )
            elif isinstance(other, cy_OptRect):
                return deref(self.thisptr) == deref( (<cy_OptRect> other).thisptr )
        elif op == 3:
            if isinstance(other, cy_Rect):
                return deref(self.thisptr) != deref( (<cy_Rect> other).thisptr )
            elif isinstance(other, cy_OptRect):
                return deref(self.thisptr) != deref( (<cy_OptRect> other).thisptr )

cdef cy_OptRect wrap_OptRect(OptRect p):
    cdef OptRect* retp = new OptRect()
    retp[0] = p
    cdef cy_OptRect r = cy_OptRect.__new__(cy_OptRect)
    r.thisptr = retp
    return r



cdef class cy_IntRect:
    cdef IntRect* thisptr
    def __cinit__(self, *args):
        if len(args) == 0:
            self.thisptr = new IntRect()
        elif len(args) == 2:
            if isinstance(args[0], cy_IntInterval) and isinstance(args[0], cy_IntInterval):
                self.thisptr = new IntRect(deref((<cy_IntInterval> args[0]).thisptr),
                                        deref(  (<cy_IntInterval> args[1]).thisptr))
            elif isinstance(args[0], cy_IntPoint) and isinstance(args[1], cy_IntPoint):
                self.thisptr = new IntRect( deref( (<cy_IntPoint> args[0]).thisptr),
                                         deref( (<cy_IntPoint> args[1]).thisptr) )
        elif len(args) == 4:
            self.thisptr = new IntRect(<IntCoord>(args[0]),
                                    <IntCoord>(args[1]),
                                    <IntCoord>(args[2]),
                                    <IntCoord>(args[3]))
    @classmethod
    def from_list(cls, lst):
        if lst == []:
            return cy_IntRect()
        if len(lst) == 1:
            return cy_IntRect(lst[0], lst[0])
        ret = cy_IntRect(lst[0], lst[1])
        for a in lst:
            ret.expandTo(a)
        return ret

#    @classmethod
#    def from_xywh(cls, cy_IntPoint xy, cy_IntPoint wh):
#        return wrap_GenericRecti(cls.thisptr.from_xywh(deref( xy.thisptr ) ,deref( wh.thisptr )))
#    def infinite(self):
#        return wrap_GenericRecti(self.thisptr.infinite())

    def __str__(self):
        return "Integer Rectangle with dimensions {}, topleft point {}".format(str(self.dimensions()), str(self.min()))
    def __repr__(self):
        return "IntRect({}, {}, {}, {})".format( str(self.left()),
                                              str(self.top()),
                                              str(self.right()),
                                              str(self.bottom()))

    def __getitem__(self, Dim2 d):
        return wrap_IntInterval( deref(self.thisptr)[d] )

    def min(self):
        return wrap_IntPoint( self.thisptr.i_min() )
    def max(self):
        return wrap_IntPoint( self.thisptr.i_max() )
    def corner(self, unsigned int i):
        return wrap_IntPoint( self.thisptr.i_corner(i) )
    def top(self):
        return self.thisptr.top()
    def bottom(self):
        return self.thisptr.bottom()
    def left(self):
        return self.thisptr.left()
    def right(self):
        return self.thisptr.right()
    def width(self):
        return self.thisptr.width()
    def height(self):
        return self.thisptr.height()
    def aspectRatio(self):
        return self.thisptr.aspectRatio()
    def dimensions(self):
        return wrap_IntPoint( self.thisptr.i_dimensions() )
    def midpoint(self):
        return wrap_IntPoint( self.thisptr.i_midpoint() )
    def area(self):
        return self.thisptr.area()
    def hasZeroArea(self):
        return self.thisptr.hasZeroArea()
    def maxExtent(self):
        return self.thisptr.maxExtent()
    def minExtent(self):
        return self.thisptr.minExtent()

    def intersects(self, cy_IntRect r):
        return self.thisptr.intersects(deref( r.thisptr ))
    def contains(cy_IntRect self, r):
        if isinstance(r, cy_IntRect):
            return self.thisptr.contains( deref( (<cy_IntRect> r).thisptr ) )
        elif isinstance(r, cy_IntPoint):
            return self.thisptr.contains( deref( (<cy_IntPoint> r).thisptr ) )
#    def contains(self, cy_IntPoint p):
#        return self.thisptr.contains(deref( p.thisptr ))


    def setLeft(self, IntCoord val):
        self.thisptr.setLeft(val)
    def setRight(self, IntCoord val):
        self.thisptr.setRight(val)
    def setTop(self, IntCoord val):
        self.thisptr.setTop(val)
    def setBottom(self, IntCoord val):
        self.thisptr.setBottom(val)
    def setMin(self, cy_IntPoint p):
        self.thisptr.setMin( deref( p.thisptr ) )
    def setMax(self, cy_IntPoint p):
        self.thisptr.setMax( deref( p.thisptr ))
    def expandTo(self, cy_IntPoint p):
        self.thisptr.expandTo( deref( p.thisptr ) )
    def unionWith(self, cy_IntRect b):
        self.thisptr.unionWith(deref( b.thisptr ))
    def expandBy(cy_IntRect self, x, y = None):
        if y is None:
            if isinstance(x, cy_IntPoint):
                self.thisptr.expandBy( deref( (<cy_IntPoint> x).thisptr ) )
            else:
                self.thisptr.expandBy( <IntCoord> x)
        else:
            self.thisptr.expandBy( <IntCoord> x,
                                   <IntCoord> y)

    def __add__(cy_IntRect self, cy_IntPoint p):
        return wrap_IntRect( deref(self.thisptr) + deref( p.thisptr ) )
    def __sub__(cy_IntRect self, cy_IntPoint p):
        return wrap_IntRect( deref(self.thisptr) - deref( p.thisptr ) )
    def __or__(cy_IntRect self, cy_IntRect o):
        return wrap_IntRect( deref(self.thisptr) | deref( o.thisptr ))
    def __richcmp__(cy_IntRect self, cy_IntRect o, int op):
        if op == 2:
            return deref(self.thisptr) == deref(o.thisptr)
        if op == 3:
            return deref(self.thisptr) != deref(o.thisptr)

cdef cy_IntRect wrap_IntRect(IntRect p):
    cdef IntRect* retp = new IntRect()
    retp[0] = p
    cdef cy_IntRect r = cy_IntRect.__new__(cy_IntRect)
    r.thisptr = retp
    return r



cdef class cy_OptIntRect:
    cdef OptIntRect* thisptr
    def __cinit__(self, *args):
        if len(args) == 0:
            self.thisptr = new OptIntRect()
        elif len(args) == 1:
            if isinstance(args[0], cy_IntRect):
                self.thisptr = new OptIntRect( deref( (<cy_IntRect> args[0]).thisptr ) )
        elif len(args) == 2:
            if isinstance(args[0], cy_OptIntInterval) and isinstance(args[0], cy_OptIntInterval):
                self.thisptr = new OptIntRect(deref((<cy_OptIntInterval> args[0]).thisptr),
                                        deref(  (<cy_OptIntInterval> args[1]).thisptr))
            elif isinstance(args[0], cy_IntPoint) and isinstance(args[1], cy_IntPoint):
                self.thisptr = new OptIntRect( deref( (<cy_IntPoint> args[0]).thisptr),
                                         deref( (<cy_IntPoint> args[1]).thisptr) )
        elif len(args) == 4:
            self.thisptr = new OptIntRect(  <IntCoord>(args[0]),
                                            <IntCoord>(args[1]),
                                            <IntCoord>(args[2]),
                                            <IntCoord>(args[3]))
    @classmethod
    def from_Rect(cls, r):
        if hasattr(r, "isEmpty"):
            if r.isEmpty():
                return cy_OptIntRect()
            else:
                return cy_OptIntRect(   r.Rect.min().x,
                                        r.Rect.min().y,
                                        r.Rect.max().x,
                                        r.Rect.max().y )
        return cy_OptIntRect(   r.min().x,
                                r.min().y,
                                r.max().x,
                                r.max().y )

    @classmethod
    def from_list(cls, lst):
        if lst == []:
            return cy_OptIntRect()
        if len(lst) == 1:
            return cy_OptIntRect(lst[0], lst[0])
        ret = cy_OptIntRect(lst[0], lst[1])
        for a in lst:
            ret.expandTo(a)
        return ret

    property Rect:
        def __get__(self):
            return wrap_IntRect(self.thisptr.get())

    def __str__(self):
        if self.isEmpty():
            return "Empty integer rectangle"
        return "Opt Integer Rectangle with dimensions {}, topleft point {}".format(str(self.Rect.dimensions()), str(self.Rect.min()))
    def __repr__(self):
        if self.isEmpty():
            return "OptIntRect()"
        return "OptIntRect({}, {}, {}, {})".format( str(self.Rect.left()),
                                              str(self.Rect.top()),
                                              str(self.Rect.right()),
                                              str(self.Rect.bottom()))


    def __bool__(self):
        return not self.thisptr.isEmpty()

    def isEmpty(self):
        return self.thisptr.isEmpty()

    def intersects(cy_OptIntRect self, other):
        if isinstance(other, cy_IntRect):
            return self.thisptr.intersects( deref( (<cy_IntRect> other).thisptr ) )
        elif isinstance(other, cy_OptIntRect):
            return self.thisptr.intersects( deref( (<cy_OptIntRect> other).thisptr ) )
    def contains(cy_OptIntRect self, other):
        if isinstance(other, cy_IntRect):
            return self.thisptr.contains( deref( (<cy_IntRect> other).thisptr ) )
        elif isinstance(other, cy_OptIntRect):
            return self.thisptr.contains( deref( (<cy_OptIntRect> other).thisptr ) )
        elif isinstance(other, cy_IntPoint):
            return self.thisptr.contains( deref( (<cy_IntPoint> other).thisptr ) )
    def unionWith(cy_OptIntRect self, other):
        if isinstance(other, cy_IntRect):
            self.thisptr.unionWith( deref( (<cy_IntRect> other).thisptr ) )
        elif isinstance(other, cy_OptIntRect):
            self.thisptr.unionWith( deref( (<cy_OptIntRect> other).thisptr ) )
    def intersectWith(cy_OptIntRect self, other):
        if isinstance(other, cy_IntRect):
            self.thisptr.intersectWith( deref( (<cy_IntRect> other).thisptr ) )
        elif isinstance(other, cy_OptIntRect):
            self.thisptr.intersectWith( deref( (<cy_OptIntRect> other).thisptr ) )
    def expandTo(self, cy_IntPoint p):
        self.thisptr.expandTo( deref(p.thisptr) )

    def __or__(cy_OptIntRect self, cy_OptIntRect other):
        return wrap_OptIntRect( deref(self.thisptr) | deref(other.thisptr) )
    def __and__(cy_OptIntRect self, other):
        if isinstance(other, cy_IntRect):
            return wrap_OptIntRect( deref(self.thisptr) & deref( (<cy_IntRect> other).thisptr) )
        elif isinstance(other, cy_OptIntRect):
            return wrap_OptIntRect( deref(self.thisptr) & deref( (<cy_OptIntRect> other).thisptr) )
    def __richcmp__(cy_OptIntRect self, other, op):
        if op == 2:
            if isinstance(other, cy_IntRect):
                return deref(self.thisptr) == deref( (<cy_IntRect> other).thisptr )
            elif isinstance(other, cy_OptIntRect):
                return deref(self.thisptr) == deref( (<cy_OptIntRect> other).thisptr )
        elif op == 3:
            if isinstance(other, cy_IntRect):
                return deref(self.thisptr) != deref( (<cy_IntRect> other).thisptr )
            elif isinstance(other, cy_OptIntRect):
                return deref(self.thisptr) != deref( (<cy_OptIntRect> other).thisptr )

cdef cy_OptIntRect wrap_OptIntRect(OptIntRect p):
    cdef OptIntRect* retp = new OptIntRect()
    retp[0] = p
    cdef cy_OptIntRect r = cy_OptIntRect.__new__(cy_OptIntRect)
    r.thisptr = retp
    return r
