#ifndef SEEN_Geom_SCALE_H
#define SEEN_Geom_SCALE_H
#include "point.h"
#include "point-ops.h"

namespace Geom {

class Scale {
private:
    Point _p;

private:
    Scale();

public:
    explicit Scale(Point const &p) : _p(p) {}
    Scale(Coord const x, Coord const y) : _p(x, y) {}
    explicit Scale(Coord const s) : _p(s, s) {}
    inline Coord operator[](Dim2 const d) const { return _p[d]; }
    inline Coord operator[](unsigned const d) const { return _p[d]; }
    inline Coord &operator[](Dim2 const d) { return _p[d]; }
    inline Coord &operator[](unsigned const d) { return _p[d]; }

    bool operator==(Scale const &o) const {
        return _p == o._p;
    }

    bool operator!=(Scale const &o) const {
        return _p != o._p;
    }
    Scale inverse() const {
        return Scale(1/_p[0], 1/_p[1]);
    }
};

} /* namespace Geom */


#endif /* !SEEN_Geom_SCALE_H */

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
