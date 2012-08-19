from cython.operator cimport dereference as deref

from numbers import Number


cdef class cy_Affine:
    def __cinit__(self, c0 = None,
                        Coord c1 = 0, 
                        Coord c2 = 0, 
                        Coord c3 = 1, 
                        Coord c4 = 0, 
                        Coord c5 = 0):
        if c0 is None:
            self.thisptr = new Affine()
        elif is_transform(c0):
            self.thisptr = new Affine( get_Affine(c0) )
        else:
            self.thisptr = new Affine(<Coord> float(c0) ,c1 ,c2 ,c3 ,c4 ,c5)

    def __str__(self): 
        return "Affine({}, {}, {}, {}, {}, {})".format( self[0],
                                                        self[1],
                                                        self[2],
                                                        self[3],
                                                        self[4],
                                                        self[5],
                                                        )
    def __repr__(self):
        #TODO
        return str(self)
    def __getitem__(self, int i):
     #TODO bounds check - exceptions is better than segfault
        return deref(self.thisptr) [i]
        
    def __mul__(cy_Affine self, other):
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
        return wrap_Affine(pow( deref(self.thisptr), n ))
    def __richcmp__(cy_Affine self, cy_Affine other, int op):
        if op == 2:
            return deref(self.thisptr) == deref(other.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(other.thisptr)

    def xAxis(self):
        return wrap_Point(self.thisptr.xAxis())
    def yAxis(self):
        return wrap_Point(self.thisptr.yAxis())

    def translation(self):
        return wrap_Point(self.thisptr.translation())
    def expansionX(self):
        return self.thisptr.expansionX()
    def expansionY(self):
        return self.thisptr.expansionY()
    def expansion(self):
        return wrap_Point(self.thisptr.expansion())
    def setXAxis(self, cy_Point vec):
        self.thisptr.setXAxis(deref( vec.thisptr ))
    def setYAxis(self, cy_Point vec):
        self.thisptr.setYAxis(deref( vec.thisptr ))
    def setTranslation(self, cy_Point loc):
        self.thisptr.setTranslation(deref( loc.thisptr ))
    def setExpansionX(self, Coord val):
        self.thisptr.setExpansionX(val)
    def setExpansionY(self, Coord val):
        self.thisptr.setExpansionY(val)
    def setIdentity(self):
        self.thisptr.setIdentity()

    def isIdentity(self, Coord eps = EPSILON):
        return self.thisptr.isIdentity(eps)
    def isTranslation(self, Coord eps = EPSILON):
        return self.thisptr.isTranslation(eps)
    def isScale(self, Coord eps = EPSILON):
        return self.thisptr.isScale(eps)
    def isUniformScale(self, Coord eps = EPSILON):
        return self.thisptr.isUniformScale(eps)
    def isRotation(self, Coord eps = EPSILON):
        return self.thisptr.isRotation(eps)
    def isHShear(self, Coord eps = EPSILON):
        return self.thisptr.isHShear(eps)
    def isVShear(self, Coord eps = EPSILON):
        return self.thisptr.isVShear(eps)
    def isNonzeroTranslation(self, Coord eps = EPSILON):
        return self.thisptr.isNonzeroTranslation(eps)
    def isNonzeroScale(self, Coord eps = EPSILON):
        return self.thisptr.isNonzeroScale(eps)
    def isNonzeroUniformScale(self, Coord eps = EPSILON):
        return self.thisptr.isNonzeroUniformScale(eps)
    def isNonzeroRotation(self, Coord eps = EPSILON):
        return self.thisptr.isNonzeroRotation(eps)
    def isNonzeroHShear(self, Coord eps = EPSILON):
        return self.thisptr.isNonzeroHShear(eps)
    def isNonzeroVShear(self, Coord eps = EPSILON):
        return self.thisptr.isNonzeroVShear(eps)
    def isZoom(self, Coord eps = EPSILON):
        return self.thisptr.isZoom(eps)
    def preservesArea(self, Coord eps = EPSILON):
        return self.thisptr.preservesArea(eps)
    def preservesAngles(self, Coord eps = EPSILON):
        return self.thisptr.preservesAngles(eps)
    def preservesDistances(self, Coord eps = EPSILON):
        return self.thisptr.preservesDistances(eps)
    def flips(self):
        return self.thisptr.flips()
    def isSingular(self, Coord eps = EPSILON):
        return self.thisptr.isSingular(eps)

    def withoutTranslation(self):
        return wrap_Affine(self.thisptr.withoutTranslation())
    def inverse(self):
        return wrap_Affine(self.thisptr.inverse())
    def det(self):
        return self.thisptr.det()
    def descrim2(self):
        return self.thisptr.descrim2()
    def descrim(self):
        return self.thisptr.descrim()
    @classmethod
    def identity(self):
        return wrap_Affine(a_identity())

    @classmethod
    def are_near(cls, A, B):
        if is_transform(A) & is_transform(B):
            return are_near(get_Affine(A), get_Affine(B))

    @classmethod
    def reflection(cls, cy_Point vector, cy_Point origin):
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
    def __cinit__(self, *args):
        if len(args) == 0:
            self.thisptr = new Translate()
        elif len(args) == 1:
            self.thisptr = new Translate( deref( (<cy_Point> args[0]).thisptr ) )
        elif len(args) == 2:
            self.thisptr = new Translate(float(args[0]), float(args[1]))        

    def __getitem__(self, Dim2 dim):
        return deref( self.thisptr ) [dim] 
    def __mul__(cy_Translate self, o):
        if isinstance(o, cy_Translate):
            return wrap_Translate(deref( self.thisptr ) * deref( (<cy_Translate>o).thisptr ))
        elif is_transform(o):
            return wrap_Affine(deref(self.thisptr) * get_Affine(o))
    def vector(self):
        return wrap_Point(self.thisptr.vector())
    def inverse(self):
        return wrap_Translate(self.thisptr.inverse())
    @classmethod
    def identity(self):
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
    def __cinit__(self, *args):
        if len(args) == 0:
            self.thisptr = new Scale()
        elif len(args) == 1:
            if isinstance(args[0], Number):
                self.thisptr = new Scale(<Coord> float(args[0]))
            elif isinstance(args[0], cy_Point):
                self.thisptr = new Scale( deref( (<cy_Point> args[0]).thisptr ) )
        elif len(args) == 2:
            self.thisptr = new Scale(float(args[0]), float(args[1]))

    def __getitem__(self, Dim2 d):
        return deref( self.thisptr ) [d]
    def __mul__(cy_Scale self, o):
        if isinstance(o, cy_Scale):
            return wrap_Scale(deref( self.thisptr ) * deref( (<cy_Scale>o).thisptr ))
        elif is_transform(o):
            return wrap_Affine(deref(self.thisptr) * get_Affine(o))
    def vector(self):
        return wrap_Point(self.thisptr.vector())
    def inverse(self):
        return wrap_Scale(self.thisptr.inverse())
    def identity(self):
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
    def __cinit__(self, *args):
        if len(args) == 0:
            self.thisptr = new Rotate()
        elif len(args) == 1:
            if isinstance(args[0], Number):
                self.thisptr = new Rotate(<Coord> float(args[0]))
            elif isinstance(args[0], cy_Point):
                self.thisptr = new Rotate( deref( (<cy_Point> args[0]).thisptr ) )
        elif len(args) == 2:
            self.thisptr = new Rotate(float(args[0]), float(args[1]))
    def vector(self):
        return wrap_Point(self.thisptr.vector())
    def __getitem__(self, Dim2 dim):
        return deref( self.thisptr ) [dim]
    def __mul__(cy_Rotate self, o):
        if isinstance(o, cy_Rotate):
            return wrap_Rotate(deref( self.thisptr ) * deref( (<cy_Rotate>o).thisptr ))
        elif is_transform(o):
            return wrap_Affine(deref(self.thisptr) * get_Affine(o))

    def inverse(self):
        return wrap_Rotate(self.thisptr.inverse())
    @classmethod
    def identity(cls):
        return wrap_Rotate(r_identity())
    @classmethod
    def from_degrees(cls, Coord deg):
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
    def __cinit__(self, Coord h):
        self.thisptr = new VShear(h)
    def factor(self):
        return self.thisptr.factor()
    def setFactor(self, Coord nf):
        self.thisptr.setFactor(nf)
    def __mul__(cy_VShear self, o):
        if isinstance(o, cy_VShear):
            return wrap_VShear(deref( self.thisptr ) * deref( (<cy_VShear>o).thisptr ))
        elif is_transform(o):
            return wrap_Affine(deref(self.thisptr) * get_Affine(o))
    
    def __richcmp__(cy_VShear self, cy_VShear hs, op):
        if op == 2:
            return deref(self.thisptr) == deref(hs.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(hs.thisptr)
    def inverse(self):
        return wrap_VShear(self.thisptr.inverse())
    @classmethod
    def identity(cls):
        return wrap_VShear( vs_identity() )


cdef cy_VShear wrap_VShear(VShear p):
    cdef VShear * retp = new VShear(0)
    retp[0] = p
    cdef cy_VShear r = cy_VShear.__new__(cy_VShear, 0)
    r.thisptr = retp
    return r

cdef class cy_HShear:
    def __cinit__(self, Coord h):
        self.thisptr = new HShear(h)
    def factor(self):
        return self.thisptr.factor()
    def setFactor(self, Coord nf):
        self.thisptr.setFactor(nf)
    def __mul__(cy_HShear self, o):
        if isinstance(o, cy_HShear):
            return wrap_HShear(deref( self.thisptr ) * deref( (<cy_HShear>o).thisptr ))
        elif is_transform(o):
            return wrap_Affine(deref(self.thisptr) * get_Affine(o))
    def __richcmp__(cy_HShear self, cy_HShear hs, op):
        if op == 2:
            return deref(self.thisptr) == deref(hs.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(hs.thisptr)
    def inverse(self):
        return wrap_HShear(self.thisptr.inverse())
    @classmethod
    def identity(cls):
        return wrap_HShear( hs_identity() )


cdef cy_HShear wrap_HShear(HShear p):
    cdef HShear * retp = new HShear(0)
    retp[0] = p
    cdef cy_HShear r = cy_HShear.__new__(cy_HShear, 0)
    r.thisptr = retp
    return r


cdef class cy_Zoom:
    def __cinit__(self, *args):
        if len(args) == 1:
            if isinstance(args[0], Number):
                self.thisptr = new Zoom(<Coord> float(args[0]))
            elif isinstance(args[0], cy_Translate):
                self.thisptr = new Zoom( deref( (<cy_Translate> args[0]).thisptr ) )
        elif len(args) == 2:
            self.thisptr = new Zoom( <Coord> float(args[0]), deref( (<cy_Translate> args[1]).thisptr ) )
                
    def __mul__(cy_Zoom self, cy_Zoom z):
        return wrap_Zoom( deref(self.thisptr) * deref( z.thisptr ))
        
    def __richcmp__(cy_Zoom self, cy_Zoom z, op):
        if op == 2:
            return deref(self.thisptr) == deref(z.thisptr)
        elif op == 3:
            return deref(self.thisptr) != deref(z.thisptr)

    def scale(self):
        return self.thisptr.scale()
    def setScale(self, Coord s):
        self.thisptr.setScale(s)
    def translation(self):
        return wrap_Point(self.thisptr.translation())
    def setTranslation(self, cy_Point p):
        self.thisptr.setTranslation(deref( p.thisptr ))
    def inverse(self):
        return wrap_Zoom(self.thisptr.inverse())
    @classmethod
    def identity(cls):
        return wrap_Zoom(z_identity())
    @classmethod
    def map_rect(self, cy_Rect old_r, cy_Rect new_r):
        return wrap_Zoom(map_rect(deref( old_r.thisptr ) ,deref( new_r.thisptr )))

cdef cy_Zoom wrap_Zoom(Zoom p):
    cdef Zoom * retp = new Zoom(0)
    retp[0] = p
    cdef cy_Zoom r = cy_Zoom.__new__(cy_Zoom, 0)
    r.thisptr = retp
    return r


cdef class cy_Eigen:
    cdef Eigen* thisptr
    def __cinit__(self, a):
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
    @property
    def vectors(self):
        return (wrap_Point(self.thisptr.vectors[0]), wrap_Point(self.thisptr.vectors[1]))
    @property
    def values(self):
        return (self.thisptr.values[0], self.thisptr.values[1])
