# If the pyx file is a C++ file, we should specify that here.
#TODO - rewrite to use ALLCAPS?

set_source_files_properties(
  _cy_point.pyx
  _cy_angle.pyx
  
  cy2geom.pyx
  PROPERTIES CYTHON_IS_CXX 1 )

# Multi-file cython modules do not appear to be working at the moment.
#cython_add_module( rectangle rectangle.pyx Rectangle.cxx )

#cython_add_module( pi_calculator pi_calculator.pyx PiCalculator.cxx )

cython_add_module( _cy_angle _cy_angle.pyx ../angle.h)
cython_add_module( _cy_point _cy_point.pyx ../point.cpp ../forward.h ../transforms.cpp ../affine.cpp)
cython_add_module( cy2geom cy2geom.pyx   ../angle.h )
