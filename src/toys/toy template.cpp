//Pick and choose what you need
#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis.h"
#include "sbasis-poly.h"

#include "path.h"
#include "path-cairo.h"
#include "path-builder.h"

#include "translate.h"
#include "translate-ops.h"
#include "solver.h"
#include "nearestpoint.cpp"
#include "sturm.h"
#include "poly-dk-solve.h"
#include "poly-laguerre-solve.h"
#include "choose.h"
#include "convex-cover.h"

#include "toy-framework.h"

class MyToy: public Toy {
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "untitled", new MyToy());
    return 0;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
