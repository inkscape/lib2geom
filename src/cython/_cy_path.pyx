from cython.operator cimport dereference as deref
from numbers import Number
from _cy_rectangle cimport cy_OptInterval, wrap_OptInterval, wrap_Rect, OptRect, wrap_OptRect
from _cy_rectangle cimport cy_Interval, wrap_Interval

from _cy_affine cimport cy_Affine, wrap_Affine, get_Affine, is_transform

from _cy_curves cimport is_Curve, get_Curve_p


cdef class cy_Iterator:
    cdef Iterator* thisptr
    cdef ConstIterator* startptr
    cdef ConstIterator* endptr

    def __cinit__(self):
        self.thisptr = new Iterator()

    def __dealloc__(self):
        del self.thisptr

    def __iter__(self):
        return self

    def __next__(self):
        if deref(<BaseIterator[ConstIterator, Path] *>  self.endptr) == deref(<BaseIterator[ConstIterator, Path] *> self.thisptr):
            raise StopIteration
        cdef Curve * r = <Curve *> &(<BaseIterator[Iterator, Path] * > self.thisptr).c_item()
        (<BaseIterator[Iterator, Path] *> self.thisptr).c_next()
        return wrap_Curve_p ( r )




cdef cy_Iterator wrap_Iterator(Iterator i, ConstIterator starti, ConstIterator endi):
    cdef Iterator * retp = new Iterator()
    retp[0] = i
    cdef cy_Iterator r = cy_Iterator.__new__(cy_Iterator)
    r.thisptr = retp

    cdef ConstIterator * endp = new ConstIterator()
    endp[0] = endi
    r.endptr = endp

    cdef ConstIterator * startp = new ConstIterator()
    startp[0] = starti
    r.startptr = startp

    return r

cdef class cy_Path:

    """Path is a ordered sequence of curves.

    You can iterate this class, accessing curves one at time, or access
    them using indices.

    Two constants, NO_STITCHING and STITCH_DISCONTINUOUS are members of
    Path namespace, and are used to specify type of stitching, if
    necessary.

    Path is either open or closed, but in both cases carries closing
    segment, connecting last and first point.

    Corresponds to Path class in 2geom.
    """

    NO_STITCHING = c_NO_STITCHING
    STITCH_DISCONTINUOUS = c_STITCH_DISCONTINUOUS

    def __cinit__(self, cy_Point p=cy_Point()):
        """Create Path containing only one point."""
        self.thisptr = new Path(deref( p.thisptr ))

    @classmethod
    def fromList(cls, l, Stitching stitching=NO_STITCHING, closed=False):
        """Create path from list of curves.

        Specify stithing and closed flag in additional arguments.
        """
        p = cy_Path()
        for curve in l:
            p.append_curve(curve, stitching)
        p.close(closed)
        return p

    @classmethod
    def fromPath(cls, cy_Path p, fr=-1, to = 1, bint closed=False):
        """Create path copying another's path curves.

        Either copy all curves, or ones from
        fr - index of first curve copied
        to
        to -  index of first curve not copied.

        Also takes closed flag.
        """
        if fr == -1:
            return wrap_Path( Path(p.thisptr.begin_const(), p.thisptr.end_default(), closed) )
        else:
            return wrap_Path( Path(p._const_iterator_at_index(fr), p._const_iterator_at_index(to), closed) )

    def __dealloc__(self):
        del self.thisptr

    def __getitem__(self, unsigned int i):
        """Get curve with index i."""
        cdef Curve * r = <Curve *> & deref(self.thisptr)[i]
        return wrap_Curve_p(r)

    def __call__(self, double t):
        """Evaluate path at time t.

        Note: t can be greater than 1 here, it can go to self.size()
        """
        return wrap_Point(deref( self.thisptr ) (t) )

    def __richcmp__(cy_Path self, cy_Path other, int op):
        if op == 2:
            return deref( self.thisptr ) == deref( other.thisptr )
        elif op == 3:
            return deref( self.thisptr ) != deref( other.thisptr )

    def __mul__(cy_Path self, m):
        """Transform path with a transform."""
        cdef Affine at
        if is_transform(m):
            at = get_Affine(m)
            return wrap_Path( deref(self.thisptr) * at )

    #This is not the fastest way, but it's pretty nice from python's perspective
    #Anyway, I would expect that performance hit is minimal, since i is generally really small
    cdef ConstIterator _const_iterator_at_index(self, int i):
        cdef ConstIterator ci = self.thisptr.begin_const()
        cdef ConstIterator * cip = &ci
        for ii in range(i):
            (<BaseIteratorConst *> cip).c_next()
        return ci

    cdef Iterator _iterator_at_index(self, int i):
        cdef Iterator ci = self.thisptr.begin()
        cdef Iterator * cip = &ci
        for ii in range(i):
            (<BaseIterator[Iterator, Path] *> cip).c_next()
        return ci

    def swap(self, cy_Path other):
        """Swap curves with another path."""
        self.thisptr.swap(deref( other.thisptr ))

#This is the same as __getitem__
#~     def at_index(self, unsigned int i):
#~         return wrap_Curve(self.thisptr.at_index(i))

    def front(self):
        """Get first curve."""
        #this is AFAIK the shortest way to do this
        cdef Curve * r = <Curve *> &self.thisptr.front()
        return wrap_Curve_p(r)

    def back(self):
        """Same as back_open."""
        cdef Curve * r = <Curve *> &self.thisptr.back()
        return wrap_Curve_p(r)

    def back_open(self):
        """Get last curve, treating self as open."""
        cdef Curve * r = <Curve *> &self.thisptr.back_open()
        return wrap_Curve_p(r)

    def back_closed(self):
        """Get last curve, treating self as closed."""
        cdef Curve * r = <Curve *> &self.thisptr.back_closed()
        return wrap_Curve_p(r)

    def back_default(self):
        """Get last curve."""
        cdef Curve * r = <Curve *> &self.thisptr.back_default()
        return wrap_Curve_p(r)

    def curves(self):
        """Same as curves_open"""
        return wrap_Iterator(self.thisptr.begin(), self.thisptr.begin_const(),  self.thisptr.end())

    def curves_open(self):
        """Return all curves as iterable, treating self as open."""
        return wrap_Iterator(self.thisptr.begin(), self.thisptr.begin_const(), self.thisptr.end_open())

    def curves_closed(self):
        """Return all curves as iterable, treating self as closed."""
        return wrap_Iterator(self.thisptr.begin(), self.thisptr.begin_const(), self.thisptr.end_closed())

    def curves_default(self):
        """Return all curves as iterable."""
        return wrap_Iterator(self.thisptr.begin(), self.thisptr.begin_const(), self.thisptr.end_default())

    def __iter__(self):
        return self.curves_default()

    def size_open(self):
        """Get number of curves, treating self as open."""
        return self.thisptr.size_open()

    def size_closed(self):
        """Get number of curves, treating self as closed."""
        return self.thisptr.size_closed()

    def size_default(self):
        """Get number of curves."""
        return self.thisptr.size_default()

    def size(self):
        """Same as size_open."""
        return self.thisptr.size()

#Does the same as size_open, which doesn't correspond with name.
#~     def max_size(self):
#~         return self.thisptr.max_size()

    def empty(self):
        """Test whether path contains no curves."""
        return self.thisptr.empty()

    def closed(self):
        """Return state of closed flag."""
        return self.thisptr.closed()

    def close(self, bint closed):
        """Set closed flag."""
        self.thisptr.close(closed)

    def bounds_fast(self):
        """Return fast bounding rectangle for path.

        It's not guaranteed to give the tighest bound.
        """
        return wrap_OptRect(self.thisptr.boundsFast())

    def bounds_exact(self):
        """Give the tighest bounding rectangle for path."""
        return wrap_OptRect(self.thisptr.boundsExact())

#~     def toPwSb(self):
#~         return wrap_Piecewise<Geom::D2<Geom::SBasis> >(self.thisptr.toPwSb())

    def point_at(self, double t):
        """Same as self(t)."""
        return wrap_Point(self.thisptr.pointAt(t))

    def value_at(self, double t, Dim2 d):
        """Same as self(t)[d]."""
        return self.thisptr.valueAt(t, d)

    def __call__(self, double t):
        """Evaluate path at time t.

        Equivalent to self[floor(t)](t-floor(t))
        """
        return wrap_Point(deref( self.thisptr ) (t) )

    def roots(self, double v, Dim2 d):
        """Find time values where self(t)[d] == v"""
        return wrap_vector_double(self.thisptr.roots(v, d))

    def all_nearest_times(self, cy_Point _point, double fr=-1, double to=1):
        """Return all values of t that |self(t) - point| is minimized."""
        if fr == -1:
            return wrap_vector_double(self.thisptr.allNearestTimes(deref( _point.thisptr )))
        return wrap_vector_double(self.thisptr.allNearestTimes(deref( _point.thisptr ), fr, to))


    def nearest_time_per_curve(self, cy_Point _point):
        """Find nearest points, return one time value per each curve."""
        return wrap_vector_double(self.thisptr.nearestTimePerCurve(deref( _point.thisptr )))

    def nearest_time(self, cy_Point _point, double fr=-1, double to=1):#, cy_double * distance_squared):
        """Return such t that |self(t) - point| is minimized."""
        if fr == -1:
            return self.thisptr.nearestTime(deref( _point.thisptr ), NULL)
        return self.thisptr.nearestTime(deref( _point.thisptr ), fr, to, NULL)

    def nearest_time_and_dist_sq(self, cy_Point _point, double fr=-1, double to=1):
        """Return such t that |self(t) - point| is minimized and square of that distance."""
        cdef double t, dist
        if fr == -1:
            t = self.thisptr.nearestTime(deref( _point.thisptr ), &dist )
        else:
            t = self.thisptr.nearestTime(deref( _point.thisptr ), fr, to, &dist)
        return (t, dist)

    def append_portion_to(self, cy_Path p, double f, double t):
        """Append portion of path to self."""
        self.thisptr.appendPortionTo(deref( p.thisptr ), f, t)

    def portion(self, Coord fr=0, Coord to=1, cy_Interval interval=None):
        """Return portion of curve between two time values.

        Alternatively use argument interval.
        """
        if interval is None:
            return wrap_Path(self.thisptr.portion(fr, to))
        else:
            return wrap_Path(self.thisptr.portion(deref( interval.thisptr )))

    def reversed(self):
        """Return reversed curve."""
        return wrap_Path(self.thisptr.reversed())

    def insert(self, int pos, curve, Stitching stitching=NO_STITCHING):
        """Insert curve into position pos.

        Args:
            pos: Position of inserted curve.
            curve: Curve to insert.
            stitching=NO_STITCHING
        """
        cdef Curve * cptr = get_Curve_p(curve)
        if cptr:
            self.thisptr.insert( self._iterator_at_index(pos), deref( cptr ), stitching )
        else:
            raise TypeError("passed curve is not C++ Curve")

    def insert_slice(self, int pos, cy_Path p, int first, int last, Stitching stitching=NO_STITCHING):
        """Insert curves to position pos.

        Args:
            pos: Position of inserted slice.
            p: Path from which slice is inserted.
            first: First inserted curve position (in p).
            last: Fist not inserted curve position (in p).
            stiching=NO_STITCHING
        """
        self.thisptr.insert(self._iterator_at_index(pos), p._const_iterator_at_index(first), p._const_iterator_at_index(last), stitching)

    def clear(self):
        """Clear all curves."""
        self.thisptr.clear()

    def erase(self, int pos, Stitching stitching=NO_STITCHING):
        """Erase curve at position pos.

        Args:
            pos: Position of erased curve.
            stitching=NO_STITCHING
        """
        self.thisptr.erase(self._iterator_at_index(pos), stitching)

    def erase_slice(self, int start, int end, Stitching stitching=NO_STITCHING):
        """Erase curves with indices [start, end).

        Args:
            start, end: Curves with indices start...end-1 are erased
            stitching=NO_STITCHING
        """
        self.thisptr.erase(self._iterator_at_index(start), self._iterator_at_index(end), stitching)

    def erase_last(self):
        """Erase last curve."""
        self.thisptr.erase_last()

    def replace(self, int replaced, curve, Stitching stitching=NO_STITCHING):
        """Replace curve at position replaced with another curve.

        Args:
            replaced: Position of replaced curve.
            curve: New curve.
            stitching=NO_STITCHING
        """
        cdef Curve * cptr = get_Curve_p(curve)
        if cptr:
            self.thisptr.replace(self._iterator_at_index(replaced), deref( cptr ), stitching)
        else:
            raise TypeError("passed curve is not C++ Curve")

    def replace_slice(self, int first_replaced, int last_replaced, curve, Stitching stitching=NO_STITCHING):
        """Replace slice of curves by new curve.

        Args:
            first_replaced, last_replace: Curves with indices
                first_replaced ... last_replaced
            curve: New curve.
            stitching=NO_STITCHING
        """
        cdef Curve * cptr = get_Curve_p(curve)
        if cptr:
            self.thisptr.replace(self._iterator_at_index(first_replaced), self._iterator_at_index(last_replaced), deref( cptr ), stitching)
        else:
            raise TypeError("passed curve is not C++ Curve")

#How to implement this nicely?
#~     def replaceByList(self, int replaced, cy_ConstIterator first, cy_ConstIterator last, Stitching stitching):
#~         self.thisptr.replace(deref( replaced.thisptr ), deref( first.thisptr ), deref( last.thisptr ), stitching)
#~     def replace(self, cy_Iterator first_replaced, cy_Iterator last_replaced, cy_ConstIterator first, cy_ConstIterator last, Stitching stitching):
#~         self.thisptr.replace(deref( first_replaced.thisptr ), deref( last_replaced.thisptr ), deref( first.thisptr ), deref( last.thisptr ), stitching)

    def start(self, cy_Point p):
        """Erase all curves and set first point."""
        self.thisptr.start(deref( p.thisptr ))

    def initial_point(self):
        """Get initial point."""
        return wrap_Point(self.thisptr.initialPoint())

    def final_point(self):
        """Get final point."""
        return wrap_Point(self.thisptr.finalPoint())

    def set_initial(self, cy_Point p):
        """Set initial point."""
        self.thisptr.setInitial(deref( p.thisptr ))

    def set_final(self, cy_Point p):
        """Set final point."""
        self.thisptr.setFinal(deref( p.thisptr ))

    def append_curve(self, curve, Stitching stitching=NO_STITCHING):
        """Append curve to path.

        Args:
            curve: Curve to append.
            stitching=NO_STITCHING
        """
        cdef Curve * cptr = get_Curve_p(curve)
        if cptr:
            self.thisptr.append( deref( cptr ), stitching)
        else:
            raise TypeError("passed curve is not C++ Curve")

    def append_SBasis(self, cy_SBasis x, cy_SBasis y, Stitching stitching=NO_STITCHING):
        """Append two SBasis functions to path.

        Args:
            x, y: SBasis functions to append.
            stitching=NO_STITCHING
        """
        cdef D2[SBasis] sb = D2[SBasis]( deref(x.thisptr), deref(y.thisptr) )
        self.thisptr.append(sb, stitching)

    def append_path(self, cy_Path other, Stitching stitching=NO_STITCHING):
        """Append another path to path.

        Args:
            other: Path to append.
            stitching=NO_STITCHING
        """
        self.thisptr.append(deref( other.thisptr ), stitching)

    def stitch_to(self, cy_Point p):
        """Set last point to p, creating stitching segment to it."""
        self.thisptr.stitchTo(deref( p.thisptr ))

cdef cy_Path wrap_Path(Path p):
    cdef Path * retp = new Path(Point())
    retp[0] = p
    cdef cy_Path r = cy_Path.__new__(cy_Path)
    r.thisptr = retp
    return r
