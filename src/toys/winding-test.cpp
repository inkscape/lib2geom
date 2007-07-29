#include "path.h"
#include "svg-path-parser.h"

#include <iostream>

#include "path-cairo.h"
#include "toy-framework.cpp"

using namespace Geom;

int winding(vector<Path> ps, Point p) {
    int wind = 0;
    for(unsigned i = 0; i < ps.size(); i++) {
        wind += ps[i].winding(p);
    }
    return wind;
}

class WindingTest: public Toy {
    vector<Path> path;
    Piecewise<D2<SBasis> > pw;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_pw_d2(cr, pw);
        *notify << "winding:" << winding(path, handles[0]) << "\n";
        Toy::draw(cr, notify, width, height, save);
    }

    public:
    WindingTest () {
        path = read_svgd("winding.svgd");
        pw = paths_to_pw(path);
        handles.push_back(Point(300,300));
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new WindingTest());
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
