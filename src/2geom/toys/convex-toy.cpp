#include <2geom/convex-cover.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

using std::vector;
using namespace Geom;

class ConvexTest: public Toy {
    PointSetHandle psh[2];
    public:
    ConvexTest () {
        handles.push_back(&psh[0]);
        handles.push_back(&psh[1]);
        for(unsigned i = 0; i < 15; i++){
            psh[0].push_back(uniform()*uniform()*400+200,
                          uniform()*uniform()*400+200);
            psh[1].push_back(uniform()*uniform()*400+200,
                          uniform()*uniform()*400+200);
	}
    }

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
        cairo_set_line_width (cr, 0.5);
        for(unsigned i = 1; i < 4; i+=2) {
            cairo_move_to(cr, 0, i*width/4);
            cairo_line_to(cr, width, i*width/4);
            cairo_move_to(cr, i*width/4, 0);
            cairo_line_to(cr, i*width/4, width);
        }
    
        vector<Point> all_hands = psh[0].pts;
        all_hands.insert(all_hands.end(), psh[1].pts.begin(), psh[1].pts.end());
        clock_t end_t = clock()+clock_t(0.025*CLOCKS_PER_SEC);
        unsigned iterations = 0;
        while(end_t > clock()) {
            Geom::ConvexHull ch(all_hands);
            iterations++;
        }
        *notify << "constructor time = " << 1000*0.1/iterations << std::endl;


        Geom::ConvexHull ch1(psh[0].pts);
        Geom::ConvexHull ch2(psh[1].pts);

        end_t = clock()+clock_t(0.025*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            graham_merge(ch1, ch2);
            iterations++;
        }
        *notify << "graham merge time = " << 1000*0.1/iterations << std::endl;

/*
        end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            merge(ch1, ch2);
            iterations++;
        }
        *notify << "merge time = " << 1000*0.1/iterations << std::endl;*/
        {
        Geom::ConvexHull gm = graham_merge(ch1, ch2);
        Geom::Point offset = Geom::Point(4, 0);

        /*cairo_set_line_width (cr, 2);
        if(gm.boundary.size() > 0) {
            cairo_move_to(cr, gm.boundary.back() + offset);
            cairo_set_source_rgba (cr, 0., 0., 0, 0.5);
            for(unsigned i = 0; i < gm.boundary.size(); i++) {
                cairo_line_to(cr, gm.boundary[i] + offset);
                draw_number(cr, gm.boundary[i] + offset, i);
            }
        }*/
        cairo_stroke(cr);
        }
        //Geom::ConvexHull m = merge(ch1, ch2);
        //ch.merge(old_mouse_point);

        //assert(ch.is_clockwise());
        //if(m.contains_point(old_mouse_point))
        //    *notify << "mouse in convex" << std::endl;

        Geom::Point offset = Geom::Point(0, -200);

        /*cairo_set_line_width (cr, 2);
        if(m.boundary.size() > 0) {
            cairo_move_to(cr, m.boundary.back() + offset);
            cairo_set_source_rgba (cr, 0., 0., 0, 0.5);
            for(unsigned i = 0; i < m.boundary.size(); i++) {
                cairo_line_to(cr, m.boundary[i] + offset);
                draw_number(cr, m.boundary[i] + offset, i);
            }
        }*/
        cairo_stroke(cr);
        cairo_set_line_width (cr, 1);

        cairo_set_source_rgba(cr, 0., 0., 1., 0.5);
        std::vector<Geom::Point> bs = bridge_points(ch1, ch2);
        for(unsigned i = 0; i < bs.size(); i+=2) {
            cairo_move_to(cr, bs[i]);
            cairo_line_to(cr, bs[i + 1]);
            //draw_number(cr, (bs[i] + bs[i + 1]) / 2, i / 2);
        }
        cairo_stroke(cr);

        cairo_set_source_rgba (cr, 1., 0., 0, 0.8);
        cairo_move_to(cr, ch1.boundary.back());
        for(unsigned i = 0; i < ch1.boundary.size(); i++) {
            cairo_line_to(cr, ch1.boundary[i]);
            draw_number(cr, ch1.boundary[i], i);
        }
        cairo_stroke(cr);

        cairo_move_to(cr, ch2.boundary.back());
        cairo_set_source_rgba (cr, 0., 1., 0, 0.8);
        for(unsigned i = 0; i < ch2.boundary.size(); i++) {
            cairo_line_to(cr, ch2.boundary[i]);
            draw_number(cr, ch2.boundary[i], i);
        }
        cairo_stroke(cr);
        Toy::draw(cr, notify, width, height, save);
    }

    bool should_draw_numbers() { return false; }
};

int main(int argc, char **argv) {
    init(argc, argv, new ConvexTest());

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
