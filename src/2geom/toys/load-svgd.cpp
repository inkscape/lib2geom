#include <2geom/d2.h>
#include <2geom/sbasis.h>

#include <2geom/shape.h>
#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <2geom/svg-path-parser.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/transforms.h>
#include <2geom/sbasis-geometric.h>

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
    //Shape bs;
    PathVector pv;
    PointHandle offset_handle;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        Geom::Translate t(offset_handle.pos);
        //Shape bst = bs * t;
        //Region bt = Region(b * t, b.isFill());
        
        cairo_set_line_width(cr, 1);
        cairo_set_source_rgb(cr, 0,0,0);
        
        //cairo_shape(cr, bst);
        cairo_path(cr, pv*t);
        cairo_stroke(cr);
        
        for(unsigned i = 0; i < pv.size(); i++) {
            if(pv[i].size() == 0) {
                *notify << "naked moveto;";
            } else 
            for(unsigned j = 0; j < pv[i].size(); j++) {
                const Curve* c = &pv[i][j];
                *notify << typeid(*c).name() << ';' ;
            }
        }

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
    public:
    BoolOps () {}

    void first_time(int argc, char** argv) {
        const char *path_b_name="star.svgd";
        if(argc > 1)
            path_b_name = argv[1];
        pv = read_svgd(path_b_name);
        std::cout << pv.size() << "\n";
        std::cout << pv[0].size() << "\n";
        pv *= Translate(-pv[0].initialPoint());
        
	Rect bounds = *pv[0].boundsExact();
        std::cout << crossings_among(pv)[0].size() << "\n";
        handles.push_back(&offset_handle);
        offset_handle.pos = bounds.midpoint() - bounds.corner(0);

        //bs = cleanup(pv);
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
