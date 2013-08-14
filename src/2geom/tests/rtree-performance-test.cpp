/*
 * Copyright 2010  Evangelos Katsikaros <vkatsikaros at yahoo dot gr>
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

#include <2geom/toys/toy-framework-2.h>

#include <sstream>
#include <getopt.h>

#include <SpatialIndex.h>
#include <glib.h>
//#include <glib/gtypes.h>

using namespace Geom;

// cmd argument stuff
char* arg_area_limit = NULL;
bool arg_area_limit_set = false;
bool arg_debug = false;

int limit = 0;

// spatial index ID management
SpatialIndex::id_type indexID;

// list of rectangles
GList *items = NULL;

// tree of rectangles
SpatialIndex::ISpatialIndex *tree;

SpatialIndex::id_type test_indexID;

void add_rectangle( int x, int y );

/* Simple Visitor used to search the tree. When Data is encountered
 * we are supposed to call the render function of the Data
 * */
class SearchVisitor : public SpatialIndex::IVisitor {
public:
        
    void visitNode(const SpatialIndex::INode& n){
    }
            
    void visitData(const SpatialIndex::IData& d){
        /* this prototype: do nothing
         * otherwise, render on buffer
         * */
    }
        
    void visitData(std::vector<const SpatialIndex::IData*>& v) {
    }
};


/* we use the this visitor after each insertion in the tree
 * The purpose is to validate that everything was stored properly
 * and test the GList pointer storage. It has no other functional
 * purpose.
 * */
class TestSearchVisitor : public SpatialIndex::IVisitor {
public:
        
    void visitNode(const SpatialIndex::INode& n) {
    }
        
    void visitData(const SpatialIndex::IData& d){
        if( test_indexID == d.getIdentifier() ){
            byte* pData = 0;
            uint32_t cLen = sizeof(GList*);
            d.getData(cLen, &pData);
            //do something...
            GList* gl = reinterpret_cast<GList*>(pData);
            Geom::Rect *member_data = (Geom::Rect *)gl->data;
            double lala = member_data->bottom();            
            std::cout << "   Tree: " << lala << std::endl;
            
            delete[] pData;           
        }
    }
        
    void visitData(std::vector<const SpatialIndex::IData*>& v) {
    }
};


int main(int argc, char **argv) {
   
    int c;

    //--------------------------------------------------------------------------
    // read cmd options
    while (1) {
        static struct option long_options[] =
            {
                /* These options set a flag. */
                /* These options don't set a flag.
                   We distinguish them by their indices. */
                {"area-limit",	required_argument,	0, 'l'},
                {"help",		no_argument,		0, 'h'},
                {"debug",	no_argument,	0, 'd'},
                {0, 0, 0, 0}
            };
        /* getopt_long stores the option index here. */
        int option_index = 0;
    
        c = getopt_long (argc, argv, "l:h:d",
                         long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1){
            break;
        }
    
        switch (c)
        {
            case 'l':
                arg_area_limit = optarg;
                arg_area_limit_set = true;
                break;
            case 'h':
                std::cerr << "Usage:  " << argv[0] << " options\n" << std::endl ;
                std::cerr << 
                    "  -l  --area-limit=NUMBER  minimum number in node.\n" <<
                    "  -d  --debug              Enable debug info (list/tree related).\n" <<
                    "  -h  --help               Print this help.\n" << std::endl;
                exit(1);
                break;
            case 'd':
                arg_debug = true;
                break;                
            case '?':
                /* getopt_long already printed an error message. */
                break;
        
            default:
                abort ();
        }
    }

    // use some of the cmd options
    if(  arg_area_limit_set  ) {
        std::stringstream s1( arg_area_limit );
        s1 >> limit;
    }
    else {
        limit = 100;
    }
    // end cmd options
    //--------------------------------------------------------------------------


    double plow[2], phigh[2];
    // spatial index memory storage manager
    SpatialIndex::IStorageManager *mem_mngr;
    // initialize spatial indexing stuff
    mem_mngr = SpatialIndex::StorageManager::createNewMemoryStorageManager();
    // fillFactor, indexCapacity, leafCapacity, dimensionality=2, variant=R*, indexIdentifier
    tree = SpatialIndex::RTree::createNewRTree(*mem_mngr, 0.7, 25, 25, 2, SpatialIndex::RTree::RV_RSTAR, indexID);    

    //-------------------------------------------
    /* generate items. add_rectangle() stores them in both list and tree
     * add rect every (20, 20).
     * In area ((0,0), (1000, 1000)) add every (100,100)
     * */
    for( int x_coord = -limit; x_coord <= limit; x_coord += 20 ) {
        for( int y_coord = -limit; y_coord <= limit; y_coord += 20 ) {
            if( x_coord >= 0 && x_coord <= 1000 &&
                y_coord >= 0 && y_coord <= 1000 )
            {
                if( x_coord % 100 == 0 && y_coord % 100 == 0) {
                    add_rectangle( x_coord, y_coord );
                }
                else{
                    add_rectangle( x_coord, y_coord );
                }
            }
            else{
                add_rectangle( x_coord, y_coord );
            }
        
        }
    }
    std::cout << "Area of objects: ( -" << limit
              << ", -" << limit
              << " ), ( " << limit
              << ", " << limit
              << " )" << std::endl;
    std::cout << "Number of Objects (indexID): " << indexID << std::endl;
    // std::cout << "GListElements: " << g_list_length << std::endl;
   
    //-------------------------------------------
    // Traverse list
    Geom::Point sa_start = Point( 0, 0 );
    Geom::Point sa_end = Point( 1000, 1000 );
    Geom::Rect search_area = Rect( sa_start, sa_end );

    Timer list_timer;
    list_timer.ask_for_timeslice();
    list_timer.start();
    
    for (GList *list = items; list; list = list->next) {
        Geom::Rect *child = (Geom::Rect *)list->data;
        if ( search_area.intersects( *child ) )
        {
            /* this prototype: do nothing
             * otherwise, render on buffer
             * */
        }          
    }  
    Timer::Time the_list_time = list_timer.lap();
    
    std::cout << std::endl;
    std::cout << "GList (full scan): " << the_list_time << std::endl;
    
    //-------------------------------------------
    // Search tree - good case
    Timer tree_timer;
    tree_timer.ask_for_timeslice();
    tree_timer.start();

    /* We search only the (0,0), (1000, 1000) where the items are less dense.
     * We expect a good performance versus the list
     * */
    // TODO IMPORTANT !!! check the plow, phigh
    // plow[0] = x1; plow[1] = y1;
    // phigh[0] = x2; phigh[1] = y2;

    plow[0] = 0;
    plow[1] = 0;
    phigh[0] = 1000;
    phigh[1] = 1000;
    
    SpatialIndex::Region search_region = SpatialIndex::Region(plow, phigh, 2);
    SearchVisitor vis = SearchVisitor();
    tree->intersectsWithQuery( search_region, vis );
    
    Timer::Time the_tree_time = tree_timer.lap();
    std::cout << "Rtree (good): " << the_tree_time << std::endl;


    //-------------------------------------------
    // Search tree - worst case
    Timer tree_timer_2;
    tree_timer_2.ask_for_timeslice();
    tree_timer_2.start();

    /* search the whole area, so all items are returned */
    plow[0] = -limit - 100;
    plow[1] = -limit - 100;
    phigh[0] = limit + 100;
    phigh[1] = limit + 100;
    
    SpatialIndex::Region search_region_2 = SpatialIndex::Region(plow, phigh, 2);
    SearchVisitor vis_2 = SearchVisitor();
    tree->intersectsWithQuery( search_region_2, vis_2 );
    
    Timer::Time the_tree_time_2 = tree_timer_2.lap();
    std::cout << "Rtree (full scan): " << the_tree_time_2 << std::endl;
    
    return 0;
}



/* Adds rectangles in a GList and a SpatialIndex rtree
 * */
void add_rectangle( int x, int y ) {
    
    Geom::Point starting_point = Point( x, y );
    Geom::Point ending_point = Point( x + 10, y + 10 );
    Geom::Rect rect_to_add = Rect( starting_point, ending_point );
    items = g_list_append( items, &rect_to_add );

    if( arg_debug ) {
        // fetch the last rect from the list
        Geom::Rect *member_data = (Geom::Rect *)( g_list_last( items ) )->data;
        double lala = member_data->bottom();
        std::cout << "List (" << indexID << "): "  << lala;
    }

    /* Create a SpatialIndex region
     * plow = left-bottom corner
     * phigh = top-right corner
     * [0] = dimension X
     * [1] = dimension Y
     * */    
    double plow[2], phigh[2];    

    plow[0] = rect_to_add.left() ;
    plow[1] = rect_to_add.bottom();    
    phigh[0] = rect_to_add.right();
    phigh[1] = rect_to_add.top();
    
    SpatialIndex::Region r = SpatialIndex::Region(plow, phigh, 2);
    /* Store Glist pointer size and GList pointer as the associated data
     * In inkscape this can be used to directly call render hooked functions
     * from SPCanvasItems
     * */
    tree->insertData( sizeof(GList*), reinterpret_cast<const byte*>( g_list_last( items ) ), r, indexID);

    // tree->insertData(0, 0, r, indexID);
    /* not used. Store zero size and a null pointer as the associated data.
     * indexId is used to retrieve from a mapping each rect
     * (example a hash map, or the indexID is also vector index)
     * */

    if( arg_debug ) {
        test_indexID = indexID;
        /* every time we add a rect, search all the tree to find the last
         * inserted ID. This is not performance-wise good (rtree only good for
         * spatial queries) this is just used for debugging reasons
         * */
        plow[0] = -limit - 100;
        plow[1] = -limit - 100;
        phigh[0] = limit + 100;
        phigh[1] = limit + 100;
    
        SpatialIndex::Region test_search_region = SpatialIndex::Region(plow, phigh, 2);
        TestSearchVisitor test_vis = TestSearchVisitor();
        // search the tree for the region. Visitor implements the search function hooks
        tree->intersectsWithQuery( test_search_region, test_vis );
    }

    indexID++;
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +)(c-basic-offset . 4))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
