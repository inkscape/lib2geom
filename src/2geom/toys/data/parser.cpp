#include <string> 
#include <fstream> 
#include <iostream> 
#include <vector> 
#include <map>
using namespace std;
typedef pair<double,double> Point;
int main(void)
{
  ifstream location_file("london-locations.csv"), path_file("london.txt");
  string id,sx,sy;
  map<string,Point> idlookup;
  while (getline(location_file,id,','))
  {
    getline(location_file,sx,',');
    getline(location_file,sy,'\n');
    char *e;
    double x = strtod(sx.c_str(),&e), y = strtod(sy.c_str(),&e);
    cout << id << " (" << x << "," << y << ")"<< endl;
    idlookup[id]=make_pair(x,y);
  }
  string l;
  vector<vector<Point> > paths;
  while (getline(path_file,l,'\n')) {
    vector<Point> ps;
    if(l.size() < 2) continue; // skip blank lines
    if(l.find(":",0)!=string::npos) continue; // skip labels
    string::size_type p=0,q;
    while((q=l.find(",",p))!=string::npos || p < l.size() && (q = l.size()-1)) {
      id = l.substr(p,q-p);
      cout << id << ",";
      ps.push_back(idlookup[id]);
      p=q+1;
    }
    paths.push_back(ps);
    cout << "*******************************************" << endl;
  }
  for(unsigned i=0;i<paths.size();i++) {
    vector<Point> ps=paths[i];
    for(unsigned j=0;j<ps.size();j++) {
      double x=ps[j].first, y=ps[j].second;
      cout << "(" << x << "," << y << ")" << ",";
    }
    cout << endl;
  }
  return(0);
}
