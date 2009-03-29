#include <2geom/redblacktree.h>


#define _REDBLACK_PRINT(x) std::cout << x << std::endl;
//comment the following if you want output during RedBlack Tree operations
//#define _REDBLACK_PRINT(x) ; 


namespace Geom{

RedBlack* RedBlackTree::search(int shape) {
    return 0;
}



void RedBlackTree::insert(Rect const &r, int shape) {

    _REDBLACK_PRINT( std::endl << "insert(Rect, int): " << r[0].min() << " , shape: " )
    insert(r[0].min(), shape); // TODO change to the min of the x0, x1 of the Rect
}

// source: book pp 251
void RedBlackTree::insert(double x_min, int shape) {

    _REDBLACK_PRINT( std::endl << "insert(double, int): " <<  x_min << " , shape: " << shape )

    // x is the new node we insert
    RedBlack *x = new RedBlack;
    x->key = x_min;
    x->data = shape;
    x->isRed = true;

    _REDBLACK_PRINT( " x: " <<  x << " x->key: " << x->key )

    tree_insert(x);


    _REDBLACK_PRINT( "*** Begin coloring" )

    print_tree();

    // we now do the coloring of the tree.    
    _REDBLACK_PRINT( "=== while( x!= root && (x->parent)->isRed )" )
    while( x!= root && (x->parent)->isRed ){
        _REDBLACK_PRINT( "--- ((x->parent)->parent)->left:" << ((x->parent)->parent)->left )
        _REDBLACK_PRINT( "--- ((x->parent)->parent)->right:" << ((x->parent)->parent)->right )
        if( x->parent == ((x->parent)->parent)->left ){
            _REDBLACK_PRINT( "   Left:" )
            RedBlack *y = new RedBlack;
            y = ((x->parent)->parent)->right;
            if( y == 0  ){ 
                /* 
                This 1st brach is not in the book, but is needed. We must check y->isRed but it is 
                undefined, so we get segfault. But 0 (undefined) means that y is a leaf, so it is 
                black by definition. So, do the same as in the else part.
                */ 
                _REDBLACK_PRINT( "y==0" )
                if( x == (x->parent)->right ){
                    x = x->parent;
                    left_rotate(x);
                }
                (x->parent)->isRed = false;
                ((x->parent)->parent)->isRed = true;
                right_rotate((x->parent)->parent);
            }
            else if( y->isRed ){
                _REDBLACK_PRINT( "y->isRed" )
                (x->parent)->isRed = false;
                y->isRed = false;
                ((x->parent)->parent)->isRed = true;
                x = (x->parent)->parent;
            }
            else{
                _REDBLACK_PRINT( "!( y->isRed)" )
                if( x == (x->parent)->right ){
                    x = x->parent;
                    left_rotate(x);
                }
                (x->parent)->isRed = false;
                ((x->parent)->parent)->isRed = true;
                right_rotate((x->parent)->parent);
            }
        }
        else{ // this branch is the same with the above if clause with "right", "left" exchanged
            _REDBLACK_PRINT( "   Right:" )
            RedBlack *y = new RedBlack;
            y = ((x->parent)->parent)->left;
            if( y == 0  ){ 
                /* 
                This 1st brach is not in the book, but is needed. We must check y->isRed but it is 
                undefined, so we get segfault. But 0 (undefined) means that y is a leaf, so it is 
                black by definition. So, do the same as in the else part.
                */ 
                _REDBLACK_PRINT( "y==0" )
                if( x == (x->parent)->left ){
                    x = x->parent;
                    right_rotate(x);
                }
                (x->parent)->isRed = false;
                ((x->parent)->parent)->isRed = true;
                left_rotate((x->parent)->parent);
            }
            else if( y->isRed ){
                _REDBLACK_PRINT( "y->isRed" )
                (x->parent)->isRed = false;
                y->isRed = false;
                ((x->parent)->parent)->isRed = true;
                x = (x->parent)->parent;
            }
            else{
                _REDBLACK_PRINT( "!( y->isRed)" )
                if( x == (x->parent)->left ){
                    x = x->parent;
                    right_rotate(x);
                }
                (x->parent)->isRed = false;
                ((x->parent)->parent)->isRed = true;
                left_rotate((x->parent)->parent);
            }
        }
        
    }
    root->isRed = false;
    _REDBLACK_PRINT( "*** Insert finished!" )
}

// from book p. 266)
void RedBlackTree::left_rotate(RedBlack* x){
    // x->right != 0 (assumption book page 266)
    // ??? hm problem ???
    _REDBLACK_PRINT( "*** left_rotate" )
    RedBlack* y = new RedBlack;
    y = x->right;    
    x->right = y->left;

    if( y->left != 0){
       (y->left)->parent = x;
    }

    y->parent = x->parent;

    if( x->parent == 0){
        root = y;
    }
    else{
        if( x == (x->parent)->left ){
            (x->parent)->left = y;
        }
        else{
            (x->parent)->right = y;
        }
    }

    y->left = x;
    x->parent = y;
}

// from book p. 266: right_rotate is inverse of left_rotate 
// same to left_rotate with "right", "left" exchanged
void RedBlackTree::right_rotate(RedBlack* x){
    // x->right != 0 (assumption book page 266)
    // ??? hm problem ??
    _REDBLACK_PRINT( "*** right_rotate" )
    RedBlack* y = new RedBlack;

    _REDBLACK_PRINT( "x->left: " << x->left )
    y = x->left;    
    x->left = y->right;
    
    if( y->right != 0){ // TODO inverse these ???
       (y->right)->parent = x;
    }

    y->parent = x->parent;

    _REDBLACK_PRINT( "lala" )
    if( x->parent == 0){
        root = y;
    }
    else{
        if( x == (x->parent)->left ){
            (x->parent)->left = y;
        }
        else{
            (x->parent)->right = y;
        }
    }

    y->right = x;
    x->parent = y;
}

// insertion in binary search tree: book page 251
// then the redblack insert performs the coloring
void RedBlackTree::tree_insert(RedBlack* z){
    _REDBLACK_PRINT( "*** Begin tree insert" )
    RedBlack* y = 0; // y <- nil

    RedBlack* x = new RedBlack();
    x = root;

    _REDBLACK_PRINT( "=== while( x != 0 )" )
    while( x != 0 ){
        y = x;
        _REDBLACK_PRINT( "--- x:" << x << " y:" << y << " z:" << z )
        _REDBLACK_PRINT( "z->key: " << z->key )
        _REDBLACK_PRINT( "x->key: " << x->key )
        if( z->key < x->key ){
            _REDBLACK_PRINT(  "left" )   
            x = x->left;
        }
        else{
            _REDBLACK_PRINT( "right" )
            x = x->right;
        }
    }

    _REDBLACK_PRINT( "=== z->parent = y" )
    z->parent = y;


    if( y == 0 ){
        _REDBLACK_PRINT( "set z root" )
        root = z;
    }
    else{
        _REDBLACK_PRINT( "z->key: " << z->key )
        _REDBLACK_PRINT( "y->key: " << y->key )
        if( z->key < y->key ){
            _REDBLACK_PRINT( " left " )
            y->left = z;
        }
        else{
            _REDBLACK_PRINT( " right " )
            y->right = z;
        }
    }
}


void RedBlackTree::erase(RedBlack* T, int shape) {
    
}

void RedBlackTree::print_tree(){
    std::cout << "*** RedBlackTree status:" << std::endl;
    inorder_tree_walk(root);
};


void RedBlackTree::inorder_tree_walk(RedBlack* x){
    int oops =0;
    if( x != 0 ){
        inorder_tree_walk(x->left);
        std::cout<< "(" << x->data << ", " << x->key << ") " ;

        if( x->left != 0 ){
            std::cout<< "L:(" << (x->left)->data << ", " << (x->left)->key << ") " ;
            if( x->key < (x->left)->key){
                std::cout<<"  !!!  ";
                oops = 1;
            }
        }
        else{
            std::cout<< "L:0 " ;
        }    

        if( x->right != 0 ){
            std::cout<< "R:(" << (x->right)->data << ", "<< (x->right)->key << ") " ;
            if( x->key > (x->right)->key){
                std::cout<<"  !!!  ";
                oops = 1;
            }
        }
        else{
            std::cout<< "R:0 " ;
        } 

        if(oops){
            std::cout<<" .......  !!! Problem ";
        }
        std::cout << std::endl;
        inorder_tree_walk(x->right);
    }
}


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
