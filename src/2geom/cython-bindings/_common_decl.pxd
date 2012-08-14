from libcpp.vector cimport vector
from libcpp.pair cimport pair

ctypedef double Coord
ctypedef int IntCoord

cdef extern from "2geom/coord.h" namespace "Geom":
    cdef Coord EPSILON
    cdef enum Dim2:
        X = 0
        Y = 1

cdef object wrap_vector_double(vector[double] v)
cdef vector[double] make_vector_double(object l)
