 %module lib2geom
 %{
 /* Includes the header in the wrapper code */
#include "coord.h"
 #include "point.h"
 using namespace Geom;
 %}
 
 /* Parse the header file to generate wrappers */
 %include "coord.h"
 %include "point.h"
