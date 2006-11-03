#include "path.h"

template <typename iterator>
static void
cubic_bezier_poly_coeff(iterator b, Geom::Point *pc) {
	double c[10] = {1, 
			-3, 3, 
			3, -6, 3,
			-1, 3, -3, 1};

	int cp = 0;

	for(int i = 0; i < 4; i++) {
		pc[i] = Geom::Point(0,0);
		++b;
	}
	for(int i = 0; i < 4; i++) {
		--b;
		for(int j = 0; j <= i; j++) {
			pc[3 - j] += c[cp]*(*b);
			cp++;
		}
	}
}

