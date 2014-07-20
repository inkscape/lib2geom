#include <2geom/d2.h>
#include <2geom/sbasis.h>

#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <2geom/svg-path-parser.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <2geom/cairo-path-sink.h>
#include <2geom/svg-path-writer.h>

#include <cstdlib>

using namespace Geom;

class LoadSVGD: public Toy {
    //Region b;
    //Shape bs;
    PathVector pv;
    PointHandle offset_handle;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        
        cairo_set_line_width(cr, 1);
        cairo_set_source_rgb(cr, 0,0,0);

        CairoPathSink sink(cr);
        sink.feed(pv * Translate(offset_handle.pos));
        cairo_stroke(cr);

        // spit out some diagnostic info about the path
        SVGPathWriter sw;
        sw.feed(pv);
        *notify << sw.str();
        /*
        for(unsigned i = 0; i < pv.size(); i++) {
            if(pv[i].size() == 0) {
                *notify << "naked moveto;";
            } else 
            for(unsigned j = 0; j < pv[i].size(); j++) {
                const Curve* c = &pv[i][j];
                const BezierCurve* bc = dynamic_cast<const BezierCurve*>(c);
                if(bc) {
                    for(unsigned k = 0; k < bc->order(); k++) {
                        *notify << (*bc)[k];
                    }
                } else {
                    *notify << typeid(*c).name() << ';' ;
                }
            }
        }*/

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
    public:
    LoadSVGD() {}

    void first_time(int argc, char** argv) {
        const char *path_b_name="star.svgd";
        if (argc > 1)
            path_b_name = argv[1];
        pv = read_svgd(path_b_name);
        std::cout << pv.size() << "\n";
        std::cout << pv[0].size() << "\n";
        pv *= Translate(-pv[0].initialPoint());
        
        Rect bounds = *pv[0].boundsExact();
        handles.push_back(&offset_handle);
        offset_handle.pos = bounds.midpoint() - bounds.corner(0);
    }
};

int main(int argc, char **argv) {
    LoadSVGD x;
    init(argc, argv, &x);
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
