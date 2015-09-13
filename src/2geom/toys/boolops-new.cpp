#include <2geom/d2.h>
#include <2geom/sbasis.h>

#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/intersection-graph.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/transforms.h>

#include <cstdlib>

using namespace Geom;

class BoolOps : public Toy {
    PathVector as, bs;
    PointHandle p;
    std::vector<Toggle> togs;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        Translate t(p.pos);
        PathVector bst = bs * t;
        Timer tm;
        tm.start();

        PathIntersectionGraph pig(as, bst);
        std::vector<Point> ix = pig.intersectionPoints();
        PathVector result;
        if (togs[0].on && !togs[1].on && !togs[2].on) {
            result = pig.getAminusB();
        }
        if (!togs[0].on && togs[1].on && !togs[2].on) {
            result = pig.getIntersection();
        }
        if (!togs[0].on && !togs[1].on && togs[2].on) {
            result = pig.getBminusA();
        }
        if (togs[0].on && togs[1].on && !togs[2].on) {
            result = as;
        }
        if (togs[0].on && !togs[1].on && togs[2].on) {
            result = pig.getXOR();
        }
        if (!togs[0].on && togs[1].on && togs[2].on) {
            result = bst;
        }
        if (togs[0].on && togs[1].on && togs[2].on) {
            result = pig.getUnion();
        }
        Timer::Time boolop_time = tm.lap();

        cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
        cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);

        cairo_set_source_rgba(cr, 1, 0, 0, 0.3);
        cairo_path(cr, as);
        cairo_fill(cr);
        cairo_set_source_rgba(cr, 0, 0, 1, 0.3);
        cairo_path(cr, bst);
        cairo_fill(cr);
        
        cairo_set_source_rgba(cr, 0, 0, 0, 1);
        cairo_set_line_width(cr, 2);

        for (unsigned i = 0; i < result.size(); ++i) {
            cairo_path(cr, result[i]);
            cairo_stroke(cr);
        }

        cairo_set_line_width(cr, 1);

        cairo_set_source_rgb(cr, 1, 0, 0);
        for (unsigned i = 0; i < ix.size(); ++i) {
            draw_handle(cr, ix[i]);
        }
        cairo_stroke(cr);


        double x = width - 90, y = height - 40;
        Point p(x, y), dpoint(25,25), xo(25,0);
        togs[0].bounds = Rect(p,     p + dpoint);
        togs[1].bounds = Rect(p + xo, p + xo + dpoint);
        togs[2].bounds = Rect(p + 2*xo, p + 2*xo + dpoint);
        draw_toggles(cr, togs);
        *notify << "boolop time: " << boolop_time << std::endl;
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    void mouse_pressed(GdkEventButton* e) {
        toggle_events(togs, e);
        Toy::mouse_pressed(e);
    }
    public:
    BoolOps () {}

    void first_time(int argc, char** argv) {
        const char *path_a_name="winding.svgd";
        const char *path_b_name="star.svgd";
        if(argc > 1)
            path_a_name = argv[1];
        if(argc > 2)
            path_b_name = argv[2];
        PathVector paths_a = read_svgd(path_a_name);
        PathVector paths_b = read_svgd(path_b_name);
        OptRect abox = paths_a.boundsExact();
        Point pt = abox ? abox->midpoint() : Point(0,0);

        as = paths_a * Geom::Translate(Point(300,300) - pt);
        bs = paths_b * Geom::Translate(-paths_b.initialPoint());

        p = PointHandle(Point(300,300));
        handles.push_back(&p);
        togs.push_back(Toggle("R", true));
        togs.push_back(Toggle("&", false));
        togs.push_back(Toggle("B", false));
    }
    //virtual bool should_draw_numbers() {return false;}
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
