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


#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#include <time.h>
#include <vector>
#include <sstream>
#include <getopt.h>

#include <2geom/orphan-code/rtree.h>
#include <2geom/orphan-code/rtree.cpp>


//using std::vector;
using namespace Geom;
using namespace std;

// make sure that in RTreeToy() constructor you assign the same number of colors 
// otherwise, they extra will be black :P
const int no_of_colors = 8;
const string search_str = "Mode: Search (Area: Click whitespace and Drag)";
const string update_str = "Mode: Update (Click Bounding Box (dark gray) and Drag) NOT implemented";
const string insert_str = "Mode: Insert (Click whitespace and Drag)" ;
const string erase_str = "Mode: Delete (Click on Bounding Box (dark gray))";
const string help_str = "'I': Insert, 'U': Update, 'S': Search, 'D': Delete";


const char* program_name;

class RTreeToy: public Toy 
{

//    PointSetHandle handle_set;
//	std::vector< Handle > rectangle_handles;
	std::vector< RectHandle > rectangles;

	Geom::Point starting_point;		// during click and drag: start point of click
	Geom::Point ending_point;		// during click and drag: end point of click (release)
	Geom::Point highlight_point;	// not used

	// colors we are going to use for different purposes
	colour color_shape, color_shape_guide;
	colour color_select_area, color_select_area_guide;	// red(a=0.6), red


	bool alter_existing_rect;
	bool add_new_rect;
	bool delete_rect;

	Rect rect_chosen;	// the rectangle of the search area
	Rect dummy_draw;	// the "helper" rectangle that is shown during the click and drag (before the mouse release)

	// save the bounding boxes of the tree in here
	std::vector< std::vector< Rect > > rects_level;
	std::vector<colour> color_rtree_level;
	unsigned drawBB_color;
	bool drawBB_color_all;

	enum the_modes { 
        INSERT_MODE = 0,
		UPDATE_MODE,
		DELETE_MODE,
		SEARCH_MODE,
	} mode;			// insert/alter, search, delete  modes
	bool drawBB;			// draw bounding boxes of RTree
	string out_str, drawBB_str, drawBB_color_str;

	// printing of the tree
	//int help_counter;	// the "x" of the label of each node
	static const int label_size = 15 ; // size the label of each node

	Geom::RTree rtree;

	void * hit;
	unsigned rect_id;


	// used for the keys that switch between modes
    enum menu_item_t
    {
        INSERT_I = 0,
		UPDATE_U,
		DELETE_D,
		SEARCH_S,
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
	    cairo_set_source_rgba( cr, color_shape );
		cairo_stroke( cr );

		// draw the shapes we save in the rtree
		for( unsigned i = 0; i < rectangles.size(); i++ ){
	        rectangles[i].draw( cr, true );
		}
	    cairo_set_source_rgba( cr, color_shape );
		cairo_stroke( cr );

		// draw a rect if we did click & drag (so that we know what we are going to create)
		if( add_new_rect ){
			dummy_draw = Rect( starting_point, ending_point );
			cairo_rectangle( cr, dummy_draw );
			if( mode == INSERT_MODE ){
				cairo_set_source_rgba( cr, color_shape_guide );
			}
			else if( mode == SEARCH_MODE ){
			    cairo_set_source_rgba( cr, color_select_area_guide );
			}
			cairo_stroke( cr );
		}

		// draw a rect for the search area
		cairo_rectangle( cr, rect_chosen );
	    cairo_set_source_rgba( cr, color_select_area );
		cairo_stroke( cr );
	
		*notify << help_str << "\n"
			<< "'T': Bounding Boxes: " << drawBB_str << ", '0'-'" << no_of_colors << "', 'P': Show Layer: " << drawBB_color_str << "\n"
			<< out_str;

		if( drawBB ){
			for(unsigned color = 0 ; color < rects_level.size() ; color++ ){
				if( drawBB_color == color || drawBB_color_all ){
					for(unsigned j = 0 ; j < rects_level[color].size() ; j++ ){
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
		if(add_new_rect &&
			( mode == INSERT_MODE || mode == SEARCH_MODE ) )
		{
			Toy::mouse_moved( e );
			ending_point = Point( e->x, e->y );
		}
	}

    void mouse_pressed( GdkEventButton* e ) {
		Toy::mouse_pressed( e );
		if(e->button == 1){		// left mouse button
			if( mode == INSERT_MODE ){
				starting_point = Point( e->x, e->y );
				ending_point = starting_point;
				add_new_rect = true;
			}
			else if( mode == SEARCH_MODE ){	
				starting_point = Point( e->x, e->y );
				ending_point = starting_point;
				add_new_rect = true;
			}
			else if( mode == DELETE_MODE ) {
				Geom::Point mouse(e->x, e->y);
				unsigned i = 0;
				for( i = 0; i < rectangles.size(); i++) {
					hit = rectangles[i].hit(mouse);
					if( hit ) {
						break;
					}
				}
				if( hit ){
					// erase specific element
					stringstream shape_id( rectangles[i].name );
					unsigned shape_id_int;
					shape_id >> shape_id_int;

					rtree.erase( rectangles[i].pos, shape_id_int );
					rectangles.erase( rectangles.begin() + i );
//					check_if_deleted( );
//					check_if_duplicates( );
					delete_rect = true;
				}
				hit = NULL;
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
			if( mode == INSERT_MODE ) {	
				if( add_new_rect ){
					ending_point = Point( e->x, e->y );
					RectHandle t( Rect(starting_point, ending_point), false );

					std::stringstream out;
					out << rect_id;;
					t.name = out.str();
					rectangles.push_back( t );
					rect_id++;

					insert_in_tree_the_last_rect();
					find_rtree_subtrees_bounding_boxes( rtree );
					add_new_rect = false;
				}
			}
			else if( mode == SEARCH_MODE ){	
				if( add_new_rect ){
					ending_point = Point( e->x, e->y );
					rect_chosen = Rect( starting_point, ending_point );

					std::vector< int > result(0);

					if( rtree.root ){
						rtree.search( rect_chosen, &result, rtree.root );
					}
					std::cout << "Search results: " << result.size() << std::endl;
					for(unsigned i = 0; i < result.size(); i++ ){
						std::cout << result[i] << ", " ;
					}
					std::cout << std::endl;

					add_new_rect = false;
				}
			}
			else if( mode == DELETE_MODE ) {	// mode: delete
				if( delete_rect ){
					delete_rect = false;
					if( rtree.root ){
						find_rtree_subtrees_bounding_boxes( rtree );
					}
					std::cout << " \nTree:\n" << std::endl;
					rtree.print_tree( rtree.root, 0 );	
					std::cout << "...done\n" << std::endl;
				}
			}
		}
		else if( e->button == 2 ){	//middle button			
		}
		else if( e->button == 3 ){	//right button
		}
    }


    void key_hit( GdkEventKey *e )
    {
        char choice = std::toupper( e->keyval );
        switch ( choice )
        {
            case 'I':
                mode = INSERT_MODE;
				out_str = insert_str;
                break;
            case 'S':
                mode = SEARCH_MODE;
				out_str = search_str; 
                break;
            case 'D':
                mode = DELETE_MODE;
				out_str = erase_str;
                break;
            case 'U':
                mode = UPDATE_MODE;
				out_str = update_str;
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
			unsigned i = rectangles.size() - 1;
			Rect r1 = rectangles[i].pos;

			stringstream shape_id( rectangles[i].name );
			unsigned shape_id_int;
			shape_id >> shape_id_int;

			// insert in R tree
			rtree.insert( r1, shape_id_int );
			std::cout << " \nTree:\n" << std::endl;
			rtree.print_tree( rtree.root, 0 );	
			std::cout << "...done\n" << std::endl;
	};


	void find_rtree_subtrees_bounding_boxes( Geom::RTree tree ){
		if( tree.root ){
			// clear existing bounding boxes 
			for(unsigned color=0; color < rects_level.size(); color++ ){
				rects_level[color].clear();
			}
			save_bb( tree.root, 0);
		}
	};

	// TODO fix this.
	void save_bb( Geom::RTreeNode* subtree_root, int depth ) 
	{	
		if( subtree_root->children_nodes.size() > 0 ){ 

			// descend in each one of the elements and call print_tree
			for( unsigned i=0; i < subtree_root->children_nodes.size(); i++ ){
			    Rect r1( subtree_root->children_nodes[ i ].bounding_box );
				rects_level[ depth ].push_back( r1 );

				if( depth == no_of_colors - 1 ){	// if we reached Nth levels of colors, roll back to color 0
					save_bb( subtree_root->children_nodes[ i ].data, 0);
				}
				else{
				    save_bb( subtree_root->children_nodes[ i ].data, depth+1);
				}
			}
		}
		// else do nothing, entries are the rects themselves...
	};



public:
    RTreeToy(unsigned rmin, unsigned rmax, char /*handlefile*/):
		rectangles(0),
	 	color_shape(0, 0, 0, 0.9), color_shape_guide(1, 0, 0, 1),
		color_select_area(1, 0, 0, 0.6 ),  color_select_area_guide(1, 0, 0, 1 ), //1, 0, 0, 1
		alter_existing_rect( false ), add_new_rect( false ), delete_rect( false ), 
		rect_chosen(), dummy_draw(),
		rects_level( no_of_colors ),
		color_rtree_level( no_of_colors, colour(0, 0, 0, 0) ),
		drawBB_color(9), drawBB_color_all(true),
		mode( INSERT_MODE ), drawBB(true),
		out_str( insert_str ), 
		drawBB_str("ON"), drawBB_color_str("all"),
		rtree( rmin, rmax, QUADRATIC_SPIT ),
		hit( 0 ), rect_id( 0 )
	{
		// only "bright" colors
		color_rtree_level[0] = colour(0, 0.80, 1, 1); 		// cyan
		color_rtree_level[1] = colour(0, 0.85, 0, 1);		// green
		color_rtree_level[2] = colour(0.75, 0, 0.75, 1);	// purple
		color_rtree_level[3] = colour(0, 0, 1, 1);			// blue
		color_rtree_level[4] = colour(1, 0.62, 0, 1);		// orange
		color_rtree_level[5] = colour(1, 0, 0.8, 1);		// pink
		color_rtree_level[6] = colour(0.47, 0.26, 0.12, 1);
		color_rtree_level[7] = colour(1, 0.90, 0, 1);		// yellow
    }

};



int main(int argc, char **argv) {

	char* min_arg = NULL;
	char* max_arg = NULL;

	int set_min_max = 0;

	int c;

	while (1)
	{
		static struct option long_options[] =
		{
			/* These options set a flag. */
			/* These options don't set a flag.
			We distinguish them by their indices. */
			{"min-nodes",	required_argument,	0, 'n'},
			{"max-nodes",	required_argument,	0, 'm'},
			{"help",		no_argument,		0, 'h'},
			{0, 0, 0, 0}
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "n:m:h",
			long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1){
			break;
		}

		switch (c)
		{
			case 'n':
			min_arg = optarg;
			set_min_max += 1;
			break;


			case 'm':
			max_arg = optarg;	
			set_min_max += 2;
			break;


			case 'h':
			std::cerr << "Usage:  " << argv[0] << " options\n" << std::endl ;
			std::cerr << 
				   "  -n  --min-nodes=NUMBER   minimum number in node.\n" <<
				   "  -m  --max-nodes=NUMBER   maximum number in node.\n" <<
				   "  -h  --help               Print this help.\n" << std::endl;
			exit(1);
			break;


			case '?':
			/* getopt_long already printed an error message. */
			break;

			default:
			abort ();
		}
	}

	unsigned rmin = 0;
	unsigned rmax = 0;

	if(	set_min_max == 3 ){
		stringstream s1( min_arg );
		s1 >> rmin;

		stringstream s2( max_arg );
		s2 >> rmax;
		if( rmax <= rmin || rmax < 2 || rmin < 1 ){
			std::cerr << "Rtree set to 2, 3" << std::endl ;
			rmin = 2;
			rmax = 3;			
		}
	}
	else{
		std::cerr << "Rtree set to 2, 3 ." << std::endl ;
		rmin = 2;
		rmax = 3;
	}


	char handlefile = 'T';
	std::cout << "rmin: " << rmin << "  rmax:" << rmax << std::endl;
    init(argc, argv, new RTreeToy( rmin, rmax, handlefile) );

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
	'I', 'U', 'S', 'D', 'T', 
	'0', '1', '2', '3', '4', '5', 'P'
};






/*		
intersection test
		Rect r1( Point(100, 100), Point(150, 150)),
				r2( Point(200, 200), Point(250, 250)),
				r3( Point(50, 50), Point(100, 100));
		OptRect a_intersection_b;
		a_intersection_b = intersect( r1, r2 );
		std::cout << "r1, r2  " << a_intersection_b.empty() << std::endl;
		a_intersection_b = intersect( r1, r3 );
		std::cout << "r1, r3  " << a_intersection_b.empty() << std::endl;
*/
