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
    NO_STITCHING = c_NO_STITCHING
    STITCH_DISCONTINUOUS = c_STITCH_DISCONTINUOUS
   # cdef Path* thisptr
#~     def __init__(self, cy_PathInternal::ConstIterator first, cy_PathInternal::ConstIterator last, bint closed):
#~         self.thisptr = new Path(deref( first.thisptr ) ,deref( last.thisptr ) ,closed)
    def __cinit__(self, *args):
        if len(args) == 0:
            self.thisptr = new Path(Point())

        elif len(args) == 1:
            if isinstance(args[0], cy_Point):
                self.thisptr = new Path(deref( (<cy_Point> args[0]).thisptr ))
    @classmethod
    def fromList(cls, l, Stitching stitching=NO_STITCHING, closed=False):
        p = cy_Path()
        for curve in l:
            p.appendCurve(curve, stitching)
        p.close(closed)
        return p
    @classmethod
    def fromPath(cls, cy_Path p, fr=-1, to = 1, bint closed=False):
        if fr == -1:
            return wrap_Path( Path(p.thisptr.begin_const(), p.thisptr.end_default(), closed) )
        else:
            return wrap_Path( Path(p._const_iterator_at_index(fr), p._const_iterator_at_index(to), closed) )
    
    def __getitem__(self, unsigned int i):
        cdef Curve * r = <Curve *> & deref(self.thisptr)[i]
        return wrap_Curve_p(r)
    
#~     def __getslice__(self, Py_ssize_t i, Py_ssize_t j):
#~         return wrap_Iterator( self.thisptr.begin(), self._const_iterator_at_index(i), self._const_iterator_at_index(j) )
        
    def __call__(self, double t):
        return wrap_Point(deref( self.thisptr ) (t) )
    
    def __richcmp__(cy_Path self, cy_Path other, int op):
        if op == 2:
            return deref( self.thisptr ) == deref( other.thisptr )
        elif op == 3:
            return deref( self.thisptr ) != deref( other.thisptr )

    def __mul__(cy_Path self, m):
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
        self.thisptr.swap(deref( other.thisptr ))
#This is the same as __getitem__
#~     def at_index(self, unsigned int i):
#~         return wrap_Curve(self.thisptr.at_index(i))
#~     def get_ref_at_index(self, unsigned int i):
#~         return wrap_::boost::shared_ptr<Geom::Curve const>(self.thisptr.get_ref_at_index(i))
    
    def front(self):
        #this is AFAIK the shortest way to do this
        cdef Curve * r = <Curve *> &self.thisptr.front()
        return wrap_Curve_p(r)
    def back(self):
        cdef Curve * r = <Curve *> &self.thisptr.back()
        return wrap_Curve_p(r)
    def back_open(self):
        cdef Curve * r = <Curve *> &self.thisptr.back_open()
        return wrap_Curve_p(r)
    def back_closed(self):
        cdef Curve * r = <Curve *> &self.thisptr.back_closed()
        return wrap_Curve_p(r)
    def back_default(self):
        cdef Curve * r = <Curve *> &self.thisptr.back_default()
        return wrap_Curve_p(r)
    
    def curves(self):
        return wrap_Iterator(self.thisptr.begin(), self.thisptr.begin_const(),  self.thisptr.end())
    
    def curves_open(self):
        return wrap_Iterator(self.thisptr.begin(), self.thisptr.begin_const(), self.thisptr.end_open())
    def curves_closed(self):
        return wrap_Iterator(self.thisptr.begin(), self.thisptr.begin_const(), self.thisptr.end_closed())
    def curves_default(self):
        return wrap_Iterator(self.thisptr.begin(), self.thisptr.begin_const(), self.thisptr.end_default())

    def __iter__(self):
        return self.curves_default()
    
    def size_open(self):
        return self.thisptr.size_open()
    def size_closed(self):
        return self.thisptr.size_closed()
    def size_default(self):
        return self.thisptr.size_default()
    def size(self):
        return self.thisptr.size()
    def max_size(self):
        return self.thisptr.max_size()
    
    def empty(self):
        return self.thisptr.empty()
    def closed(self):
        return self.thisptr.closed()
    def close(self, bint closed):
        self.thisptr.close(closed)
    
    def boundsFast(self):
        return wrap_OptRect(self.thisptr.boundsFast())
    def boundsExact(self):
        return wrap_OptRect(self.thisptr.boundsExact())
    
#~     def toPwSb(self):
#~         return wrap_Piecewise<Geom::D2<Geom::SBasis> >(self.thisptr.toPwSb())
    
    def pointAt(self, double t):
        return wrap_Point(self.thisptr.pointAt(t))
    def valueAt(self, double t, Dim2 d):
        return self.thisptr.valueAt(t, d)
    
    def __call__(self, double t):
        return wrap_Point(deref( self.thisptr ) (t) )
    
    def roots(self, double v, Dim2 d):
        return wrap_vector_double(self.thisptr.roots(v, d))
    
    def allNearestPoints(self, cy_Point _point, double fr=-1, double to=1):
        if fr == -1:
            return wrap_vector_double(self.thisptr.allNearestPoints(deref( _point.thisptr )))
        return wrap_vector_double(self.thisptr.allNearestPoints(deref( _point.thisptr ), fr, to))
        

    def nearestPointPerCurve(self, cy_Point _point):
        return wrap_vector_double(self.thisptr.nearestPointPerCurve(deref( _point.thisptr )))
        
    def nearestPoint(self, cy_Point _point, double fr=-1, double to=1):#, cy_double * distance_squared):
        if fr == -1:
            return self.thisptr.nearestPoint(deref( _point.thisptr ), NULL)
        return self.thisptr.nearestPoint(deref( _point.thisptr ), fr, to, NULL)
    def nearestPointAndDistSq(self, cy_Point _point, double fr=-1, double to=1):
        cdef double t, dist
        if fr == -1:
            t = self.thisptr.nearestPoint(deref( _point.thisptr ), &dist )
        else:
            t = self.thisptr.nearestPoint(deref( _point.thisptr ), fr, to, &dist)
        return (t, dist)
        
    
    def appendPortionTo(self, cy_Path p, double f, double t):
        self.thisptr.appendPortionTo(deref( p.thisptr ), f, t)
    
    def portion(self, *args):
        if len(args) == 1:
            return wrap_Path(self.thisptr.portion(deref( (<cy_Interval> args[0]).thisptr )))
        elif len(args) == 2:
            return wrap_Path(self.thisptr.portion(float(args[0]), float(args[1])))

    def reverse(self):
        return wrap_Path(self.thisptr.reverse())
    
    def insert(self, int pos, curve, Stitching stitching=NO_STITCHING):
        cdef Curve * cptr = get_Curve_p(curve)
        if cptr:
            self.thisptr.insert( self._iterator_at_index(pos), deref( cptr ), stitching )
        else:
            raise TypeError("passed curve is not C++ Curve")
    def insertSlice(self, int pos, cy_Path p, int first, int last, Stitching stitching=NO_STITCHING):
        self.thisptr.insert(self._iterator_at_index(pos), p._const_iterator_at_index(first), p._const_iterator_at_index(last), stitching)
    
    def clear(self):
        self.thisptr.clear()
    
    def erase(self, int pos, Stitching stitching=NO_STITCHING):
        self.thisptr.erase(self._iterator_at_index(pos), stitching)
        
    def eraseSlice(self, int start, int end, Stitching stitching=NO_STITCHING):
        self.thisptr.erase(self._iterator_at_index(start), self._iterator_at_index(end), stitching)
    
    def eraseLast(self):
        self.thisptr.erase_last()
    
    def replace(self, int replaced, curve, Stitching stitching=NO_STITCHING):
        cdef Curve * cptr = get_Curve_p(curve)
        if cptr:
            self.thisptr.replace(self._iterator_at_index(replaced), deref( cptr ), stitching)
        else:
            raise TypeError("passed curve is not C++ Curve")
        
    def replaceSlice(self, int first_replaced, int last_replaced, curve, Stitching stitching=NO_STITCHING):
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
        self.thisptr.start(deref( p.thisptr ))
    
    def initialPoint(self):
        return wrap_Point(self.thisptr.initialPoint())
    def finalPoint(self):
        return wrap_Point(self.thisptr.finalPoint())
    
    def setInitial(self, cy_Point p):
        self.thisptr.setInitial(deref( p.thisptr ))
    def setFinal(self, cy_Point p):
        self.thisptr.setFinal(deref( p.thisptr ))
    
    def appendCurve(self, curve, Stitching stitching=NO_STITCHING):
        cdef Curve * cptr = get_Curve_p(curve)
        if cptr:
            self.thisptr.append( deref( cptr ), stitching)
        else:
            raise TypeError("passed curve is not C++ Curve")
    def appendSBasis(self, cy_SBasis x, cy_SBasis y, Stitching stitching=NO_STITCHING):
        cdef D2[SBasis] sb = D2[SBasis]( deref(x.thisptr), deref(y.thisptr) )
        self.thisptr.append(sb, stitching)
    def appendPath(self, cy_Path other, Stitching stitching=NO_STITCHING):
        self.thisptr.append(deref( other.thisptr ), stitching)
    
    def stitchTo(self, cy_Point p):
        self.thisptr.stitchTo(deref( p.thisptr ))

cdef cy_Path wrap_Path(Path p):
    cdef Path * retp = new Path(Point())
    retp[0] = p
    cdef cy_Path r = cy_Path.__new__(cy_Path)
    r.thisptr = retp
    return r

cdef vector[Path] make_vector_Path(object l):
    cdef vector[Path] ret
    for i in l:
        ret.push_back( deref( (<cy_Path> i).thisptr ) )
    return ret

#~ cdef object wrap_vector_Path(vector[Path] v):
#~     r = []
#~     cdef unsigned int i
#~     for i in range(v.size()):
#~         r.append( wrap_Path(v[i]) )
#~     return r
