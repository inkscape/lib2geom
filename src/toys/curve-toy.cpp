#include "d2.h"

#include "sbasis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-geometric.h"

#include "path-cairo.h"
#include "toy-framework.h"

#include <vector>
using std::vector;
using namespace Geom;

class ArcBez: public Toy {
public:
    virtual void first_time() {
      for(int i = 0; i < 4; i++)
	handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    }

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        D2<SBasis> B = handles_to_sbasis(handles.begin(), 3);
        cairo_md_sb(cr, B);
        cairo_stroke(cr);
        
        Toy::draw(cr, notify, width, height, save);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new ArcBez());

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
