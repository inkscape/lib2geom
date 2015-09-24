#include <2geom/d2.h>
#include <2geom/sbasis.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework.h>

using namespace Geom;

class MyToy: public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        //draw code here
        Toy::draw(cr, notify, width, height, save,timer_stream);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
