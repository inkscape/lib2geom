from numbers import Number

from _common_decl cimport *
from cython.operator cimport dereference as deref

from _cy_affine cimport cy_Affine, get_Affine, is_transform


cdef class cy_GenericInterval:
    """
    Represents all numbers between min and max.

    Corresponds to GenericInterval in 2geom. min and max can be arbitrary
    python object, they just have to implement arithmetic operations and
    comparison - fractions.Fraction works. This is a bit experimental,
    it leak memory right now.
    """

    cdef GenericInterval[WrappedPyObject]* thisptr

    def __cinit__(self, u = 0, v = None):
        """Create GenericInterval from either one or two values."""
        if v is None:
            self.thisptr = new GenericInterval[WrappedPyObject]( WrappedPyObject(u) )
        else:
            self.thisptr = new GenericInterval[WrappedPyObject]( WrappedPyObject(u), WrappedPyObject(v) )

    def __str__(self):
        """str(self)"""
        return "[{}, {}]".format(self.min(), self.max())

    def __repr__(self):
        """repr(self)"""
        if self.is_singular():
            return "GenericInterval({})".format( str(self.min()) )
        return "GenericInterval({}, {})".format( str(self.min()) , str(self.max()) )

    def __dealloc__(self):
        del self.thisptr

    @classmethod
    def from_Interval(self, i):
        """Create GenericInterval with same minimum and maximum as argument."""
        return cy_GenericInterval( i.min(), i.max() )

    @classmethod
    def from_list(self, lst):
        """Create GenericInterval containing all values in list."""
        if len(lst) == 0:
            return cy_GenericInterval()
        ret = cy_GenericInterval(lst[0])
        for i in lst[1:]:
            ret.expand_to(i)
        return ret

    def min(self):
        """Return minimal value of interval."""
        return self.thisptr.min().getObj()

    def max(self):
        """Return maximal value of interval."""
        return self.thisptr.max().getObj()
    def extent(self):
        """Return difference between maximal and minimal value."""
        return self.thisptr.extent().getObj()

    def middle(self):
        """Return midpoint of interval."""
        return self.thisptr.middle().getObj()

    def is_singular(self):
        """Test for one-valued interval."""
        return self.thisptr.isSingular()

    def set_min(self, val):
        """Set minimal value."""
        self.thisptr.setMin( WrappedPyObject(val) )

    def set_max(self, val):
        """Set maximal value."""
        self.thisptr.setMax( WrappedPyObject(val) )

    def expand_to(self, val):
        """Create smallest superset of self containing value."""
        self.thisptr.expandTo( WrappedPyObject(val) )

    def expand_by(self, val):
        """Push both boundaries by value."""
        self.thisptr.expandBy( WrappedPyObject(val) )
    def union_with(self, cy_GenericInterval interval):
        """self = self | other"""
        self.thisptr.unionWith( deref(interval.thisptr) )

    def contains(self, other):
        """Check if interval contains value."""
        return self.thisptr.contains( WrappedPyObject(other) )

    def contains_interval(self, cy_GenericInterval other):
        """Check if interval contains every point of interval."""
        return self.thisptr.contains( deref(other.thisptr) )

    def intersects(self, cy_GenericInterval other):
        """Check for intersecting intervals."""
        return self.thisptr.intersects(deref( other.thisptr ))

    def __neg__(self):
        """Return interval with negated boundaries."""
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
        """Add interval or value to self.

        Interval I+J consists of all values i+j such that i is in I and
        j is in J

        Interval I+x consists of all values i+x such that i is in I.
        """
        if isinstance(other, cy_GenericInterval):
            return self._add_interval(other)
        else:
            return self._add_pyobj(other)

    def __sub__(cy_GenericInterval self, other):
        """Substract interval or value.

        Interval I-J consists of all values i-j such that i is in I and
        j is in J

        Interval I-x consists of all values i-x such that i is in I.
        """
        if isinstance(other, cy_GenericInterval):
            return self._sub_interval(other)
        else:
            return self._sub_pyobj(other)

    def __or__(cy_GenericInterval self, cy_GenericInterval I):
        """Return a union of two intervals"""
        return wrap_GenericInterval(deref(self.thisptr)|deref(I.thisptr))

    def _eq(self, cy_GenericInterval other):
        return deref(self.thisptr)==deref(other.thisptr)

    def _neq(self, cy_GenericInterval other):
        return deref(self.thisptr)!=deref(other.thisptr)

    def __richcmp__(cy_GenericInterval self, other, op):
        """Intervals are not ordered."""
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

    """Class representing optionally empty interval.

    Empty interval has False bool value, and using methods that require
    non-empty interval will result in ValueError. This is supposed to be
    used this way:

    >>> C = A & B
    >>> if C:
    >>>     print C.min()

    This class represents GenericOptInterval with python object boundaries.
    It tries to model behaviour of boost::optional.
    """

    cdef GenericOptInterval[WrappedPyObject]* thisptr

    def __cinit__(self, u = None, v = None):
        """Create interval from boundaries.

        Using no arguments, you will end up with empty interval."""
        if u is None:
            self.thisptr = new GenericOptInterval[WrappedPyObject]()
        elif v is None:
            self.thisptr = new GenericOptInterval[WrappedPyObject](WrappedPyObject(u))
        else:
            self.thisptr = new GenericOptInterval[WrappedPyObject](WrappedPyObject(u), WrappedPyObject(v) )

    def __bool__(self):
        """Logical value of interval, False only for empty interval."""
        return not self.thisptr.isEmpty()

    def __str__(self):
        """str(self)"""
        if not self:
            return "[]"
        return "[{}, {}]".format(self.Interval.min(), self.Interval.max())

    def __repr__(self):
        """repr(self)"""
        if not self:
            return "GenericOptInterval()"
        if self.Interval.isSingular():
            return "GenericOptInterval({})".format( str(self.Interval.min()) )
        return "GenericOptInterval({}, {})".format( str(self.Interval.min()) , str(self.Interval.max()) )

    def __dealloc__(self):
        del self.thisptr

    @classmethod
    def from_Interval(self, i):
        """Create interval from existing interval."""
        if hasattr(i, "isEmpty"):
            if i.isEmpty():
                return cy_GenericOptInterval()
            else:
                return cy_GenericOptInterval.from_Interval(i.Interval)
        return cy_GenericOptInterval( i.min(), i.max() )

    @classmethod
    def from_list(self, lst):
        """Create interval containing all values in list.

        Empty list will result in empty interval."""
        if len(lst) == 0:
            return cy_GenericOptInterval()
        ret = cy_GenericOptInterval(lst[0])
        for i in lst[1:]:
            ret.Interval.expandTo(i)
        return ret

    property Interval:
        """Get underlying GenericInterval."""
        def __get__(self):
            if self.is_empty():
                raise ValueError("Interval is empty.")
            else:
                return wrap_GenericInterval(self.thisptr.get())

    def is_empty(self):
        """Check whether interval is empty set."""
        return self.thisptr.isEmpty()

    def union_with(self, cy_GenericOptInterval o):
        """self = self | other"""
        self.thisptr.unionWith( deref(o.thisptr) )

    def intersect_with(cy_GenericOptInterval self, cy_GenericOptInterval o):
        """self = self & other"""
        self.thisptr.intersectWith( deref(o.thisptr) )

    def __or__(cy_GenericOptInterval self, cy_GenericOptInterval o):
        """Return a union of two intervals."""
        return wrap_GenericOptInterval(deref(self.thisptr) | deref(o.thisptr))

    def __and__(cy_GenericOptInterval self, cy_GenericOptInterval o):
        """Return an intersection of two intervals."""
        return wrap_GenericOptInterval(deref(self.thisptr) & deref(o.thisptr))

    def __richcmp__(cy_GenericOptInterval self, cy_GenericOptInterval o, int op):
        """Intervals are not ordered."""
        if op == 2:
            return deref(self.thisptr) == deref(o.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(o.thisptr)
        return NotImplemented


    def _get_Interval_method(self, name):
        def f(*args, **kwargs):
            if self.is_empty():
                raise ValueError("GenericOptInterval is empty.")
            else:
                return self.Interval.__getattribute__(name)(*args, **kwargs)
        return f

    def __getattr__(self, name):

        Interval_methods = set(['contains', 'contains_interval', 
        'expand_by', 'expand_to', 'extent', 'from_Interval', 'from_list', 
        'intersects', 'is_singular', 'max', 'middle', 'min', 'set_max', 
        'set_min', 'union_with'])

        if name in Interval_methods:
            return self._get_Interval_method(name)
        else:
            raise AttributeError("GenericOptInterval instance has no attribute \"{}\"".format(name))

    def _wrap_Interval_method(self, name, *args, **kwargs):
        if self.isEmpty():
            raise ValueError("GenericOptInterval is empty.")
        else:
            return self.Interval.__getattr__(name)(*args, **kwargs)

    #declaring these by hand, because they take fixed number of arguments,
    #which is enforced by cython

    def __neg__(self):
        """Return interval with negated boundaries."""
        return self._wrap_Interval_method("__sub__")

    def __add__(cy_Interval self, other):
        """Add interval or value to self.

        Interval I+J consists of all values i+j such that i is in I and
        j is in J

        Interval I+x consists of all values i+x such that i is in I.
        """
        return self._wrap_Interval_method("__add__", other)

    def __sub__(cy_Interval self, other):
        """Substract interval or value.

        Interval I-J consists of all values i-j such that i is in I and
        j is in J

        Interval I-x consists of all values i-x such that i is in I.
        """
        return self._wrap_Interval_method("__sub__", other)

cdef cy_GenericOptInterval wrap_GenericOptInterval(GenericOptInterval[WrappedPyObject] p):
    cdef GenericOptInterval[WrappedPyObject] * retp = new GenericOptInterval[WrappedPyObject]()
    retp[0] = p
    cdef cy_GenericOptInterval r = cy_GenericOptInterval.__new__(cy_GenericOptInterval)
    r.thisptr = retp
    return r


cdef class cy_Interval:

    """Class representing interval on real line.

    Corresponds to Interval class in 2geom.
    """

    def __cinit__(self, u = None, v = None):
        """Create interval from it's boundaries.

        One argument will create interval consisting that value, no
        arguments create Interval(0).
        """
        if u is None:
            self.thisptr = new Interval()
        elif v is None:
            self.thisptr = new Interval(<Coord>float(u))
        else:
            self.thisptr = new Interval(<Coord>float(u), <Coord>float(v))

    def __str__(self):
        """str(self)"""
        return "[{}, {}]".format(self.min(), self.max())

    def __repr__(self):
        """repr(self)"""
        if self.is_singular():
            return "Interval({})".format( str(self.min()) )
        return "Interval({}, {})".format( str(self.min()) , str(self.max()) )

    def __dealloc__(self):
        del self.thisptr

    @classmethod
    def from_Interval(c, i):
        """Create Interval with same boundaries as argument."""
        return cy_Interval( i.min(), i.max() )

    @classmethod
    def from_list(cls, lst):
        """Create interval containg all values in a list."""
        if len(lst) == 0:
            return cy_Interval()
        ret = cy_Interval(lst[0])
        for i in lst[1:]:
            ret.expand_to(i)
        return ret

    def min(self):
        """Return minimal boundary."""
        return self.thisptr.min()

    def max(self):
        """Return maximal boundary."""
        return self.thisptr.max()

    def extent(self):
        """Return length of interval."""
        return self.thisptr.extent()

    def middle(self):
        """Return middle value."""
        return self.thisptr.middle()

    def set_min(self, Coord val):
        """Set minimal value."""
        self.thisptr.setMin(val)

    def set_max(self, Coord val):
        """Set maximal value."""
        self.thisptr.setMax(val)

    def expand_to(self, Coord val):
        """Set self to smallest superset of set containing value."""
        self.thisptr.expandTo(val)

    def expand_by(self, Coord amount):
        """Move both boundaries by amount."""
        self.thisptr.expandBy(amount)

    def union_with(self, cy_Interval a):
        """self = self | other"""
        self.thisptr.unionWith(deref( a.thisptr ))

#    Not exposing this - deprecated
#    def __getitem__(self, unsigned int i):
#        return deref(self.thisptr)[i]

    def is_singular(self):
        """Test if interval contains only one value."""
        return self.thisptr.isSingular()

    def isFinite(self):
        """Test for finiteness of interval's extent."""
        return self.thisptr.isFinite()

    def contains(cy_Interval self, other):
        """Test if interval contains number."""
        return self.thisptr.contains(float(other))

    def contains_interval(cy_Interval self, cy_Interval other):
        """Test if interval contains another interval."""
        return self.thisptr.contains( deref(other.thisptr) )

    def intersects(self, cy_Interval val):
        """Test for intersection of intervals."""
        return self.thisptr.intersects(deref( val.thisptr ))

    def interior_contains(cy_Interval self, other):
        """Test if interior of iterval contains number."""
        return self.thisptr.interiorContains(float(other))

    def interior_contains_interval(cy_Interval self, cy_Interval other):
        """Test if interior of interval contains another interval."""
        return self.thisptr.interiorContains( <Interval &> deref(other.thisptr) )


    def interior_intersects(self, cy_Interval val):
        """Test for intersection of interiors of two points."""
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
        """Intervals are not ordered."""
        if isinstance(other, cy_Interval):
            return self._cmp_Interval(other, op)
        elif isinstance(other, cy_IntInterval):
            return self._cmp_IntInterval(other, op)

    def __neg__(self):
        """Return interval with negated boundaries."""
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
        """Multiply interval by interval or number.

        Multiplying by number simply multiplies boundaries,
        multiplying intervals creates all values that can be written as
        product i*j of i in I and j in J.
        """
        if isinstance(other, Number):
            return self._mul_number(float(other))
        else:
            return self._mul_interval(other)

    def __add__(cy_Interval self, other):
        """Add interval or value to self.

        Interval I+J consists of all values i+j such that i is in I and
        j is in J

        Interval I+x consists of all values i+x such that i is in I.
        """
        if isinstance(other, Number):
            return self._add_number(float(other))
        else:
            return self._add_interval(other)

    def __sub__(cy_Interval self, other):
        """Substract interval or value.

        Interval I-J consists of all values i-j such that i is in I and
        j is in J

        Interval I-x consists of all values i-x such that i is in I.
        """
        if isinstance(other, Number):
            return self._sub_number(float(other))
        else:
            return self._sub_interval(other)

    def __div__(cy_Interval self, Coord s):
        """Divide boundaries by number."""
        return wrap_Interval(deref(self.thisptr)/s)

    def __or__(cy_Interval self, cy_Interval I):
        """Return union of two intervals."""
        return wrap_Interval(deref(self.thisptr)|deref(I.thisptr))

    def round_outwards(self):
        """Create the smallest IntIterval that is superset."""
        return wrap_IntInterval(self.thisptr.roundOutwards())

    def round_inwards(self):
        """Create the largest IntInterval that is subset."""
        return wrap_OptIntInterval(self.thisptr.roundInwards())

cdef cy_Interval wrap_Interval(Interval p):
    cdef Interval * retp = new Interval()
    retp[0] = p
    cdef cy_Interval r = cy_Interval.__new__(cy_Interval)
    r.thisptr = retp
    return r


cdef class cy_OptInterval:

    """Class representing optionally empty interval on real line.

    Empty interval has False bool value, and using methods that require
    non-empty interval will result in ValueError. This is supposed to be
    used this way:

    >>> C = A & B
    >>> if C:
    >>>     print C.min()

    This class represents OptInterval. It tries to model behaviour of
    boost::optional.
    """

    def __cinit__(self, u = None, v = None):
        """Create optionally empty interval form it's endpoints.

        No arguments will result in empty interval.
        """
        if u is None:
            self.thisptr = new OptInterval()
        elif v is None:
            self.thisptr = new OptInterval(<Coord>float(u))
        else:
            self.thisptr = new OptInterval(<Coord>float(u), <Coord>float(v))

    def __bool__(self):
        """Only empty interval is False."""
        return not self.thisptr.isEmpty()

    def __str__(self):
        """str(self)"""
        if not self:
            return "[]"
        return "[{}, {}]".format(self.Interval.min(), self.Interval.max())

    def __repr__(self):
        """repr(self)"""
        if not self:
            return "OptInterval()"
        if self.Interval.isSingular():
            return "OptInterval({})".format( str(self.Interval.min()) )
        return "OptInterval({}, {})".format( str(self.Interval.min()) , str(self.Interval.max()) )

    def __dealloc__(self):
        del self.thisptr

    @classmethod
    def from_Interval(cls, i):
        """Create interval from other (possibly empty) interval."""
        if hasattr(i, "isEmpty"):
            if i.isEmpty():
                return cy_OptInterval()
            else:
                return cy_OptInterval.from_Interval(i.Interval)
        return cy_OptInterval( i.min(), i.max() )

    @classmethod
    def from_list(self, lst):
        """Create interval containing all values in list.

        Empty list will result in empty interval."""
        if len(lst) == 0:
            return cy_OptInterval()
        ret = cy_OptInterval(lst[0])
        for i in lst[1:]:
            ret.Interval.expandTo(i)
        return ret

    property Interval:
        """Get underlying Interval."""
        def __get__(self):
            if self.is_empty():
                raise ValueError("Interval is empty.")
            else:
                return wrap_Interval(self.thisptr.get())

    def is_empty(self):
        """Test for empty interval."""
        return self.thisptr.isEmpty()

    def union_with(self, cy_OptInterval o):
        """self = self | other"""
        self.thisptr.unionWith( deref(o.thisptr) )

    def intersect_with(cy_OptInterval self, cy_OptInterval o):
        """self = self & other"""
        self.thisptr.intersectWith( deref(o.thisptr) )

    def __or__(cy_OptInterval self, cy_OptInterval o):
        """Return union of intervals."""
        return wrap_OptInterval(deref(self.thisptr) | deref(o.thisptr))

    def __and__(cy_OptInterval self, cy_OptInterval o):
        """Return intersection of intervals."""
        return wrap_OptInterval(deref(self.thisptr) & deref(o.thisptr))

    def _get_Interval_method(self, name):
        def f(*args, **kwargs):
            if self.is_empty():
                raise ValueError("OptInterval is empty.")
            else:
                return self.Interval.__getattribute__(name)(*args, **kwargs)
        return f

    def __getattr__(self, name):

        Interval_methods = set(['contains', 'contains_interval', 'expand_by', 
        'expand_to', 'extent', 'from_Interval', 'from_list', 
        'interior_contains', 'interior_contains_interval', 
        'interior_intersects', 'intersects', 'isFinite', 'is_singular', 
        'max', 'middle', 'min', 'round_inwards', 'round_outwards', 
        'set_max', 'set_min', 'union_with'])

        if name in Interval_methods:
            return self._get_Interval_method(name)
        else:
            raise AttributeError("OptInterval instance has no attribute \"{}\"".format(name))

    def _wrap_Interval_method(self, name, *args, **kwargs):
        if self.isEmpty():
            raise ValueError("OptInterval is empty.")
        else:
            return self.Interval.__getattr__(name)(*args, **kwargs)

    #declaring these by hand, because they take fixed number of arguments,
    #which is enforced by cython

    def __neg__(self):
        """Return interval with negated boundaries."""
        return self._wrap_Interval_method("__sub__")

    def __mul__(cy_Interval self, other):
        """Multiply interval by interval or number.

        Multiplying by number simply multiplies boundaries,
        multiplying intervals creates all values that can be written as
        product i*j of i in I and j in J.
        """
        return self._wrap_Interval_method("__mul__", other)

    def __add__(cy_Interval self, other):
        """Add interval or value to self.

        Interval I+J consists of all values i+j such that i is in I and
        j is in J

        Interval I+x consists of all values i+x such that i is in I.
        """
        return self._wrap_Interval_method("__add__", other)

    def __sub__(cy_Interval self, other):
        """Substract interval or value.

        Interval I-J consists of all values i-j such that i is in I and
        j is in J

        Interval I-x consists of all values i-x such that i is in I.
        """
        return self._wrap_Interval_method("__sub__", other)

    def __div__(cy_Interval self, other):
        """Divide boundaries by number."""
        return self._wrap_Interval_method("__div__", other)

cdef cy_OptInterval wrap_OptInterval(OptInterval p):
    cdef OptInterval * retp = new OptInterval()
    retp[0] = p
    cdef cy_OptInterval r = cy_OptInterval.__new__(cy_OptInterval)
    r.thisptr = retp
    return r


cdef class cy_IntInterval:

    """Class representing interval of integers.

    Corresponds to IntInterval class in 2geom.
    """

    cdef IntInterval* thisptr

    def __cinit__(self, u = None, v = None):
        """Create interval from it's boundaries.

        One argument will create interval consisting that value, no
        arguments create IntInterval(0).
        """
        if u is None:
            self.thisptr = new IntInterval()
        elif v is None:
            self.thisptr = new IntInterval(<IntCoord>int(u))
        else:
            self.thisptr = new IntInterval(<IntCoord>int(u), <IntCoord>int(v))

    def __str__(self):
        """str(self)"""
        return "[{}, {}]".format(self.min(), self.max())

    def __repr__(self):
        """repr(self)"""
        if self.is_singular():
            return "IntInterval({})".format( str(self.min()) )
        return "IntInterval({}, {})".format( str(self.min()) , str(self.max()) )

    def __dealloc__(self):
        del self.thisptr

    @classmethod
    def from_Interval(cls, i):
        return cy_IntInterval( int(i.min()), int(i.max()) )

    @classmethod
    def from_list(cls, lst):
        if len(lst) == 0:
            return cy_IntInterval()
        ret = cy_IntInterval(lst[0])
        for i in lst[1:]:
            ret.expand_to(i)
        return ret

    def min(self):
        """Return minimal boundary."""
        return self.thisptr.min()

    def max(self):
        """Return maximal boundary."""
        return self.thisptr.max()

    def extent(self):
        """Return length of interval."""
        return self.thisptr.extent()

    def middle(self):
        """Return middle value."""
        return self.thisptr.middle()

    def set_min(self, IntCoord val):
        """Set minimal value."""
        self.thisptr.setMin(val)

    def set_max(self, IntCoord val):
        """Set maximal value."""
        self.thisptr.setMax(val)

    def expand_to(self, IntCoord val):
        """Set self to smallest superset of set containing value."""
        self.thisptr.expandTo(val)

    def expand_by(self, IntCoord amount):
        """Move both boundaries by amount."""
        self.thisptr.expandBy(amount)

    def union_with(self, cy_IntInterval a):
        """self = self | other"""
        self.thisptr.unionWith(deref( a.thisptr ))

#    Not exposing this - deprecated
#    def __getitem__(self, unsigned int i):
#        return deref(self.thisptr)[i]

    def is_singular(self):
        """Test if interval contains only one value."""
        return self.thisptr.isSingular()

    def contains(cy_IntInterval self, other):
        """Test if interval contains number."""
        return self.thisptr.contains(<IntCoord> int(other))

    def contains_interval(cy_IntInterval self, cy_IntInterval other):
        """Test if interval contains another interval."""
        return self.thisptr.contains( deref(other.thisptr) )

    def intersects(self, cy_IntInterval val):
        """Test for intersection with other interval."""
        return self.thisptr.intersects(deref( val.thisptr ))

    def __richcmp__(cy_IntInterval self, cy_IntInterval other, op):
        """Intervals are not ordered."""
        if op == 2:
            return deref(self.thisptr) == deref(other.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(other.thisptr)

    def __neg__(self):
        """Negate interval's endpoints."""
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
        """Add interval or value to self.

        Interval I+J consists of all values i+j such that i is in I and
        j is in J

        Interval I+x consists of all values i+x such that i is in I.
        """
        if isinstance(other, Number):
            return self._add_number(int(other))
        else:
            return self._add_interval(other)

    def __sub__(cy_IntInterval self, other):
        """Substract interval or value.

        Interval I-J consists of all values i-j such that i is in I and
        j is in J

        Interval I-x consists of all values i-x such that i is in I.
        """
        if isinstance(other, Number):
            return self._sub_number(int(other))
        else:
            return self._sub_interval(other)

    def __or__(cy_IntInterval self, cy_IntInterval I):
        """Return union of two intervals."""
        return wrap_IntInterval(deref(self.thisptr)|deref(I.thisptr))

cdef cy_IntInterval wrap_IntInterval(IntInterval p):
    cdef IntInterval * retp = new IntInterval()
    retp[0] = p
    cdef cy_IntInterval r = cy_IntInterval.__new__(cy_IntInterval)
    r.thisptr = retp
    return r

cdef class cy_OptIntInterval:

    """Class representing optionally empty interval of integers.

    Empty interval has False bool value, and using methods that require
    non-empty interval will result in ValueError. This is supposed to be
    used this way:

    >>> C = A & B
    >>> if C:
    >>>     print C.min()

    This class represents OptIntInterval. It tries to model behaviour of
    boost::optional.
    """

    cdef OptIntInterval* thisptr

    def __cinit__(self, u = None, v = None):
        """Create optionally empty interval form it's endpoints.

        No arguments will result in empty interval.
        """
        if u is None:
            self.thisptr = new OptIntInterval()
        elif v is None:
            self.thisptr = new OptIntInterval(<IntCoord>int(u))
        else:
            self.thisptr = new OptIntInterval(<IntCoord>int(u), <IntCoord>int(v))

    def __bool__(self):
        """Only empty interval is False."""
        return not self.thisptr.isEmpty()

    def __str__(self):
        """str(self)"""
        if not self:
            return "[]"
        return "[{}, {}]".format(self.Interval.min(), self.Interval.max())

    def __repr__(self):
        """repr(self)"""
        if not self:
            return "OptIntInterval()"
        if self.Interval.isSingular():
            return "OptIntInterval({})".format( str(self.Interval.min()) )
        return "OptIntInterval({}, {})".format( str(self.Interval.min()) , str(self.Interval.max()) )

    def __dealloc__(self):
        del self.thisptr

    @classmethod
    def from_Interval(self, i):
        """Create interval from other (possibly empty) interval."""
        if hasattr(i, "isEmpty"):
            if i.isEmpty():
                return cy_OptIntInterval()
            else:
                return cy_OptIntInterval.from_Interval(i.Interval)
        return cy_OptIntInterval( i.min(), i.max() )

    @classmethod
    def from_list(self, lst):
        """Create interval containing all values in list.

        Empty list will result in empty interval."""
        if len(lst) == 0:
            return cy_OptIntInterval()
        ret = cy_OptIntInterval(lst[0])
        for i in lst[1:]:
            ret.Interval.expandTo(i)
        return ret

    property Interval:
        """Get underlying interval."""
        def __get__(self):
            return wrap_IntInterval(self.thisptr.get())

    def is_empty(self):
        """Test for empty interval."""
        return self.thisptr.isEmpty()

    def union_with(self, cy_OptIntInterval o):
        """self = self | other"""
        self.thisptr.unionWith( deref(o.thisptr) )

    def intersect_with(cy_OptIntInterval self, cy_OptIntInterval o):
        """self = self & other"""
        self.thisptr.intersectWith( deref(o.thisptr) )

    def __or__(cy_OptIntInterval self, cy_OptIntInterval o):
        """Return a union of two intervals."""
        return wrap_OptIntInterval(deref(self.thisptr) | deref(o.thisptr))

    def __and__(cy_OptIntInterval self, cy_OptIntInterval o):
        """Return an intersection of two intervals."""
        return wrap_OptIntInterval(deref(self.thisptr) & deref(o.thisptr))

    #TODO decide how to implement various combinations of comparisons!

    def _get_Interval_method(self, name):
        def f(*args, **kwargs):
            if self.is_empty():
                raise ValueError("OptInterval is empty.")
            else:
                return self.Interval.__getattribute__(name)(*args, **kwargs)
        return f

    def __getattr__(self, name):

        Interval_methods = set(['contains', 'contains_interval', 
        'expand_by', 'expand_to', 'extent', 'from_Interval', 'from_list', 
        'intersects', 'is_singular', 'max', 'middle', 'min', 'set_max', 
        'set_min', 'union_with'])

        if name in Interval_methods:
            return self._get_Interval_method(name)
        else:
            raise AttributeError("OptIntInterval instance has no attribute \"{}\"".format(name))

    def _wrap_Interval_method(self, name, *args, **kwargs):
        if self.isEmpty():
            raise ValueError("OptIntInterval is empty.")
        else:
            return self.Interval.__getattr__(name)(*args, **kwargs)

    #declaring these by hand, because they take fixed number of arguments,
    #which is enforced by cython

    def __neg__(self):
        """Negate interval's endpoints."""
        return self._wrap_Interval_method("__sub__")


    def __add__(cy_Interval self, other):
        """Add interval or value to self.

        Interval I+J consists of all values i+j such that i is in I and
        j is in J

        Interval I+x consists of all values i+x such that i is in I.
        """
        return self._wrap_Interval_method("__add__", other)

    def __sub__(cy_Interval self, other):
        """Substract interval or value.

        Interval I-J consists of all values i-j such that i is in I and
        j is in J

        Interval I-x consists of all values i-x such that i is in I.
        """
        return self._wrap_Interval_method("__sub__", other)

cdef cy_OptIntInterval wrap_OptIntInterval(OptIntInterval p):
    cdef OptIntInterval * retp = new OptIntInterval()
    retp[0] = p
    cdef cy_OptIntInterval r = cy_OptIntInterval.__new__(cy_OptIntInterval)
    r.thisptr = retp
    return r


cdef class cy_GenericRect:

    """Class representing axis aligned rectangle, with arbitrary corners.

    Plane in which the rectangle lies can have any object as a coordinates,
    as long as they implement arithmetic operations and comparison.

    This is a bit experimental, corresponds to GenericRect[C] templated
    with (wrapped) python object.
    """

    cdef GenericRect[WrappedPyObject]* thisptr

    def __cinit__(self, x0=0, y0=0, x1=0, y1=0):
        """Create rectangle from it's top-left and bottom-right corners."""
        self.thisptr = new GenericRect[WrappedPyObject](WrappedPyObject(x0),
                                                        WrappedPyObject(y0),
                                                        WrappedPyObject(x1),
                                                        WrappedPyObject(y1))

    def __str__(self):
        """str(self)"""
        return "Rectangle with dimensions {}, topleft point {}".format(
            str(self.dimensions()),
            str(self.min()))

    def __repr__(self):
        """repr(self)"""
        return "Rect({}, {}, {}, {})".format( str(self.left()),
                                              str(self.top()),
                                              str(self.right()),
                                              str(self.bottom()) )


    def __dealloc__(self):
        del self.thisptr

    @classmethod
    def from_intervals(self, I, J):
        """Create rectangle from two intervals.

        First interval corresponds to side parallel with x-axis,
        second one with side parallel with y-axis."""
        return cy_GenericRect(I.min(), I.max(), J.min(), J.max())

    @classmethod
    def from_list(cls, lst):
        """Create rectangle containing all points in list.

        These points are represented simply by 2-tuples.
        """
        ret = cy_GenericRect()
        for a in lst:
            ret.expand_to(a)
        return ret

    @classmethod
    def from_xywh(cls, x, y, w, h):
        """Create rectangle from topleft point and dimensions."""
        return wrap_GenericRect( from_xywh(WrappedPyObject(x),
                                        WrappedPyObject(y),
                                        WrappedPyObject(w),
                                        WrappedPyObject(h)))

    def __getitem__(self, Dim2 d):
        """self[i]"""
        return wrap_GenericInterval( deref(self.thisptr)[d] )

    def min(self):
        """Get top-left point."""
        return wrap_PyPoint( self.thisptr.min() )

    def max(self):
        """Get bottom-right point."""
        return wrap_PyPoint( self.thisptr.max() )

    def corner(self, unsigned int i):
        """Get corners (modulo) indexed from 0 to 3."""
        return wrap_PyPoint( self.thisptr.corner(i) )

    def top(self):
        """Get top coordinate."""
        return self.thisptr.top().getObj()

    def bottom(self):
        """Get bottom coordinate."""
        return self.thisptr.bottom().getObj()

    def left(self):
        """Get left coordinate."""
        return self.thisptr.left().getObj()

    def right(self):
        """Get right coordinate."""
        return self.thisptr.right().getObj()

    def width(self):
        """Get width."""
        return self.thisptr.width().getObj()

    def height(self):
        """Get height."""
        return self.thisptr.height().getObj()

    #For some reason, Cpp aspectRatio returns Coord.
    def aspectRatio(self):
        """Get ratio between width and height."""
        return float(self.width())/float(self.height())

    def dimensions(self):
        """Get dimensions as tuple."""
        return wrap_PyPoint( self.thisptr.dimensions() )

    def midpoint(self):
        """Get midpoint as tuple."""
        return wrap_PyPoint( self.thisptr.midpoint() )

    def area(self):
        """Get area."""
        return self.thisptr.area().getObj()

    def has_zero_area(self):
        """Test for area being zero."""
        return self.thisptr.hasZeroArea()

    def max_extent(self):
        """Get bigger value from width, height."""
        return self.thisptr.maxExtent().getObj()

    def min_extent(self):
        """Get smaller value from width, height."""
        return self.thisptr.minExtent().getObj()

    def intersects(self, cy_GenericRect r):
        """Check if rectangle intersects another rectangle."""
        return self.thisptr.intersects(deref( r.thisptr ))

    def contains(self, r):
        """Check if rectangle contains point represented as tuple."""
        if not isinstance(r, tuple):
            raise TypeError("Tuple required to create point.")
        return self.thisptr.contains( make_PyPoint(r) )

    def contains_rect(self, cy_GenericRect r):
        """Check if rectangle contains another rect."""
        return self.thisptr.contains( deref(r.thisptr) )

    def set_left(self, val):
        """Set left coordinate."""
        self.thisptr.setLeft( WrappedPyObject(val) )

    def set_right(self, val):
        """Set right coordinate."""
        self.thisptr.setRight( WrappedPyObject(val) )

    def set_top(self, val):
        """Set top coordinate."""
        self.thisptr.setTop( WrappedPyObject(val) )

    def set_bottom(self, val):
        """Set bottom coordinate."""
        self.thisptr.setBottom( WrappedPyObject(val) )

    def set_min(self, p):
        """Set top-left point."""
        self.thisptr.setMin(make_PyPoint(p))

    def set_max(self, p):
        """Set bottom-right point."""
        self.thisptr.setMax(make_PyPoint(p))

    def expand_to(self, p):
        """Expand rectangle to contain point represented as tuple."""
        self.thisptr.expandTo(make_PyPoint(p))

    def union_with(self, cy_GenericRect b):
        """self = self | other."""
        self.thisptr.unionWith(deref( b.thisptr ))

    def expand_by(self, x, y = None):
        """Expand both intervals.

        Either expand them both by one value, or each by different value.
        """
        if y is None:
            self.thisptr.expandBy(WrappedPyObject(x))
        else:
            self.thisptr.expandBy(WrappedPyObject(x),
                                  WrappedPyObject(y))


    def __add__(cy_GenericRect self, p):
        """Offset rectangle by point."""
        return wrap_GenericRect( deref(self.thisptr) + make_PyPoint(p) )

    def __sub__(cy_GenericRect self, p):
        """Offset rectangle by -point."""
        return wrap_GenericRect( deref(self.thisptr) - make_PyPoint(p) )

    def __or__(cy_GenericRect self, cy_GenericRect o):
        """Return union of two rects - it's actualy bounding rect of union."""
        return wrap_GenericRect( deref(self.thisptr) | deref( o.thisptr ))

    def __richcmp__(cy_GenericRect self, cy_GenericRect o, int op):
        """Rectangles are not ordered."""
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

    """Class representing axis-aligned rectangle in 2D real plane.

    Corresponds to Rect class in 2geom."""

    def __cinit__(self, Coord x0=0, Coord y0=0, Coord x1=0, Coord y1=0):
        """Create Rect from coordinates of its top-left and bottom-right corners."""
        self.thisptr = new Rect(x0, y0, x1, y1)

    def __str__(self):
        """str(self)"""
        return "Rectangle with dimensions {}, topleft point {}".format(str(self.dimensions()), str(self.min()))

    def __repr__(self):
        """repr(self)"""
        return "Rect({}, {}, {}, {})".format( str(self.left()),
                                              str(self.top()),
                                              str(self.right()),
                                              str(self.bottom()))

    def __dealloc__(self):
        del self.thisptr

    @classmethod
    def from_points(cls, cy_Point p0, cy_Point p1):
        """Create rectangle from it's top-left and bottom-right corners."""
        return wrap_Rect( Rect(deref(p0.thisptr), deref(p1.thisptr)) )

    @classmethod
    def from_intervals(cls, I, J):
        """Create rectangle from two intervals representing its sides."""
        return wrap_Rect( Rect( float(I.min()),
                                float(J.min()),
                                float(I.max()),
                                float(J.max()) ) )

    @classmethod
    def from_list(cls, lst):
        """Create rectangle containg all points in list."""
        if lst == []:
            return cy_Rect()
        if len(lst) == 1:
            return cy_Rect.from_points(lst[0], lst[0])
        ret = cy_Rect.from_points(lst[0], lst[1])
        for a in lst:
            ret.expand_to(a)
        return ret

    @classmethod
    def from_xywh(cls, x, y, w, h):
        """Create rectangle from it's topleft point and dimensions."""
        return wrap_Rect( from_xywh(<Coord> x,
                                    <Coord> y,
                                    <Coord> w,
                                    <Coord> h) )

    @classmethod
    def infinite(self):
        """Create infinite rectangle."""
        return wrap_Rect(infinite())

    def __getitem__(self, Dim2 d):
        """self[d]"""
        return wrap_Interval( deref(self.thisptr)[d] )

    def min(self):
        """Get top-left point."""
        return wrap_Point( self.thisptr.min() )

    def max(self):
        """Get bottom-right point."""
        return wrap_Point( self.thisptr.max() )

    def corner(self, unsigned int i):
        """Get corners (modulo) indexed from 0 to 3."""
        return wrap_Point( self.thisptr.corner(i) )

    def top(self):
        """Get top coordinate."""
        return self.thisptr.top()

    def bottom(self):
        """Get bottom coordinate."""
        return self.thisptr.bottom()

    def left(self):
        """Get left coordinate."""
        return self.thisptr.left()

    def right(self):
        """Get right coordinate."""
        return self.thisptr.right()

    def width(self):
        """Get width."""
        return self.thisptr.width()

    def height(self):
        """Get height."""
        return self.thisptr.height()

    def aspect_ratio(self):
        """Get ratio between width and height."""
        return self.thisptr.aspectRatio()

    def dimensions(self):
        """Get dimensions as point."""
        return wrap_Point( self.thisptr.dimensions() )

    def midpoint(self):
        """Get midpoint."""
        return wrap_Point( self.thisptr.midpoint() )

    def area(self):
        """Get area."""
        return self.thisptr.area()

    def has_zero_area(self, Coord eps = EPSILON):
        """Test for area being zero."""
        return self.thisptr.hasZeroArea(eps)

    def max_extent(self):
        """Get bigger value from width, height."""
        return self.thisptr.maxExtent()

    def min_extent(self):
        """Get smaller value from width, height."""
        return self.thisptr.minExtent()

    def intersects(self, cy_Rect r):
        """Check if rectangle intersects another rectangle."""
        return self.thisptr.intersects(deref( r.thisptr ))

    def contains(self, cy_Point r):
        """Check if rectangle contains point."""
        return self.thisptr.contains( deref(r.thisptr) )

    def contains_rect(self, cy_Rect r):
        """Check if rectangle contains another rect."""
        return self.thisptr.contains( deref(r.thisptr) )

    def interior_intersects(self, cy_Rect r):
        """Check if interior of self intersects another rectangle."""
        return self.thisptr.interiorIntersects(deref( r.thisptr ))

    def interior_contains(self, cy_Point other):
        """Check if interior of self contains point."""
        return self.thisptr.interiorContains( deref( (<cy_Point> other).thisptr ) )

    def interior_contains_rect(self, other):
        """Check if interior of self contains another rectangle."""
        if isinstance(other, cy_Rect):
            return self.thisptr.interiorContains( deref( (<cy_Rect> other).thisptr ) )
        elif isinstance(other, cy_OptRect):
            return self.thisptr.interiorContains( deref( (<cy_OptRect> other).thisptr ) )

    def set_left(self, Coord val):
        """Set left coordinate."""
        self.thisptr.setLeft(val)

    def set_right(self, Coord val):
        """Set right coordinate."""
        self.thisptr.setRight(val)

    def set_top(self, Coord val):
        """Set top coordinate."""
        self.thisptr.setTop(val)

    def set_bottom(self, Coord val):
        """Set bottom coordinate."""
        self.thisptr.setBottom(val)

    def set_min(self, cy_Point p):
        """Set top-left point."""
        self.thisptr.setMin( deref( p.thisptr ) )

    def set_max(self, cy_Point p):
        """Set bottom-right point."""
        self.thisptr.setMax( deref( p.thisptr ))

    def expand_to(self, cy_Point p):
        """Expand rectangle to contain point represented as tuple."""
        self.thisptr.expandTo( deref( p.thisptr ) )

    def union_with(self, cy_Rect b):
        """self = self | other."""
        self.thisptr.unionWith(deref( b.thisptr ))

    def expand_by(cy_Rect self, x, y = None):
        """Expand both intervals.

        Either expand them both by one value, or each by different value.
        """
        if y is None:
            if isinstance(x, cy_Point):
                self.thisptr.expandBy( deref( (<cy_Point> x).thisptr ) )
            else:
                self.thisptr.expandBy( <Coord> x)
        else:
            self.thisptr.expandBy( <Coord> x,
                                   <Coord> y)

    def __add__(cy_Rect self, cy_Point p):
        """Offset rectangle by point."""
        return wrap_Rect( deref(self.thisptr) + deref( p.thisptr ) )

    def __sub__(cy_Rect self, cy_Point p):
        """Offset rectangle by -point."""
        return wrap_Rect( deref(self.thisptr) - deref( p.thisptr ) )

    def __mul__(cy_Rect self, t):
        """Apply transform to rectangle."""
        cdef Affine at
        if is_transform(t):
            at = get_Affine(t)
            return wrap_Rect( deref(self.thisptr) * at )

    def __or__(cy_Rect self, cy_Rect o):
        """Return union of two rects - it's actualy bounding rect of union."""
        return wrap_Rect( deref(self.thisptr) | deref( o.thisptr ))

    def __richcmp__(cy_Rect self, o, int op):
        """Rectangles are not ordered."""
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

    def round_inwards(self):
        """Create OptIntRect rounding inwards."""
        return wrap_OptIntRect(self.thisptr.roundInwards())

    def round_outwards(self):
        """Create IntRect rounding outwards."""
        return wrap_IntRect(self.thisptr.roundOutwards())

    @classmethod
    def distanceSq(cls, cy_Point p, cy_Rect rect):
        """Compute square of distance between point and rectangle."""
        return distanceSq( deref(p.thisptr), deref(rect.thisptr) )

    @classmethod
    def distance(cls, cy_Point p, cy_Rect rect):
        """Compute distance between point and rectangle."""
        return distance( deref(p.thisptr), deref(rect.thisptr) )

cdef cy_Rect wrap_Rect(Rect p):
    cdef Rect* retp = new Rect()
    retp[0] = p
    cdef cy_Rect r = cy_Rect.__new__(cy_Rect)
    r.thisptr = retp
    return r


cdef class cy_OptRect:

    """Class representing optionally empty rect in real plane.

    This class corresponds to OptRect in 2geom, and it tries to mimick
    the behaviour of boost::optional. In addition to OptRect methods,
    this class passes calls for Rect methods to underlying Rect class,
    or throws ValueError when it's empty.
    """

    def __cinit__(self, x0=None, y0=None, x1=None, y1=None):
        """Create OptRect from coordinates of top-left and bottom-right corners.

        No arguments will result in empty rectangle.
        """
        if x0 is None:
            self.thisptr = new OptRect()
        else:
            self.thisptr = new OptRect( float(x0),
                                        float(y0),
                                        float(x1),
                                        float(y1) )

    def __str__(self):
        """str(self)"""
        if self.is_empty():
            return "Empty OptRect."
        return "OptRect with dimensions {}, topleft point {}".format(str(self.dimensions()), str(self.min()))

    def __repr__(self):
        """repr(self)"""
        if self.is_empty():
            return "OptRect()"
        return "OptRect({}, {}, {}, {})".format( str(self.left()),
                                                 str(self.top()),
                                                 str(self.right()),
                                                 str(self.bottom()))

    def __dealloc__(self):
        del self.thisptr

    @classmethod
    def from_points(cls, cy_Point p0, cy_Point p1):
        """Create rectangle from it's top-left and bottom-right corners."""
        return wrap_OptRect( OptRect(deref(p0.thisptr), deref(p1.thisptr)) )

    @classmethod
    def from_intervals(cls, I, J):
        """Create rectangle from two intervals representing its sides."""
        if hasattr(I, "isEmpty"):
            if I.isEmpty():
                return cy_OptRect()

        if hasattr(J, "isEmpty"):
            if J.isEmpty():
                return cy_OptRect()

        return wrap_OptRect( OptRect( float(I.min()),
                                      float(J.min()),
                                      float(I.max()),
                                      float(J.max()) ) )

    @classmethod
    def from_rect(cls, r):
        """Create OptRect from other, possibly empty, rectangle."""
        if hasattr(r, "isEmpty"):
            if r.isEmpty():
                return cy_OptRect()

        return cy_OptRect(  r.min().x,
                            r.min().y,
                            r.max().x,
                            r.max().y )

    @classmethod
    def from_list(cls, lst):
        """Create OptRect containing all points in the list.

        Empty list will result in empty OptRect.
        """
        if lst == []:
            return cy_OptRect()
        if len(lst) == 1:
            return cy_OptRect.from_points(lst[0], lst[0])
        ret = cy_OptRect.from_points(lst[0], lst[1])
        for a in lst:
            ret.expand_to(a)
        return ret

    property Rect:
        """Get underlying Rect."""
        def __get__(self):
            if self.is_empty():
                raise ValueError("Rect is empty.")
            else:
                return wrap_Rect(self.thisptr.get())

    def __bool__(self):
        """OptRect is False only when it's empty."""
        return not self.thisptr.isEmpty()

    def is_empty(self):
        """Check for OptRect containing no points."""
        return self.thisptr.isEmpty()

    def intersects(self, other):
        """Check if rectangle intersects another rectangle."""
        if isinstance(other, cy_Rect):
            return self.thisptr.intersects( deref( (<cy_Rect> other).thisptr ) )
        elif isinstance(other, cy_OptRect):
            return self.thisptr.intersects( deref( (<cy_OptRect> other).thisptr ) )

    def contains(self, cy_Point r):
        """Check if rectangle contains point."""
        return self.thisptr.contains( deref(r.thisptr) )

    def contains_rect(self, other):
        """Check if rectangle contains another rect."""
        if isinstance(other, cy_Rect):
            return self.thisptr.contains( deref( (<cy_Rect> other).thisptr ) )
        elif isinstance(other, cy_OptRect):
            return self.thisptr.contains( deref( (<cy_OptRect> other).thisptr ) )

    def union_with(self, other):
        """self = self | other."""
        if isinstance(other, cy_Rect):
            self.thisptr.unionWith( deref( (<cy_Rect> other).thisptr ) )
        elif isinstance(other, cy_OptRect):
            self.thisptr.unionWith( deref( (<cy_OptRect> other).thisptr ) )

    def intersect_with(self, other):
        """self = self & other."""
        if isinstance(other, cy_Rect):
            self.thisptr.intersectWith( deref( (<cy_Rect> other).thisptr ) )
        elif isinstance(other, cy_OptRect):
            self.thisptr.intersectWith( deref( (<cy_OptRect> other).thisptr ) )

    def expand_to(self, cy_Point p):
        """Expand rectangle to contain point represented as tuple."""
        self.thisptr.expandTo( deref(p.thisptr) )

    def __or__(cy_OptRect self, cy_OptRect other):
        """Return union of two rects - it's actualy bounding rect of union."""
        return wrap_OptRect( deref(self.thisptr) | deref(other.thisptr) )

    def __and__(cy_OptRect self, other):
        """Return intersection of two rectangles."""
        if isinstance(other, cy_Rect):
            return wrap_OptRect( deref(self.thisptr) & deref( (<cy_Rect> other).thisptr) )
        elif isinstance(other, cy_OptRect):
            return wrap_OptRect( deref(self.thisptr) & deref( (<cy_OptRect> other).thisptr) )

    def __richcmp__(cy_OptRect self, other, op):
        """Rectangles are not ordered."""
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
        return NotImplemented

    def _get_Rect_method(self, name):
        def f(*args, **kwargs):
            if self.is_empty():
                raise ValueError("OptRect is empty.")
            else:
                return self.Rect.__getattribute__(name)(*args, **kwargs)
        return f

    def __getattr__(self, name):
        Rect_methods = set(['area', 'aspectRatio', 'bottom', 'contains',
        'contains_rect', 'corner', 'dimensions', 'distance', 'distanceSq',
        'expand_by', 'expand_to', 'has_zero_area', 'height', 'infinite', 
        'interior_contains', 'interior_contains_rect',
        'interior_intersects', 'intersects', 'left', 'max', 'max_extent',
        'midpoint', 'min', 'min_extent', 'right', 'round_inwards',
        'round_outwards', 'set_bottom', 'set_left', 'set_max', 'set_min',
        'set_right', 'set_top', 'top', 'union_with', 'width'])

        if name in Rect_methods:
            return self._get_Rect_method(name)
        else:
            raise AttributeError("OptRect instance has no attribute \"{}\"".format(name))

    def _wrap_Rect_method(self, name, *args, **kwargs):
        if self.isEmpty():
            raise ValueError("OptRect is empty.")
        else:
            return self.Rect.__getattr__(name)(*args, **kwargs)

    #declaring these by hand, because they take fixed number of arguments,
    #which is enforced by cython

    def __getitem__(self, i):
        """self[d]"""
        return self._wrap_Rect_method("__getitem__", i)

    def __add__(self, other):
        """Offset rectangle by point."""
        return self._wrap_Rect_method("__add__", other)

    def __mul__(self, other):
        """Apply transform to rectangle."""
        return self._wrap_Rect_method("__mul__", other)

    def __sub__(self, other):
        """Offset rectangle by -point."""
        return self._wrap_Rect_method("__sub__", other)


cdef cy_OptRect wrap_OptRect(OptRect p):
    cdef OptRect* retp = new OptRect()
    retp[0] = p
    cdef cy_OptRect r = cy_OptRect.__new__(cy_OptRect)
    r.thisptr = retp
    return r



cdef class cy_IntRect:

    """Class representing axis-aligned rectangle in 2D with integer coordinates.

    Corresponds to IntRect class (typedef) in 2geom."""

    cdef IntRect* thisptr

    def __cinit__(self, IntCoord x0=0, IntCoord y0=0, IntCoord x1=0, IntCoord y1=0):
        """Create IntRect from coordinates of its top-left and bottom-right corners."""
        self.thisptr = new IntRect(x0, y0, x1, y1)

    def __str__(self):
        """str(self)"""
        return "IntRect with dimensions {}, topleft point {}".format(
            str(self.dimensions()),
            str(self.min()))

    def __repr__(self):
        """repr(self)"""
        return "IntRect({}, {}, {}, {})".format( str(self.left()),
                                                 str(self.top()),
                                                 str(self.right()),
                                                 str(self.bottom()))

    def __dealloc__(self):
        del self.thisptr

    @classmethod
    def from_points(cls, cy_IntPoint p0, cy_IntPoint p1):
        """Create rectangle from it's top-left and bottom-right corners."""
        return wrap_IntRect( IntRect(deref(p0.thisptr), deref(p1.thisptr)) )

    @classmethod
    def from_intervals(cls, I, J):
        """Create rectangle from two intervals representing its sides."""
        return cy_IntRect(  int(I.min()),
                            int(J.min()),
                            int(I.max()),
                            int(J.max()) )

    @classmethod
    def from_list(cls, lst):
        """Create rectangle containg all points in list."""
        if lst == []:
            return cy_IntRect()
        if len(lst) == 1:
            return cy_IntRect(lst[0], lst[0])
        ret = cy_IntRect(lst[0], lst[1])
        for a in lst:
            ret.expand_to(a)
        return ret

    #didn't manage to declare from_xywh for IntRect
    @classmethod
    def from_xywh(cls, x, y, w, h):
        """Create rectangle from it's topleft point and dimensions."""
        return cy_IntRect(int(x),
                          int(y),
                          int(x) + int(w),
                          int(y) + int(h) )

    def __getitem__(self, Dim2 d):
        """self[d]"""
        return wrap_IntInterval( deref(self.thisptr)[d] )

    def min(self):
        """Get top-left point."""
        return wrap_IntPoint( self.thisptr.i_min() )

    def max(self):
        """Get bottom-right point."""
        return wrap_IntPoint( self.thisptr.i_max() )

    def corner(self, unsigned int i):
        """Get corners (modulo) indexed from 0 to 3."""
        return wrap_IntPoint( self.thisptr.i_corner(i) )

    def top(self):
        """Get top coordinate."""
        return self.thisptr.top()

    def bottom(self):
        """Get bottom coordinate."""
        return self.thisptr.bottom()

    def left(self):
        """Get left coordinate."""
        return self.thisptr.left()

    def right(self):
        """Get right coordinate."""
        return self.thisptr.right()

    def width(self):
        """Get width."""
        return self.thisptr.width()

    def height(self):
        """Get height."""
        return self.thisptr.height()

    def aspect_ratio(self):
        """Get ratio between width and height."""
        return self.thisptr.aspectRatio()

    def dimensions(self):
        """Get dimensions as IntPoint."""
        return wrap_IntPoint( self.thisptr.i_dimensions() )

    def midpoint(self):
        """Get midpoint."""
        return wrap_IntPoint( self.thisptr.i_midpoint() )

    def area(self):
        """Get area."""
        return self.thisptr.area()

    def has_zero_area(self):
        """Test for area being zero."""
        return self.thisptr.hasZeroArea()

    def max_extent(self):
        """Get bigger value from width, height."""
        return self.thisptr.maxExtent()

    def min_extent(self):
        """Get smaller value from width, height."""
        return self.thisptr.minExtent()

    def intersects(self, cy_IntRect r):
        """Check if rectangle intersects another rectangle."""
        return self.thisptr.intersects(deref( r.thisptr ))

    def contains(self, cy_IntPoint r):
        """Check if rectangle contains point."""
        return self.thisptr.contains( deref(r.thisptr) )

    def contains_rect(self, cy_IntRect r):
        """Check if rectangle contains another rect."""
        return self.thisptr.contains( deref(r.thisptr) )

    def set_left(self, IntCoord val):
        """Set left coordinate."""
        self.thisptr.setLeft(val)

    def set_right(self, IntCoord val):
        """Set right coordinate."""
        self.thisptr.setRight(val)

    def set_top(self, IntCoord val):
        """Set top coordinate."""
        self.thisptr.setTop(val)

    def set_bottom(self, IntCoord val):
        """Set bottom coordinate."""
        self.thisptr.setBottom(val)

    def set_min(self, cy_IntPoint p):
        """Set top-left point."""
        self.thisptr.setMin( deref( p.thisptr ) )

    def set_max(self, cy_IntPoint p):
        """Set bottom-right point."""
        self.thisptr.setMax( deref( p.thisptr ))

    def expand_to(self, cy_IntPoint p):
        """Expand rectangle to contain point represented as tuple."""
        self.thisptr.expandTo( deref( p.thisptr ) )

    def union_with(self, cy_IntRect b):
        """self = self | other."""
        self.thisptr.unionWith(deref( b.thisptr ))

    def expand_by(cy_IntRect self, x, y = None):
        """Expand both intervals.

        Either expand them both by one value, or each by different value.
        """
        if y is None:
            if isinstance(x, cy_IntPoint):
                self.thisptr.expandBy( deref( (<cy_IntPoint> x).thisptr ) )
            else:
                self.thisptr.expandBy( <IntCoord> x)
        else:
            self.thisptr.expandBy( <IntCoord> x,
                                   <IntCoord> y)

    def __add__(cy_IntRect self, cy_IntPoint p):
        """Offset rectangle by point."""
        return wrap_IntRect( deref(self.thisptr) + deref( p.thisptr ) )

    def __sub__(cy_IntRect self, cy_IntPoint p):
        """Offset rectangle by -point."""
        return wrap_IntRect( deref(self.thisptr) - deref( p.thisptr ) )

    def __or__(cy_IntRect self, cy_IntRect o):
        """Return union of two rects - it's actualy bounding rect of union."""
        return wrap_IntRect( deref(self.thisptr) | deref( o.thisptr ))

    def __richcmp__(cy_IntRect self, cy_IntRect o, int op):
        """Rectangles are not ordered."""
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

    """Class representing optionally empty rect in with integer coordinates.

    This class corresponds to OptIntRect in 2geom, and it tries to mimick
    the behaviour of boost::optional. In addition to OptIntRect methods,
    this class passes calls for IntRect methods to underlying IntRect class,
    or throws ValueError when it's empty.
    """

    cdef OptIntRect* thisptr
    
    def __cinit__(self, x0=None, y0=None, x1=None, y1=None):
        """Create OptIntRect from coordinates of top-left and bottom-right corners.

        No arguments will result in empty rectangle.
        """
        if x0 is None:
            self.thisptr = new OptIntRect()
        else:
            self.thisptr = new OptIntRect( int(x0),
                                        int(y0),
                                        int(x1),
                                        int(y1) )
    
    def __str__(self):
        """str(self)"""
        if self.isEmpty():
            return "Empty OptIntRect"
        return "OptIntRect with dimensions {}, topleft point {}".format(
            str(self.Rect.dimensions()), 
            str(self.Rect.min()))
    
    def __repr__(self):
        """repr(self)"""
        if self.isEmpty():
            return "OptIntRect()"
        return "OptIntRect({}, {}, {}, {})".format( str(self.Rect.left()),
                                                    str(self.Rect.top()),
                                                    str(self.Rect.right()),
                                                    str(self.Rect.bottom()))
    
    def __dealloc__(self):
        del self.thisptr
    
    @classmethod
    def from_points(cls, cy_IntPoint p0, cy_IntPoint p1):
        """Create rectangle from it's top-left and bottom-right corners."""
        return wrap_OptIntRect( OptIntRect(deref(p0.thisptr), deref(p1.thisptr)) )

    @classmethod
    def from_intervals(cls, I, J):
        """Create rectangle from two intervals representing its sides."""
        if hasattr(I, "isEmpty"):
            if I.isEmpty():
                return cy_OptIntRect()

        if hasattr(J, "isEmpty"):
            if J.isEmpty():
                return cy_OptIntRect()

        return wrap_OptIntRect( OptIntRect( int(I.min()),
                                            int(J.min()),
                                            int(I.max()),
                                            int(J.max()) ) )

    @classmethod
    def from_rect(cls, r):
        """Create OptIntRect from other, possibly empty, rectangle."""
        if hasattr(r, "isEmpty"):
            if r.isEmpty():
                return cy_OptIntRect()
        return cy_OptIntRect(   r.min().x,
                                r.min().y,
                                r.max().x,
                                r.max().y )

    @classmethod
    def from_list(cls, lst):
        """Create OptIntRect containing all points in the list.

        Empty list will result in empty OptIntRect.
        """
        if lst == []:
            return cy_OptIntRect()
        if len(lst) == 1:
            return cy_OptIntRect.from_points(lst[0], lst[0])
        ret = cy_OptIntRect.from_points(lst[0], lst[1])
        for a in lst:
            ret.expand_to(a)
        return ret

    property Rect:
        """Get underlying IntRect."""
        def __get__(self):
            return wrap_IntRect(self.thisptr.get())

    def __bool__(self):
        """OptIntRect is False only when it's empty."""
        return not self.thisptr.isEmpty()

    def is_empty(self):
        """Check for OptIntRect containing no points."""
        return self.thisptr.isEmpty()

    def intersects(cy_OptIntRect self, other):
        """Check if rectangle intersects another rectangle."""
        if isinstance(other, cy_IntRect):
            return self.thisptr.intersects( deref( (<cy_IntRect> other).thisptr ) )
        elif isinstance(other, cy_OptIntRect):
            return self.thisptr.intersects( deref( (<cy_OptIntRect> other).thisptr ) )
            
    def contains(self, cy_IntPoint other):
        """Check if rectangle contains point."""
        return self.thisptr.contains( deref(other.thisptr) )

    def contains_rect(cy_OptIntRect self, other):
        """Check if rectangle contains another rectangle."""
        if isinstance(other, cy_IntRect):
            return self.thisptr.contains( deref( (<cy_IntRect> other).thisptr ) )
        elif isinstance(other, cy_OptIntRect):
            return self.thisptr.contains( deref( (<cy_OptIntRect> other).thisptr ) )

            
    def union_with(cy_OptIntRect self, other):
        """self = self | other."""
        if isinstance(other, cy_IntRect):
            self.thisptr.unionWith( deref( (<cy_IntRect> other).thisptr ) )
        elif isinstance(other, cy_OptIntRect):
            self.thisptr.unionWith( deref( (<cy_OptIntRect> other).thisptr ) )
            
    def intersect_with(cy_OptIntRect self, other):
        """self = self & other."""
        if isinstance(other, cy_IntRect):
            self.thisptr.intersectWith( deref( (<cy_IntRect> other).thisptr ) )
        elif isinstance(other, cy_OptIntRect):
            self.thisptr.intersectWith( deref( (<cy_OptIntRect> other).thisptr ) )
            
    def expand_to(self, cy_IntPoint p):
        """Expand rectangle to contain point."""
        self.thisptr.expandTo( deref(p.thisptr) )

    def __or__(cy_OptIntRect self, cy_OptIntRect other):
        """Return union of two rects - it's actualy bounding rect of union."""
        return wrap_OptIntRect( deref(self.thisptr) | deref(other.thisptr) )
        
    def __and__(cy_OptIntRect self, other):
        """Return intersection of two rectangles."""
        if isinstance(other, cy_IntRect):
            return wrap_OptIntRect( deref(self.thisptr) & deref( (<cy_IntRect> other).thisptr) )
        elif isinstance(other, cy_OptIntRect):
            return wrap_OptIntRect( deref(self.thisptr) & deref( (<cy_OptIntRect> other).thisptr) )

    def __richcmp__(cy_OptIntRect self, other, op):
        """Rectangles are not ordered."""
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

    def _get_Rect_method(self, name):
        def f(*args, **kwargs):
            if self.is_empty():
                raise ValueError("OptIntRect is empty.")
            else:
                return self.Rect.__getattribute__(name)(*args, **kwargs)
        return f

    def __getattr__(self, name):

        Rect_methods = set(['area', 'aspect_ratio', 'bottom', 'contains', 
        'contains_rect', 'corner', 'dimensions', 'expand_by', 'expand_to', 
        'from_intervals', 'from_list', 'from_points', 'from_xywh', 
        'has_zero_area', 'height', 'intersects', 'left', 'max', 
        'max_extent', 'midpoint', 'min', 'min_extent', 'right', 
        'set_bottom', 'set_left', 'set_max', 'set_min', 'set_right', 
        'set_top', 'top', 'union_with', 'width'])

        if name in Rect_methods:
            return self._get_Rect_method(name)
        else:
            raise AttributeError("OptIntRect instance has no attribute \"{}\"".format(name))

    def _wrap_Rect_method(self, name, *args, **kwargs):
        if self.isEmpty():
            raise ValueError("OptIntRect is empty.")
        else:
            return self.Rect.__getattr__(name)(*args, **kwargs)

    #declaring these by hand, because they take fixed number of arguments,
    #which is enforced by cython

    def __getitem__(self, i):
        """self[d]"""
        return self._wrap_Rect_method("__getitem__", i)

    def __add__(self, other):
        """Offset rectangle by point."""
        return self._wrap_Rect_method("__add__", other)

    def __sub__(self, other):
        """Offset rectangle by -point."""
        return self._wrap_Rect_method("__sub__", other)

cdef cy_OptIntRect wrap_OptIntRect(OptIntRect p):
    cdef OptIntRect* retp = new OptIntRect()
    retp[0] = p
    cdef cy_OptIntRect r = cy_OptIntRect.__new__(cy_OptIntRect)
    r.thisptr = retp
    return r
