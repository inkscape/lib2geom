from cython.operator cimport dereference as deref

from _common_decl cimport *

cdef extern from "2geom/angle.h" namespace "Geom":
    cdef cppclass Angle:
        Angle()
        Angle(Coord)
        
        Coord radians()
        Coord radians0()
        Coord degrees()
        Coord degreesClock()
        
        Angle &operator+(Angle &)
        Angle &operator-(Angle &)
        bint operator==(Angle &)
        bint operator!=(Angle &)
        
cdef extern from "2geom/angle.h" namespace "Geom::Angle":
    Angle from_radians(Coord d)
    Angle from_degrees(Coord d)
    Angle from_degrees_clock(Coord d) 

cdef class cy_Angle:
    #TODO add constructor from Point
    cdef Angle* thisptr
    
    def __cinit__(self, double x):
        self.thisptr = new Angle(x)
    cdef _rewrite_ptr(self, Angle* p):
        del self.thisptr
        self.thisptr = p
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
    def from_radians(cls, rad):
        #there might be more idiomatic way to do this
        return wrap_Angle(from_radians(rad))
    @classmethod
    def from_degrees(cls, d):
        return wrap_Angle(from_degrees(d))
    @classmethod
    def from_degrees_clock(cls, d):
        return wrap_Angle(from_degrees_clock(d))
    def __add__(cy_Angle self, cy_Angle other):
        return wrap_Angle(deref(other.thisptr) + deref(self.thisptr))
    def __sub__(cy_Angle self, cy_Angle other):
        return wrap_Angle(deref(other.thisptr) - deref(self.thisptr))
       
        
    def __richcmp__(cy_Angle self, cy_Angle other, int op):
        #angles are not ordered
        if op == 2:
            return deref(other.thisptr) == deref(self.thisptr)
        elif op == 3:
            return deref(other.thisptr) != deref(self.thisptr)
        return NotImplemented

cdef cy_Angle wrap_Angle(Angle p):
    cdef Angle * retp = new Angle()
    retp[0] = p
    cdef cy_Angle r = cy_Angle.__new__(cy_Angle, 0)
    r.thisptr = retp
    return r
