#include <cmath>

bool near(double a, double b) { return fabs(a - b) < 0.0001; }

#include "crossing.h"

#include <iostream>

using namespace Geom;

int main(int argc, char** argv) {
    std::vector<int> x;
    for(int i = 0; i < 10; i++) x.push_back(i);
    int j = 0;
    for(Eraser<std::vector<int> > i(&x); ; i++) {
        std::cout << *i << "\n";
        i.erase();
        j++;
        if(j == 10) break;
    }
    for(int i = 0; i < x.size(); i++) { std::cout << x[i] << "\n"; }
    return 1;
}
