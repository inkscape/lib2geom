#include <2geom/convex-hull.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

using std::vector;
using namespace Geom;
using namespace std;
struct PtLexCmp{
    bool operator()(const Point &a, const Point &b) {
        return (a[0] < b[0]) || ((a[0] == b[0]) and (a[1] < b[1]));
    }
};
// draw ax + by + c = 0
void draw_line_in_rect(cairo_t*cr, Rect &r, Point n, Point p) {
    boost::optional<LineSegment> ls = rect_line_intersect(r, LineSegment(p, p + rot90(n)));
    if(ls) {
	cairo_move_to(cr, (*ls)[0]);
	cairo_line_to(cr, (*ls)[1]);
	cairo_stroke(cr);
    }
}


ConvexHull rect2convexhull(Rect const & r) {
    ConvexHull ch;
    for(int i = 0; i < 4; i++)
        ch.merge(r.corner(i));
    return ch;
}

void rot_cal(cairo_t* cr, ConvexHull const &ch) {
    Point tb = ch.back();
    for(unsigned i = 0; i < ch.size(); i++) {
        Point tc = ch[i];
        Point n = -rot90(tb-tc);
        Point ta = *ch.furthest(n);
        cairo_move_to(cr, tc);
        cairo_line_to(cr, ta);
        tb = tc;
    }
}

const bool ch2_tests = true;

/*ConvexHull inset_ch(ConvexHull ch, double dist) {
    ConvexHull ret;
    for(int i = 0; i < ch.size(); i++) {
        Point bisect = ((ch[i+1] - ch[i]) + (ch[i-1] - ch[i]))*0.5;
        ret.boundary.push_back(unit_vector(bisect)*dist+ch[i]);
    }
    return ret;
    }*/

ConvexHull inset_ch(ConvexHull ch, double dist) {
    ConvexHull ret;
    for(unsigned i = 0; i < ch.size(); i++) {
        Point bisect = (rot90(ch[i+1] - ch[i]));
        ret.merge(unit_vector(bisect)*dist+ch[i]);
        ret.merge(unit_vector(bisect)*dist+ch[i+1]);
    }
    return ret;
}

class ConvexTest: public Toy {
    PointSetHandle psh[2];
    PointHandle direction_handle;
    PointSetHandle test_window;
    public:
    ConvexTest () {
        handles.push_back(&psh[0]);
        if(ch2_tests)
            handles.push_back(&psh[1]);
        if(0) {
            handles.push_back(&direction_handle);
            direction_handle.pos = Point(10,10);
        }
        for(unsigned i = 0; i < 5; i++){
            psh[0].push_back(uniform()*uniform()*400+200,
                          uniform()*uniform()*400+200);
            if(ch2_tests)
                psh[1].push_back(uniform()*uniform()*400+200,
                                 uniform()*uniform()*400+200);
	}
        if(0) {
            handles.push_back(&test_window);
            test_window.push_back(Point(100,100));
            test_window.push_back(Point(200,200));
        }
    }

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
        cairo_set_line_width (cr, 0.5);
        for(unsigned i = 1; i < 4; i+=2) {
            cairo_move_to(cr, 0, i*width/4);
            cairo_line_to(cr, width, i*width/4);
            cairo_move_to(cr, i*width/4, 0);
            cairo_line_to(cr, i*width/4, width);
        }
    
        if(0) {
            vector<Point> all_hands = psh[0].pts;
            all_hands.insert(all_hands.end(), psh[1].pts.begin(), psh[1].pts.end());
            clock_t end_t = clock()+clock_t(0.025*CLOCKS_PER_SEC);
            unsigned iterations = 0;
            while(end_t > clock()) {
                Geom::ConvexHull ch(all_hands);
                iterations++;
            }
            *notify << "constructor time = " << 1000*0.1/iterations << std::endl;
        }

        Geom::ConvexHull ch1(psh[0].pts);
        Geom::ConvexHull ch2(psh[1].pts);

        if(0) {
            clock_t end_t = clock()+clock_t(0.025*CLOCKS_PER_SEC);
            unsigned iterations = 0;
            while(end_t > clock()) {
                graham_merge(ch1, ch2);
                iterations++;
            }
            *notify << "graham merge time = " << 1000*0.1/iterations << std::endl;
        }
/*
        end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            merge(ch1, ch2);
            iterations++;
        }
        *notify << "merge time = " << 1000*0.1/iterations << std::endl;*/
        if(ch2_tests) {
            Geom::ConvexHull gm = graham_merge(ch1, ch2);
            //Geom::Point offset = Geom::Point(4, 0);
        
            if(0) {
                Point cent;
                gm.centroid_and_area(cent);
                draw_cross(cr, cent);
                cairo_move_to(cr, cent);
                cairo_line_to(cr, direction_handle.pos);
                cairo_stroke(cr);
                Point const * futh =  gm.furthest(direction_handle.pos - cent);
                draw_cross(cr, *futh);
                {
                    Point a, b, c;
                    //double dia = gm.narrowest_diameter(a, b, c);
                    cairo_save(cr);
                    cairo_set_line_width(cr, 2);
                    cairo_move_to(cr, b);
                    cairo_line_to(cr, c);
                    cairo_move_to(cr, a);
                    cairo_line_to(cr, (c-b)*dot(a-b, c-b)/dot(c-b,c-b)+b);
                    cairo_stroke(cr);
                    //std::cout << a << ", " << b << ", " << c << ": " << dia << "\n";
                    cairo_restore(cr);
                }
            }
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

        //Geom::Point offset = Geom::Point(0, -200);

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
        
        if(ch2_tests) {
            cairo_save(cr);
            cairo_set_source_rgba(cr, 0., 0., 1., 0.5);
            cairo_set_line_width(cr, 3);
            std::vector<std::pair<int,int> > bs = bridges(ch1, ch2);
            for(unsigned i = 0; i < bs.size(); i++) {
                draw_line_seg_with_arrow(cr, ch1[bs[i].first], ch2[bs[i].second]);
                cairo_stroke(cr);
                //draw_number(cr, (bs[i] + bs[i + 1]) / 2, i / 2);
            }
            cairo_restore(cr);
        }
        
        if(0) {
            rot_cal(cr, ch1);
            cairo_stroke(cr);
            
            Rect window_r(Point(0,0), Point(width, height));
            
            {
                Point cent;
                ch1.centroid_and_area(cent);
                draw_cross(cr, cent);
                cairo_move_to(cr, cent);
                cairo_line_to(cr, direction_handle.pos);
                cairo_stroke(cr);
                Point dir = direction_handle.pos - cent;
                Point const * futh1 =  ch1.furthest(dir);
                draw_line_in_rect(cr, window_r, dir, *futh1);
                draw_cross(cr, *futh1);
                Point const * futh2 =  ch1.furthest(-dir);
                draw_line_in_rect(cr, window_r, -dir, *futh2);
                draw_cross(cr, *futh2);
            }
        }
        if(1) {
            cairo_save(cr);
            cairo_set_source_rgba (cr, 1., 0., 0, 0.3);
            cairo_set_line_width(cr, 30);
            cairo_convex_hull(cr, merge(ch1, ch2));
            cairo_stroke(cr);
            cairo_restore(cr);
        }
        
        cairo_set_source_rgba (cr, 1., 0., 0, 0.8);
        cairo_convex_hull(cr, ch1);
        cairo_stroke(cr);

        if(ch2_tests) {
            cairo_set_source_rgba (cr, 0., 1., 0, 0.8);
            cairo_convex_hull(cr, ch2);
            cairo_stroke(cr);
        }
        if(0) {
            cairo_set_source_rgb(cr, 0,0,0);
            Rect r(test_window.pts[0], test_window.pts[1]);
            ConvexHull ch = rect2convexhull(r);
            cairo_convex_hull(cr, ch);
            cairo_stroke(cr);
            ConvexHull intr = intersection(ch, ch1);
            //cairo_rectangle(cr, r);
            cairo_convex_hull(cr, intr);
            cairo_set_source_rgba(cr, 0,0,0,0.3);
            
            cairo_fill(cr);
        }
        Toy::draw(cr, notify, width, height, save,timer_stream);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
