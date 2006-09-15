#define __Geom_VALUES_C__

#include "rect-l.h"


/*
The following predefined objects are for reference
and comparison.
*/
NRMatrix Geom_MATRIX_IDENTITY =
       {{1.0, 0.0, 0.0, 1.0, 0.0, 0.0}};
NRRect   Geom_RECT_EMPTY =
       {Geom_HUGE, Geom_HUGE, -Geom_HUGE, -Geom_HUGE};
NRRectL  Geom_RECT_L_EMPTY =
       {Geom_HUGE_L, Geom_HUGE_L, -Geom_HUGE_L, -Geom_HUGE_L};
NRRectL  Geom_RECT_S_EMPTY =
       {Geom_HUGE_S, Geom_HUGE_S, -Geom_HUGE_S, -Geom_HUGE_S};

/** component_vectors[i] is like $e_i$ in common mathematical usage;
    or equivalently $I_i$ (where $I$ is the identity matrix). */
Geom::Point const component_vectors[] = {Geom::Point(1., 0.),
				       Geom::Point(0., 1.)};

