#include "d2.h"
#include "d2.cpp"
#include "sbasis.h"

#include "path-cairo.h"
#include "toy-framework.cpp"

using namespace Geom;

class MyToy: public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        //draw code here
        Toy::draw(cr, notify, width, height, save);
    }

    public:
    MyToy () {
        //Initialization here
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new MyToy());
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
