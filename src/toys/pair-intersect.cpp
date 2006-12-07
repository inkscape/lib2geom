#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis.h"
#include "s-basis-2d.h"

#include "path.h"
#include "path-cairo.h"
#include "path-builder.h"

#include <iterator>
#include "translate.h"
#include "translate-ops.h"

#include "toy-framework.h"

using std::vector;
const unsigned bez_ord = 5;
using namespace Geom;

class PairIntersect: public Toy {
virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    cairo_set_line_width (cr, 0.5);
    
    multidim_sbasis<2> B = bezier_to_sbasis<2, bez_ord-1>(handles.begin());
    cairo_md_sb(cr, B);

    B = bezier_to_sbasis<2, bez_ord-1>(handles.begin()+bez_ord);
    cairo_md_sb(cr, B);

    Toy::draw(cr, notify, width, height, save);
}
public:
PairIntersect () {
    for(int j = 0; j < 2; j++)
    for(unsigned i = 0; i < bez_ord; i++) handles.push_back(Geom::Point(uniform()*400, uniform()*400));
}
};

int main(int argc, char **argv) {   
    init(argc, argv, "Pair Intersect", new PairIntersect());

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
