from cython.operator cimport dereference as deref
from numbers import Number
from _cy_rectangle cimport cy_OptInterval, wrap_OptInterval, wrap_Rect, OptRect, wrap_OptRect
from _cy_rectangle cimport cy_Interval, wrap_Interval   

from _cy_affine cimport cy_Affine, wrap_Affine, get_Affine, is_transform

from _cy_curves cimport is_Curve, get_Curve_p

cdef class cy_Region:
    cdef Region* thisptr
    def __cinit__(self, cy_Path p, cy_OptRect b = None, direction = None):
        if b is None:
            if direction is None:
                self.thisptr = new Region( deref(p.thisptr) )
            else:  
                self.thisptr = new Region( deref(p.thisptr), <bint> bool(direction) )
        else:
            if direction is None:
                self.thisptr = new Region( deref(p.thisptr), deref(b.thisptr) )
            else:
                self.thisptr = new Region(deref( p.thisptr ), deref( b.thisptr ), direction )
    def __mul__(cy_Region self, m):
        cdef Affine at
        if is_transform(m):
            at = get_Affine(m)
            return wrap_Region(deref( self.thisptr ) * at)

    def size(self):
        return self.thisptr.size()
    
    def isFill(self):
        return self.thisptr.isFill()
    
    def asFill(self):
        return wrap_Region(self.thisptr.asFill())
    
    def asHole(self):
        return wrap_Region(self.thisptr.asHole())
    
    def boundsFast(self):
        return wrap_Rect(self.thisptr.boundsFast())
    
    def contains(self, cy_Point p):
        return self.thisptr.contains(deref( p.thisptr ))
    
    def contains(self, cy_Region other):
        return self.thisptr.contains(deref( other.thisptr ))
    
    def includes(self, cy_Point p):
        return self.thisptr.includes(deref( p.thisptr ))
        
    def inverse(self):
        return wrap_Region(self.thisptr.inverse())

    def invariants(self):
        return self.thisptr.invariants()

cdef cy_Region wrap_Region(Region p):
    cdef Region * retp = new Region()
    retp[0] = p
    cdef cy_Region r = cy_Region.__new__(cy_Region)
    r.thisptr = retp
    return r


cdef class cy_Shape:
    cdef Shape* thisptr

    def __init__(self):
        self.thisptr = new Shape()
    def __init__(self, cy_Region r):
        self.thisptr = new Shape(deref( r.thisptr ))
    def __init__(self, cy_Regions r):
        self.thisptr = new Shape(deref( r.thisptr ))
    def __init__(self, bint f):
        self.thisptr = new Shape(f)
    def __init__(self, cy_Regions r, bint f):
        self.thisptr = new Shape(deref( r.thisptr ) ,f)

    def __cinit__(self, cy_Region r=None, bint fill=True):
        if r is None:
            self.thisptr = self.thisptr(fill)
        
    
    def getContent(self):
        return wrap_Regions(self.thisptr.getContent())
    def isFill(self):
        return self.thisptr.isFill()
    def size(self):
        return self.thisptr.size()
    def __getitem__(self, unsigned int ix):
        return wrap_Region(deref( self.thisptr ) [] ix)
    def inverse(self):
        return wrap_Shape(self.thisptr.inverse())
    def __mul__(self, cy_Affine m):
        return wrap_Shape(deref( self.thisptr ) * deref( m.thisptr ))
    def contains(self, cy_Point p):
        return self.thisptr.contains(deref( p.thisptr ))
    def inside_invariants(self):
        return self.thisptr.inside_invariants()
    def region_invariants(self):
        return self.thisptr.region_invariants()
    def cross_invariants(self):
        return self.thisptr.cross_invariants()
    def invariants(self):
        return self.thisptr.invariants()
    def containment_list(self, cy_Point p):
        return wrap_::std::vector<unsigned int, std::allocator<unsigned int> >(self.thisptr.containment_list(deref( p.thisptr )))
    def update_fill(self):
        self.thisptr.update_fill()
