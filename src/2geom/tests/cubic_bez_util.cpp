#include "cubic_bez_util.h"
//TODO: move unit testing into tests
#ifdef UNIT_TEST
int main(int argc, char** argv) {
	Geom::Point handle[4] = {Geom::Point(0,1),
			      Geom::Point(0,0),
			      Geom::Point(0,0),
			      Geom::Point(1,0)};
	Geom::Point pc[4];
	cubic_bezier_poly_coeff(handle, pc);
	
	for(int i = 0; i < 4; i++) {
		printf("%g, %g -> %g, %g\n", handle[i][0], handle[i][1],
		       pc[i][0], pc[i][1]);
	}
	handle[0] = Geom::Point(0,0);
	handle[1] = Geom::Point(1,0);
	handle[2] = Geom::Point(0,1);
	handle[3] = Geom::Point(0,0);
	
	cubic_bezier_poly_coeff(handle, pc);
	
	for(int i = 0; i < 4; i++) {
		printf("%g, %g -> %g, %g\n", handle[i][0], handle[i][1],
		       pc[i][0], pc[i][1]);
	}
}
#endif
