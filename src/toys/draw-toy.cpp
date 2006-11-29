#include "path.h"
#include "path-cairo.h"
#include "path-builder.h"

#include "matrix.h"

#include "toy-framework.h"

/*2geom crit:
L2 = pythagorean distance. Its ok to call it L2 internally, but have a distance alias.  Maybe there is one.

path convenience constructors - eg, spline, poly, etc.

Perhaps we should split the docs into useage/complexity levels.  eg, "basic", "advanced".  Not sure if there is a good way to doxy this.
(L0, L1, L2 would be put in the advanced section)
*/

Geom::Point reflect_about(Geom::Point pnt, Geom::Point line) {
    Geom::Point norm = unit_vector(line);
    return pnt * from_basis(norm.cw(), norm);
}

class DrawToy: public Toy {
    //Knot : Och : Och : Knot : Och : Och : Knot : Och : Och : ...
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0, 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
        for(int i = 0; i < handles.size(); i+=3) {
            draw_circ(cr, handles[i]);
            draw_number(cr, handles[i], i/3);
        }
        cairo_set_source_rgba (cr, 0, 0, 0.5, 1);
        for(int i = 2; i < handles.size(); i+=3) {
            draw_circ(cr, handles[i]);
            draw_circ(cr, handles[i-1]);
        }

        cairo_set_source_rgba (cr, 0.5, 0, 0, 1);
        for(int i = 3; i < handles.size(); i+=3) {
            draw_line_seg(cr, handles[i-2], handles[i-3]);
            draw_line_seg(cr, handles[i], handles[i-1]);
        }

        cairo_set_source_rgba (cr, 0, 0, 0, 1);
        Geom::PathSetBuilder builder;
        if(handles.size() > 3) {
            Geom::Point mid = handles[0];
            builder.start_subpath(handles[0]);
            for(int i = 0; i < handles.size() - 3; i+=3) {
                builder.push_cubic(handles[i], handles[i+1], handles[i+2], handles[i+3]);
            }
        }
        cairo_PathSet(cr, builder.peek());
        cairo_stroke(cr);
    }
    void mouse_pressed(GdkEventButton* e) {
        Geom::Point mouse(e->x, e->y);
        int close_i = 0;
        float close_d = 1000;
        for(int i = 0; i < handles.size(); i+=1) {
            if(Geom::L2(mouse - handles[i]) < close_d) {
                close_d = Geom::L2(mouse - handles[i]);
                close_i = i;
            }
        }
        if(close_d < 5) {
             if(e->button==3) handles.erase(handles.begin() += close_i); else selected = close_i;
        } else {
             if(e->button==1) {
                 if(handles.size() > 0) {
                     if(handles.size() == 1) {
                         handles.push_back((handles[0] * 2 + mouse) / 3);
                         handles.push_back((handles[0] + mouse * 2) / 3);
                     } else {
                         Geom::Point prev = handles[handles.size() - 1];
                         Geom::Point curve = prev - handles[handles.size() - 2];
                         handles.push_back(prev + curve);
                         handles.push_back(mouse + curve);
                     }
                 }
                 handles.push_back(mouse);
             } else {
                 selected = close_i;
             }
        }
    }

    void mouse_moved(GdkEventMotion* e) {
        Geom::Point mouse(e->x, e->y);
        
        if(e->state & (GDK_BUTTON1_MASK) && selected != -1) {
            if (selected % 3 == 0) {
                Geom::Point diff = mouse - handles[selected];
                if(handles.size() - 1 > selected) handles[selected + 1] += diff;
                if(selected != 0) handles[selected - 1] += diff; 
            }
            Toy::mouse_moved(e);
        }
    }

    bool should_draw_numbers() { return false; }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
