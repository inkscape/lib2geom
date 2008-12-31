#include <vector>
#include <stdio.h>
#include <iostream>
#include <2geom/choose.h>

double W2(unsigned n, unsigned j, unsigned k) {
    if(j == k)
        return 1;
    unsigned q = (n+1)/2;
    if(k > n-k) return W2(n, n-j, n-k);
    if(k < 0) return 0;
    if(k >= q) return 0;
    if(j >= n-k) return 0;
    if(j < k) return 0;
    return choose<double>(n-2*k-1, j-k) * 
        choose<double>(2*k+1,k) /
        choose<double>(n,j);
}
double Tm1(unsigned n, unsigned j, unsigned k) {
    if(j == k)
        return 1;
    unsigned q = (n+1)/2;
    if(k > n-k) return Tm1(n, n-j, n-k);
    if(k < 0) return 0;
    if(k >= q) return 0;
    if(j >= n-k) return 0;
    if(j < k) return 0;
    return choose<double>(n-2*k-1, j-k);
}

double W(unsigned n, unsigned j, unsigned k) {
    unsigned q = (n+1)/2;
    if((n & 1) == 0 && j == q && k == q)
        return 1;
    if(k > n-k) return W(n, n-j, n-k);
    if(k < 0) return 0;
    if(k >= q) return 0;
    if(j >= n-k) return 0;
    if(j < k) return 0;
    return choose<double>(n-2*k-1, j-k) /
        choose<double>(n,j);
}

int main() {
	for(int j = 0; j < 10; j++) {
		for(int i = 0; i < j+1; i++) {
			std::cout << "(" << j << "," << i << ")";
			std::cout << choose<float>(j, i) << " ";
		}
		std::cout << std::endl;
	}
	for(int n = 1; n < 5; n++) {
		std::cout << "n = " << n << std::endl;
		for(int j = 0; j <= n; j++) {
			for(int i = 0; i <= n; i++) {
				std::cout << W(n, j, i) << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
}
