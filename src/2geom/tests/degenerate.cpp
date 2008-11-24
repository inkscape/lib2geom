#include <2geom/sbasis.h>
#include <vector>
#include <iostream>

#include "mersennetwister.h"

using namespace Geom;
using namespace std;

Geom::SBasis zeros(double t, int ord) {
  Geom::SBasis res(1);
  for(int i = 0; i < ord; i++)
    res *= Geom::Linear(-t, 1-t);
  return res;
}

void
generate_degenerate_cases(vector<Geom::SBasis> &sbs) {
  double rs[] = {0, 1, 0.5, 0.3, 0.001, 1e-7, 1.0-1e-4};
  vector<double> root_sites(rs, rs+sizeof(rs)/sizeof(double));
  for(unsigned i = 0; i < root_sites.size(); i++) {
    cout << root_sites[i] << endl;
    for(int o = 0; o < 4; o++) {
      sbs.push_back(zeros(root_sites[i], o));
      cout << "eval at " << root_sites[i] << " " <<  sbs.back()(root_sites[i]) << endl;
    }
  }
}


void generate_random_sbasis(vector<Geom::SBasis> &sbs) {
  MTRand rnd(1);
  for(int i = 0; i < 100; i++) {
    SBasis sb;
    for(int j = int(rnd.rand()*4); j >= 0; j--) {
      sb.push_back(Linear(rnd.randNorm(), rnd.randNorm()));
    }
    //cout << sb << endl;
    sbs.push_back(sb);
  }
}

void 
print_sbs(vector<Geom::SBasis> &sbs) {
  for(unsigned i = 0; i < sbs.size(); i++) {
    cout << sbs[i] << endl;
  }
}

void 
try_roots(vector<Geom::SBasis> &sbs) {
  for(unsigned i = 0; i < sbs.size(); i++) {
    vector<double> rs = roots(sbs[i]);
    cout << sbs[i] << endl;
    for(unsigned ri = 0; ri < rs.size(); ri++) {
      cout << rs[ri] << " ";
    }
    cout << endl;
  }
}

#ifndef degenerate_imported

int main(int argc, char**argv) {
  vector<Geom::SBasis> sbs;
  //generate_degenerate_cases(sbs);
  generate_random_sbasis(sbs);
  //print_sbs(sbs);
  try_roots(sbs);
}
#endif
