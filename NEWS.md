lib2geom v1.0.0
===============

2geom is a C++ library of mathematics for paths, curves, and other
geometric calculations, designed to be well suited for vector graphics:
Bézier curves, conics, paths, intersections, transformations, and basic
geometries.

This library was originally developed to restructure and improve the
various path data structures in Inkscape, but the design intent is for
it to be of general utility to others.  The codebase has shipped as and
been maintained as part of the Inkscape software for over a decade.

It has been a very long time since 2geom saw a packaged release, and so
much has changed it is hard to reduce to a summary.  Recent work has
focused on updating its source control, build, test, and packaging
systems to more modern technologies for both Linux and Windows, and on
restoring the py2geom python extension package to usability.  Attention
in recent years has also focused on stabilization and code quality
improvement.

The primary motivation for this 1.0 release is in support of overall
efforts towards the (future) Inkscape 1.0 release, but we also hope that
having this as a distinct package from Inkscape itself will facilitate
wider collaboration.  We would love to hear people find good uses for
this beyond the Inkscape codebase!


