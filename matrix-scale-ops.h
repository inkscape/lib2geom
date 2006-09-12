#ifndef SEEN_LIBGeom_Geom_MATRIX_SCALE_OPS_H
#define SEEN_LIBGeom_Geom_MATRIX_SCALE_OPS_H
/** \file 
 * Declarations (and definition if inline) of operator blah (Geom::Matrix, Geom::scale). 
 */

#include "forward.h"

Geom::Matrix operator/(Geom::Matrix const &m, Geom::scale const &s);

Geom::Matrix operator*(Geom::Matrix const &m, Geom::scale const &s);


#endif /* !SEEN_LIBGeom_Geom_MATRIX_SCALE_OPS_H */
