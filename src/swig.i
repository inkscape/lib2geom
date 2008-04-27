 %module lib2geom
 %{
 /* Includes the header in the wrapper code */
#include "coord.h"
 #include "point.h"
 #include "sbasis.h"
 #include "sbasis-math.h"
 #include "sbasis-geometric.h"
 #include "sbasis-to-bezier.h"
 #include "path.h"
 #include "transforms.h"
 #include "matrix.h"
 using namespace Geom;
 %}
 
 /* Parse the header file to generate wrappers */
 %include "coord.h"
 %include "point.h"
 %include "sbasis.h"
 %include "linear.h"
 //%include "sbasis-math.h"
 //%include "piecewise.h"
 %include "d2.h"
 //%include "sbasis-geometric.h"

 //%template(PwSb) Geom::Piecewise<Geom::SBasis>;
 //%template(PwD2Sb) Geom::Piecewise<Geom::D2<Geom::SBasis> >;