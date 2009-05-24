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
#include <vector>

//using std::vector;
using namespace Geom;
using namespace std;

// make sure that in RTreeToy() constructor you assign the same number of colors 
// otherwise, they extra will be black :P
const int no_of_colors = 8;
const string search_str = "Mode: Search";
const string insert_str = "Mode: Insert (Click whitespace and Drag) - Update (Click handle and Drag NOT implemented)" ;
const string erase_str = "Mode: Delete(NOT implemented)";
const string help_str = "'A': Insert/Update, 'B': Search, 'C': Delete";

class RTreeToy: public Toy 
{

    PointSetHandle handle_set;

	Geom::Point starting_point;		// during click and drag: start point of click
	Geom::Point ending_point;		// during click and drag: end point of click (release)
	Geom::Point highlight_point;	// not used

	// colors we are going to use for different purposes
	colour color_shape, color_shape_guide;
	colour color_select_area, color_select_area_guide;	// red(a=0.6), red


	int alter_existing_rect;
	int add_new_rect;

	Rect rect_chosen;	// the rectangle of the search area
	Rect dummy_draw;	// the "helper" rectangle that is shown during the click and drag (before the mouse release)

	// save the bounding boxes of the tree in here
	std::vector< std::vector< Rect > > rects_level;
	std::vector<colour> color_rtree_level;
	int drawBB_color;
	bool drawBB_color_all;

	int mode;			// insert/alter, search, delete  modes
	bool drawBB;			// draw bounding boxes of RTree
	string out_str, drawBB_str, drawBB_color_str;

	// printing of the tree
	//int help_counter;	// the "x" of the label of each node
	static const int label_size = 15 ; // size the label of each node

	Geom::RTree rtree;

	// used for the keys that switch between modes
    enum menu_item_t
    {
        INSERT_A = 0,
		DELETE,
		SEARCH,
		BB_TOGGLE_T,
		BB_DRAW_0,
		BB_DRAW_1,
		BB_DRAW_2,
		BB_DRAW_3,
		BB_DRAW_4,
		BB_DRAW_5,
		BB_DRAW_ALL_O,
        TOTAL_ITEMS // this one must be the last item
    };
    static const char* menu_items[TOTAL_ITEMS];
    static const char keys[TOTAL_ITEMS];



    void draw( cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream ) {
        cairo_set_line_width( cr, 1 );

		// draw the rects that we have in the handles
		for( unsigned i=0; i<handle_set.pts.size(); i=i+2 ){
	        Rect r1( handle_set.pts[ i ], handle_set.pts[ i+1 ] );
		    cairo_rectangle( cr, r1 );
		}
	    cairo_set_source_rgba( cr, color_shape );
		cairo_stroke( cr );

		// draw a rect if we click & drag (so that we know what we are going to create)
		if(add_new_rect){
			dummy_draw = Rect( starting_point, ending_point );
			cairo_rectangle( cr, dummy_draw );
			if( mode == 0){
				cairo_set_source_rgba( cr, color_shape_guide );
			}
			else if( mode == 1){
			    cairo_set_source_rgba( cr, color_select_area_guide );
			}
			cairo_stroke( cr );
		}

		// draw a rect for the search area
		cairo_rectangle( cr, rect_chosen );
	    cairo_set_source_rgba( cr, color_select_area );
		cairo_stroke( cr );
	
		*notify << help_str << std::endl 
			<< "'T': Bounding Boxes: " << drawBB_str << ", '0'-'" << no_of_colors << "', 'P': Show Layer: " << drawBB_color_str << std::endl 
			<< out_str;

		if( drawBB ){
			for(unsigned color=0; color < rects_level.size(); color++ ){
				if( drawBB_color == color || drawBB_color_all ){
					for(unsigned j=0; j < rects_level[color].size(); j++ ){
						cairo_rectangle( cr, rects_level[color][j] );
					}
					cairo_set_source_rgba( cr, color_rtree_level[color] );
					cairo_stroke( cr );
				}
			}
		}

		Toy::draw( cr, notify, width, height, save,timer_stream );
    }        
    
    void mouse_moved( GdkEventMotion* e ){
		if( !( alter_existing_rect && mode == 1 ) ){
			Toy::mouse_moved( e );
		}

		if(add_new_rect){
			ending_point = Point( e->x, e->y );
		}
	}

    void mouse_pressed( GdkEventButton* e ) {
		Toy::mouse_pressed( e );
		if(e->button == 1){		// left mouse button
			if( mode == 0 ){	// mode: insert / alter
				if(!selected) {
					starting_point = Point( e->x, e->y );
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
					starting_point = Point( e->x, e->y );
					ending_point = starting_point;
					add_new_rect = 1;
				}
				else{
					alter_existing_rect = 1;
				}
			}
			else if( mode == 2 ) {	// mode: delete
			}
		}
		else if( e->button == 2 ){	//middle button
		}
		else if( e->button == 3 ){	//right button
		}
    }

    virtual void mouse_released( GdkEventButton* e ) {
		Toy::mouse_released( e );
		if( e->button == 1 ) { 		//left mouse button
			if( mode == 0) {	// mode: insert / alter
				if( add_new_rect ){
					ending_point = Point( e->x, e->y );
					handle_set.push_back( starting_point );
					handle_set.push_back( ending_point );
					insert_in_tree_the_last_rect();
					find_rtree_subtrees_bounding_boxes( rtree );
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
					ending_point = Point( e->x, e->y );
					rect_chosen = Rect( starting_point, ending_point );

					std::vector< int > result(0);

					// search
					if( rtree.root ){
						rtree.search( rect_chosen, &result, rtree.root );
					}
					std::cout << "Search results: " << result.size() << std::endl;
					//Rect const &search_area, std::vector< int > result, const RTreeNode* subtree
					for(unsigned i = 0; i < result.size(); i++ ){
						std::cout << result[i] << ", " ;
					}
					std::cout << std::endl;

					add_new_rect = 0;
				}
				else if( alter_existing_rect ){ // do nothing					
					alter_existing_rect = 0;
				}
			}
			else if( mode == 2) {	// mode: delete

			}
		}
		else if( e->button == 2 ){	//middle button			
		}
		else if( e->button == 3 ){	//right button

		}
		//std::cout<< "Toy: end mouse_released" << std::endl;
    }


    void key_hit( GdkEventKey *e )
    {
        char choice = std::toupper( e->keyval );
        switch ( choice )
        {
            case 'A':
                mode = 0;
				out_str = insert_str;
                break;
            case 'B':
                mode = 1;
				out_str = search_str; 
                break;
            case 'C':
                mode = 2;
				out_str = erase_str;
                break;
			case 'T':
				
				if( drawBB ){
					drawBB = false;
					drawBB_str = "OFF";
				}
				else{
					drawBB = true;
					drawBB_str = "ON";
				}
				break;
            case 'P':
                drawBB_color_all = true;
				drawBB_color = 9;
				drawBB_color_str = "all";
                break;
            case '0':
                drawBB_color_all = false;
				drawBB_color = 0;
				drawBB_color_str = "0";
                break;
            case '1':
                drawBB_color_all = false;
				drawBB_color = 1;
				drawBB_color_str = "1";
                break;
            case '2':
                drawBB_color_all = false;
				drawBB_color = 2;
				drawBB_color_str = "2";
                break;
            case '3':
                drawBB_color_all = false;
				drawBB_color = 3;
				drawBB_color_str = "3";
                break;
            case '4':
                drawBB_color_all = false;
				drawBB_color = 4;
				drawBB_color_str = "4";
                break;
            case '5':
                drawBB_color_all = false;
				drawBB_color = 5;
				drawBB_color_str = "5";
                break;
            case '6':
                drawBB_color_all = false;
				drawBB_color = 6;
				drawBB_color_str = "6";
                break;
            case '7':
                drawBB_color_all = false;
				drawBB_color = 7;
				drawBB_color_str = "7";
                break;
        }
        redraw();
    }


	void insert_in_tree_the_last_rect(){
			//std::cout<< "Toy: insert_in_tree_the_last_rect" << std::endl;
			unsigned i = handle_set.pts.size() - 2;
		    Rect r1( handle_set.pts[ i ], handle_set.pts[ i+1 ] );
			// insert in R tree
			rtree.insert( r1, i );
			std::cout << " \nTree:\n" << std::endl;
			rtree.print_tree( rtree.root, 0 );	
			std::cout << "...done\n" << std::endl;
	};


	void find_rtree_subtrees_bounding_boxes( Geom::RTree tree ){
		//std::cout<< "Toy: find_rtree_subtrees_bounding_boxes: " << rects_level.size() << std::endl;
		if( tree.root ){
			// clear existing bounding boxes 
			for(unsigned color=0; color < rects_level.size(); color++ ){
				//std::cout<< "color:" << color << std::endl;
				rects_level[color].clear();
			}
			save_bb( tree.root, 0);
		}
	};

	// TODO fix this.
	void save_bb( Geom::RTreeNode* subtree_root, int depth ) 
	{	
		//std::cout<< "Toy: save_bb: " << rects_level.size() << std::endl;
		if( subtree_root->children_nodes.size() > 0 ){ 

			// descend in each one of the elements and call print_tree
			for( unsigned i=0; i < subtree_root->children_nodes.size(); i++ ){
				//std::cout<< "Toy: save_bb: save: " << depth ;
			    Rect r1( subtree_root->children_nodes[ i ].bounding_box );
				rects_level[ depth ].push_back( r1 );

				if( depth == no_of_colors - 1 ){	// if we reached Nth levels of colors, roll back to color 0
					//std::cout<< "   descend in: reset color: 0" << std::endl;
					save_bb( subtree_root->children_nodes[ i ].data, 0);
				}
				else{
					//std::cout<< "   descend in: " << depth+1 << std::endl;
				    save_bb( subtree_root->children_nodes[ i ].data, depth+1);
				}
			}
		}
		// else{} do nothing, Leave  entries are the rects themselves...
	};



public:
    RTreeToy(unsigned rmax, unsigned rmin, char handlefile ):
	 	color_shape(0, 0, 0, 1), color_shape_guide(0, 1, 0, 1),
		color_select_area(1, 0, 0, 0.9 ),  color_select_area_guide(0, 1, 0, 1 ), //1, 0, 0, 1
		//alter_existing_rect(0), add_new_rect(0), 
		rect_chosen(), dummy_draw(),
		rects_level( no_of_colors ),
		color_rtree_level( no_of_colors, colour(0, 0, 0, 0) ),
		drawBB_color(9), drawBB_color_all(true),
		mode(0), drawBB(true),
		//help_str(),
		out_str( insert_str ), 
		drawBB_str("ON"), drawBB_color_str("all"),
		rtree( rmin, rmax, QUADRATIC_SPIT )
	{
		std::cout<< "INIT\n";
        if( handles.empty() ) {
            handles.push_back( &handle_set );
        }	
		color_rtree_level[0] = colour(0, 0.58, 1, 1);
		color_rtree_level[1] = colour(0, 0.45, 0, 1);
		color_rtree_level[2] = colour(0.53, 0, 0.66, 1);
		color_rtree_level[3] = colour(0, 0, 1, 1);
		color_rtree_level[4] = colour(1, 0.62, 0, 1);
		color_rtree_level[5] = colour(1, 0, 0.8, 1);
		color_rtree_level[6] = colour(0.47, 0.26, 0.12, 1);
		color_rtree_level[7] = colour(0, 0, 0.5, 1);


    }

	void first_time(int argc, char** argv){
		std::cout << "argc = " << argc << std::endl;
		for(int i = 0; i < argc; i++){
			std::cout << "argv[" << i << "] = " << argv[i] << std::endl; 	
		}
	}
};



int main(int argc, char **argv) {
	std::cout << "---------------------------------------------------------"<< std::endl;
    std::cout << "Let's play with the R- Tree! ONLY Insert works now!!!"<< std::endl;
    std::cout << " Key A: insert/alter mode                                   "<< std::endl;
    std::cout << " * Left click and drag on white area: create a rectangle"<< std::endl;
	std::cout << " *NOT READY: Left click and drag on handler: alter a rectangle"<< std::endl;
    std::cout << " Key B: search mode                                   "<< std::endl;
	std::cout << " * Left click and drag on white area: \"search\" for nodes that intersect red area"<< std::endl;
    std::cout << " NOT READY: Key C: delete mode                                   "<< std::endl;
	std::cout << " * Left click on handler: delete for a rectangle"<< std::endl;
	std::cout << "---------------------------------------------------------"<< std::endl;
	// rtree max nodes: -rmax
	// rtree min nodes: -rmin
	unsigned int rmax = 3;
	unsigned int rmin = 2;
	// handle filename: -f
	char handlefile = 'T';

    init(argc, argv, new RTreeToy(rmax, rmin, handlefile) );

    return 0;
}
	  	 
const char* RTreeToy::menu_items[] =
{
    "Insert / Alter Rectangle",
    "Search Rectangle",
    "Delete Reactangle",
    "Toggle"
};

const char RTreeToy::keys[] =
{
     'A', 'B', 'C', 'T', 
	'0', '1', '2', '3', '4', '5', 'P'
};


/*		
intersection test
		Rect r1( Point(100, 100), Point(150, 150)),
				r2( Point(200, 200), Point(250, 250)),
				r3( Point(50, 50), Point(100, 100));
		OptRect a_intersection_b;
		a_intersection_b = intersect( r1, r2 );
		std::cout << "r1, r2  " << a_intersection_b.isEmpty() << std::endl;
		a_intersection_b = intersect( r1, r3 );
		std::cout << "r1, r3  " << a_intersection_b.isEmpty() << std::endl;
*/
