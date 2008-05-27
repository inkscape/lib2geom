#include "d2.h"
#include "sbasis.h"

#include "shape.h"
#include "path.h"
#include "svg-path-parser.h"
#include "path-intersection.h"

#include "path-cairo.h"
#include "toy-framework.h"
#include "transforms.h"
#include "sbasis-geometric.h"

#include <cstdlib>

using namespace Geom;

class SVGArc : public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        Toy::draw(cr, notify, width, height, save);
    }

    void key_hit(GdkEventKey *e) {
    }

public:
    SVGArc () {}

    void first_time(int argc, char** argv) {
        const char *path_a_name="winding.svgd";
        if(argc > 1)
            path_a_name = argv[1];
        std::vector<Path> paths_a = read_svgd(path_a_name);
             
        handles.push_back(Point(400,400));
    }
    virtual bool should_draw_numbers() {return false;}
};

int main(int argc, char **argv) {
    init(argc, argv, new SVGArc());
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
