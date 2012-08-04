# If the pyx file is a C++ file, we should specify that here.
#TODO - rewrite to use ALLCAPS?

set_source_files_properties(
  _common_decl.pxd
  
  _cy_primitives.pxd
  _cy_primitives.pyx
  
  _cy_rectangle.pxd
  _cy_rectangle.pyx
  
  _cy_affine.pxd
  _cy_affine.pyx
  
  _cy_curves.pxd
  _cy_curves.pyx
  
  cy2geom.pyx

  PROPERTIES CYTHON_IS_CXX 1)

# Multi-file cython modules do not appear to be working at the moment.
cython_add_module( _cy_primitives _cy_primitives.pyx)

cython_add_module( _cy_rectangle _cy_rectangle.pyx)

cython_add_module( _cy_affine _cy_affine.pyx)

cython_add_module( _cy_curves _cy_curves.pyx)

target_link_libraries(_cy_primitives
    #TODO! linking to static lib2geom.a gives -fPIC error, to compile 
    #you have to enable building dynamic library in cmake . -i
    gsl gslcblas 2geom
)
target_link_libraries(_cy_rectangle
    gsl gslcblas 2geom
)

target_link_libraries(_cy_affine
    gsl gslcblas 2geom
)

target_link_libraries(_cy_curves
    gsl gslcblas 2geom
)

cython_add_module( cy2geom cy2geom.pyx)

add_test(cython-primitives python2 test-primitives.py)
add_test(cython-rectangle python2 test-rectangle.py)
add_test(cython-affine python2 test-affine.py)
add_test(cython-curves python2 test-curves.py)
