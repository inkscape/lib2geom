#include <2geom/rtree.h>
//#include <algorithm>
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


#define _REDBLACK_PRINT(x) std::cout << x << std::endl;
//comment the following if you want output during RedBlack Tree operations
//#define _REDBLACK_PRINT(x) ; 


namespace Geom{

/*
insert insert a new index entry into the r-tree

Insert:
I1) find position of new record.
    choose_leaf will find a leaf node L (position) in which to place r
I2) add record to leaf node
    if L has room for another entry install E
    else split_node will obtain L and LL containing E and old entries of L
    from the available split_nodes we chose quadtratic-cost algorithm (just to begin with sth)
    // TODO implement more of them AND TODO make their usage parameter in tree
I3) propagate changes upward 
I4) grow tree taller 
*/
void RTree::insert(Rect const &r, int shape, unsigned min_nodes){
    RTreeNode *position = 0;
    // I1
    position = choose_leaf(r, shape);
    // I2
    quadratic_split(position, min_nodes);
    // 
    // TODO I3
    // TODO I4
}

/* I1 -------------------------------------------------------------------------
choose_leaf will find a leaf node L in which to place r

1) Initialize: set N to be the root node
2) Leaf Check: if N is leaf return N
3) Choose subtree: If N not leaf then   
    let F be an entry in N whose rect Fi needs least enlargment to incude r
    ties resolved with rect of smallest area
4) descend until a leaf is reached: set N to the child node pointed to by F and goto 2.
*/
RTreeNode* RTree::choose_leaf(Rect const &r, int shape){
    RTreeNode *pos = root;

    double min_enlargment = std::numeric_limits<double>::max();
    double current_enlargment = 0;
    int node_min_enlargment = 0;


    while(pos->children_leaves.size() == 0){
        for(unsigned i=0; i<= pos->children_nodes.size(); i++){
            // TODO find_enlargment
            current_enlargment = find_enlargment( pos->children_nodes[i]->bounding_box, r);
            // TODO tie not solved!
            if(current_enlargment < min_enlargment){
                node_min_enlargment = i;
                min_enlargment = current_enlargment;
            }
        }
        pos = pos->children_nodes[node_min_enlargment];
    }
    return pos;
}

// TODO find_enlargment
double RTree::find_enlargment(Rect const &children_node, Rect const &new_node){
    return 0.0;
}

/* I2 -------------------------------------------------------------------------
Quadratic Split
QS1) Pick first entry for each group:
    Appy pick_seeds to choose 2 entries to be the first elements of the groups. Assign each to a group
QS2) check if done:
    if all entries have been assinged stop
    if one group has so few entries that all the rest must be assignmed to it, in order for it to 
    have the min number , assign them and stop
QS3) select entry and assign:
    goto 2.
*/
void RTree::quadratic_split(RTreeNode *s, unsigned min_nodes){
//    std::vector<RTreeNode> group_a;
//    std::vector<RTreeNode> group_b;
    RTreeNode group_a;
    RTreeNode group_b;

    std::vector<RTreeNode*>::iterator it;



    // QS1
    std::pair<unsigned, unsigned> initial_seeds;
    initial_seeds = pick_seeds(s);

    unsigned num_of_not_assigned = s->children_nodes.size() - 2; // so far we have assinged 2 out of all

    // each element is true if the node has been assinged to either "a" or "b"
    std::vector<bool> assigned_v( s->children_nodes.size() );
    std::fill( assigned_v.begin(), assigned_v.end(), false );

    assigned_v[ initial_seeds.first ] = true;
    group_a.children_nodes.push_back( s->children_nodes[initial_seeds.first] );
    it = group_a.children_nodes.end();
    it--;
    group_a.bounding_box = (*it)->bounding_box;

    assigned_v[ initial_seeds.second ] = true;
    group_b.children_nodes.push_back( s->children_nodes[initial_seeds.second] );
    it = group_b.children_nodes.end();
    it--;
    group_b.bounding_box = (*it)->bounding_box;


    // QS2 
    std::pair<double, quadratic_split_group_to_add>  next_element;
    while(num_of_not_assigned){
        if(group_a.children_nodes.size() + num_of_not_assigned <= min_nodes){
            // add the non-assigned to group_a
            for(unsigned i=0; i<assigned_v.size(); i++){
                if(assigned_v[i] == false){
                    group_a.children_nodes.push_back( s->children_nodes[i] );
                    assigned_v[i] = true;

                    it = group_a.children_nodes.end();
                    it--;
                    group_a.bounding_box.unionWith( (*it)->bounding_box );

                }
            }
            break;           
        }

        if(group_b.children_nodes.size() + num_of_not_assigned <= min_nodes){
            // add the non-assigned to group_b
            for(unsigned i=0; i<assigned_v.size(); i++){
                if(assigned_v[i] == false){
                    group_b.children_nodes.push_back( s->children_nodes[i] );
                    assigned_v[i] = true;

                    it = group_b.children_nodes.end();
                    it--;
                    group_b.bounding_box.unionWith( (*it)->bounding_box );
                }
            }
            break;
        }

        // QS3
        next_element = pick_next(group_a, group_b, s, assigned_v);
        if( next_element.second == ADD_TO_GROUP_A ){
            group_a.children_nodes.push_back( s->children_nodes[ next_element.first ] );
        }   
        else{
            group_b.children_nodes.push_back( s->children_nodes[ next_element.first ] );
        }

        num_of_not_assigned--;
    }
}

/*
1) caclulate ineffeciency of grouping entries together:
    Foreach pair of entries E1 (i), E2 (j) compose rectangle J (i_union_j) inlcuding E1, E2. 
    Calculate d = area(i_union_j) - area(i) - area(j)
2) choose the most wastefull pair:
    Choose pair with largest d
*/

std::pair<unsigned, unsigned> RTree::pick_seeds(RTreeNode *s){
    double current_d = 0;
    double max_d = std::numeric_limits<double>::min();
    unsigned seed_i = 0;
    unsigned seed_j = 0;


    // 1
    for(unsigned i=0; i<= s->children_nodes.size(); i++){
        // with j=i we check only the upper (diagonal) half  
        // with j=i+1 we also avoid checking for j==i (we don't need this one)
        for(unsigned j=i+1; i<= s->children_nodes.size(); i++){
            double area_i = s->children_nodes[i]->bounding_box.area();
            double area_j = s->children_nodes[j]->bounding_box.area();

            Rect i_union_j( s->children_nodes[i]->bounding_box );
            i_union_j.unionWith( s->children_nodes[j]->bounding_box );
            double area_i_union_j = i_union_j.area();

            current_d = area_i_union_j - area_j - area_i;

            // 2
            if(current_d > max_d){
                max_d = current_d;
                seed_i = i;
                seed_j = j;
            }            
        }
    }
    return std::make_pair(seed_i, seed_j);
}

std::pair<double, quadratic_split_group_to_add> RTree::pick_next(RTreeNode group_a, RTreeNode group_b, RTreeNode *s, std::vector<bool> assigned_v){
    double max_increase_difference = std::numeric_limits<double>::min();
    unsigned max_increase_difference_i = 0;
    double current_increase_difference = 0; 

    quadratic_split_group_to_add group_to_add;

    for(unsigned i=0; i<assigned_v.size(); i++){
        if(assigned_v[i] == false){
            double increase_area_a = 0;

            Rect temp_a(group_a.bounding_box);
            temp_a.unionWith( s->bounding_box );
            increase_area_a = temp_a.area() - s->children_nodes[i]->bounding_box.area();

            double increase_area_b = 0;
            Rect temp_b(group_b.bounding_box);
            temp_b.unionWith( s->bounding_box );
            increase_area_b = temp_b.area() - s->children_nodes[i]->bounding_box.area();

            current_increase_difference = std::abs( increase_area_a - increase_area_b );
            if( current_increase_difference > max_increase_difference ){
                max_increase_difference = current_increase_difference;
                max_increase_difference_i = i;

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


    return std::make_pair(max_increase_difference_i, group_to_add);
    //return max_increase_difference_i;
}

/*
TODO after insert
search
erase

update ???
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
