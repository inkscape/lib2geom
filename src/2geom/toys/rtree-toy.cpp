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


#include <2geom/rtree.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <time.h>
using std::vector;
using namespace Geom;
using namespace std;


class RTreeToy: public Toy 
{
    PointSetHandle handle_set;
	Geom::Point starting_point;		// during click and drag: start point of click
	Geom::Point ending_point;		// during click and drag: end point of click (release)
	Geom::Point highlight_point;	// not used

	Geom::RTree rtree;
	//RedBlack* search_result;
	//RedBlack temp_deleted_node;

	// colors we are going to use for different purposes
	colour color_shape, color_shape_guide,
		color_rtree_l0,
		color_rtree_l1,
		color_rtree_l2,
		color_rtree_l3 
	; 				

	colour color_select_area, color_select_area_guide;	// red(a=0.6), red

	int alter_existing_rect;
	int add_new_rect;

	Rect rect_chosen;	// the rectangle of the search area
	Rect dummy_draw;	// the "helper" rectangle that is shown during the click and drag (before the mouse release)
	int mode;			// insert/alter, search, delete  modes
	string help_str, out_str, status_str;

	// printing of the tree
	int help_counter;	// the "x" of the label of each node
	static const int label_size = 15 ; // size the label of each node

	// used for the keys that switch between modes
    enum menu_item_t
    {
        INSERT = 0,
		DELETE,
		SEARCH,
        TOTAL_ITEMS // this one must be the last item
    };
    static const char* menu_items[TOTAL_ITEMS];
    static const char keys[TOTAL_ITEMS];



    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_line_width( cr, 1 );

		// draw the rects that we have in the handles
		for( unsigned i=0; i<handle_set.pts.size(); i=i+2 ){
	        Rect r1( handle_set.pts[i], handle_set.pts[i+1] );
		    cairo_rectangle( cr, r1 );
		}
	    cairo_set_source_rgba( cr, color_shape);
		cairo_stroke( cr );

		// draw a rect if we click & drag (so that we know what we are going to create)
		if(add_new_rect){
			dummy_draw = Rect( starting_point, ending_point );
			cairo_rectangle( cr, dummy_draw );
			if( mode == 0){
				cairo_set_source_rgba( cr, color_shape_guide);
			}
			else if( mode == 1){
			    cairo_set_source_rgba( cr, color_select_area_guide );
			}
			cairo_stroke( cr );
		}

		// draw a rect for the search area
		cairo_rectangle( cr, rect_chosen );
	    cairo_set_source_rgba( cr, color_select_area);
		cairo_stroke( cr );
	
		*notify << status_str << std::endl << out_str << std::endl << help_str;

		Toy::draw( cr, notify, width, height, save,timer_stream );
		//draw_tree_in_toy( cr ,rbtree_x.root, 0);
		//help_counter=0;
    }        
    
    void mouse_moved(GdkEventMotion* e){
		if( !( alter_existing_rect && mode == 1 ) ){
			Toy::mouse_moved(e);
		}

		if(add_new_rect){
			ending_point = Point(e->x, e->y);
		}
	}

    void mouse_pressed(GdkEventButton* e) {
		Toy::mouse_pressed(e);
		if(e->button == 1){		// left mouse button
			if( mode == 0 ){	// mode: insert / alter
				if(!selected) {
					starting_point = Point(e->x, e->y);
					ending_point = starting_point;
					add_new_rect = 1;
				}
				else
				{
					// TODO find the selected rect 
					// ideas : from Handle *selected ???
					//std::cout <<find_selected_rect(selected) << std::endl ;
					alter_existing_rect = 1;
				}
			}
			else if( mode == 1 ){	// mode: search
				if(!selected) {
					starting_point = Point(e->x, e->y);
					ending_point = starting_point;
					add_new_rect = 1;
				}
				else{
					alter_existing_rect = 1;
				}
			}
			else if( mode == 2) {	// mode: delete
			}
		}
		else if(e->button == 2){	//middle button
		}
		else if(e->button == 3){	//right button
		}
    }

    virtual void mouse_released(GdkEventButton* e) {
		Toy::mouse_released(e);
		if( e->button == 1 ) { 		//left mouse button
			if( mode == 0) {	// mode: insert / alter
				if( add_new_rect ){
					ending_point = Point(e->x, e->y);
					handle_set.push_back(starting_point);
					handle_set.push_back(ending_point);
					insert_in_tree_the_last_rect();
					add_new_rect = 0;
				}
				else if( alter_existing_rect ){
					//TODO update rect (and tree)
					// delete selected rect
					// insert altered
					alter_existing_rect = 0;
				}
			}
			else if( mode == 1 ){	// mode: search
				if( add_new_rect ){
					ending_point = Point(e->x, e->y);
					rect_chosen = Rect(starting_point, ending_point);

					// search in the X axis
					Coord a = rect_chosen[0].min();
					Coord b = rect_chosen[0].max();
				/*	search_result = rbtree_x.search( Interval( a, b ) );
					if(search_result){
						std::cout << "Found: (" << search_result->data << ": " << search_result->key() 
							<< ", " << search_result->high() << " : " << search_result->subtree_max << ") " 
							<< std::endl;
					}
					else{
						std::cout << "Nothing found..."<< std::endl;
					}*/
					add_new_rect = 0;
				}
				else if(alter_existing_rect){
					// do nothing
					alter_existing_rect = 0;
				}
			}
			else if( mode == 2) {	// mode: delete

			}
		}
		else if(e->button == 2){	//middle button			
		}
		else if(e->button == 3){	//right button

		}
    }


    void key_hit(GdkEventKey *e)
    {
        char choice = std::toupper(e->keyval);
        switch ( choice )
        {
            case 'A':
                mode = 0;
				out_str = "Mode: Insert - Alter(NOT implemented)";
				status_str = "";
                break;
            case 'B':
                mode = 1;
				out_str = "Mode: Search";
				status_str = "";
                break;
            case 'C':
                mode = 2;
				out_str = "Mode: Delete(NOT implemented)";
				status_str = "";
                break;
        }
        redraw();
    }

	void insert_in_tree_the_last_rect(){
			unsigned i = handle_set.pts.size() - 2;
		    Rect r1(handle_set.pts[i], handle_set.pts[i+1]);
			// insert in R tree
			rtree.insert( r1, i );
			std::cout << " \nTree:\n" << std::endl;
			rtree.print_tree( rtree.root, 0);	
	};

	void draw_tree_in_toy(cairo_t* cr, Geom::RTree* n, int depth = 0) {
		if(n){
/*			if(n->left){
				draw_tree_in_toy(cr, n->left, depth+1);
			}
			help_counter += 1;
			//drawthisnode(cr, x*10, depth*10);
			if(n->isRed){
				cairo_set_source_rgba (cr, color_select_area_guide);
			}
			else{
				cairo_set_source_rgba (cr, color_rect_guide);
			}
			
			cairo_stroke(cr);

			Geom::Point text_point = Point( help_counter*15, depth*15 );
			char label[4];
			sprintf( label,"%d",n->data ); // instead of std::itoa(depth, label, 10); 

			draw_text(cr, text_point, label);
			////////////////////////////////////////////////////////////////
			if(n->right){
				draw_tree_in_toy(cr, n->right, depth+1);
			}
*/
		}
	};



public:
    RTreeToy(): 	color_shape(0, 0, 0, 0.6), color_shape_guide(0, 0, 0, 1),
					color_rtree_l0(1, 0, 1, 1), color_rtree_l1(0, 1, 1, 1),
					color_rtree_l2(1, 1, 0, 1), color_rtree_l3(0, 0, 1, 1), 
					//color_rect(0, 0, 0, 0.6), color_rect_guide(0, 0, 0, 1), 

					color_select_area(1, 0, 0, 0.6 ),  color_select_area_guide(1, 0, 0, 1 ),

					alter_existing_rect(0), add_new_rect(0), mode(0), help_str("A: Insert/Alter, B: Search, C: Delete"),
					out_str("Mode: Insert - Alter(NOT implemented)"), status_str("Welcome!"),
					help_counter(0), 
					rtree(2,5)
	{
        if( handles.empty() ) {
            handles.push_back( &handle_set );
        }
		Rect rect_chosen();
		Rect dummy_draw();
    }


};



int main(int argc, char **argv) {
	std::cout << "---------------------------------------------------------"<< std::endl;
    std::cout << "Let's play with the Red Black Tree! ONLY Insert works now!!!"<< std::endl;
    std::cout << " Key A: insert/alter mode                                   "<< std::endl;
    std::cout << " * Left click and drag on white area: create a rectangle"<< std::endl;
	std::cout << " *NOT READY: Left click and drag on handler: alter a rectangle"<< std::endl;
    std::cout << " Key B: search mode                                   "<< std::endl;
	std::cout << " * Left click and drag on white area: \"search\" for nodes that intersect red area"<< std::endl;
    std::cout << " NOT READY: Key C: delete mode                                   "<< std::endl;
	std::cout << " * Left click on handler: delete for a rectangle"<< std::endl;
	std::cout << "---------------------------------------------------------"<< std::endl;
    init(argc, argv, new RTreeToy);
    return 0;
}
	  	 
const char* RTreeToy::menu_items[] =
{
    "Insert / Alter Rectangle",
    "Search Rectangle",
    "Delete Reactangle"
};

const char RTreeToy::keys[] =
{
     'A', 'B', 'C'
};
