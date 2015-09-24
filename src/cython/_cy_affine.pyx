from cython.operator cimport dereference as deref

from numbers import Number


cdef class cy_Affine:

    """Class representing affine transform in 2D plane.

    Corresponds to Affine class in 2geom.
    """

    def __cinit__(self, c0=None,
                        Coord c1=0,
                        Coord c2=0,
                        Coord c3=1,
                        Coord c4=0,
                        Coord c5=0):
        """Create Affine instance from either transform or from coefficients."""
        if c0 is None:
            self.thisptr = new Affine()
        elif is_transform(c0):
            self.thisptr = new Affine( get_Affine(c0) )
        else:
            self.thisptr = new Affine(<Coord> float(c0) ,c1 ,c2 ,c3 ,c4 ,c5)

    def __str__(self):
        """str(self)"""
        return "Affine({}, {}, {}, {}, {}, {})".format( self[0],
                                                        self[1],
                                                        self[2],
                                                        self[3],
                                                        self[4],
                                                        self[5],
                                                        )
    def __repr__(self):
        """repr(self)"""
        return str(self)

    def __dealloc__(self):
        del self.thisptr

    def __getitem__(self, int i):
        """Get coefficients."""
        if i >= 6:
            raise IndexError("Affine has only 6 coefficients.")
        return deref(self.thisptr) [i]

    def __mul__(cy_Affine self, other):
        """Compose with another transformation."""
        if isinstance(other, cy_Affine):
            return wrap_Affine( deref(self.thisptr) * deref( (<cy_Affine> other).thisptr ) )
        elif isinstance(other, cy_Translate):
            return wrap_Affine( deref(self.thisptr) * deref( (<cy_Translate> other).thisptr ) )
        elif isinstance(other, cy_Scale):
            return wrap_Affine( deref(self.thisptr) * deref( (<cy_Scale> other).thisptr ) )
        elif isinstance(other, cy_Rotate):
            return wrap_Affine( deref(self.thisptr) * deref( (<cy_Rotate> other).thisptr ) )
        elif isinstance(other, cy_HShear):
            return wrap_Affine( deref(self.thisptr) * deref( (<cy_HShear> other).thisptr ) )
        elif isinstance(other, cy_VShear):
            return wrap_Affine( deref(self.thisptr) * deref( (<cy_VShear> other).thisptr ) )
        elif isinstance(other, cy_Zoom):
            return wrap_Affine( deref(self.thisptr) * deref( (<cy_Zoom> other).thisptr ) )

    def __pow__(cy_Affine self, int n, z):
        """Compose with self n times."""
        return wrap_Affine(pow( deref(self.thisptr), n ))

    def __richcmp__(cy_Affine self, cy_Affine other, int op):
        if op == 2:
            return deref(self.thisptr) == deref(other.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(other.thisptr)

    def x_axis(self):
        """Transformation of unit x vector without translation."""
        return wrap_Point(self.thisptr.xAxis())

    def y_axis(self):
        """Transformation of unit y vector without translation."""
        return wrap_Point(self.thisptr.yAxis())

    def translation(self):
        """Translation of transformation."""
        return wrap_Point(self.thisptr.translation())

    def expansion_X(self):
        """Expansion of unit x vector."""
        return self.thisptr.expansionX()

    def expansion_Y(self):
        """Expansion of unit y vector."""
        return self.thisptr.expansionY()

    def expansion(self):
        """Point( self.expansion_X(), self.expansion_Y() )"""
        return wrap_Point(self.thisptr.expansion())

    def set_X_axis(self, cy_Point vec):
        """Set transformation of x unit vector without translation."""
        self.thisptr.setXAxis(deref( vec.thisptr ))

    def set_Y_axis(self, cy_Point vec):
        """Set transformation of y unit vector without translation."""
        self.thisptr.setYAxis(deref( vec.thisptr ))

    def set_translation(self, cy_Point loc):
        """Set translation of origin."""
        self.thisptr.setTranslation(deref( loc.thisptr ))

    def set_expansion_X(self, Coord val):
        """Set expansion of x unit vector."""
        self.thisptr.setExpansionX(val)

    def set_expansion_Y(self, Coord val):
        """Set expansion of y unit vector."""
        self.thisptr.setExpansionY(val)

    def set_identity(self):
        """Set self to identity transformation."""
        self.thisptr.setIdentity()

    def is_identity(self, Coord eps=EPSILON):
        """Return true if self is close to identity transform.

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.isIdentity(eps)

    def is_translation(self, Coord eps=EPSILON):
        """Return true if self is close to transformation.

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.isTranslation(eps)

    def is_scale(self, Coord eps=EPSILON):
        """Return true if self is close to scale.

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.isScale(eps)

    def is_uniform_scale(self, Coord eps=EPSILON):
        """Return true if self is close to uniform scale.

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.isUniformScale(eps)

    def is_rotation(self, Coord eps=EPSILON):
        """Return true if self is close to rotation.

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.isRotation(eps)

    def is_HShear(self, Coord eps=EPSILON):
        """Return true if self is close to horizontal shear.

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.isHShear(eps)

    def is_VShear(self, Coord eps=EPSILON):
        """Return true if self is close to vertical shear.

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.isVShear(eps)

    def is_nonzero_translation(self, Coord eps=EPSILON):
        """Return true if self is close to translation and is identity.

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.isNonzeroTranslation(eps)

    def is_nonzero_scale(self, Coord eps=EPSILON):
        """Return true if self is close to scale and is identity.

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.isNonzeroScale(eps)

    def is_nonzero_uniform_scale(self, Coord eps=EPSILON):
        """Return true if self is close to scale and is identity.

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.isNonzeroUniformScale(eps)

    def is_nonzero_rotation(self, Coord eps=EPSILON):
        """Return true if self is close to rotation and is identity.

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.isNonzeroRotation(eps)

    def is_nonzero_HShear(self, Coord eps=EPSILON):
        """Return true if self is close to horizontal shear and is not indentity.

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.isNonzeroHShear(eps)

    def is_nonzero_VShear(self, Coord eps=EPSILON):
        """Return true if self is close to vertical shear and is not indentit.

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.isNonzeroVShear(eps)

    def is_zoom(self, Coord eps=EPSILON):
        """Return true if self is close to zoom.

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.isZoom(eps)

    def preserves_area(self, Coord eps=EPSILON):
        """Return true if areas are preserved after transformation

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.preservesArea(eps)

    def preserves_angles(self, Coord eps=EPSILON):
        """Return true if angles are preserved after transformation

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.preservesAngles(eps)

    def preserves_distances(self, Coord eps=EPSILON):
        """Return true if distances are preserved after transformation

        Use second argument eps to specify tolerance.
        """
        return self.thisptr.preservesDistances(eps)

    def flips(self):
        """Return true if transformation flips - it has negative scaling."""
        return self.thisptr.flips()

    def is_singular(self, Coord eps=EPSILON):
        """Check whether transformation matrix is singular."""
        return self.thisptr.isSingular(eps)

    def without_translation(self):
        """Return transformation without translation part."""
        return wrap_Affine(self.thisptr.withoutTranslation())

    def inverse(self):
        """Return inverse transformation."""
        return wrap_Affine(self.thisptr.inverse())

    def det(self):
        """Return determinant of transformation matrix."""
        return self.thisptr.det()

    def descrim2(self):
        """Return absolute value of self.det()"""
        return self.thisptr.descrim2()

    def descrim(self):
        """Return square root of self.descrim2()"""
        return self.thisptr.descrim()

    @classmethod
    def identity(self):
        """Create identity transformation."""
        return wrap_Affine(a_identity())

    @classmethod
    def are_near(cls, A, B, Coord eps=EPSILON):
        """Test if two transforms are near."""
        if is_transform(A) & is_transform(B):
            return are_near(get_Affine(A), get_Affine(B), eps)

    @classmethod
    def reflection(cls, cy_Point vector, cy_Point origin):
        """Create transformation reflecting along line specified by vector and origin."""
        return wrap_Affine( reflection( deref(vector.thisptr), deref(origin.thisptr) ) )

cdef cy_Affine wrap_Affine(Affine p):
    cdef Affine * retp = new Affine()
    retp[0] = p
    cdef cy_Affine r = cy_Affine.__new__(cy_Affine)
    r.thisptr = retp
    return r

cdef Affine get_Affine(t):
    if isinstance(t, cy_Affine ):
        return deref( (<cy_Affine> t).thisptr )
    elif isinstance(t, cy_Translate):
        return <Affine> deref( (<cy_Translate> t).thisptr )
    elif isinstance(t, cy_Scale):
        return <Affine> deref( (<cy_Scale> t).thisptr )
    elif isinstance(t, cy_Rotate):
        return <Affine> deref( (<cy_Rotate> t).thisptr )
    elif isinstance(t, cy_HShear):
        return <Affine> deref( (<cy_HShear> t).thisptr )
    elif isinstance(t, cy_VShear):
        return <Affine> deref( (<cy_VShear> t).thisptr )
    elif isinstance(t, cy_Zoom):
        return <Affine> deref( (<cy_Zoom> t).thisptr )

cdef bint is_transform(t):
    return any([isinstance(t, cy_Affine),
                isinstance(t, cy_Translate),
                isinstance(t, cy_Scale),
                isinstance(t, cy_Rotate),
                isinstance(t, cy_HShear),
                isinstance(t, cy_VShear),
                isinstance(t, cy_Zoom)
                ])


cdef class cy_Translate:

    """Translation in 2D plane

    Corresponds to Translate class in 2geom.
    """

    def __cinit__(self, *args):
        """Create Translate instance form point or it's two coordinates."""
        if len(args) == 0:
            self.thisptr = new Translate()
        elif len(args) == 1:
            self.thisptr = new Translate( deref( (<cy_Point> args[0]).thisptr ) )
        elif len(args) == 2:
            self.thisptr = new Translate(float(args[0]), float(args[1]))

    def __dealloc__(self):
        del self.thisptr


    def __getitem__(self, Dim2 dim):
        """Get components of displacement vector."""
        return deref( self.thisptr ) [dim]

    def __mul__(cy_Translate self, o):
        """Compose with another transformation."""
        if isinstance(o, cy_Translate):
            return wrap_Translate(deref( self.thisptr ) * deref( (<cy_Translate>o).thisptr ))
        elif is_transform(o):
            return wrap_Affine(deref(self.thisptr) * get_Affine(o))

    def __pow__(cy_Translate self, int n, z):
        """Compose with self n times."""
        return wrap_Translate(pow( deref(self.thisptr), n ))

    def vector(self):
        """Get displacement vector."""
        return wrap_Point(self.thisptr.vector())

    def inverse(self):
        """Return inverse transformation."""
        return wrap_Translate(self.thisptr.inverse())

    @classmethod
    def identity(self):
        """Create identity translation."""
        return wrap_Translate(t_identity())

    def __richcmp__(cy_Translate self, cy_Translate t, op):
        if op == 2:
            return deref(self.thisptr) == deref(t.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(t.thisptr)

cdef cy_Translate wrap_Translate(Translate p):
    cdef Translate * retp = new Translate()
    retp[0] = p
    cdef cy_Translate r = cy_Translate.__new__(cy_Translate)
    r.thisptr = retp
    return r


cdef class cy_Scale:

    """Scale in 2D plane.

    Corresponds to Scale in 2geom.
    """

    def __cinit__(self, *args):
        """Create scale from number, point or it's two coordinates.

        One number creates uniform scale, point or two numbers create
        scale with different x and y scale factor.
        """
        if len(args) == 0:
            self.thisptr = new Scale()
        elif len(args) == 1:
            if isinstance(args[0], Number):
                self.thisptr = new Scale(<Coord> float(args[0]))
            elif isinstance(args[0], cy_Point):
                self.thisptr = new Scale( deref( (<cy_Point> args[0]).thisptr ) )
        elif len(args) == 2:
            self.thisptr = new Scale(float(args[0]), float(args[1]))

    def __dealloc__(self):
        del self.thisptr

    def __getitem__(self, Dim2 d):
        """Get scale factors for each axis."""
        return deref( self.thisptr ) [d]

    def __mul__(cy_Scale self, o):
        """Compose with another transformation."""
        if isinstance(o, cy_Scale):
            return wrap_Scale(deref( self.thisptr ) * deref( (<cy_Scale>o).thisptr ))
        elif is_transform(o):
            return wrap_Affine(deref(self.thisptr) * get_Affine(o))

    def __pow__(cy_Scale self, int n, z):
        """Compose with self n times."""
        return wrap_Scale(pow( deref(self.thisptr), n ))

    def vector(self):
        """Get both scale factors as a point."""
        return wrap_Point(self.thisptr.vector())

    def inverse(self):
        """Return inverse transformation."""
        return wrap_Scale(self.thisptr.inverse())

    @classmethod
    def identity(self):
        """Create identity scale."""
        return wrap_Scale(s_identity())

    def __richcmp__(cy_Scale self, cy_Scale s, op):
        if op == 2:
            return deref(self.thisptr) == deref(s.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(s.thisptr)

cdef cy_Scale wrap_Scale(Scale p):
    cdef Scale * retp = new Scale()
    retp[0] = p
    cdef cy_Scale r = cy_Scale.__new__(cy_Scale)
    r.thisptr = retp
    return r


cdef class cy_Rotate:

    """Rotation in 2D plane.

    Corresponds to Rotate in 2geom.
    """

    def __cinit__(self, *args):
        """Create new Rotate instance, specifying angle.

        Use one number to set the angle, or point/its two coordinates,
        using point's angle with x-axis as a rotation angle.
        """
        if len(args) == 0:
            self.thisptr = new Rotate()
        elif len(args) == 1:
            if isinstance(args[0], Number):
                self.thisptr = new Rotate(<Coord> float(args[0]))
            elif isinstance(args[0], cy_Point):
                self.thisptr = new Rotate( deref( (<cy_Point> args[0]).thisptr ) )
        elif len(args) == 2:
            self.thisptr = new Rotate(float(args[0]), float(args[1]))

    def __dealloc__(self):
        del self.thisptr

    def vector(self):
        """Return Point(1, 0)*self."""
        return wrap_Point(self.thisptr.vector())

    def __getitem__(self, Dim2 dim):
        """Get components of self.vector()"""
        return deref( self.thisptr ) [dim]

    def __mul__(cy_Rotate self, o):
        """Compose with another transformation."""
        if isinstance(o, cy_Rotate):
            return wrap_Rotate(deref( self.thisptr ) * deref( (<cy_Rotate>o).thisptr ))
        elif is_transform(o):
            return wrap_Affine(deref(self.thisptr) * get_Affine(o))

    def __pow__(cy_Rotate self, int n, z):
        """Compose with self n times."""
        return wrap_Rotate(pow( deref(self.thisptr), n ))

    def inverse(self):
        """Return inverse transformation."""
        return wrap_Rotate(self.thisptr.inverse())

    @classmethod
    def identity(cls):
        """Create identity rotation."""
        return wrap_Rotate(r_identity())

    @classmethod
    def from_degrees(cls, Coord deg):
        """Create rotation from angle in degrees."""
        return wrap_Rotate(from_degrees(deg))

    def __richcmp__(cy_Rotate self, cy_Rotate r, op):
        if op == 2:
            return deref(self.thisptr) == deref(r.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(r.thisptr)

cdef cy_Rotate wrap_Rotate(Rotate p):
    cdef Rotate * retp = new Rotate()
    retp[0] = p
    cdef cy_Rotate r = cy_Rotate.__new__(cy_Rotate)
    r.thisptr = retp
    return r


cdef class cy_VShear:

    """Vertical shear in 2D plane

    Corresponds to VShear in 2geom.
    """

    def __cinit__(self, Coord h):
        """Create VShear instance form shearing factor."""
        self.thisptr = new VShear(h)

    def __dealloc__(self):
        del self.thisptr

    def factor(self):
        """Get shearing factor."""
        return self.thisptr.factor()

    def set_factor(self, Coord nf):
        """Set shearing factor."""
        self.thisptr.setFactor(nf)

    def __mul__(cy_VShear self, o):
        """Compose with another transformation."""
        if isinstance(o, cy_VShear):
            return wrap_VShear(deref( self.thisptr ) * deref( (<cy_VShear>o).thisptr ))
        elif is_transform(o):
            return wrap_Affine(deref(self.thisptr) * get_Affine(o))

    def __pow__(cy_VShear self, int n, z):
        """Compose with self n times."""
        return wrap_VShear(pow( deref(self.thisptr), n ))

    def inverse(self):
        """Return inverse transformation."""
        return wrap_VShear(self.thisptr.inverse())

    @classmethod
    def identity(cls):
        """Create identity VShear."""
        return wrap_VShear( vs_identity() )

    def __richcmp__(cy_VShear self, cy_VShear hs, op):
        if op == 2:
            return deref(self.thisptr) == deref(hs.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(hs.thisptr)



cdef cy_VShear wrap_VShear(VShear p):
    cdef VShear * retp = new VShear(0)
    retp[0] = p
    cdef cy_VShear r = cy_VShear.__new__(cy_VShear, 0)
    r.thisptr = retp
    return r

cdef class cy_HShear:

    """Horizontal shear in 2D plane

    Corresponds to HShear in 2geom.
    """

    def __cinit__(self, Coord h):
        """Create HShear instance form shearing factor."""
        self.thisptr = new HShear(h)

    def __dealloc__(self):
        del self.thisptr

    def factor(self):
        """Get shearing factor."""
        return self.thisptr.factor()

    def set_factor(self, Coord nf):
        """Set shearing factor."""
        self.thisptr.setFactor(nf)

    def __mul__(cy_HShear self, o):
        """Compose with another transformation."""
        if isinstance(o, cy_HShear):
            return wrap_HShear(deref( self.thisptr ) * deref( (<cy_HShear>o).thisptr ))
        elif is_transform(o):
            return wrap_Affine(deref(self.thisptr) * get_Affine(o))

    def __pow__(cy_HShear self, int n, z):
        """Compose with self n times."""
        return wrap_HShear(pow( deref(self.thisptr), n ))

    def inverse(self):
        """Return inverse transformation."""
        return wrap_HShear(self.thisptr.inverse())

    @classmethod
    def identity(cls):
        """Create identity HShear."""
        return wrap_HShear( hs_identity() )

    def __richcmp__(cy_HShear self, cy_HShear hs, op):
        if op == 2:
            return deref(self.thisptr) == deref(hs.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(hs.thisptr)

cdef cy_HShear wrap_HShear(HShear p):
    cdef HShear * retp = new HShear(0)
    retp[0] = p
    cdef cy_HShear r = cy_HShear.__new__(cy_HShear, 0)
    r.thisptr = retp
    return r


cdef class cy_Zoom:

    """Zoom in 2D plane, consisting of uniform scale and translation.

    Corresponds to Zoom in 2geom.
    """

    def __cinit__(self, Coord scale=1, cy_Translate translate=cy_Translate()):
        """Create Zoom from scale factor and translation"""
        self.thisptr = new Zoom( scale, deref( translate.thisptr ) )

    def __dealloc__(self):
        del self.thisptr

    def __mul__(cy_Zoom self, cy_Zoom z):
        """Compose with another transformation."""
        return wrap_Zoom( deref(self.thisptr) * deref( z.thisptr ))

    def __pow__(cy_Zoom self, int n, z):
        """Compose with self n times."""
        return wrap_Zoom(pow( deref(self.thisptr), n ))

    def __richcmp__(cy_Zoom self, cy_Zoom z, op):
        if op == 2:
            return deref(self.thisptr) == deref(z.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(z.thisptr)

    def scale(self):
        """Get scale factor."""
        return self.thisptr.scale()

    def set_scale(self, Coord s):
        """Set scale factor."""
        self.thisptr.setScale(s)

    def translation(self):
        """Get translation as a point."""
        return wrap_Point(self.thisptr.translation())

    def set_translation(self, cy_Point p):
        """Set translation."""
        self.thisptr.setTranslation(deref( p.thisptr ))

    def inverse(self):
        """Return inverse transformation."""
        return wrap_Zoom(self.thisptr.inverse())

    @classmethod
    def identity(cls):
        """Create identity zoom."""
        return wrap_Zoom(z_identity())

    @classmethod
    def map_rect(self, cy_Rect old_r, cy_Rect new_r):
        """Create zooming used to go from old rectangle to new."""
        return wrap_Zoom(map_rect(deref( old_r.thisptr ) ,deref( new_r.thisptr )))

cdef cy_Zoom wrap_Zoom(Zoom p):
    cdef Zoom * retp = new Zoom(0)
    retp[0] = p
    cdef cy_Zoom r = cy_Zoom.__new__(cy_Zoom, 0)
    r.thisptr = retp
    return r


cdef class cy_Eigen:

    """Class computing eigenvalues and eigenvectors of 2x2 matrix.

    Corresponds to Eigen class in 2geom.
    """

    cdef Eigen* thisptr

    def __cinit__(self, a):
        """Create Eigen form 2D transform or 2x2 list - matrix."""
        cdef Affine at
        cdef double m[2][2]
        if is_transform(a):
            at = get_Affine(a)
            self.thisptr = new Eigen(at)
        else:
            for i in range(2):
                for j in range(2):
                    m[i][j] = a[i][j]
            self.thisptr = new Eigen(m)

    def __dealloc__(self):
        del self.thisptr

    @property
    def vectors(self):
        """Eigenvectors of matrix."""
        return (wrap_Point(self.thisptr.vectors[0]), wrap_Point(self.thisptr.vectors[1]))

    @property
    def values(self):
        """Eigenvalues of matrix."""
        return (self.thisptr.values[0], self.thisptr.values[1])
