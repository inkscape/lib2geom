from cython.operator cimport dereference as deref
from _common_decl cimport *
from numbers import Number


cdef class cy_Interval:
    cdef Interval* thisptr
    def __cinit__(self, u = None, v = None):
        if u is None:
            self.thisptr = new Interval()
        elif v is None:
            self.thisptr = new Interval(<Coord>float(u))
        else:
            self.thisptr = new Interval(<Coord>float(u), <Coord>float(v))
    @classmethod
    def from_Interval(self, i):
        return wrap_Interval(Interval( <Coord>i.min(), <Coord>i.max() ))
        
    @classmethod
    #TODO this is not done by the C++ method
    def from_list(cls, lst):
        if len(lst) == 0:
            return cy_Interval()
        ret = cy_Interval(lst[0])
        for i in lst[1:]:
            ret.expandTo(i)
        return ret

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
    cdef OptInterval* thisptr
    def __cinit__(self, u = None, v = None):
        if u is None:
            self.thisptr = new OptInterval()
        elif v is None:
            self.thisptr = new OptInterval(<Coord>float(u))
        else:   
            self.thisptr = new OptInterval(<Coord>float(u), <Coord>float(v))
    @classmethod
    def from_Interval(self, i):
        return wrap_OptInterval(OptInterval( <Coord>i.min(), <Coord>i.max() ))
    def __bool__(self):
        return not self.thisptr.isEmpty()

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
    def from_IntInterval(self, i):
        return wrap_IntInterval(IntInterval( <IntCoord>i.min(), <IntCoord>i.max() ))
        
    @classmethod
    #TODO this is not done by the C++ method
    def from_list(cls, lst):
        if len(lst) == 0:
            return cy_IntInterval()
        ret = cy_IntInterval(lst[0])
        for i in lst[1:]:
            ret.expandTo(i)
        return ret

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
        return wrap_OptIntInterval(OptIntInterval( <IntCoord>i.min(), <IntCoord>i.max() ))

    def __bool__(self):
        return not self.thisptr.isEmpty()

    #provides access to underlying interval
    #TODO shouldn't OptInterval actually provide Interval's methods?
    property IntInterval:
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

cdef cy_OptIntInterval wrap_OptIntInterval(OptIntInterval p):
    cdef OptIntInterval * retp = new OptIntInterval()
    retp[0] = p
    cdef cy_OptIntInterval r = cy_OptIntInterval.__new__(cy_OptIntInterval)
    r.thisptr = retp
    return r
