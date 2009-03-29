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
 initial toy for redblack trees
*/


#include <2geom/redblacktree.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <time.h>
using std::vector;
using namespace Geom;
using namespace std;


void draw_redblack_tree(cairo_t* cr, Geom::RedBlack *x, int depth) {

	// works like the RedBlack::print_tree() (inorder function)
    if( x != 0 ){
        draw_redblack_tree(cr, x->left, depth+1);

        //print line and depth on the key of the node
		Geom::Point text_point = Point(x->key, 10);
		char label[4];
		sprintf(label,"%d",depth); // instead of std::itoa(depth, label, 10); 

		cairo_set_source_rgba (cr, 1, 0, 1, 1);
        draw_text(cr, text_point, label);

		Geom::Point line_2nd_point = Point(x->key, 500);
		//draw_line(text_point, line_2nd_point);
		cairo_move_to(cr, text_point );
        cairo_line_to(cr, line_2nd_point);
		cairo_set_source_rgba (cr, 0.5, 1, 0, 0.25);
        cairo_stroke(cr);

		draw_redblack_tree(cr, x->right, depth+1);
    }
}


void insert_in_tree_the_last_rect(Geom::RedBlackTree *rbt, PointSetHandle *handle_set){
		unsigned i = handle_set->pts.size() - 2;
        Rect r1(handle_set->pts[i], handle_set->pts[i+1]);
		rbt->insert(r1, i);	
		rbt->print_tree();
}


class RedBlackToy: public Toy 
{
    PointSetHandle handle_set;
	Geom::Point starting_point;
	Geom::Point ending_point;

	Geom::RedBlackTree rbt;

	int alter_existing_rect;
	int add_new_rect;
	int enable_printing; // used for debug - it disables the tree printing

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_line_width (cr, 1);

		/* 
		a) Don't "update" the quadtree if one handle is changed. Create a new one 
			TODO change this with delete-insert
		b) every 2 Handles is one rect. Then we insert rect in quadtree [1]
		*/
		
		for(unsigned i=0; i<handle_set.pts.size(); i=i+2)
		{
	        Rect r1(handle_set.pts[i], handle_set.pts[i+1]);

		    cairo_set_source_rgba (cr, 0.5, 1, 0, 1);
		    cairo_set_line_width(cr, 0.3);
		    cairo_rectangle(cr, r1);
            // the value to insert in the redblacktree: bounding box and the shapeID
		}

		
		
		Toy::draw(cr, notify, width, height, save,timer_stream);

		cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
		// draw all the quads of the quadtree (as rectangles on screen)
//        draw_quad_tree(cr, qt.root, qt.bx0, qt.by0, qt.bx1 - qt.bx0);
        cairo_stroke(cr);

		Toy::draw(cr, notify, width, height, save,timer_stream);
		draw_redblack_tree(cr, rbt.root, 0);

    }        
    
    void mouse_pressed(GdkEventButton* e) {
		Toy::mouse_pressed(e);
		if(!selected) 
		{
			starting_point = Point(e->x, e->y);
			add_new_rect = 1;
		}
		else
		{
			alter_existing_rect = 1;
		}
    }
    virtual void mouse_released(GdkEventButton* e) {
		Toy::mouse_released(e);
		if(add_new_rect)
		{
			ending_point = Point(e->x, e->y);
			//REMEMBER: handle same point issue in index, not in application level
			handle_set.push_back(starting_point);
			handle_set.push_back(ending_point);
			insert_in_tree_the_last_rect(&rbt, &handle_set);
		}
		else if(alter_existing_rect){
			//TODO update rect
			alter_existing_rect = 0;
		}

    }




public:
    RedBlackToy(): alter_existing_rect(0), add_new_rect(0), enable_printing(1){
        if(handles.empty()) {
            handles.push_back(&handle_set);
        }
    }


};

int main(int argc, char **argv) {
    std::cout << "Let's play with the RedBlakTree!\nBy click and drag you create a new rectangle.\n ONLY Insert works now!!!\n" << std::endl;
    init(argc, argv, new RedBlackToy);
    return 0;
}
	  	 
