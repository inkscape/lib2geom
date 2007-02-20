#include "path-cairo.h"

#include <iterator>
#include <iostream>
#include <vector>

#include "toy-framework.cpp"

#include "path2.h"
#include "read-svgd.h"
#include "md-pw-sb.h"

using namespace Geom;

void cairo_md_pw(cairo_t *cr, md_pw_sb<2> const &p) {
    pw_sb x = partition(p[0], p[1].cuts), y = partition(p[1], p[0].cuts);
    for(int i = 0; i < x.size(); i++) {
        MultidimSBasis<2> B;
        B[0] = x[i]; B[1] = y[i];
        cairo_md_sb(cr, B);
    }
}

class DistortToy: public Toy {
    std::vector<Geom::Path2::Path> p;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        SBasis distort;
        distort.push_back(BezOrd(0));
        distort.push_back(BezOrd(0,1));
        //Theoretically we could concatenate the md_pw_sb and do it in one go
        for(int i = 0; i < p.size(); i++) {
            md_pw_sb<2> foo = p[i].toMdSb();
            cairo_md_pw(cr, /*((pw_sb)distort) **/ (BezOrd(.5) * foo) );
        }

        Toy::draw(cr, notify, width, height, save);
    }

    public:
    DistortToy () {
        FILE* f = fopen("banana.svgd", "r");
        p = read_svgd(f);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "Distort Toy", new DistortToy());
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
