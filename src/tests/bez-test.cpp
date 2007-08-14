#include "point.h"

template <unsigned order> 
inline Geom::Point Bez(double t, Geom::Point* b) {
    Geom::Point child[order];
    for(unsigned i = 0; i < order; i++)
        child[i] = (1-t)*b[i] + t*b[i+1];
    return Bez<order-1>(t, child);
}

template <> 
inline Geom::Point Bez<0>(double t, Geom::Point* b) { return *b;}



#include <iostream>
int main(int argc, char** argv) {
	Geom::Point a[] = {Geom::Point(1,0),Geom::Point(2,3),Geom::Point(2,5)};
	Geom::Point p = Bez<1>(0.5, a);
	
	std::cout << p << std::endl;
}
