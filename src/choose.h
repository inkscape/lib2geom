/*
 * choose.h
 *
 * Copyright 2006 Nathan Hurst <njh@mail.csse.monash.edu.au>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef _CHOOSE_H
#define _CHOOSE_H

// XXX: Can we keep only the left terms easily? 
// this would more than halve the array
// row index becomes n2 = n/2, row2 = n2*(n2+1)/2, row = row2*2+(n&1)?n2:0
// we could also leave off the ones

template <typename T>
T choose(unsigned n, unsigned k) {
    static std::vector<T> pascals_triangle;
    static unsigned rows_done = 0;
    // indexing is (0,0,), (1,0), (1,1), (2, 0)...
    // to get (i, j) i*(i+1)/2 + j
    if(k < 0 || k > n) return 0;
    if(rows_done <= n) {// we haven't got there yet
        if(rows_done == 0) {
            pascals_triangle.push_back(1);
            rows_done = 1;
        }
        while(rows_done <= n) {
            unsigned p = pascals_triangle.size() - rows_done;
            pascals_triangle.push_back(1);
            for(int i = 0; i < rows_done-1; i++) {
                pascals_triangle.push_back(pascals_triangle[p] 
                                           + pascals_triangle[p+1]);
		p++;
            }
            pascals_triangle.push_back(1);
            rows_done ++;
        }
    }
    unsigned row = (n*(n+1))/2;
    return pascals_triangle[row+k];
}

#endif
