#include "path.h"
#include "path-cairo.h"
#include "path-builder.h"

#include "toy-framework.h"

/*2geom crit:
L2 = pythagorean distance. Its ok to call it L2 internally, but have a distance alias.  Maybe there is one.

path convenience constructors - eg, spline, poly, etc.

Perhaps we should split the docs into useage/complexity levels.  eg, "basic", "advanced".  Not sure if there is a good way to doxy this.
(L0, L1, L2 would be put in the advanced section)
*/

class DrawToy: public Toy {
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        Geom::PathSetBuilder builder;
        if(handles.size() > 3) {
            Geom::Point mid = handles[0];
            builder.start_subpath(handles[0]);
            for(int i = 3; i < handles.size(); i+=2) {
                builder.push_cubic(handles[i-2], handles[i-2]*2 - handles[i-3], handles[i-1], handles[i]);
            }
        }
        cairo_PathSet(cr, builder.peek());

        Toy::draw(cr, notify, width, height, save);
    }
    void mouse_pressed(GdkEventButton* e) {
        Geom::Point mouse(e->x, e->y);
        int close_i = 0;
        float close_d = 1000;
        for(int i = 0; i < handles.size(); i++) {
            if(Geom::L2(mouse - handles[i]) < close_d) {
                close_d = Geom::L2(mouse - handles[i]);
                close_i = i;
            }
        }
        if(close_d < 5) {if(e->button==3) handles.erase(handles.begin() += close_i); else Toy::mouse_pressed(e);}
        else            {if(e->button==1) handles.push_back(mouse); else Toy::mouse_pressed(e);}
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "draw-toy", new DrawToy());
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
