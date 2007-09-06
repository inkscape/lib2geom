#include "d2.h"

#include "sbasis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-geometric.h"
#include "sbasis-to-bezier.h"

#include "path-cairo.h"
#include "toy-framework.h"

#include <vector>
using std::vector;
using namespace Geom;

class ArcBez: public Toy {
public:
    void first_time(int argc, char** argv) {
        unsigned order = 3;
        if(argc > 1)
            sscanf(argv[1], "%u", &order);
        for(int i = 0; i < order; i++)
            handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    }

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        D2<SBasis> B = handles_to_sbasis(handles.begin(), handles.size()-1);
        Path P = path_from_sbasis(B, 1);
        *notify << P.size();
        cairo_path(cr, P);
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
