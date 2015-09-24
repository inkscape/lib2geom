from cython.operator cimport dereference as deref
from numbers import Number
from _cy_rectangle cimport cy_OptInterval, wrap_OptInterval, wrap_Rect, OptRect, wrap_OptRect
from _cy_rectangle cimport cy_Interval, wrap_Interval   

from _cy_affine cimport cy_Affine, wrap_Affine, get_Affine, is_transform

from _cy_curves cimport is_Curve, get_Curve_p
from _cy_path cimport wrap_Path


cdef class cy_Circle:
    
    """Circle in 2D plane.
    
    Corresponds to Circle class in 2geom.
    """

    cdef Circle* thisptr

    def __cinit__(self, cy_Point center=None, double r=0):
        """Create circle from center and radius."""
        if center is None:
            self.thisptr = new Circle()
        else:
            self.thisptr = new Circle(deref( center.thisptr ), r)

    @classmethod
    def from_coefficients(self, double A, double B, double C, double D):
        """Create circle form implicit equation coefficients:
        
        Implicit equation is Ax**2 + Ay**2 + Bx + Cy + D = 0
        """
        return wrap_Circle( Circle(A, B, C, D) )

    @classmethod
    def from_points(self, points):
        """Create best fitting circle from at least three points."""
        return wrap_Circle( Circle( make_vector_point(points) ) )

    def set_center(self, cy_Point c):
        """Set coordinates of center."""
        self.thisptr.setCenter(deref(c.thisptr))

    def set_radius(self, double r):
        """Set the circle's radius."""
        self.thisptr.setRadius(r)

    def set_coefficients(self, double A, double B, double C, double D):
        """Set implicit equation coefficients:
        
        Implicit equation is Ax**2 + Ay**2 + Bx + Cy + D = 0
        """
        self.thisptr.set(A, B, C, D)
    
    def fit(self, points):
        """Set circle to the best fit of at least three points."""
        self.thisptr.fit( make_vector_point(points) )
        
    def arc(self, cy_Point initial, cy_Point inner, cy_Point final, bint _svg_compliant=True):
        """Get (SVG)EllipticalArc.
        
        Args:
            initial: Initial point of arc
            inner: Inner point of arc.
            final: Final point of arc.
        """
        return wrap_EllipticalArc( deref(self.thisptr.arc(deref( initial.thisptr ), deref( inner.thisptr ), deref( final.thisptr ))) )

    def center(self):
        """Get center of circle in point."""
        return wrap_Point(self.thisptr.center())

    def radius(self):
        """Get radius of circle."""
        return self.thisptr.radius()

cdef cy_Circle wrap_Circle(Circle p):
    cdef Circle * retp = new Circle()
    retp[0] = p
    cdef cy_Circle r = cy_Circle.__new__(cy_Circle)
    r.thisptr = retp
    return r

cdef class cy_Ellipse:
    """Ellipse in 2D plane.
    
    Corresponds to Ellipse class in 2geom.
    """
    
    cdef Ellipse* thisptr
    
    def __cinit__(self, cy_Point center=None, rx=0, ry=0, double a=0):
        """Create new ellipse:
        
        Args:
            center: Center of ellipse (between foci)
            rx, ry: major and minor semi-axis.
            a: angle of major axis.
        """
        if center is None:
            self.thisptr = new Ellipse()
        else:
            self.thisptr = new Ellipse(center.x, center.y, rx, ry, a)
    
    @classmethod
    def from_coefficients(cls, double A, double B, double C, double D, double E, double F):
        """Create ellipse from coefficients of implicit equation.
        
        Implicit equation has form Ax**2 + Bxy + Cy**2 + Dx + Ey + F = 0
        """
        return wrap_Ellipse(Ellipse(A, B, C, D, E, F))
    
    @classmethod
    def from_circle(cls, cy_Circle c):
        """Create ellipse identical to circle."""
        return wrap_Ellipse(Ellipse(deref( c.thisptr )))
    
    @classmethod
    def from_points(cls, points):
        """Create ellipse fitting at least 5 points."""
        return wrap_Ellipse( Ellipse( make_vector_point(points) ) )
    
    def set(self, cy_Point center, double rx, double ry, double a):
        """Set center, rays and angle of ellipse.
        
        Args:
            center: Center of ellipse.
            rx, ry: Major and minor semi-axis.
            a: angle of major axis.
        self.thisptr.set(center.x, center.y, rx, ry, a)
        """

    def set_coefficients(self, double A, double B, double C, double D, double E, double F):
        """Set coefficients of implicit equation.
        
        Implicit equation has form Ax**2 + Bxy + Cy**2 + Dx + Ey + F = 0
        """
        self.thisptr.set(A, B, C, D, E, F)

    def set_points(self, points):
        """Set ellipse to the best fit of at least five points."""
        self.thisptr.set( make_vector_point(points) )

    def arc(self, cy_Point initial, cy_Point inner, cy_Point final, bint svg_compliant=True):
        """Get (SVG)EllipticalArc.
        
        Args:
            initial: Initial point of arc
            inner: Inner point of arc.
            final: Final point of arc.
        """
        return wrap_EllipticalArc( deref(self.thisptr.arc(deref( initial.thisptr ), deref( inner.thisptr ), deref( final.thisptr ))) )

    def center(self):
        """Get center of ellipse."""
        return wrap_Point(self.thisptr.center())

    def ray(self, Dim2 d):
        """Get major/minor semi-axis."""
        return self.thisptr.ray(d)

    def rot_angle(self):
        """Get angle of major axis."""
        return self.thisptr.rot_angle()

    def implicit_form_coefficients(self):
        """Get coefficients of implicit equation in list."""
        return wrap_vector_double(self.thisptr.implicit_form_coefficients())

    def transformed(self, m):
        """Return transformed ellipse."""
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
