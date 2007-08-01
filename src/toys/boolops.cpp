#include "d2.h"
#include "sbasis.h"

#include "shape.h"
#include "path.h"
#include "svg-path-parser.h"
#include "path-intersection.h"

#include "path-cairo.h"
#include "toy-framework.cpp"

using namespace Geom;

class BoolOps: public Toy {
    vector<Path> path_a, path_b;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        Paths none;
        Shape a(path_a.front(), none), b(path_b.front(), none);
        
        cairo_path(cr, path_a.front());
        cairo_path(cr, path_b.front());
        cairo_stroke(cr);
        //std::streambuf* cout_buffer = std::cout.rdbuf();
        //std::cout.rdbuf(notify->rdbuf());
        Shapes res = shape_union(a, b);
        std::cout << "yes!!!";
        //std::cout.rdbuf(cout_buffer);

        Toy::draw(cr, notify, width, height, save);
    }

    public:
    BoolOps () {
        path_a = read_svgd("winding.svgd");
        path_b = read_svgd("monk.svgd");
        handles.push_back(Point(300,300));
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new BoolOps());
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
