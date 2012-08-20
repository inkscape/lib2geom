from _common_decl cimport *

from _cy_rectangle cimport Rect, cy_Rect, wrap_Rect
from _cy_primitives cimport Point, cy_Point, wrap_Point


cdef extern from "2geom/affine.h" namespace "Geom":
    cdef cppclass Affine:

        Affine(Affine &)
        Affine()
        Affine(Coord, Coord, Coord, Coord, Coord, Coord)

        Coord operator[](unsigned int)

        Affine & operator*(Affine &)
        Affine & operator*(Translate &)
        Affine & operator*(Scale &)
        Affine & operator*(Rotate &)
        Affine & operator*(HShear &)
        Affine & operator*(VShear &)
        Affine & operator*(Zoom &)

        bint operator==(Affine &)
        bint operator!=(Affine &)

        Point xAxis()
        Point yAxis()
        Point translation()
        Coord expansionX()
        Coord expansionY()
        Point expansion()

        void setXAxis(Point &)
        void setYAxis(Point &)
        void setTranslation(Point &)
        void setExpansionX(Coord)
        void setExpansionY(Coord)
        void setIdentity()

        bint isIdentity(Coord)
        bint isTranslation(Coord)
        bint isScale(Coord)
        bint isUniformScale(Coord)
        bint isRotation(Coord)
        bint isHShear(Coord)
        bint isVShear(Coord)
        bint isNonzeroTranslation(Coord)
        bint isNonzeroScale(Coord)
        bint isNonzeroUniformScale(Coord)
        bint isNonzeroRotation(Coord)
        bint isNonzeroHShear(Coord)
        bint isNonzeroVShear(Coord)
        bint isZoom(Coord)
        bint preservesArea(Coord)
        bint preservesAngles(Coord)
        bint preservesDistances(Coord)
        bint flips()
        bint isSingular(Coord)

        Affine withoutTranslation()
        Affine inverse()

        Coord det()
        Coord descrim2()
        Coord descrim()

    bint are_near(Affine &, Affine &, Coord)
    Affine a_identity "Geom::Affine::identity" ()

cdef extern from "2geom/transforms.h" namespace "Geom":
    Affine reflection(Point &, Point &)
    #TODO find out how cython __pow__ works
    Affine      pow(Affine &, int)
    Translate   pow(Translate &, int)
    Scale       pow(Scale &, int)
    Rotate      pow(Rotate &, int)
    HShear      pow(HShear &, int)
    VShear      pow(VShear &, int)
    Zoom        pow(Zoom &, int)

cdef class cy_Affine:
    cdef Affine* thisptr

cdef cy_Affine wrap_Affine(Affine)

#helper functions
cdef Affine get_Affine(t)
cdef bint is_transform(t)


cdef extern from "2geom/transforms.h" namespace "Geom":
    cdef cppclass Translate:
        Translate(Translate &)
        Translate()
        Translate(Point &)
        Translate(Coord, Coord)
        Coord operator[](Dim2)
        Coord operator[](unsigned int)
        Translate & operator*(Translate &)
        Affine & operator*(Affine &)
        bint operator==(Translate &)
        bint operator!=(Translate &)

        Affine operator()

        Point vector()
        Translate inverse()

    Translate t_identity "Geom::Translate::identity" ()

cdef class cy_Translate:
    cdef Translate* thisptr


cdef extern from "2geom/transforms.h" namespace "Geom":
    cdef cppclass Scale:
        Scale(Scale &)
        Scale()
        Scale(Point &)
        Scale(Coord, Coord)
        Scale(Coord)
        Coord operator[](Dim2)
        Scale & operator*(Scale &)
        Affine & operator*(Affine &)
        bint operator==(Scale &)
        bint operator!=(Scale &)

        Affine operator()

        Point vector()
        Scale inverse()
        Scale identity()

    Scale s_identity "Geom::Scale::identity" ()

cdef class cy_Scale:
    cdef Scale* thisptr


cdef extern from "2geom/transforms.h" namespace "Geom":
    cdef cppclass Rotate:
        Rotate(Rotate &)
        Rotate()
        Rotate(Coord)
        Rotate(Point &)
        Rotate(Coord, Coord)
        Point vector()

        Coord operator[](Dim2)
        Coord operator[](unsigned int)
        Rotate & operator*(Rotate &)
        Affine & operator*(Affine &)
        bint operator==(Rotate &)
        bint operator!=(Rotate &)

        Affine operator()
        Rotate inverse()

    Rotate r_identity "Geom::Rotate::identity" ()

cdef extern from "2geom/transforms.h" namespace "Geom::Rotate":
    Rotate from_degrees(Coord)


cdef class cy_Rotate:
    cdef Rotate* thisptr

cdef extern from "2geom/transforms.h" namespace "Geom":
    cdef cppclass VShear:
        VShear(VShear &)
        VShear(Coord)
        Coord factor()
        void setFactor(Coord)

        VShear &operator*(VShear)
        Affine & operator*(Affine &)
        bint operator==(VShear &)
        bint operator!=(VShear &)
        Affine operator()

        VShear inverse()

    VShear vs_identity "Geom::VShear::identity"()

cdef class cy_VShear:
    cdef VShear* thisptr


cdef extern from "2geom/transforms.h" namespace "Geom":
    cdef cppclass HShear:
        HShear(HShear &)
        HShear(Coord)
        Coord factor()
        void setFactor(Coord)
        HShear &operator*(HShear)
        Affine & operator*(Affine &)
        bint operator==(HShear &)
        bint operator!=(HShear &)
        Affine operator()

        HShear inverse()

    HShear hs_identity "Geom::HShear::identity"()

cdef class cy_HShear:
    cdef HShear* thisptr


cdef extern from "2geom/transforms.h" namespace "Geom":
    cdef cppclass Zoom:
        Zoom(Zoom &)
        Zoom(Coord)
        Zoom(Translate &)
        Zoom(Coord, Translate &)

        Zoom & operator*(Zoom &)
        Affine & operator*(Affine &)
        bint operator==(Zoom &)
        bint operator!=(Zoom &)

        Affine operator()

        Coord scale()
        void setScale(Coord)
        Point translation()
        void setTranslation(Point &)

        Zoom inverse()

        Zoom()

    Zoom z_identity "Geom::Zoom::identity" ()

cdef extern from "2geom/transforms.h" namespace "Geom::Zoom":
    Zoom map_rect(Rect &, Rect &)

cdef class cy_Zoom:
    cdef Zoom* thisptr


cdef extern from "2geom/affine.h" namespace "Geom":
    cdef cppclass Eigen:
        Point *vectors
        double *values
        Eigen(Affine &)
        Eigen(double[2][2])
