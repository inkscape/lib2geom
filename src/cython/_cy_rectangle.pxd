from _common_decl cimport *
from cpython.ref cimport PyObject

from _cy_primitives cimport Point, cy_Point, wrap_Point
from _cy_primitives cimport IntPoint, cy_IntPoint, wrap_IntPoint

cdef extern from "2geom/affine.h" namespace "Geom":
    cdef cppclass Affine:
        pass

cdef extern from "2geom/cython-bindings/wrapped-pyobject.h" namespace "Geom":

    cdef cppclass WrappedPyObject:
        WrappedPyObject()
        WrappedPyObject(object)
        object getObj()

    cdef cppclass PyPoint:
        PyPoint()
        PyPoint(WrappedPyObject, WrappedPyObject)
        WrappedPyObject operator[](int)
        

cdef extern from "2geom/generic-interval.h" namespace "Geom":

    cdef cppclass GenericInterval[C]:

        GenericInterval()
        GenericInterval(C)
        GenericInterval(C, C)

        C min()
        C max()
        C extent()
        C middle()
    
        bint isSingular()
        bint contains(C)
        bint contains(GenericInterval[C] &) 
        bint intersects(GenericInterval[C] &)

        void setMin(C)
        void setMax(C)
        void expandTo(C)
        void expandBy(C)
        void unionWith(GenericInterval[C] &)
        
        GenericInterval[C] &operator+(C)
        GenericInterval[C] &operator-(C)
        GenericInterval[C] &operator-()
        GenericInterval[C] &operator+(GenericInterval[C] &)
        GenericInterval[C] &operator-(GenericInterval[C] &)
        GenericInterval[C] &operator|(GenericInterval[C] &)
        bint operator==(GenericInterval[C] &)
        bint operator!=(GenericInterval[C] &)


    cdef cppclass GenericOptInterval[C]:
    
        GenericOptInterval()
        GenericOptInterval(C)
        GenericOptInterval(C, C)

        GenericInterval get() 

        C min()
        C max()
        C extent()
        C middle()

        bint isEmpty()        
        bint isSingular()
        bint contains(C)
        bint contains(GenericOptInterval[C] &) 
        bint intersects(GenericOptInterval[C] &)
        
        void setMin(C)
        void setMax(C)
        void expandTo(C)
        void expandBy(C)
        void unionWith(GenericOptInterval[C] &)
        void intersectWith(GenericOptInterval &)


        GenericOptInterval[C] &operator+(C)
        GenericOptInterval[C] &operator-(C)
        GenericOptInterval[C] &operator-()
        GenericOptInterval[C] &operator+(GenericOptInterval[C] &)
        GenericOptInterval[C] &operator-(GenericOptInterval[C] &)
        GenericOptInterval[C] &operator|(GenericOptInterval[C] &)
        GenericOptInterval[C] &operator&(GenericOptInterval[C] &)
        bint operator==(GenericOptInterval[C] &)
        bint operator!=(GenericOptInterval[C] &)


cdef extern from "2geom/int-interval.h" namespace "Geom":
    ctypedef GenericInterval[IntCoord] IntInterval
    ctypedef GenericOptInterval[IntCoord] OptIntInterval


#redeclaring inherited methods, other option is to cast all
#pointers from Interval to GenericInterval[Coord] in extension class
cdef extern from "2geom/interval.h" namespace "Geom":
    
    cdef cppclass Interval:
    
        Interval()
        Interval(Coord)
        Interval(Coord, Coord)
        
        Coord min()
        Coord max()
        Coord extent()
        Coord middle()
    
        bint isSingular()
        bint contains(Coord)
        bint contains(Interval &) 
        bint intersects(Interval &)
    
        void setMin(Coord)
        void setMax(Coord)
        void expandTo(Coord)
        void expandBy(Coord)
        void unionWith(Interval &)

        bint isFinite()
        bint interiorContains(Coord)
        bint interiorContains(Interval &)
        bint interiorIntersects(Interval &)
        
        Interval & operator*(Coord)
        Interval & operator/(Coord)
        Interval & operator*(Interval &)
        bint operator==(IntInterval &)
        bint operator==(Interval &)
        bint operator!=(Interval &)
        bint operator!=(IntInterval &)                
        Interval &operator+(Coord)
        Interval &operator-(Coord)
        Interval &operator-()
        Interval &operator+(Interval &)
        Interval &operator-(Interval &)
        Interval &operator|(Interval &)
        
        IntInterval roundOutwards()
        OptIntInterval roundInwards()

    
    cdef cppclass OptInterval:

        OptInterval(OptInterval &)
        OptInterval()
        OptInterval(Interval &)
        OptInterval(Coord)
        OptInterval(Coord, Coord)
        OptInterval(GenericOptInterval[double] &)
        OptInterval(IntInterval &)
        OptInterval(OptIntInterval &)
    
        Interval get()
        bint isEmpty()
        void unionWith(OptInterval &)
        void intersectWith(OptInterval &)
        
        OptInterval &operator|(OptInterval &)
        OptInterval &operator&(OptInterval &)

cdef class cy_Interval:
    cdef Interval* thisptr

cdef cy_Interval wrap_Interval(Interval p)

cdef class cy_OptInterval:
    cdef OptInterval* thisptr

cdef cy_OptInterval wrap_OptInterval(OptInterval p)


cdef extern from "2geom/generic-rect.h" namespace "Geom":
    cdef cppclass GenericRect[C]:
        GenericRect()
        GenericRect(GenericInterval[C] &, GenericInterval[C] &)
        GenericRect(Point &, Point &)
        GenericRect(PyPoint &, PyPoint &)
        GenericRect(IntPoint &, IntPoint &)
        GenericRect(C, C, C, C)

        GenericInterval[C] & operator[](Dim2)

        PyPoint min()
        PyPoint max()
        PyPoint corner(unsigned int)
        
        IntPoint i_min "min" ()
        IntPoint i_max "max" ()
        IntPoint i_corner "corner" (unsigned int)
        
        C top()
        C bottom()
        C left()
        C right()
        C width()
        C height()
        Coord aspectRatio()
        
        PyPoint dimensions()
        PyPoint midpoint()

        IntPoint i_dimensions "dimensions" ()
        IntPoint i_midpoint "midpoint" ()
        
        C area()
        bint hasZeroArea()
        C maxExtent()
        C minExtent()
        bint intersects(GenericRect[C] &)
        bint contains(GenericRect[C] &)

        bint contains(PyPoint &)
        bint contains(IntPoint &)

        void setLeft(C)
        void setRight(C)
        void setTop(C)
        void setBottom(C)

        void setMin(PyPoint &)
        void setMax(PyPoint &)
        void expandTo(PyPoint &)
        void setMin(IntPoint &)
        void setMax(IntPoint &)
        void expandTo(IntPoint &)
        
        void unionWith(GenericRect[C] &)
        void expandBy(C)
        void expandBy(C, C)
        
        void expandBy(PyPoint &)
        void expandBy(IntPoint &)
        
        GenericRect[C] & operator+(PyPoint &)
        GenericRect[C] & operator+(IntPoint &)
                
        GenericRect[C] & operator-(PyPoint &)
        GenericRect[C] & operator-(IntPoint &)

        GenericRect[C] & operator|(GenericRect[C] &)
        bint operator==(GenericRect[C] &)
        bint operator!=(GenericRect[C] &)

    
    cdef cppclass GenericOptRect[C]:
        GenericOptRect()
        GenericOptRect(GenericRect[C] &)
        GenericOptRect(C, C, C, C)
        GenericOptRect(Point &, Point &)
        GenericOptRect(GenericOptInterval[C] &, GenericOptInterval[C] &)
        
        GenericRect[C] get()
        
        bint isEmpty()
        bint intersects(GenericRect[C] &)
        bint contains(GenericRect[C] &)
        bint intersects(GenericOptRect[C] &)
        bint contains(GenericOptRect[C] &)
        
        bint contains(Point &)
        bint contains(IntPoint &)
        
        void unionWith(GenericRect[C] &)
        void unionWith(GenericOptRect[C] &)
        void intersectWith(GenericRect[C] &)
        void intersectWith(GenericOptRect[C] &)
        
        void expandTo(Point &)
        
        GenericOptRect[C] &operator|(GenericOptRect[C] &)
        GenericOptRect[C] &operator&(GenericRect[C] &)
        GenericOptRect[C] &operator&(GenericOptRect[C] &)
        
        bint operator==(GenericOptRect[C] &)
        bint operator==(GenericRect[C] &)

        bint operator!=(GenericOptRect[C] &)
        bint operator!=(GenericRect[C] &)

cdef extern from "2geom/generic-rect.h" namespace "Geom::GenericRect<Geom::WrappedPyObject>":
    GenericRect[WrappedPyObject] from_xywh(WrappedPyObject, WrappedPyObject, WrappedPyObject, WrappedPyObject)
    GenericRect[WrappedPyObject] from_xywh(PyPoint &, PyPoint &)

cdef extern from "2geom/int-rect.h" namespace "Geom":
    ctypedef GenericRect[IntCoord] IntRect

cdef extern from "2geom/rect.h" namespace "Geom":
    cdef cppclass Rect:
        Rect()
        Rect(Interval &, Interval &)
        Rect(Point &, Point &)
        Rect(Coord, Coord, Coord, Coord)

        Interval & operator[](Dim2)
        Point min()
        Point max()
        Point corner(unsigned int)
        Coord top()
        Coord bottom()
        Coord left()
        Coord right()
        Coord width()
        Coord height()
        Coord aspectRatio()
        Point dimensions()
        Point midpoint()
        Coord area()
        bint hasZeroArea()
        Coord maxExtent()
        Coord minExtent()

        bint intersects(Rect &)
        bint contains(Rect &)
        bint contains(Point &)

        void setLeft(Coord)
        void setRight(Coord)
        void setTop(Coord)
        void setBottom(Coord)
        void setMin(Point &)
        void setMax(Point &)
        void expandTo(Point &)
        void unionWith(Rect &)
        void expandBy(Coord)
        void expandBy(Coord, Coord)
        void expandBy(Point &)

        Rect & operator+(Point &)
        Rect & operator-(Point &)
        Rect & operator|(Rect &)
        bint operator==(Rect &)
        bint operator!=(Rect &)   
        bint operator==(IntRect &)
        bint operator!=(IntRect &)
        
        bint hasZeroArea(Coord)
        bint interiorIntersects(Rect &)
        bint interiorContains(Point &)
        bint interiorContains(Rect &)
        bint interiorContains(OptRect &)

        IntRect roundOutwards()
        OptIntRect roundInwards()
        Rect &operator*(Affine &)

    Coord distanceSq(Point &, Rect &)
    Coord distance(Point &, Rect &)


    cdef cppclass OptRect:
        OptRect()
        OptRect(Rect &)
        OptRect(Coord, Coord, Coord, Coord)
        OptRect(Point &, Point &)
        OptRect(OptInterval &, OptInterval &)
        
        Rect get()
        
        bint isEmpty()
        bint intersects(Rect &)
        bint contains(Rect &)
        bint intersects(OptRect &)
        bint contains(OptRect &)
        bint contains(Point &)
        
        void unionWith(Rect &)
        void unionWith(OptRect &)
        void intersectWith(Rect &)
        void intersectWith(OptRect &)
        void expandTo(Point &)
        
        OptRect &operator|(OptRect &)
        OptRect &operator&(Rect &)
        OptRect &operator&(OptRect &)
        
        bint operator==(OptRect &)
        bint operator==(Rect &)

        bint operator!=(OptRect &)
        bint operator!=(Rect &)
                
cdef extern from "2geom/rect.h" namespace "Geom::Rect":
    Rect from_xywh(Coord, Coord, Coord, Coord)
    Rect from_xywh(Point &, Point &)
    Rect infinite()

cdef class cy_Rect:
    cdef Rect* thisptr
    
cdef cy_Rect wrap_Rect(Rect p)

cdef class cy_OptRect:
    cdef OptRect* thisptr
    
cdef cy_OptRect wrap_OptRect(OptRect p)


cdef extern from "2geom/int-rect.h" namespace "Geom":
    #redeclaring because cython complains about ambiguous overloading otherwise
    cdef cppclass OptIntRect:
        OptIntRect()
        OptIntRect(IntRect &)
        OptIntRect(IntCoord, IntCoord, IntCoord, IntCoord)
        OptIntRect(IntPoint &, IntPoint &)
        OptIntRect(OptIntInterval &, OptIntInterval &)
        
        IntRect get()
        
        bint isEmpty()
        bint intersects(IntRect &)
        bint contains(IntRect &)
        bint intersects(OptIntRect &)
        bint contains(OptIntRect &)
        
        bint contains(Point &)
        bint contains(IntPoint &)
        
        void unionWith(IntRect &)
        void unionWith(OptIntRect &)
        void intersectWith(IntRect &)
        void intersectWith(OptIntRect &)
        
        void expandTo(IntPoint &)
        
        OptIntRect &operator|(OptIntRect &)
        OptIntRect &operator&(IntRect &)
        OptIntRect &operator&(OptIntRect &)
        
        bint operator==(OptIntRect &)
        bint operator==(IntRect &)

        bint operator!=(OptIntRect &)
        bint operator!=(IntRect &)

