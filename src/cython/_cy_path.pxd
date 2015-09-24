from _common_decl cimport *

from libcpp.vector cimport vector
from libcpp.pair cimport pair

from _cy_rectangle cimport Interval, OptInterval, Rect, OptRect
from _cy_affine cimport Affine
from _cy_curves cimport Curve, cy_Curve, wrap_Curve_p
from _cy_curves cimport SBasis, cy_SBasis

from _cy_primitives cimport Point, cy_Point, wrap_Point


cdef extern from "2geom/d2.h" namespace "Geom":
    cdef cppclass D2[T]:
        D2()
        D2(T &, T &)
        T& operator[](unsigned i)  

#~ ctypedef int BaseIteratorConst "BaseIterator<ConstIterator, Path const>"

cdef extern from "2geom/path.h" namespace "Geom::PathInternal":
    cdef cppclass BaseIterator[C, P]:
        Curve & c_item "operator*" ()
        C & c_next "operator++" ()
        C & c_next "operator++" (int)
        bint operator==(BaseIterator[C, P])


    cdef cppclass ConstIterator:
        ConstIterator()
    cdef cppclass Iterator:
        Iterator()
        ConstIterator & operator()        
        
    cdef cppclass BaseIteratorConst "Geom::PathInternal::BaseIterator<Geom::PathInternal::ConstIterator, Path const>":
#~         Curve & c_item "operator*" ()
        ConstIterator & c_next "operator++" ()
        ConstIterator & c_next "operator++" (int)
#~         bint operator==(BaseIterator[C, P])
        
 
cdef extern from "2geom/path.h" namespace "Geom::Path":
    cdef enum Stitching:
        c_NO_STITCHING "Path::NO_STITCHING" = 0,
        c_STITCH_DISCONTINUOUS "Path::STITCH_DISCONTINUOUS" 

cdef extern from "2geom/path.h" namespace "Geom":
    cdef cppclass Path:
        Path(Path &)
        Path(Point)
        Path(ConstIterator &, ConstIterator &, bint)
        void swap(Path &)
        Curve & operator[](unsigned int)
#~         Curve & at_index(unsigned int)
#~         ::boost::shared_ptr<Geom::Curve const> get_ref_at_index(unsigned int)
        Curve & front()
        Curve & back()
        Curve & back_open()
        Curve & back_closed()
        Curve & back_default()
        ConstIterator begin_const "begin" ()
        ConstIterator end()
        Iterator begin()
#~         Iterator end()
        ConstIterator end_open()
        ConstIterator end_closed()
        ConstIterator end_default()
        size_t size_open()
        size_t size_closed()
        size_t size_default()
        size_t size()
        size_t max_size()
        bint empty()
        bint closed()
        void close(bint)
        OptRect boundsFast()
        OptRect boundsExact()
#~         Piecewise<Geom::D2<Geom::SBasis> > toPwSb()
        bint operator==(Path &)
        bint operator!=(Path &)
        Path operator*(Affine &)
#~         Path & operator*=(Affine &)
        Point pointAt(double)
        double valueAt(double, Dim2)
        Point operator()(double) except +
        vector[double] roots(double, Dim2)
        vector[double] allNearestTimes(Point &, double, double)
        vector[double] allNearestTimes(Point &)
        vector[double] nearestTimePerCurve(Point &)
        double nearestTime(Point &, double, double, double *)
        double nearestTime(Point &, double *)
        void appendPortionTo(Path &, double, double)
        Path portion(double, double)
        Path portion(Interval)
        Path reversed()
        void insert(Iterator &, Curve &, Stitching) except +
        void insert(Iterator &, ConstIterator &, ConstIterator &, Stitching)
        void clear()
        void erase(Iterator &, Stitching)
        void erase(Iterator &, Iterator &, Stitching)
        void erase_last()
        void replace(Iterator &, Curve &, Stitching)
        void replace(Iterator &, Iterator &, Curve &, Stitching)
        void replace(Iterator &, ConstIterator &, ConstIterator &, Stitching)
        void replace(Iterator &, Iterator &, ConstIterator &, ConstIterator &, Stitching)
        void start(Point)
        Point initialPoint()
        Point finalPoint()
        void setInitial(Point &)
        void setFinal(Point &)
        void append(Curve &, Stitching)
        void append(D2[SBasis] &, Stitching)
        void append(Path &, Stitching)
        void stitchTo(Point &)

cdef class cy_Path:
#~     NO_STITCHING = c_NO_STITCHING
#~     STITCH_DISCONTINUOUS = c_STITCH_DISCONTINUOUS
    cdef Path* thisptr
    cdef ConstIterator _const_iterator_at_index(self, int i)
    cdef Iterator _iterator_at_index(self, int i)

cdef cy_Path wrap_Path(Path p)
