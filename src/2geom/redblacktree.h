/**
 * \file
 * \brief 
 * Implementation of Red-Black Tree as described in 
 * Intorduction to Algorithms. Cormen et al. Mc Grow Hill. 1990. pp 263-280
 * 
 * The intention is to implement interval trees mentioned in the same book, after the red-black.
 * Interval are heavily based on red-black trees (most operations are the same). So, we begin first 
 * with implementing red-black!
 *
 * Authors:
 *      ? <?@?.?>
 * 
 * Copyright 2009-2009 Evangelos Katsikaros
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
 *
 */

#include <vector>
//#include <cassert>
#include <limits>
#include <cfloat>

#include <2geom/d2.h>
#include <2geom/interval.h>

namespace Geom{

class RedBlack{
public:
    RedBlack *left, *right, *parent;
    bool isRed;
    // max( x->left->subtree_max, x->right->subtree_max, x->high )
    Coord subtree_max;

private:
    // We'll use 2geom's interval for interval trees. Key will be the min of the interval
    Interval *interval;

public:    
    int data;

    RedBlack(): left(0), right(0), parent(0), isRed(false), subtree_max(0.0), /*key(0.0),*/ interval(0), data(0) {
    }

    RedBlack(Coord min, Coord max): left(0), right(0), parent(0), isRed(false), subtree_max(0.0), /*key(0.0),*/ data(0) {
        setInterval( min, max );
    }

    inline Coord key(){ return interval->min(); };
    inline Coord high(){ return interval->max(); };

    inline void setInterval( Coord min, Coord max ){
        interval = new Interval( min, max );  // TODO garbage???
    }

    inline void setInterval( Interval i ){
        // the i and its node might be erased in the future so we save interval in here (is this possible ? TODO)
        interval = new Interval( i.min(), i.max() ); 
    }

    inline Interval getInterval(){
        return Interval( interval->min(), interval->max() );
    }
};


class RedBlackTree{
public:
    RedBlack* root;

    RedBlackTree(): root(0) {}

    void insert(Rect const &r, int shape, int dimension);
    void insert(Coord dimension_min, Coord dimension_max, int shape);

    void erase(Rect const &r);
    void erase(int shape);

    //RedBlack* search(int shape);
    RedBlack* search(Rect const &r, int dimension);
    RedBlack* search(Interval *i);

    void print_tree();
private:
    void inorder_tree_walk(RedBlack* x);
    RedBlack* tree_minimum(RedBlack* x);
    RedBlack* tree_successor(RedBlack* x);

    void left_rotate(RedBlack* x);
    void right_rotate(RedBlack* x);
    void tree_insert(RedBlack* x);

    void update_max(RedBlack* x);

    RedBlack* erase(RedBlack* x); // TODO why rerutn pointer? to collect garbage ???
    void erase_fixup(RedBlack* x);

};

}; //close namespace

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
