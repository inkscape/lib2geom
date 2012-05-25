from cython.operator cimport dereference as deref

ctypedef double Coord

cdef extern from "2geom/angle.h" namespace "Geom":
    cdef cppclass Angle:
        Angle()
        Angle(Coord)
        
        
        Coord radians()
        Coord radians0()
        Coord degrees()
        Coord degreesClock()

cdef extern from "2geom/angle.h" namespace "Geom::Angle":
    Angle from_radians(Coord d)

cdef class cy_Angle:
    cdef Angle* thisptr
    def __cinit__(self, double x):
        self.thisptr = new Angle(x)
    cdef _rewrite_ptr(self, Angle* p):
        del self.thisptr
        self.thisptr = p
    cdef Angle * _return_ptr(self):
        return self.thisptr
    def __dealloc__(self):
        del self.thisptr

    def radians(self):
        return self.thisptr.radians()
    def radians0(self):
        return self.thisptr.radians0()
    def degrees(self):
        return self.thisptr.degrees()
    def degreesClock(self):
        return self.thisptr.degreesClock()

        
    @classmethod
    def from_radians(self, rad):
    #there might be more idiomatic way to do this
        cdef Angle * retp = new Angle(0)
        retp[0] = from_radians(rad)
        cdef cy_Angle r =  wrap_Angle(retp)
        return r


    
cdef cy_Angle wrap_Angle(Angle * p_angle):
    cdef cy_Angle ret = cy_Angle.__new__(cy_Angle,0)
    ret.thisptr = p_angle
    return ret
