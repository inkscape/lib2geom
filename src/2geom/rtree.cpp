#include <2geom/rtree.h>
#include <limits>

/*
Based on source (BibTex):
@inproceedings{DBLP:conf/sigmod/Guttman84,
  author    = {Antonin Guttman},
  title     = {R-Trees: A Dynamic Index Structure for Spatial Searching},
  booktitle = {SIGMOD Conference},
  year      = {1984},
  pages     = {47-57},
  ee        = {http://doi.acm.org/10.1145/602259.602266, db/conf/sigmod/Guttman84.html},
}
*/


#define _RTREE_PRINT(x) std::cout << x << std::endl;
//comment the following if you want output during RedBlack Tree operations
//#define _RTREE_PRINT(x) ; 


namespace Geom{

/*=============================================================================
                            insert
===============================================================================
insert a new index entry E into the R-tree:

I1) find position of new record:
    choose_leaf will find a leaf node L (position) in which to place r
I2) add record to leaf node:
    if L has room for another entry install E
    else split_node will obtain L and LL containing E and all the old entries of L
    from the available splitting strategies we chose quadtratic-cost algorithm (just to begin 
    with sth)
    // TODO implement more of them AND TODO make their usage parameter in tree
I3) propagate changes upward:
    Invoke adjust_tree on L, also passing LL if a split was performed.
I4) grow tree taller:
    // TODO
*/

void RTree::insert( Rect const &r, int shape ){
    _RTREE_PRINT("\n=====================================");
    _RTREE_PRINT("insert");
    insert( r, shape, min_nodes, max_nodes );
}



void RTree::insert( Rect const &r, int shape, unsigned min_nodes, unsigned max_nodes ){
    _RTREE_PRINT("insert private");
    RTreeNode *position = 0;

    // if tree is unsued create the root Node, not described in source, stupid me :P
    if(root == 0){
        root = new RTreeNode();
    }

    
    _RTREE_PRINT("I1");     // I1
    position = choose_leaf( r );


    //bool split_performed = false;
    std::pair<RTreeNode, RTreeNode> splitted_groups;

    // position IS a leaf node (choose_leaf)
    if( position->children_leaves.size() < max_nodes ){
        _RTREE_PRINT("I2 no split: " << position->children_leaves.size() );     // I2
        position->children_leaves.push_back( std::make_pair( r, shape) );
    }
    else{
        _RTREE_PRINT("I2 split: " << position->children_leaves.size() );     // I2
        splitted_groups = quadratic_split( position, min_nodes );

        _RTREE_PRINT("I3");    // I3
        bool root_split_performed = adjust_tree( position, splitted_groups, /*split_performed,*/ min_nodes, max_nodes);
        _RTREE_PRINT("root split: " << root_split_performed);


        _RTREE_PRINT("I4");    // I4
        // TODO chick this, it was done late at night :P
        if( root_split_performed ){
            RTreeNode* new_root = new RTreeNode();
            
            if( splitted_groups.first.children_nodes.size() > 0 ){
                _RTREE_PRINT("  non leaf");     // non leaf node: splitted nodes 

                Rect bounding_box_a(splitted_groups.first.children_nodes[0].first );
                for(unsigned i=1; i < splitted_groups.first.children_nodes.size(); i++){
                    bounding_box_a.unionWith( splitted_groups.first.children_nodes[i].first  );
                }
                std::pair<Rect, RTreeNode*> new_entry_a =  std::make_pair( bounding_box_a, &splitted_groups.first );
                // new root is certainly non-leaf node (since splitted nodes are also non-leaves)
                new_root->children_nodes.push_back( new_entry_a );

                Rect bounding_box_b(splitted_groups.second.children_nodes[0].first );
                for(unsigned i=1; i < splitted_groups.second.children_nodes.size(); i++){
                    bounding_box_b.unionWith( splitted_groups.second.children_nodes[i].first  );
                }
                std::pair<Rect, RTreeNode*> new_entry_b =  std::make_pair( bounding_box_b, &splitted_groups.second );
                new_root->children_nodes.push_back( new_entry_b );
            }
            else{
                _RTREE_PRINT("  leaf");     // leaf node: splitted nodes 

                Rect bounding_box_a(splitted_groups.first.children_leaves[0].first );
                for(unsigned i=1; i < splitted_groups.first.children_leaves.size(); i++){
                    bounding_box_a.unionWith( splitted_groups.first.children_leaves[i].first  );
                }
                std::pair<Rect, RTreeNode*> new_entry_a =  std::make_pair( bounding_box_a, &splitted_groups.first );
                // new root will now be a non-leaf (because splitted were leaves)
                new_root->children_nodes.push_back( new_entry_a );

                Rect bounding_box_b(splitted_groups.second.children_leaves[0].first );
                for(unsigned i=1; i < splitted_groups.second.children_leaves.size(); i++){
                    bounding_box_b.unionWith( splitted_groups.second.children_leaves[i].first  );
                }
                std::pair<Rect, RTreeNode*> new_entry_b =  std::make_pair( bounding_box_b, &splitted_groups.second );
                // new root will now not be a leaf
                new_root->children_nodes.push_back( new_entry_b );
                
            }
            root = new_root; 
        }
    }

    _RTREE_PRINT("done");
}

/* I1 =========================================================================

choose_leaf will find a leaf node L in which to place r

1) Initialize: set N to be the root node
2) Leaf Check: if N is leaf return N
3) Choose subtree: If N not leaf then   
    let F be an entry in N whose rect Fi needs least enlargement to incude r
    ties resolved with rect of smallest area
4) descend until a leaf is reached: set N to the child node pointed to by F and goto 2.
*/
RTreeNode* RTree::choose_leaf( Rect const &r ){

    _RTREE_PRINT("  CL1");// CL1
    RTreeNode *pos = root;

    double min_enlargement;
    double current_enlargement;
    int node_min_enlargement;

    _RTREE_PRINT("  CL2");    // CL2 Leaf check
    while( pos->children_nodes.size() != 0 ){
        _RTREE_PRINT("  CL3");    // CL3
        min_enlargement = std::numeric_limits<double>::max();
        current_enlargement = 0;
        node_min_enlargement = 0;

        for(unsigned i=0; i< pos->children_nodes.size(); i++){
            current_enlargement = find_enlargement( pos->children_nodes[i].first, r );
            // TODO tie not solved!
            if( current_enlargement < min_enlargement ){
                node_min_enlargement = i;
                min_enlargement = current_enlargement;
            }
        }
        _RTREE_PRINT("  CL4");    // CL4
        // descend to the node with the min_enlargement
        pos = pos->children_nodes[node_min_enlargement].second; 
    }
    
    return pos;
}

/* 
find_enlargement:
// TODO check carefully
enlargement that "a" needs in order to incude "b"
usually 
b is the new rect we want to insert.
a is the rect of the node we try to see if b should go in.
*/
// union_rect.area() - a.area() - b.area();  old *stupid* version
double RTree::find_enlargement( Rect const &a, Rect const &b ){
    _RTREE_PRINT("      find_enlargement");
    Rect union_rect(a);
    union_rect.unionWith(b);
    return union_rect.area() - b.area();
}

/* I2 =========================================================================

Quadratic Split
QS1) Pick first entry for each group:
    Appy pick_seeds to choose 2 entries to be the first elements of the groups. Assign each one of 
    them to one group
QS2) check if done:
    a) if all entries have been assinged stop
    b) if one group has so few entries that all the rest must be assignmed to it, in order for it to 
    have the min number , assign them and stop
QS3) select entry and assign:
    Inkvoke pick_next() to choose the next entry to assign. 
    *[in pick_next] Add it to the group whose covering rectangle will have to be enlrarged least to 
    accomodate it. Resolve ties by adding the entry to the group with the smaller are, then to the 
    one with fewer entries, then to either of the two.
    goto 2.
*/
std::pair<RTreeNode, RTreeNode> RTree::quadratic_split( RTreeNode *s, unsigned min_nodes ){

    // s is the original leaf node or non-leaf node
    RTreeNode group_a; // a is the 1st group
    RTreeNode group_b; // b is the 2nd group


    _RTREE_PRINT("  QS1");     // QS1
    std::pair<unsigned, unsigned> initial_seeds;
    initial_seeds = pick_seeds(s);

    // if non-leaf node: s
    if( s->children_nodes.size() > 0 ){
        _RTREE_PRINT("  non-leaf node");         
        // each element is true if the node has been assinged to either "a" or "b"
        std::vector<bool> assigned_v( s->children_nodes.size() );
        std::fill( assigned_v.begin(), assigned_v.end(), false );

        group_a.children_nodes.push_back( s->children_nodes[initial_seeds.first] );
        assigned_v[ initial_seeds.first ] = true;

        group_b.children_nodes.push_back( s->children_nodes[initial_seeds.second] );
        assigned_v[ initial_seeds.second ] = true;

        _RTREE_PRINT("  QS2");     // QS2 
        std::vector< std::pair<Rect, RTreeNode*> >::iterator it;
        unsigned num_of_not_assigned = s->children_nodes.size() - 2; // so far we have assinged 2 out of all

        while( num_of_not_assigned ){// QS2 a
            _RTREE_PRINT("  QS2 b");     // QS2 b
            /* 
                we are on NON leaf node so children of splitted groups must be nodes

                Check each group to see if one group has so few entries that all the rest must 
                be assignmed to it, in order for it to have the min number.
            */
            if( group_a.children_nodes.size() + num_of_not_assigned <= min_nodes ){
                // add the non-assigned to group_a
                for(unsigned i = 0; i < assigned_v.size(); i++){
                    if(assigned_v[i] == false){
                        group_a.children_nodes.push_back( s->children_nodes[i] );
                        assigned_v[i] = true;
                    }
                }
                break;           
            }

            if( group_b.children_nodes.size() + num_of_not_assigned <= min_nodes ){
                // add the non-assigned to group_b
                for( unsigned i = 0; i < assigned_v.size(); i++ ){
                    if( assigned_v[i] == false ){
                        group_b.children_nodes.push_back( s->children_nodes[i] );
                        assigned_v[i] = true;
                    }
                }
                break;
            }

            _RTREE_PRINT("  QS3");     // QS3
            std::pair<unsigned, enum_add_to_group>  next_element;
            next_element = pick_next( group_a, group_b, s, assigned_v );
            unsigned element = next_element.first;
            if( next_element.second == ADD_TO_GROUP_A ){
                group_a.children_nodes.push_back( s->children_nodes[ element ] );
            }   
            else{
                group_b.children_nodes.push_back( s->children_nodes[ element ] );
            }

            num_of_not_assigned--;
        }
    }
    // else leaf node: s
    else{
        _RTREE_PRINT("  leaf node"); 
        // each element is true if the node has been assinged to either "a" or "b"
        std::vector<bool> assigned_v( s->children_leaves.size() );
        std::fill( assigned_v.begin(), assigned_v.end(), false );

        // assign 1st seed to group a
        group_a.children_leaves.push_back( s->children_leaves[initial_seeds.first] );
        assigned_v[ initial_seeds.first ] = true;

        // assign 2nd seed to group b
        group_b.children_leaves.push_back( s->children_leaves[initial_seeds.second] );
        assigned_v[ initial_seeds.second ] = true;

        _RTREE_PRINT("  QS2");    // QS2 
        unsigned num_of_not_assigned = s->children_leaves.size() - 2; // so far we have assinged 2 out of all
        std::vector< std::pair<Rect, int> >::iterator it;

        while(num_of_not_assigned){// QS2 a
            _RTREE_PRINT("  QS2 b");    // QS2 b
            /* 
                we are on leaf node so children of splitted groups must be leaves

                Check each group to see if one group has so few entries that all the rest must 
                be assignmed to it, in order for it to have the min number.
            */
            if( group_a.children_leaves.size() + num_of_not_assigned <= min_nodes ){
                // add the non-assigned to group_a
                for( unsigned i = 0; i < assigned_v.size(); i++ ){
                    if( assigned_v[i] == false ){
                        group_a.children_leaves.push_back( s->children_leaves[i] );
                        assigned_v[i] = true;
                    }
                }
                break;           
            }

            if( group_b.children_leaves.size() + num_of_not_assigned <= min_nodes ){
                // add the non-assigned to group_b
                for( unsigned i = 0; i < assigned_v.size(); i++ ){
                    if( assigned_v[i] == false ){
                        group_b.children_leaves.push_back( s->children_leaves[i] );
                        assigned_v[i] = true;
                    }
                }
                break;
            }

            _RTREE_PRINT("  QS3");    // QS3
            std::pair<unsigned, enum_add_to_group>  next_element;
            next_element = pick_next(group_a, group_b, s, assigned_v);
            unsigned element = next_element.first;
            if( next_element.second == ADD_TO_GROUP_A ){
                group_a.children_leaves.push_back( s->children_leaves[ element ] );
            }   
            else{
                group_b.children_leaves.push_back( s->children_leaves[ element ] );
            }

            num_of_not_assigned--;
        }
    }

    return std::make_pair( group_a, group_b );
}

/*
PS1) caclulate ineffeciency of grouping entries together:
    Foreach pair of entries E1 (i), E2 (j) compose rectangle J (i_union_j) inlcuding E1, E2. 
    Calculate d = area(i_union_j) - area(i) - area(j)
PS2) choose the most wastefull pair:
    Choose pair with largest d
*/

std::pair<unsigned, unsigned> RTree::pick_seeds( RTreeNode *s ){
    double current_d = 0;
    double max_d = std::numeric_limits<double>::min();
    unsigned seed_i = 0;
    unsigned seed_j = 0;
    _RTREE_PRINT("      pick_seeds");  

    // if non leaf node: s
    if( s->children_nodes.size() > 0 ){
        _RTREE_PRINT("      non leaf");    
        _RTREE_PRINT("      PS1");    // PS1
        for( unsigned i = 0; i < s->children_nodes.size(); i++ ){
            // with j=i we check only the upper (diagonal) half  
            // with j=i+1 we also avoid checking for j==i (we don't need it)
            for( unsigned j = i+1; i < s->children_nodes.size(); i++ ){
                current_d = find_enlargement( s->children_nodes[i].first, s->children_nodes[j].first );

                _RTREE_PRINT("      PS2");    // PS2
                if( current_d > max_d ){
                    max_d = current_d;
                    seed_i = i;
                    seed_j = j;
                }            
            }
        }
    }
    // else leaf node: s
    else{
        _RTREE_PRINT("      leaf node");
        _RTREE_PRINT("      PS1");    // PS1
        for( unsigned i = 0; i < s->children_leaves.size(); i++ ){
            // with j=i we check only the upper (diagonal) half  
            // with j=i+1 we also avoid checking for j==i (we don't need this one)
            for( unsigned j = i+1; i < s->children_leaves.size(); i++ ){
                current_d = find_enlargement( s->children_leaves[i].first, s->children_leaves[j].first );

                _RTREE_PRINT("      PS2");    // PS2
                if( current_d > max_d ){
                    max_d = current_d;
                    seed_i = i;
                    seed_j = j;
                }            
            }
        }
    }
    return std::make_pair( seed_i, seed_j );
}
/* 
changed
++
                current_d = find_enlargement( s->children_nodes[i].first, s->children_nodes[j].first);

-- all the following
                double area_i = s->children_leaves[i].first.area();
                double area_j = s->children_leaves[j].first.area();

                Rect i_union_j( s->children_leaves[i].first );
                i_union_j.unionWith( s->children_leaves[j].first );
                double area_i_union_j = i_union_j.area();

                current_d = area_i_union_j - area_j - area_i;
*/



/*
pick_next:
select one remaining entry for classification in a group

PN1) Determine cost of putting each entry in each group:
    Foreach entry E not yet in a group, calulate 
    d1= area increase required in the cover rect of Group 1 to inlcude E
    d2= area increase required in the cover rect of Group 2 to inlcude E
PN2) Find entry with greatest preference for each group:
    Choose any entry with the maximum difference between d1 and d2

*/

std::pair<unsigned, enum_add_to_group> RTree::pick_next( RTreeNode group_a, 
                                                            RTreeNode group_b, 
                                                            RTreeNode *s, 
                                                            std::vector<bool> assigned_v )
{
    double max_increase_difference = std::numeric_limits<double>::min();
    unsigned max_increase_difference_node = 0;
    double current_increase_difference = 0; 

    enum_add_to_group group_to_add;

    // calculate the bounding boxes of the 2 new groups. This info isn't available, since they 
    // have no parent nodes (so that the parent node knows the bounding box)
    Rect bounding_box_a;
    Rect bounding_box_b;

    double increase_area_a = 0;
    double increase_area_b = 0;

    _RTREE_PRINT("      pick_next");    

    // if non leaf node
    if( group_a.children_nodes.size() > 0 ){
        _RTREE_PRINT("      non leaf");    

        // calculate the bounding boxes of the 2 new groups. This info isn't available, since they 
        // have no parent nodes (so that the parent node knows the bounding box)
        bounding_box_a = Rect( group_a.children_nodes[0].first );
        bounding_box_b = Rect( group_b.children_nodes[0].first );
        for( unsigned i = 1; i < group_a.children_nodes.size(); i++ ){
            bounding_box_a.unionWith( group_a.children_nodes[i].first );
        }
        for( unsigned i = 1; i < group_b.children_nodes.size(); i++ ){
            bounding_box_b.unionWith( group_b.children_nodes[i].first );
        }

        _RTREE_PRINT("      PN1");    // PN1
        for( unsigned i = 0; i < assigned_v.size(); i++ ){
            if( assigned_v[i] == false ){

                increase_area_a = bounding_box_a.area() - s->children_nodes[i].first.area();
                increase_area_b = bounding_box_b.area() - s->children_nodes[i].first.area();

                _RTREE_PRINT("      PN2");    // PN2
                current_increase_difference = std::abs( increase_area_a - increase_area_b );
                if( current_increase_difference > max_increase_difference ){
                    max_increase_difference = current_increase_difference;
                    max_increase_difference_node = i;

                    // TODO tie not solved!
                    if( increase_area_a < increase_area_b ){
                        group_to_add = ADD_TO_GROUP_A;
                    }
                    else{
                        group_to_add = ADD_TO_GROUP_B;
                    }
                }
            }
        }
    }
    // else leaf node
    else{
        _RTREE_PRINT("      leaf");    

        // calculate the bounding boxes of the 2 new groups. This info isn't available, since they 
        // have no parent nodes (so that the parent node knows the bounding box)
        bounding_box_a = Rect( group_a.children_leaves[0].first );
        bounding_box_b = Rect( group_b.children_leaves[0].first );
        for( unsigned i = 1; i < group_a.children_leaves.size(); i++ ){
            bounding_box_a.unionWith( group_a.children_leaves[i].first );
        }
        for( unsigned i = 1; i < group_b.children_leaves.size(); i++ ){
            bounding_box_b.unionWith( group_b.children_leaves[i].first );
        }

        _RTREE_PRINT("      PN1");    // PN1
        for( unsigned i = 0; i < assigned_v.size(); i++ ){
            if( assigned_v[i] == false ){

                increase_area_a = bounding_box_a.area() - s->children_leaves[i].first.area();
                increase_area_b = bounding_box_b.area() - s->children_leaves[i].first.area();

                _RTREE_PRINT("      PN2");    // PN2
                current_increase_difference = std::abs( increase_area_a - increase_area_b );
                if( current_increase_difference > max_increase_difference ){
                    max_increase_difference = current_increase_difference;
                    max_increase_difference_node = i;

                    // TODO tie not solved!
                    if( increase_area_a < increase_area_b ){
                        group_to_add = ADD_TO_GROUP_A;
                    }
                    else{
                        group_to_add = ADD_TO_GROUP_B;
                    }
                }
            }
        }
    }

/*
    double increase_area_a = 0;
    double increase_area_b = 0;

    _RTREE_PRINT("      PN1");    // PN1
    for( unsigned i = 0; i < assigned_v.size(); i++ ){
        if( assigned_v[i] == false ){

            increase_area_a = bounding_box_a.area() - s->children_nodes[i].first.area();
            increase_area_b = bounding_box_b.area() - s->children_nodes[i].first.area();

            _RTREE_PRINT("      PN2");    // PN2
            current_increase_difference = std::abs( increase_area_a - increase_area_b );
            if( current_increase_difference > max_increase_difference ){
                max_increase_difference = current_increase_difference;
                max_increase_difference_node = i;

                // TODO tie not solved!
                if( increase_area_a < increase_area_b ){
                    group_to_add = ADD_TO_GROUP_A;
                }
                else{
                    group_to_add = ADD_TO_GROUP_B;
                }
            }
        }
    }
*/

    return std::make_pair( max_increase_difference_node, group_to_add );
}

/* I3 =========================================================================

adjust_tree:
Ascend from a leaf node L to root, adjusting covering rectangles and propagating node splits as
necessary

We modified this one from the source in the step AT1 and AT5

AT1) Initialize:
    Set N=L
    IF L was spilt previously, set NN to be the resulting second node AND
    (not mentioned in the original source but that's what it should mean)
    Assign all entries of first node to L
AT2) check if done:
    IF N is root stop
AT3) adjust covering rectangle in parent entry
    Let P be the parent of N
    Let EN be the N's entry in P
    Adjust EN bounding box so that it tightly enclosses all entry rectangles in N
AT4) Propagate node split upward
    IF N has a partner NN resulting from an earlier split create a new entry ENN with ENN "p" 
    pointing to NN and ENN bounding box enclosing all rectangles in NN
        IF there is room in P add NN
        ELSE invoke split_node to produce P an PP containing ENN and all P's old entries.
AT5) Move up to next level
    Set N=P, 
    IF a split occurred, set NN=PP 
    goto AT1 (originally it was goto AT2)
*/

bool RTree::adjust_tree(    RTreeNode* position, 
                            std::pair<RTreeNode, RTreeNode>  &splitted_groups, 
                            //bool split_performed, 
                            unsigned min_nodes,
                            unsigned max_nodes )
{


    RTreeNode* parent;
    bool root_split_performed = false;
    bool split_performed = true;
    // pick randomly, the 1st entry of the node. Search for this spatial area in the tree. From the
    // results find the exact node.
  
    // TODO check what if in the 1st iteration position == root
//    while( position != root ){

    _RTREE_PRINT("  adjust_tree");    
    while( true ){

        _RTREE_PRINT("  AT2");    // AT2  
        // check for loop BREAK
        if( position == root ){
            _RTREE_PRINT("  AT1");    // repeat AT1
            if( split_performed ){
                
                if( position->children_nodes.size() > 0 ){
                    _RTREE_PRINT("  non leaf");    // non leaf-node
                    position->children_nodes.clear();
                    for( unsigned i=0; i < splitted_groups.first.children_nodes.size(); i++ ){
                        position->children_nodes.push_back( splitted_groups.first.children_nodes[i] );
                    }
                }
                else{
                    _RTREE_PRINT("  leaf node");     // leaf-node
                    position->children_leaves.clear();
                    for( unsigned i=0; i < splitted_groups.first.children_leaves.size(); i++ ){
                        position->children_leaves.push_back( splitted_groups.first.children_leaves[i] );
                    }
                }
            }
            root_split_performed = true;
            break;
        }

        _RTREE_PRINT("  AT1");    // AT1
        if( split_performed ){
            
            if( position->children_nodes.size() > 0 ){
                _RTREE_PRINT("  PS1");    // non leaf-node: position
                position->children_nodes.clear();
                for( unsigned i=0; i < splitted_groups.first.children_nodes.size(); i++ ){
                    position->children_nodes.push_back( splitted_groups.first.children_nodes[i] );
                }
            }
            else{
                _RTREE_PRINT("  PS1");    // leaf-node: positions
                position->children_leaves.clear();
                for( unsigned i=0; i < splitted_groups.first.children_leaves.size(); i++ ){
                    position->children_leaves.push_back( splitted_groups.first.children_leaves[i] );
                }
            }
        }
        _RTREE_PRINT("  AT3");    // AT3
        parent = find_parent( root, position->children_nodes[0].first, position);
        unsigned child_in_parent;
        for( child_in_parent = 0; child_in_parent < parent->children_nodes.size(); child_in_parent++ ){
            if( parent->children_nodes[child_in_parent].second == position){
                break;
            }
        }
        _RTREE_PRINT("  recalculate bounding box of parent ");    
        if( position->children_nodes.size() > 0 ){
            _RTREE_PRINT("  non leaf");    // non leaf-node: position
            parent->children_nodes[ child_in_parent ].first = Rect( position->children_nodes[0].first );
            for( unsigned i=1; i < position->children_nodes.size(); i++ ){
                parent->children_nodes[ child_in_parent ].first.unionWith( position->children_nodes[i].first );
            }
        }
        else{ 
            _RTREE_PRINT("  leaf");    // leaf-node: position
            parent->children_nodes[ child_in_parent ].first = Rect( position->children_leaves[0].first );
            for( unsigned i=1; i < position->children_leaves.size(); i++ ){
                parent->children_leaves[ child_in_parent ].first.unionWith( position->children_leaves[i].first );
            }
        }

        _RTREE_PRINT("  AT4");    // AT4
        if( split_performed ){
            // create new entry
            Rect new_entry_bounding( splitted_groups.second.children_nodes[0].first );
            for(unsigned i = 0; i < splitted_groups.second.children_nodes.size(); i++ ){
                new_entry_bounding.unionWith( splitted_groups.second.children_nodes[i].first );
            }

            std::pair<Rect, RTreeNode*> new_entry = std::make_pair( new_entry_bounding, &splitted_groups.second );
            
            if( parent->children_nodes.size() < max_nodes ){
                parent->children_nodes.push_back( new_entry );
                split_performed = false;
            }
            else{
                // TODO CHECK this closer invoke split node
                splitted_groups = quadratic_split( parent, min_nodes );
                split_performed = true;
            }
            position = parent;
        }
    }

    return root_split_performed;
}

/*
find_parent:
The source only mentions that we should "find" the parent. But it doesn't seay how. So we made a 
modification of search. 

Initially we take the root, a rect of the node, of which the parent we look for and the node we seek

We do a spatial search for this rect. If we find get an intersecttion with the rect we check if the
child is the one we look for.
If not we call find_parent again recursively

TODO check this more. sth looks suspicious
*/

RTreeNode* RTree::find_parent( RTreeNode* subtree_root, 
                                Rect search_area, 
                                RTreeNode* wanted){
    _RTREE_PRINT("find_parent");    
   
    if( subtree_root->children_nodes.size() > 0 ){
        _RTREE_PRINT("non leaf");     // non leaf node
        for( unsigned i=0; i < subtree_root->children_nodes.size(); i++ ){
            if( subtree_root->children_nodes[i].second == wanted){
                break;
            }
            if( subtree_root->children_nodes[i].first.intersects( search_area ) ){
              find_parent( subtree_root->children_nodes[i].second, search_area, wanted);
            }
        }
    }
    return subtree_root;
}




/*
    helper function

    print the elements of the tree
    based on ordered tree walking 
*/
void RTree::print_tree(RTreeNode* subtree_root, int depth){
    for(int j=0; j< depth; j++){
        std::cout << "  ";
    }
    _RTREE_PRINT("print_tree");// leaf node

    if( subtree_root->children_nodes.size() > 0 ){ 
        for(int j=0; j< depth; j++){
            std::cout << "  ";
        }
        _RTREE_PRINT("non leaf");// non leaf node

        // descend in each one of the elements and call print_tree
        for( unsigned i=0; i < subtree_root->children_nodes.size(); i++ ){
            //print spaces for indentation
            for(int j=0; j< depth; j++){
                std::cout << "  ";
            }
            std::cout << subtree_root->children_nodes[i].first << std::endl;
            print_tree( subtree_root->children_nodes[i].second, depth++);
        }
    }
    else{   
        for(int j=0; j< depth; j++){
            std::cout << "  ";
        }
         _RTREE_PRINT("leaf");//leaf node

        //print spaces for indentation
        for(int j=0; j< depth; j++){
            std::cout << "  " ;
        }
        std::cout << subtree_root->children_leaves.size() << ": ";

        // print all the elements of the node
        for( unsigned i=0; i < subtree_root->children_leaves.size(); i++ ){
            std::cout << subtree_root->children_leaves[i].second << ", ";
        }
        std::cout << std::endl;
    }
}


/*=============================================================================
TODO                            search
===============================================================================
*/


/*=============================================================================
TODO                            erase
===============================================================================
*/

/*=============================================================================
TODO                            update
===============================================================================
*/


};

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
