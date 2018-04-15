#include <2geom/d2.h>
#include <2geom/intersection-graph.h>
#include <2geom/path.h>
#include <2geom/sbasis.h>
#include <2geom/svg-path-parser.h>
#include <2geom/transforms.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#include <cstdlib>
#include <boost/algorithm/minmax_element.hpp>

using namespace Geom;

class BoolOps : public Toy {
    PathVector as, bs;
    Line ah, bh;
    PointHandle path_handles[4];
    std::vector<Toggle> togs;
    bool path_handles_inited = false;

public:
    BoolOps() = default;

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        if (!path_handles_inited) {
            Rect vp(Point(10,10), Point(width-10, height-10));
            setup_path_handles(vp);
        }

        Line aht(path_handles[0].pos, path_handles[1].pos);
        Line bht(path_handles[2].pos, path_handles[3].pos);

        PathVector ast = as * ah.transformTo(aht);
        PathVector bst = bs * bh.transformTo(bht);

        Timer tm;
        tm.start();

        PathIntersectionGraph pig(ast, bst);
        std::vector<Point> dix, ix, wpoints;
        ix = pig.intersectionPoints();
        dix = pig.intersectionPoints(true);
        wpoints = pig.windingPoints();
        PathVector result, f_in, f_out;

        if (pig.valid()) {
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
                result = ast;
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
        }
        
        if (togs[5].on || togs[6].on) {
            pig.fragments(f_in, f_out);
        }
        Timer::Time boolop_time = tm.lap();

        cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
        cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);

        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_path(cr, result);
        cairo_fill(cr);

        cairo_set_line_width(cr, 1);
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
        cairo_path(cr, ast);

        cairo_stroke(cr);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_path(cr, bst);
        cairo_stroke(cr);
        
        if (togs[5].on) {
            cairo_set_source_rgb(cr, 1, 0, 0);
            cairo_path(cr, f_in);
            cairo_stroke(cr);
        }
        if (togs[6].on) {
            cairo_set_source_rgb(cr, 0, 0, 1);
            cairo_path(cr, f_out);
            cairo_stroke(cr);
        }

        //cairo_set_line_width(cr, 1);

        if (togs[7].on) {
            cairo_set_source_rgb(cr, 0, 1, 1);
            for (auto wpoint : wpoints) {
                draw_handle(cr, wpoint);
            }
            cairo_stroke(cr);
        }

        if (togs[3].on) {
            cairo_set_source_rgb(cr, 0, 1, 0);
            for (auto i : ix) {
                draw_handle(cr, i);
            }
            cairo_stroke(cr);
        }

        if (togs[4].on) {
            cairo_set_source_rgb(cr, 1, 0, 0);
            for (auto i : dix) {
                draw_handle(cr, i);
            }
            cairo_stroke(cr);
        }


        double x = width - 90, y = height - 40, y2 = height - 80;
        Point p(x, y), p2(x, y2), dpoint(25,25), xo(25,0);
        togs[0].bounds = Rect(p,     p + dpoint);
        togs[1].bounds = Rect(p + xo, p + xo + dpoint);
        togs[2].bounds = Rect(p + 2*xo, p + 2*xo + dpoint);
        
        togs[3].bounds = Rect(p2 - 2*xo, p2 - 2*xo + dpoint);
        togs[4].bounds = Rect(p2 - xo, p2 - xo + dpoint);
        togs[5].bounds = Rect(p2,     p2 + dpoint);
        togs[6].bounds = Rect(p2 + xo, p2 + xo + dpoint);
        togs[7].bounds = Rect(p2 + 2*xo, p2 + 2*xo + dpoint);
        draw_toggles(cr, togs);

        *notify << ix.size() << " intersections";
        if (dix.size() != 0) {
            *notify << " + " << dix.size() << " defective";
        }
        if (pig.valid()) {
            *notify << "\nboolop time: " << boolop_time << std::endl;
        } else {
            *notify << "\nboolop failed, time: " << boolop_time << std::endl;
        }
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    void mouse_pressed(GdkEventButton* e) override {
        toggle_events(togs, e);
        Toy::mouse_pressed(e);
    }

    void first_time(int argc, char** argv) override {
        const char *path_a_name="svgd/winding.svgd";
        const char *path_b_name="svgd/star.svgd";
        if(argc > 1)
            path_a_name = argv[1];
        if(argc > 2)
            path_b_name = argv[2];

        as = read_svgd(path_a_name);
        bs = read_svgd(path_b_name);

        OptRect abox = as.boundsExact();
        OptRect bbox = bs.boundsExact();

        if (!abox) {
            std::clog << "Error: path A is empty" << std::endl;
        }
        if (!bbox) {
            std::clog << "Error: path B is empty" << std::endl;
        }
        if (!abox || !bbox) {
            std::exit(1);
        }

        std::vector<Point> anodes = as.nodes();
        std::vector<Point> bnodes = bs.nodes();

        typedef std::vector<Point>::iterator Iter;
        std::pair<Iter, Iter> apts =
            boost::minmax_element(anodes.begin(), anodes.end(), Point::LexLess<Y>());
        std::pair<Iter, Iter> bpts =
            boost::minmax_element(bnodes.begin(), bnodes.end(), Point::LexLess<Y>());

        ah = Line(*apts.first, *apts.second);
        bh = Line(*bpts.first, *bpts.second);

        togs.push_back(Toggle("R", true));
        togs.push_back(Toggle("&", false));
        togs.push_back(Toggle("B", false));

        togs.push_back(Toggle("X", true));
        togs.push_back(Toggle("D", true));
        togs.push_back(Toggle("I", false));
        togs.push_back(Toggle("O", false));
        togs.push_back(Toggle("W", false));
    }
    
    void setup_path_handles(Rect const &viewport) {
        Line aht = ah * as.boundsExact()->transformTo(viewport, Aspect(ALIGN_XMID_YMID));
        Line bht = bh * bs.boundsExact()->transformTo(viewport, Aspect(ALIGN_XMID_YMID));

        path_handles[0] = PointHandle(aht.initialPoint());
        path_handles[1] = PointHandle(aht.finalPoint());
        path_handles[2] = PointHandle(bht.initialPoint());
        path_handles[3] = PointHandle(bht.finalPoint());

        for (auto & path_handle : path_handles) {
            handles.push_back(&path_handle);
        }
        path_handles_inited = true;
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
