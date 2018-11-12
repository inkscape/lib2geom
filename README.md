2Geom: easy 2D graphics library
===============================

What is this?
-------------

2Geom is a C++ 2D geometry library geared towards robust processing of
computational geometry data associated with vector graphics. The primary
design consideration is ease of use and clarity. It is dual-licensed
under LGPL 2.1 and MPL 1.1.

The library is descended from a set of geometric routines present in
Inkscape, a vector graphics editor based around the Scalable Vector
Graphics format, the most widespread vector graphics interchange format
on the Web and a W3C Recommendation. Due to this legacy, not all parts
of the API form a coherent whole (yet).

Rendering is outside the scope of this library, and it is assumed
something like libcairo or similar is employed for this.  2geom
concentrates on higher level algorithms and geometric computations.


Features List
-------------

* C++
* Functional programming style.
* Points
* Efficient affine transformations
* Rectangles
* Convex Hulls
* Bounded error
* General purpose paths:
  + Exact elliptical arcs
  + Area
  + Centroid and bending moments
* Path Locations:
  + Determination of special spots (e.g. maximum curvature)
  + Splitting
  + Point, tangent, curvature at location
  + Efficient arc length and inverse arc length
* Path algebra:
  + Computations such as offset curves can be written with their mathematical definition
    and still get a bounded error, efficient curve. (preliminary trials indicate offset
    done this way out performs the method used in Inkscape)
* Arbitrary distortion (with bounded error):
  + Mesh distorts
  + Computational distorts such as the GIMP's 'vortex' plugin
  + 3d mapping (perspective, flag, sphere)
* Exact boolean ops (elliptic arcs remain elliptic arcs)
* Efficient 2d database
* Implicit function plotting
* NURBs input and output
* Tunable path simplification
* PDoF constraint system for CAD/CAGD


Dependencies
------------

To build 2Geom, you will need:

* [Boost](http://www.boost.org/) (headers only)
* [glib](https://wiki.gnome.org/Projects/GLib)
* [GNU Scientific Library](http://www.gnu.org/software/gsl/)
* [Ragel](http://www.colm.net/open-source/ragel/) (if you want to modify the SVG path parser)
* [GTK+ 2](http://www.gtk.org/) (for demo programs)


Building
--------

2Geom uses CMake as the build and configuration system. To build, type:

    cmake .
    make

To run tests:

    make test

Also check out some of the interactive programs in src/toys.

Documentation is generated from source comments using Doxygen.
Run `doxygen` in the project root to generate documentation in
`doc/html`.


API / ABI Stability
-------------------

Version 1.0 of 2Geom marks its first official release.  With this
release the library's API/ABI is considered stable:

 * All public APIs will not be renamed or have their parameters changed
   without providing backwards-compatible aliases.

 * New functionality added to these APIs will not change their meaning
   or fundamental behaviors.

 * If an API needs to be removed or replaced, it will be declared
   deprecated but will remain in the API until the next major version.
   Warnings will be issued when the deprecated method is called.

 * We'll only break backwards compatibility of these APIs if a bug or
   security hole makes it completely unavoidable.

Improvements that would break the API/ABI will be noted in our bug
tracker, for a future Version 2.0 release.


Further information
-------------------

Communications related to 2Geom development happen on a
[SourceForge mailing list](https://lists.sourceforge.net/lists/listinfo/lib2geom-devel).

The primary user of 2Geom is [Inkscape](https://inkscape.org/en/).
API-breaking changes to 2Geom will require corresponding changes to
Inkscape.
