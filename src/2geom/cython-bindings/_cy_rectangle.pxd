from _common_decl cimport Coord, IntCoord, Dim2, EPSILON


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

        GenericOptInterval get()


        C min()
        C max()
        C extent()
        C middle()
        bint isSingular()
        bint contains(C)
        bint contains(GenericOptInterval[C] &) 
        bint intersects(GenericOptInterval[C] &)
        void setMin(C)
        void setMax(C)
        void expandTo(C)
        void expandBy(C)
        void unionWith(GenericOptInterval[C] &)

        bint isEmpty()

        void intersectWith(GenericOptInterval &)
        
        GenericOptInterval &operator&(GenericOptInterval &)

        GenericOptInterval[C] &operator+(C)
        GenericOptInterval[C] &operator-(C)
        GenericOptInterval[C] &operator-()
        GenericOptInterval[C] &operator+(GenericOptInterval[C] &)
        GenericOptInterval[C] &operator-(GenericOptInterval[C] &)
        GenericOptInterval[C] &operator|(GenericOptInterval[C] &)
        bint operator==(GenericOptInterval[C] &)
        bint operator!=(GenericOptInterval[C] &)
    #TODO
    #GenericInterval[C] unify(GenericInterval[C], GenericInterval[C])
#cdef extern from "2geom/generic-interval.h" namespace "Geom::GenericInterval":
    #GenericInterval[C] from_array(C *, unsigned int)

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
