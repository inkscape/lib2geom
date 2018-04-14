#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <2geom/svg-path-parser.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#include <2geom/cairo-path-sink.h>
#include <2geom/svg-path-writer.h>

#include <cstdlib>

using namespace Geom;

/**
 * @brief SVG path data loading toy.
 *
 * A very simple toy that loads a file containing raw SVG path data
 * and displays it scaled so that it fits inside the window.
 *
 * Use this toy to see what the path data looks like without
 * pasting it into the d= attribute of a path in Inkscape.
 */
class LoadSVGD: public Toy {
    PathVector pv;
    OptRect bounds;

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {

        Rect viewport(Point(10, 10), Point(width-10, height-10));
        PathVector res = pv * bounds->transformTo(viewport, Aspect(ALIGN_XMID_YMID));

        CairoPathSink sink(cr);
        sink.feed(res);

        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_fill_preserve(cr);
        cairo_set_line_width(cr, 1);
        cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
        cairo_set_source_rgb(cr, 0,0,0);
        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
    public:
    LoadSVGD() {}

    void first_time(int argc, char** argv) override {
        const char *path_b_name="star.svgd";
        if (argc > 1)
            path_b_name = argv[1];
        pv = read_svgd(path_b_name);
        bounds = pv.boundsExact();
        if (!bounds) {
            std::cerr << "Empty path, aborting" << std::endl;
            std::exit(1);
        }
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
