ctypedef double Coord

cdef extern from "2geom/coord.h" namespace "Geom":
    cdef Coord EPSILON
