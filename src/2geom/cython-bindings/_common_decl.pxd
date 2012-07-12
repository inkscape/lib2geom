ctypedef double Coord
ctypedef int IntCoord

cdef extern from "2geom/coord.h" namespace "Geom":
    cdef Coord EPSILON
    cdef enum Dim2:
        X = 0
        Y = 1





