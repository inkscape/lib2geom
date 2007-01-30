/*
 * conjugate_gradient.h
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

#ifndef _CONJUGATE_GRADIENT_H
#define _CONJUGATE_GRADIENT_H

#include <valarray>

double
inner(std::valarray<double> const &x, 
      std::valarray<double> const &y);

void 
conjugate_gradient(std::valarray<double> const &A, 
		   std::valarray<double> &x, 
		   std::valarray<double> const &b, 
		   unsigned n, double tol,
		   unsigned max_iterations, bool ortho1);
#endif // _CONJUGATE_GRADIENT_H
