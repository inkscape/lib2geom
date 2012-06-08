# If the pyx file is a C++ file, we should specify that here.
#TODO - rewrite to use ALLCAPS?

set_source_files_properties(
  _common_decl.pxd
  _cy_primitives.pxd
  _cy_primitives.pyx
  cy2geom.pyx

  PROPERTIES CYTHON_IS_CXX 1 )

# Multi-file cython modules do not appear to be working at the moment.
cython_add_module( _cy_primitives _cy_primitives.pyx )

target_link_libraries(_cy_primitives
    #TODO! linking to static lib2geom.a gives -fPIC error, to compile 
    #you have to enable building dynamic library in cmake . -i
    gsl gslcblas 2geom
)
set_target_properties(_cy_primitives )

cython_add_module( cy2geom cy2geom.pyx)

