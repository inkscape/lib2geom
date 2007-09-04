#include "d2.h"
#include "sbasis.h"

#include "shape.h"
#include "path.h"
#include "svg-path-parser.h"

#include "path-cairo.h"
#include "toy-framework.cpp"
#include "transforms.h"
#include "sbasis-geometric.h"

#include <cstdlib>

using namespace Geom;

void cairo_region(cairo_t *cr, Region const &r) {
    cairo_set_source_rgba(cr, 0, 0, 0, 1); //rand_d(), rand_d(), rand_d(), .75);
    double d = 5.;
    if(!r.isFill()) cairo_set_dash(cr, &d, 1, 0);
    cairo_path(cr, r);
    cairo_fill(cr);
    cairo_set_dash(cr, &d, 0, 0);
}

void cairo_regions(cairo_t *cr, Regions const &p) {
    srand(0); 
    for(Regions::const_iterator j = p.begin(); j != p.end(); j++)
        cairo_region(cr, *j);
}

void cairo_shape(cairo_t *cr, Shape const &s) {
    cairo_regions(cr, s.getContent());
}


void mark_mono(cairo_t *cr, Shape const &a) {
    for(unsigned j = 0; j < a.size(); j++) {
        Path p = Path(a[j]);
        std::vector<double> sp = path_mono_splits(p);
        for(unsigned i = 0; i < sp.size(); i++) {
            draw_cross(cr, p.pointAt(sp[i]));
            cairo_stroke(cr);
        }
    }
}

Shape cleanup(std::vector<Path> const &ps) {
    Piecewise<D2<SBasis> > pw = paths_to_pw(ps);
    double area;
    Point centre;
    Geom::centroid(pw, centre, area);
    std::cout << area << "\n";
    if(area > 1)
        return sanitize(ps) * Geom::Translate(-centre);
    else
        return sanitize(ps);
}

class BoolOps: public Toy {
    //Region b;
    Shape bs;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        Geom::Translate t(handles[0]);
        Shape bst = bs * t;
        //Region bt = Region(b * t, b.isFill());
        
        cairo_set_line_width(cr, 1);
        
        cairo_shape(cr, bst);
        
        Toy::draw(cr, notify, width, height, save);
    }
    public:
    BoolOps () {}

    void first_time(int argc, char** argv) {
        char *path_b_name="star.svgd";
        if(argc > 1)
            path_b_name = argv[1];
        std::vector<Path> paths_b = read_svgd(path_b_name);
        
	Rect bounds = paths_b[0].boundsExact();
	    std::cout << crossings_among(paths_b)[0].size() << "\n";
        handles.push_back(bounds.midpoint() - bounds.corner(0));

        bs = cleanup(paths_b);
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
