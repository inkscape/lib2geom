[![Travis CI status](https://api.travis-ci.org/inkscape/lib2geom.png)](https://api.travis-ci.org/inkscape/lib2geom.png)

2Geom: easy 2D graphics library
===============================

What is this?
-------------

2Geom is a C++ 2D geometry library geared towards processing data
associated with vector graphics. The primary design consideration
is ease of use and clarity. It is dual-licensed under LGPL 2.1 and
MPL 1.1.

The library is descended from a set of geometric routines present in
Inkscape, a vector graphics editor based around the Scalable Vector
Graphics format, the most widespread vector graphics interchange format
on the Web and a W3C Recommendation. Due to this legacy, not all parts
of the API form a coherent whole (yet).

Dependencies
------------

To build 2Geom, you will need:

* [Boost](http://www.boost.org/) (headers only)
* [glib](https://wiki.gnome.org/Projects/GLib)
* [GNU Scientific Library](http://www.gnu.org/software/gsl/)
* BLAS implementation, such as [ATLAS](http://math-atlas.sourceforge.net/)
* [Ragel](http://www.colm.net/open-source/ragel/) (if you want to modify the SVG path parser)
* [GTK+ 2](http://www.gtk.org/) (for demo programs)

Building
--------

2Geom uses GoogleTest as git submodule, so make sure to initialize it after
initial repository clone:

    git submodule init
    git submodule update

2Geom uses CMake as the build and configuration system. To build, type:

    cmake .
    make

To run tests:

    make test

Also check out some of the interactive programs in src/2geom/toys.
Documentation is generated from source comments using Doxygen.
Run `doxygen` in the project root to generate documentation in
`doc/html`.

ABI note
--------

2Geom does not make any guarantees of ABI or API stability at this time,
and there are no official releases. You are using it at your own risk.
If you want to use it in your project, consider including it as a Git
submodule.

Further information
-------------------

Communications related to 2Geom development happen on a
[SourceForge mailing list](https://lists.sourceforge.net/lists/listinfo/lib2geom-devel).

The primary user of 2Geom is [Inkscape](https://inkscape.org/en/).
API-breaking changes to 2Geom will require corresponding changes to
Inkscape.
