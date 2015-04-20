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
    PathVector pv;
    PointHandle offset_handle;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        
        cairo_set_line_width(cr, 1);
        cairo_set_source_rgb(cr, 0,0,0);

        PathVector res = pv * Translate(offset_handle.pos);
        CairoPathSink sink(cr);
        sink.feed(res);
        cairo_stroke(cr);

        // spit out some diagnostic info about the path
        SVGPathWriter sw;
        //sw.setOptimize(true);
        sw.feed(res);
        *notify << sw.str();

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
