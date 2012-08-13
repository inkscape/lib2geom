from _common_decl cimport *

from libcpp.vector cimport vector
from libcpp.pair cimport pair

from _cy_rectangle cimport Interval, OptInterval, Rect, OptRect
from _cy_rectangle cimport cy_OptRect
from _cy_affine cimport is_transform, get_Affine, Affine
from _cy_curves cimport Curve, cy_Curve, wrap_Curve, wrap_Curve_p
from _cy_curves cimport SBasis, cy_SBasis

from _cy_path cimport Path, cy_Path

from _cy_primitives cimport Point, cy_Point, wrap_Point

#~ cdef extern from "2geom/path.h" namespace "Geom":
#~     cdef cppclass Path

cdef extern from "2geom/region.h" namespace "Geom":
    cdef cppclass Region:
        Region()
        Region(Path &)
        Region(Path &, bint)
        Region(Path &, OptRect &)
        Region(Path &, OptRect &, bint)
        unsigned int size()
        bint isFill()
        Region asFill()
        Region asHole()
        Rect boundsFast()
        bint contains(Point &)
        bint contains(Region &)
        bint includes(Point &)
        Region inverse()
        Region operator*(Affine &)
        bint invariants()
        
    ctypedef vector[Region] Regions
    
    Regions sanitize_path(Path &)
    Regions regions_from_paths(vector[Path] &)
    vector[Path] paths_from_regions(Regions &)
    Regions region_boolean(bint, Region &, Region &)
#~     Regions region_boolean(bint, Region &, Region &, Crossings &, Crossings &)
#~     Regions region_boolean(bint, Region &, Region &, Crossings &)
#~     Crossinegs crossings(Region &, Region &)
    unsigned int outer_index(Regions &)
#~     Shape shape_boolean(bint, Shape &, Shape &, CrossingSet &)

cdef extern from "2geom/shape.h" namespace "Geom":
    cdef cppclass Shape:
        Shape()
        Shape(Region &)
        Shape(Regions &)
        Shape(bint)
        Shape(Regions &, bint)
        Regions getContent()
        bint isFill()
        unsigned int size()
        Region & operator[](unsigned int)
        Shape inverse()
        Shape operator*(Affine &)
        bint contains(Point &)
        bint inside_invariants()
        bint region_invariants()
        bint cross_invariants()
        bint invariants()
        vector[unsigned int] containment_list(Point)
        void update_fill()
#~ Shape sanitize(::std::vector<Geom::Path, std::allocator<Geom::Path> > &)
#~ CrossingSet crossings_between(Shape &, Shape &)
#~ Shape boolop(Shape &, Shape &, unsigned int, CrossingSet &)
#~ Shape boolop(Shape &, Shape &, unsigned int, CrossingSet &)
#~ Shape boolop(Shape &, Shape &, unsigned int)
#~ void add_to_shape(Shape &, Path &, bint)
#~ unsigned int crossing_along(double, unsigned int, unsigned int, bint, Crossings &)
#~ void crossing_dual(unsigned int &, unsigned int &, CrossingSet &)
#~ Shape stopgap_cleaner(::std::vector<Geom::Path, std::allocator<Geom::Path> > &)
#~ ::std::vector<Geom::Path, std::allocator<Geom::Path> > desanitize(Shape &)
#~ Shape shape_boolean(bint, Shape &, Shape &)
#~ Shape shape_boolean(bint, Shape &, Shape &, CrossingSet &)
