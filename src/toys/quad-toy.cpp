/** 
 * adaptive quad tree for display graph, search operations
 * (njh)
 */
#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "s-basis-2d.h"

#include "path-cairo.h"

#include <iterator>
#include "quadtree.h"

#include "toy-framework.h"

using std::vector;

unsigned total_pieces_sub;
unsigned total_pieces_inc;

void draw_quad_tree(cairo_t* cr, Quad *q, double x, double y, double d) {
    if(q) {
        cairo_rectangle(cr, x, y, d, d);
        cairo_stroke(cr);
        double dd = d/2;
        draw_quad_tree(cr, q->children[0], x, y, dd);
        draw_quad_tree(cr, q->children[1], x+dd, y, dd);
        draw_quad_tree(cr, q->children[2], x, y+dd, dd);
        draw_quad_tree(cr, q->children[3], x+dd, y+dd, dd);
    }
}

// returns true if the subtree is empty, and deletes any empty subtrees.
bool clean_quad_tree(Quad *q) { 
    if(q) {
        bool all_clean = q->data.empty();
        for(int i = 0; i < 4; i++)
            if(clean_quad_tree(q->children[i])) {
                delete q->children[i];
                q->children[i] = 0;
            } else if(q->children[i])
                all_clean = false;
        if(all_clean) {
            return true;
        }
    }
    return false;
}

class QuadToy: public Toy {
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
        for(int i = 0; i < handles.size(); i++) {
            std::ostringstream notify;
            notify << i;
            draw_circ(cr, handles[i]);
            cairo_move_to(cr, handles[i]);
            PangoLayout* layout = pango_cairo_create_layout (cr);
            pango_layout_set_text(layout, 
                                  notify.str().c_str(), -1);
    
            PangoFontDescription *font_desc = pango_font_description_new();
            pango_font_description_set_family(font_desc, "Sans");
            const int size_px = 10;
            pango_font_description_set_absolute_size(font_desc, size_px * 1024.0);
            pango_layout_set_font_description(layout, font_desc);
            PangoRectangle logical_extent;
            pango_layout_get_pixel_extents(layout,
                                           NULL,
                                           &logical_extent);
            pango_cairo_show_layout(cr, layout);
        }
        cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
        cairo_set_line_width (cr, 0.5);
        for(int i = 1; i < 4; i+=2) {
            cairo_move_to(cr, 0, i*height/4);
            cairo_line_to(cr, width, i*height/4);
            cairo_move_to(cr, i*width/4, 0);
            cairo_line_to(cr, i*width/4, height);
        }
        
        QuadTree qt;
        
        cairo_new_sub_path(cr);
        
        for(unsigned i = 0; i < handles.size()/2; i++) {
            Geom::Point p0 = handles[i*2];
            Geom::Point p1 = handles[i*2+1];
            Geom::Point centre = (p0 + p1)/2;
            double rad = Geom::L2(p0 - centre);
            cairo_arc (cr, centre[0], centre[1], rad, 0., 2 * M_PI);
            //cairo_move_to(cr, p0);
            //cairo_line_to(cr, p1);
            cairo_stroke(cr);
            double x0 = centre[0]-rad;//std::min(p0[0], p1[0]);
            double x1 = centre[0]+rad;//std::max(p0[0], p1[0]);
            double y0 = centre[1]-rad;//std::min(p0[1], p1[1]);
            double y1 = centre[1]+rad;//std::max(p0[1], p1[1]);
            qt.insert(x0, y0, x1, y1, i);
        }
        clean_quad_tree(qt.root);

        cairo_set_source_rgba (cr, 0.5, 0.125, 0, 1);
        draw_quad_tree(cr, qt.root, qt.bx0, qt.by0, qt.bx1 - qt.bx0);
    
        *notify << "total pieces subdivision = " << total_pieces_sub << std::endl; 
        *notify << "total pieces inc = " << total_pieces_inc; 
    
        Toy::draw(cr, notify, width, height, save);
    }

    virtual bool should_draw_bounds () { return false; }
    virtual bool should_draw_numbers () { return false; }

    public:
    QuadToy() {
        for(int i = 0; i < 100; i++) {
            Geom::Point p(uniform() * 400, uniform() * 400);
            handles.push_back(p);
            handles.push_back(p + Geom::Point(uniform() * 40, uniform() * 40));
        }
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "QuadToy", new QuadToy);
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
