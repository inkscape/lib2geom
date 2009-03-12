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

[1] In the quad trees we insert rectangles. We are interested in bounding boxes 
of shapes so we don't bother with points of shapes. 
  Each "bounding box" is defined by two Handles. Every 2 Handles in handle_set 
is one rect. We insert the rect in the QuadTree

  QuadTree::insert(Geom::Rect const &r, int shape);

  So for this demo we suppose that we insert bounding boxes (Rect) of shapes and
the shape ID (int). The shape id is just the position of the 1st Handle in 
handle_set. This position doesn't change.

[2] We do not "update" the QuadTree with every change of one Rect. Everytime we 
call draw() we create a new QuadTree and we insert the Rects in it with the same
order (order they appear in handle_set). We do this since "updating" the 
QuadTree is complex (and perhaps costly).
  So, for this demo it's much-much easier to simply create a new QuadTree with the 
updated values.

[3] (clean_quad_tree is now removed and embedded in the Quadtree class)
the clean_quad_tree is used only for the initial insertion if the inital
bounding box doesn't fit the 1st rect. And it doesn't since initial bounding box
is (0,0) (1,1). Bounding box doubles its size until it covers the rect.

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

void print_quadtree_state(Geom::Quad *q, int depth, std::vector<char> parent_history)
{
	if(q) 
	{
		std::cout << " depth " << depth << ": ";
		for(unsigned i=0; i < parent_history.size(); i++)
		{
			std::cout << parent_history[i] << "->";
		}	
		std::cout << " : ";

		if( !q->data.empty() )
		{
			for(unsigned i=0; i < q->data.size(); i++)
			{
				std::cout << q->data[i] << ", ";
			}	
			std::cout << std::endl;
		}
		else
		{
			std::cout << "E ";
			std::cout << std::endl;
		}
		std::vector<char> v(parent_history);
		v.push_back('0');
		print_quadtree_state(q->children[0], depth+1, v);

		v.pop_back();
		v.push_back('1');
		print_quadtree_state(q->children[1], depth+1, v);

		v.pop_back();
		v.push_back('2');
		print_quadtree_state(q->children[2], depth+1, v);

		v.pop_back();
		v.push_back('3');
		print_quadtree_state(q->children[3], depth+1, v);
	}	
}




class QuadToy2: public Toy 
{
    PointSetHandle handle_set;
	Geom::Point starting_point;
	Geom::Point ending_point;

	int alter_existing_rect;
    int create_new_rect;
	int added_new_rect;
	int enable_printing; // used for debug - it disables the tree printing

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_line_width (cr, 1);

		/* 
		a) Don't "update" the quadtree if one handle is changed. Create a new one (see [2]).
		b) every 2 Handles is one rect. Then we insert rect in quadtree [1]
		*/
		Geom::QuadTree qt;
		for(unsigned i=0; i<handle_set.pts.size(); i=i+2)
		{
	        Rect r1(handle_set.pts[i], handle_set.pts[i+1]);

		    cairo_set_source_rgba (cr, 0.5, 1, 0, 1);
		    cairo_set_line_width(cr, 0.3);
		    cairo_rectangle(cr, r1);

			// TODO correct the same point issue in Quad, not application level
			// either reject new same point OR have a list of them

            // the value to insert in the quadtree: bounding box and the shapeID
            qt.insert(r1, i); 
		}
		
		Toy::draw(cr, notify, width, height, save,timer_stream);

		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
		// draw all the quads of the quadtree (as rectangles on screen)
        draw_quad_tree(cr, qt.root, qt.bx0, qt.by0, qt.bx1 - qt.bx0);
        cairo_stroke(cr);

		Toy::draw(cr, notify, width, height, save,timer_stream);

		// print changes in the status of the tree
		if( ( alter_existing_rect || added_new_rect ) && enable_printing )
		{
			std::cout << "Print QuadTree layout (NE=0 NW=1 SE=2 SW=3)" << std::endl;
			std::vector<char> v;
			v.push_back('R');
			print_quadtree_state(qt.root, 0, v);
			std::cout << "---------------------" << std::endl;

			added_new_rect = 0;
		}
    }        
    
    void mouse_pressed(GdkEventButton* e) {
		Toy::mouse_pressed(e);
		//std::cout << e->button << std::endl;
		if(!selected) 
		{
			starting_point = Point(e->x, e->y);
			create_new_rect = 1;
		}
		else
		{
			alter_existing_rect = 1;
		}
    }

    virtual void mouse_released(GdkEventButton* e) {
		Toy::mouse_released(e);
		if(create_new_rect)
		{
			ending_point = Point(e->x, e->y);
			
			if(starting_point == ending_point)
			{
				ending_point = Point( (e->x) + 2, (e->y) + 2 );
			}
			handle_set.push_back(starting_point);
			handle_set.push_back(ending_point);
			create_new_rect = 0;
			added_new_rect = 1;
		}
		alter_existing_rect = 0;
    }


public:
    QuadToy2(): alter_existing_rect(0), create_new_rect(0), added_new_rect(0), enable_printing(1){
        if(handles.empty()) {
            handles.push_back(&handle_set);
        }
    }


};

int main(int argc, char **argv) {
    std::cout << "Let's play with the QuadTree!\nBy click and drag you create a new rectangle." << std::endl;
    init(argc, argv, new QuadToy2);
    return 0;
}
	  	 
