/*
 * Copyright 2009  Evangelos Katsikaros <vkatsikaros at yahoo dot gr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

/*
I am still very inexperienced with lib2geom
so don't use these examples as a reference :)

----------------------------------------------

This toy is used to understand how the quadtrees work here.

Notes: (not in a particular order)
In the quad trees we insert rectangles since we are interested in bounding boxes
and not points.

TODO check the root insertion
TODO clean_quad_tree()

*/

#include <2geom/quadtree.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <time.h>
using std::vector;
using namespace Geom;
using namespace std;


void draw_quad_tree(cairo_t* cr, Geom::Quad *q, double x, double y, double d) {
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
bool clean_quad_tree(Geom::Quad *q) { 
    if(q) 
	{
        bool all_clean = q->data.empty();
        for(unsigned i = 0; i < 4; i++)
		{
            if(clean_quad_tree(q->children[i]))
			{
                delete q->children[i];
                q->children[i] = 0;
            } 
			else if(q->children[i])
			{
                all_clean = false;
			}
		}
        if(all_clean)
		{
            return true;
        }
    }
    return false;
}



class QuadToy2: public Toy {
    PointSetHandle handle_set;

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_line_width (cr, 1);

		// every 2 Handles is one rect.
		// and insert rect in quadtree
		Geom::QuadTree qt;

		for(unsigned i=0; i<handle_set.pts.size(); i=i+2)
		{
	        Rect r1(handle_set.pts[i], handle_set.pts[i+1]);

		    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
		    cairo_set_line_width(cr, 0.3);
		    cairo_rectangle(cr, r1);

            // the value to insert in the quadtree
			// the bounding box and the shapeID
            qt.insert(r1, i); 
		}

		clean_quad_tree(qt.root); //TODO why is this needed ???

        cairo_set_source_rgba (cr, 0.5, 0.125, 0, 1);
        draw_quad_tree(cr, qt.root, qt.bx0, qt.by0, qt.bx1 - qt.bx0);
        
        cairo_stroke(cr);
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }        

    int mouse_down;
	Geom::Point starting_point;
	Geom::Point ending_point;

    
    void mouse_pressed(GdkEventButton* e) {
		Toy::mouse_pressed(e);
		if(!selected) {
			starting_point = Point(e->x, e->y);
			std::cout << " start mouse_pressed at " << starting_point << std::endl;
			mouse_down = 1;
		}
    }


    virtual void mouse_released(GdkEventButton* e) {
		Toy::mouse_released(e);
		if(mouse_down)
		{
			ending_point = Point(e->x, e->y);
			std::cout << " end mouse_release at " << ending_point  << std::endl;
			mouse_down = 0;
			// TODO check for B L, T R ???
			handle_set.push_back(starting_point);
			handle_set.push_back(ending_point);
		}		
    }

public:
    QuadToy2(){
        if(handles.empty()) {
            handles.push_back(&handle_set);
        }
    }


};

int main(int argc, char **argv) {
    std::cout << "Let's play with the QuadTree!\nBy clicking you can create new points on the camvas. We suppose that each point defines a rectangle of fixed dimensions." << std::endl;
    init(argc, argv, new QuadToy2);
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
//vim:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :

 	  	 
