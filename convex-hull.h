#ifndef SEEN_Geom_CONVEX_HULL_H
#define SEEN_Geom_CONVEX_HULL_H

/* ex:set et ts=4 sw=4: */

/*
 * A class representing the convex hull of a set of points.
 *
 * Copyright 2004  MenTaLguY <mental@rydia.net>
 *
 * This code is licensed under the GNU GPL; see COPYING for more information.
 */

#include <libnr/rect.h>

namespace Geom {

class ConvexHull {
public:
    explicit ConvexHull(Point const &p) : _bounds(p, p) {}

    Point midpoint() const {
        return _bounds.midpoint();
    }

    void add(Point const &p) {
        _bounds.expandTo(p);
    }
    void add(Rect const &p) {
        // Note that this is a hack.  when convexhull actually works
        // you will need to add all four points.
        _bounds.expandTo(p.min());
        _bounds.expandTo(p.max());
    }
    void add(ConvexHull const &h) {
        _bounds.expandTo(h._bounds);
    }
		
    Rect const &bounds() const {
        return _bounds;
    }
	
private:
    Rect _bounds;
};
} /* namespace Geom */

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
