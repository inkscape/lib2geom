ctypedef double Coord

cdef extern from "2geom/coord.h" namespace "Geom":
    cdef Coord EPSILON
    enum Dim2:
        X = 0
        Y = 1





