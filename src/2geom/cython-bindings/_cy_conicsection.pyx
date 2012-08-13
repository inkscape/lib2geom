from cython.operator cimport dereference as deref
from numbers import Number
from _cy_rectangle cimport cy_OptInterval, wrap_OptInterval, wrap_Rect, OptRect, wrap_OptRect
from _cy_rectangle cimport cy_Interval, wrap_Interval   

from _cy_affine cimport cy_Affine, wrap_Affine, get_Affine, is_transform

from _cy_curves cimport is_Curve, get_Curve_p
from _cy_path cimport wrap_Path


cdef class cy_Circle:
    cdef Circle* thisptr
    def __cinit__(self, cy_Point center=None, double r=0):
        if center is None:
            self.thisptr = new Circle()
        else:
            self.thisptr = new Circle(deref( center.thisptr ), r)
    @classmethod
    def fromCoefficients(self, double A, double B, double C, double D):
        return wrap_Circle( Circle(A, B, C, D) )
    @classmethod
    def fromPoints(self, points):
        return wrap_Circle( Circle( make_vector_point(points) ) )
    def set(self, double cx, double cy, double r):
        self.thisptr.set(cx, cy, r)
    def setCoefficients(self, double A, double B, double C, double D):
        self.thisptr.set(A, B, C, D)
    def setPoints(self, points):
        self.thisptr.set( make_vector_point(points) )
    def arc(self, cy_Point initial, cy_Point inner, cy_Point final, bint _svg_compliant=True):
        if _svg_compliant:
            return wrap_SVGEllipticalArc( deref(<SVGEllipticalArc *> self.thisptr.arc(deref( initial.thisptr ), deref( inner.thisptr ), deref( final.thisptr ), _svg_compliant)) )
        else:
            return wrap_EllipticalArc( deref(self.thisptr.arc(deref( initial.thisptr ), deref( inner.thisptr ), deref( final.thisptr ), _svg_compliant)) )
    #changing this function to return Path
    def getPath(self):
        cdef vector[Path] v
        self.thisptr.getPath(v)
        return wrap_Path(v[0])
    def center(self):
        return wrap_Point(self.thisptr.center())
#~     def center(self, Dim2 d):
#~         return self.thisptr.center(d)
    def ray(self):
        return self.thisptr.ray()

cdef cy_Circle wrap_Circle(Circle p):
    cdef Circle * retp = new Circle()
    retp[0] = p
    cdef cy_Circle r = cy_Circle.__new__(cy_Circle)
    r.thisptr = retp
    return r

cdef class cy_Ellipse:
    cdef Ellipse* thisptr
    def __cinit__(self, cy_Point center=None, rx=0, ry=0, double a=0):
        if center is None:
            self.thisptr = new Ellipse()
        else:
            self.thisptr = new Ellipse(center.x, center.y, rx, ry, a)
    @classmethod
    def fromCoefficients(cls, double A, double B, double C, double D, double E, double F):
        return wrap_Ellipse(Ellipse(A, B, C, D, E, F))
    @classmethod
    def fromCircle(cls, cy_Circle c):
        return wrap_Ellipse(Ellipse(deref( c.thisptr )))
    @classmethod
    def fromPoints(cls, points):
        return wrap_Ellipse( Ellipse( make_vector_point(points) ) )
    def set(self, cy_Point center, cy_Point ray, double a):
        self.thisptr.set(center.x, center.y, ray.x, ray.y, a)
    def setCoefficients(self, double A, double B, double C, double D, double E, double F):
        self.thisptr.set(A, B, C, D, E, F)
    def setPoints(self, points):
        self.thisptr.set( make_vector_point(points) )
    def arc(self, cy_Point initial, cy_Point inner, cy_Point final, bint svg_compliant=True):
        if svg_compliant:
            return wrap_SVGEllipticalArc( deref(<SVGEllipticalArc *> self.thisptr.arc(deref( initial.thisptr ), deref( inner.thisptr ), deref( final.thisptr ), svg_compliant)) )
        else:
            return wrap_EllipticalArc( deref(self.thisptr.arc(deref( initial.thisptr ), deref( inner.thisptr ), deref( final.thisptr ), svg_compliant)) )
    def center(self):
        return wrap_Point(self.thisptr.center())
    def ray(self, Dim2 d):
        return self.thisptr.ray(d)
    def rot_angle(self):
        return self.thisptr.rot_angle()
    def implicit_form_coefficients(self):
        return wrap_vector_double(self.thisptr.implicit_form_coefficients())
    def transformed(self, m):
        cdef Affine at
        if is_transform(m):
            at = get_Affine(m)
        return wrap_Ellipse(self.thisptr.transformed(at))
        
cdef cy_Ellipse wrap_Ellipse(Ellipse p):
    cdef Ellipse * retp = new Ellipse()
    retp[0] = p
    cdef cy_Ellipse r = cy_Ellipse.__new__(cy_Ellipse)
    r.thisptr = retp
    return r
