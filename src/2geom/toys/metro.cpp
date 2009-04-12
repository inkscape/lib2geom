/** Generates approximate metromap lines
 * Copyright njh
 * Copyright Tim Dwyer
 * Published in ISVC 2008, Las Vegas, Nevada, USA
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include <gtk/gtk.h>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <cairo-pdf.h>
#include <2geom/point.h>
#include <2geom/geom.h>
#include <2geom/toys/toy-framework-2.h>

using std::string;
using std::vector;
using std::pair;
using std::make_pair;
using std::ifstream;
using std::map;
using std::cout;
using std::endl;
using namespace Geom;

vector<vector<Point> > paths;

class sufficient_stats{
public:
    double Sx, Sy, Sxx, Syy, Sxy;
    double n;
    
    sufficient_stats() : Sx(0), Sy(0), Sxx(0), Syy(0), Sxy(0), n(0) {}
    void
    operator+=(Point p) {
        Sx += p[0];
        Sy += p[1];
        Sxx += p[0]*p[0];
        Syy += p[1]*p[1];
        Sxy += p[0]*p[1];
        n += 1.0;
    }
    void
    operator-=(Point p) {
        Sx -= p[0];
        Sy -= p[1];
        Sxx -= p[0]*p[0];
        Syy -= p[1]*p[1];
        Sxy -= p[0]*p[1];
        n -= 1.0;
    }
    /*** What is the best regression we can do? . */
    Point best_normal() {
        return rot90(unit_vector(Point(n*Sxx - Sx*Sx,
                                n*Sxy - Sx*Sy)));
    }
    /*** Compute the best line for the points, given normal. */
    double best_line(Point normal, const double dist,
                     double & mean) {
        mean = (normal[0]*Sx + normal[1]*Sy);
        return normal[0]*normal[0]*Sxx
            + normal[1]*normal[1]*Syy
            + 2*normal[0]*normal[1]*Sxy
            - 2*dist*mean + n*dist*dist;
    }
    /*** Returns the index to the angle in angles that has the line of best fit
     * passing through mean */
    unsigned best_schematised_line(vector<Point>& angles, Point p,
                                   double & /*mean*/, double & cost) {
        cost = DBL_MAX;
        unsigned bestAngle;
        for(unsigned i=0;i<angles.size();i++) {
            Point n = unit_vector(rot90(angles[i]));
            double dist = dot(n, p);
            double mean;
            double bl = best_line(n, dist, mean);
            if(bl < cost) {
                cost = bl;
                bestAngle = i;
            }
        }
        return bestAngle;
    }
    /*** Compute the best line for the points, given normal. */
    double best_angled_line(Point normal,
                            double & mean) {
        mean = (normal[0]*Sx + normal[1]*Sy);
        double dist = mean/n;
        return normal[0]*normal[0]*Sxx
            + normal[1]*normal[1]*Syy
            + 2*normal[0]*normal[1]*Sxy
            - 2*dist*mean + n*dist*dist;
    }
};

sufficient_stats
operator+(sufficient_stats const & a, sufficient_stats const &b) {
    sufficient_stats ss;
    ss.Sx = a.Sx + b.Sx;
    ss.Sy = a.Sy + b.Sy;
    ss.Sxx = a.Sxx + b.Sxx;
    ss.Sxy = a.Sxy + b.Sxy;
    ss.Syy = a.Syy + b.Syy;
    ss.n = a.n + b.n;
    return ss;
}

sufficient_stats
operator-(sufficient_stats const & a, sufficient_stats const &b) {
    sufficient_stats ss;
    ss.Sx = a.Sx - b.Sx;
    ss.Sy = a.Sy - b.Sy;
    ss.Sxx = a.Sxx - b.Sxx;
    ss.Sxy = a.Sxy - b.Sxy;
    ss.Syy = a.Syy - b.Syy;
    ss.n = a.n - b.n;
    return ss;
}

inline std::ostream &operator<< (std::ostream &out_file, const sufficient_stats &s) {
    out_file << "Sx: " << s.Sx
             << "Sy: " << s.Sy
             << "Sxx: " << s.Sxx
             << "Sxy: " << s.Sxy
             << "Syy: " << s.Syy
             << "n: " << s.n;
    return out_file;
}



class fit{
public:
    vector<Point> input;
    vector<Point> solution;

    vector<pair<Point,Point> > lines;
    vector<int> thickness;
    
    void
    draw(cairo_t* cr) {
        /*
        if(solution.size() > 1) {
            //cairo_set_line_width (cr, 1);
            cairo_move_to(cr, solution[0]);
            for(unsigned i = 1; i < solution.size(); i++) {
                cairo_line_to(cr, solution[i]);
            }
        }
        */
        //cairo_stroke(cr);
        for(unsigned i = 0;i<lines.size();i++) {
            if(thickness.size()>i) {
                cairo_set_line_width (cr, thickness[i]);
            }
            cairo_move_to(cr, lines[i].first);
            cairo_line_to(cr, lines[i].second);
            cairo_stroke(cr);
        }
    }
    
    void endpoints() {
        solution.push_back(input[0]);
        solution.push_back(input.back());
    }
    
    void arbitrary();
    void linear_reg();
    
    // sufficient stats from start to each point
    vector<sufficient_stats> ac_ss;
    
    /*** Compute the least squares error for a line between two points on the line. */
    double measure(unsigned from, unsigned to, double & mean) {
        sufficient_stats ss = ac_ss[to+1] - ac_ss[from];
        
        Point n = unit_vector(rot90(input[to] - input[from]));
        double dist = dot(n, input[from]);
        return ss.best_line(n, dist, mean);
    }
    
    /*** Compute the best line for the points, given normal. */
    double best_angled_line(unsigned from, unsigned to,
                            Point n,
                            double & mean) {
        sufficient_stats ss = ac_ss[to+1] - ac_ss[from];
        return ss.best_angled_line(n, mean);
    }
    
    double simple_measure(unsigned from, unsigned to, double & mean) {
        Point n = unit_vector(rot90(input[to] - input[from]));
        double dist = dot(n, input[from]); // distance from origin
        double error = 0;
        mean = 0;
        for(unsigned l = from; l <= to; l++) {
            double d = dot(input[l], n) - dist;
            mean += dot(input[l], n);
            error += d*d;
        }
        return error;
    }

    void simple_dp();
    void C_simple_dp();

    void C_endpoint();
    
    vector<Geom::Point> angles;
    fit(vector<Point> const & in)
        :input(in) {
            /*
        Geom::Point as[] = {Point(0,1), Point(1,0), Point(1,1), Point(1,-1)};
        for(unsigned c = 0; c < 4; c++) {
            angles.push_back(unit_vector(as[c]));
        }
        */
        sufficient_stats ss;
        ss.Sx = ss.Sy = ss.Sxx = ss.Sxy = ss.Syy = 0;
        ac_ss.push_back(ss);
        for(unsigned l = 0; l < input.size(); l++) {
            ss += input[l];
            ac_ss.push_back(ss);
        }
    }
    
    class block{
    public:
        unsigned next;
        unsigned angle;
        sufficient_stats ss;
        double cost;
    };
    vector<block> blocks;
    void test();
    void merging_version();
    void schematised_merging(unsigned number_of_directions);

    double get_block_line(block& b, Point& d, Point& n, Point& c) {
        n = unit_vector(rot90(d));
        c = Point(b.ss.Sx/b.ss.n,b.ss.Sy/b.ss.n);
        return 0;
    }
};

class build_bounds{
public:
    int total_n;
    Point starting[2];
    Rect combined;
    void add_point(Point const &P) {
        if(total_n < 2) {
            starting[total_n] = P;
            total_n += 1;
            if(total_n == 2)
                combined = Rect(starting[0], starting[1]);
        } else {
            combined.expandTo(P);
            total_n += 1;
        }
    }
    OptRect result() const {
        if(total_n > 1)
            return combined;
        return OptRect();
    }
    build_bounds() : total_n(0) {}
};

/**
 * returns a point which is portionally between topleft and bottomright in the same way that p was
 * in src.
 */
Point map_point(Point p, Rect src, Point topleft, Point bottomright) {
    p -= src.min();
    p[0] /= src[0].extent();
    p[1] /= src[1].extent();
    //cout << p << endl;
    return Point(topleft[0]*(1-p[0]) + bottomright[0]*p[0],
                 topleft[1]*(1-p[1]) + bottomright[1]*p[1]);
}

void parse_data(vector<vector<Point> >& paths, 
                string location_file_name,
                string path_file_name) {
    ifstream location_file(location_file_name.c_str()), 
        path_file(path_file_name.c_str());
    string id,sx,sy;
    map<string,Point> idlookup;
    build_bounds bld_bounds;
    while (getline(location_file,id,','))
    {
        getline(location_file,sx,',');
        getline(location_file,sy,'\n');
        char *e;
        // negative for coordinate system
        Point p(strtod(sx.c_str(),&e), strtod(sy.c_str(),&e));
	//cout << id << p << endl;
        idlookup[id] = p;
    }

    
    string l;
    while (getline(path_file,l,'\n')) {
        vector<Point> ps;
        if(l.size() < 2) continue; // skip blank lines
        if(l.find(":",0)!=string::npos) continue; // skip labels
        string::size_type p=0,q;
        while((q=l.find(",",p))!=string::npos || (p < l.size() && (q = l.size()))) {
            id = l.substr(p,q-p);
            //cout << id << endl;
	    Point pt = 2*idlookup[id];
            //cout << pt << endl;
            bld_bounds.add_point(pt);
            ps.push_back(pt);
            p=q+1;
        }
        paths.push_back(ps);
        //cout << "*******************************************" << endl;
    }
    Rect bounds = *bld_bounds.result();
    //cout << bounds.min() << " - " << bounds.max() << endl;
    for(unsigned i = 0; i < paths.size(); i++) {
        for(unsigned j = 0; j < paths[i].size();j++) {
            paths[i][j] = map_point(paths[i][j], bounds, 
                                    Point(0,512), Point(512*bounds[0].extent()/bounds[1].extent(),0));
        }
    }
    /*
    for(map<string,Point>::iterator it = idlookup.begin();
        it != idlookup.end(); it++) {
        (*it).second = map_point((*it).second, bounds, 
                                 Point(0,0), Point(100,100));
                                 //Point(0, 512), Point(512,0));
        cout << (*it).first << ":" << (*it).second <<  endl;
        }*/
        /*
    unsigned biggest = 0, biggest_i;
    for(unsigned i=0;i<paths.size();i++) {
        vector<Point> ps=paths[i];
        if(ps.size()>biggest) {
            biggest_i=i;
            biggest = ps.size();
        }
        for(unsigned j=0;j<ps.size();j++) {
            double x=ps[j][0], y=ps[j][1];
            cout << "(" << x << "," << y << ")" << ",";
        }
        cout << endl;
    }
        */
}

void extremePoints(vector<Point> const & pts, Point const & dir, 
                   Point & min, Point & max) {
    double minProj = DBL_MAX, maxProj = -DBL_MAX;
    for(unsigned i=0;i<pts.size();i++) {
        double p = dot(pts[i],dir);
        if(p < minProj) {
            minProj = p;
            min = pts[i];
        }
        if(p > maxProj) {
            maxProj = p;
            max = pts[i];
        }
    }
} 

void fit::test() {
    sufficient_stats ss;
    const unsigned N = input.size();
    for(unsigned i = 0; i < N; i++) {
        ss += input[i];
    }
    double best_bl = DBL_MAX;
    unsigned best;
    for(unsigned i=0;i<angles.size();i++) {
        Point n = unit_vector(rot90(angles[i]));
        double dist = dot(n, input[0]);
        double mean;
        double bl = ss.best_line(n, dist, mean);
        if(bl < best_bl) {
            best = i;
            best_bl = bl;
        }
        mean/=N;
        Point d = angles[i];
        Point a = mean*n;
        Point min, max;
        extremePoints(input,d,min,max);
        Point b = dot(min,d)*d;
        Point c = dot(max,d)*d;
        Point start = a+b;
        Point end = a+c;
        lines.push_back(make_pair(start,end));
        thickness.push_back(1);
    }
    thickness[best] = 4;
}

void fit::schematised_merging(unsigned number_of_directions) {
    const double link_cost = 0;
    const unsigned N = input.size()-1;
    blocks.resize(N);
    for(unsigned i = 0; i<number_of_directions ; i++) {
        double t = M_PI*i/float(number_of_directions);
        angles.push_back(Point(cos(t),sin(t)));
    }
    // pairs
    for(unsigned i = 0; i < N; i++) {
        block b;
        sufficient_stats ss;
        ss += input[i];
        ss += input[i+1];
        b.ss = ss;
        double mean, newcost;
        b.angle = ss.best_schematised_line(angles, input[i], mean, newcost);
        b.cost = link_cost + newcost;
        b.next = i+1;
        blocks[i] = b;
        //std::cout << ss 
        //<< std::endl;
    }
    
    // merge(a,b)
    while(N>1)
    {
        block best_block;
        unsigned best_idx = 0;
        double best = 0;
        unsigned beg = 0;
        unsigned middle = blocks[beg].next;
        unsigned end = blocks[middle].next;
        while(middle < N) {
            sufficient_stats ss = blocks[beg].ss + blocks[middle].ss;
            //ss -= input[middle];
            double mean, newcost;
            unsigned bestAngle = ss.best_schematised_line(angles, input[beg], mean, newcost);
            double deltaCost = -link_cost - blocks[beg].cost - blocks[middle].cost 
                + newcost;
            /*std::cout << beg << ", "
                      << middle << ", "
                      << end << ", "
                      << deltaCost <<"; "
                      << newcost <<"; "
                      << mean << ": "
                      << ss 
                      << std::endl;*/
            //if(deltaCost < best) {
            if(blocks[beg].angle==blocks[middle].angle) {
                best = deltaCost;
                best = -1;
                best_idx = beg;
                best_block.ss = ss;
                best_block.cost = newcost;
                best_block.next = end;
                best_block.angle = bestAngle;
            }
            beg = middle;
            middle = end;
            end = blocks[end].next;
        }
        if(best < 0)
            blocks[best_idx] = best_block;
        else // no improvement possible
            break;
    }
    {
        solution.resize(0); // empty list
        unsigned beg = 0;
        unsigned prev = 0;
        while(beg < N) {
            block b = blocks[beg];
            {
                Point n, c;
                Point n1, c1;
                Point d = angles[b.angle];
                get_block_line(b,d,n,c);
                Point start = c, end = c+10*angles[b.angle];
                Line ln = Line::fromNormalDistance(n, dot(c,n));
                if(beg==0) {
                    //start = intersection of b.line and 
                    //        line through input[0] orthogonal to b.line
                    OptCrossing c = intersection(ln,
                                            Line::fromNormalDistance(d, dot(d,input[0])));
                    assert(c);
                    start = ln.pointAt(c->ta);
                    //line_intersection(n, dot(c,n), d, dot(d,input[0]), start);
                } else {
                    //start = intersection of b.line and blocks[prev].line
                    block p = blocks[prev];
                    if(b.angle!=p.angle) {
                        get_block_line(p,angles[p.angle],n1,c1);
                        //line_intersection(n, dot(c,n), n1, dot(c1,n1), start);
                        OptCrossing c = intersection(ln,
                                                   Line::fromNormalDistance(n1, dot(c1,n1)));
                        assert(c);
                        start = ln.pointAt(c->ta);
                    }
                }

                if (b.next < N) {
                    //end = intersection of b.line and blocks[b.next].line
                    block next = blocks[b.next];
                    if(b.angle!=next.angle) {
                        get_block_line(next,angles[next.angle],n1,c1);
                        //line_intersection(n, dot(c,n), n1, dot(c1,n1), end);
                        OptCrossing c = intersection(ln,
                                                     Line::fromNormalDistance(n1, dot(c1,n1)));
                        assert(c);
                        end = ln.pointAt(c->ta);
                    }
                } else {
                    //end = intersection of b.line and
                    //      line through input[N-1] orthogonal to b.line
                    //line_intersection(n, dot(c,n), d, dot(d,input[N]), end);
                    OptCrossing c = intersection(ln,
                                            Line::fromNormalDistance(d, dot(d,input[N])));
                    assert(c);
                    end = ln.pointAt(c->ta);
                }                
                lines.push_back(make_pair(start,end));
            }
            prev = beg;
            beg = b.next;
        }
    }
}
void fit::merging_version() {
    const double link_cost = 100;
    const unsigned N = input.size();
    blocks.resize(N);
    // pairs
    for(unsigned i = 0; i < N; i++) {
        block b;
        sufficient_stats ss;
        ss.Sx = ss.Sy = ss.Sxx = ss.Sxy = ss.Syy = 0;
        ss.n = 0;
        ss += input[i];
        ss += input[i+1];
        b.ss = ss;
        b.cost = link_cost;
        b.next = i+1;
        blocks[i] = b;
        //std::cout << ss 
        //<< std::endl;
    }
    
    // merge(a,b)
    while(1)
    {
        block best_block;
        unsigned best_idx = 0;
        double best = 0;
        unsigned beg = 0;
        unsigned middle = blocks[beg].next;
        unsigned end = blocks[middle].next;
        while(end != N) {
            sufficient_stats ss = blocks[beg].ss + blocks[middle].ss;
            ss -= input[middle];
            double mean;
            Point normal = unit_vector(rot90(input[end] - input[beg]));
            double dist = dot(normal, input[beg]);
            double newcost = ss.best_line(normal, dist, mean);
            double deltaCost = -link_cost - blocks[beg].cost - blocks[middle].cost 
                + newcost;
            /*std::cout << beg << ", "
                      << middle << ", "
                      << end << ", "
                      << deltaCost <<"; "
                      << newcost <<"; "
                      << mean << ": "
                      << ss 
                      << std::endl;*/
            if(deltaCost < best) {
                best = deltaCost;
                best_idx = beg;
                best_block.ss = ss;
                best_block.cost = newcost;
                best_block.next = end;
            }
            beg = middle;
            middle = end;
            end = blocks[end].next;
        }
        if(best < 0)
            blocks[best_idx] = best_block;
        else // no improvement possible
            break;
    }
    {
        solution.resize(0); // empty list
        unsigned beg = 0;
        while(beg != N) {
            solution.push_back(input[beg]);
            beg = blocks[beg].next;
        }
    }
}


void fit::arbitrary() {
    /*solution.resize(input.size());
      copy(input.begin(), input.end(), solution.begin());*/
    // normals
    
    double best_error = INFINITY;
    double best_mean = 0;
    unsigned best_angle = 0;
    for(unsigned i = 0; i < angles.size(); i++) {
        Point angle = angles[i];
        double mean = 0;
        double error = 0;
        for(unsigned l = 0; l < input.size(); l++) {
            mean += dot(input[i], angle);
        }
        mean /= input.size();
        for(unsigned l = 0; l < input.size(); l++) {
            double d = dot(input[i], angle) - mean;
            error += d*d;
        }
        if(error < best_error) {
            best_mean = mean;
            best_error = error;
            best_angle = i;
        }
    }
    Point angle = angles[best_angle];
    solution.push_back(angle*best_mean + dot(input[0], rot90(angle))*rot90(angle));
    solution.push_back(angle*best_mean + dot(input.back(), rot90(angle))*rot90(angle));
}

class reg_line{
public:
    Point parallel, centre, normal;
    double Sr, Srr;
    unsigned n;
};

template<class T>
reg_line
line_best_fit(T b, T e) {
    double Sx = 0,
        Sy = 0, 
        Sxx = 0, 
        Syy = 0, 
        Sxy = 0;
    unsigned n = e - b;
    reg_line rl;
    rl.n = n;
    for(T p = b; p != e; p++) {
        Sx += (*p)[0];
        Sy += (*p)[1];
        Sxx += (*p)[0]*(*p)[0];
        Syy += (*p)[1]*(*p)[1];
        Sxy += (*p)[0]*(*p)[1];
    }
    
    rl.parallel = unit_vector(Point(n*Sxx - Sx*Sx,
                 n*Sxy - Sx*Sy));
    rl.normal = rot90(rl.parallel);
    rl.centre = Point(Sx/n, Sy/n);
    rl.Sr = rl.Srr = 0;
    for(T p = b; p != e; p++) {
        double r = dot(rl.parallel, (*p) - rl.centre);
        rl.Sr += fabs(r);
        rl.Srr += r*r;
    }
    return rl;
}

void fit::linear_reg() {
    reg_line rl = line_best_fit(input.begin(),
                  input.end());
    solution.push_back(rl.centre + dot(rl.parallel, input[0] - rl.centre)*rl.parallel);
    solution.push_back(rl.centre + dot(rl.parallel, input.back() - rl.centre)*rl.parallel);
}

void fit::simple_dp() {
    const unsigned N = input.size();
    vector<unsigned> prev(N);
    vector<double> penalty(N);
    const double bend_pen = 100;
    
    for(unsigned i = 1; i < input.size(); i++) {
        double mean;
        double best = measure(0, i, mean);
        unsigned best_prev = 0;
        for(unsigned j = 1; j < i; j++) {
            double err = penalty[j] + bend_pen + measure(j, i, mean);
            if(err < best) {
                best = err;
                best_prev = j;
            }
        }
        penalty[i] = best;
        prev[i] = best_prev;
    }
    
    unsigned i = prev.size()-1;
    while(i > 0) {
        solution.push_back(input[i]);
        i = prev[i];
    }
    solution.push_back(input[i]);
    reverse(solution.begin(), solution.end());
}

void fit::C_endpoint() {
    const unsigned N = input.size();
    
    double best_mean;
    double best = best_angled_line(0, N-1, angles[0], best_mean);
    unsigned best_dir = 0;
    for(unsigned c = 1; c < angles.size(); c++) {
        double m;
        double err = best_angled_line(0, N-1, angles[c], m);
        if(err < best) {
            best = err;
            best_mean = m;
            best_dir = c;
        }

    }
    Point dir = angles[best_dir];
    Point dirT = rot90(dir);
    Point centre = dir*best_mean/N;
    
    solution.push_back(centre + dot(dirT, input[0] - centre)*dirT);
    solution.push_back(centre + dot(dirT, input.back() - centre)*dirT);
}

void fit::C_simple_dp() {
    const unsigned N = input.size();
    
    vector<int> prev(N);
    vector<double> penalty(N);
    vector<unsigned> dir(N);
    vector<double> mean(N);
    const double bend_pen = 0;
    
    for(unsigned i = 1; i < input.size(); i++) {
        double best_mean;
        double best = best_angled_line(0, i, angles[0], best_mean);
        unsigned best_prev = 0;
        unsigned best_dir = 0;
        for(unsigned c = 1; c < angles.size(); c++) {
            double m;
            double err = best_angled_line(0, i, angles[c], m);
            if(err < best) {
                best = err;
                best_mean = m;
                best_dir = c;
                best_prev = 0;
            }

        }
        for(unsigned j = 1; j < i; j++) {
            for(unsigned c = 0; c < angles.size(); c++) {
                double m;
                if(c == dir[j])
                    continue;
                double err = penalty[j] + bend_pen + 
                    best_angled_line(j, i, angles[c], m);
                if(err < best) {
                    best = err;
                    best_mean = m;
                    best_dir = c;
                    best_prev = j;
                }

            }
        }
        penalty[i] = best;
        prev[i] = best_prev;
        dir[i] = best_dir;
        mean[i] = best_mean;
    }
    
    prev[0] = -1;
    unsigned i = prev.size()-1;
    unsigned pi = i;
    while(i > 0) {
        Point bdir = angles[dir[i]];
        Point bdirT = rot90(bdir);
        Point centre = bdir*mean[i]/N;
        solution.push_back(centre + dot(bdirT, input[i] - centre)*bdirT);
        solution.push_back(centre + dot(bdirT, input[pi] - centre)*bdirT);
        pi = i;
        i = prev[i];
    }
    /*Point a = angles[dir[i]];
    Point aT = rot90(a);
    solution.push_back(a*mean[i] + 
    dot(input[i], aT)*aT);*/
    reverse(solution.begin(), solution.end());
}






class MetroMap: public Toy {
public:
  vector<PointSetHandle> metro_lines;
  PointHandle directions;
  
  virtual bool should_draw_numbers() { return false; }
  
  virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
    double slider_margin = 20;
    double slider_top = 20;
    double slider_bot = 200;
    directions.pos[X] = slider_margin;
    if (directions.pos[Y]<slider_top) directions.pos[Y] = slider_top; 
    if (directions.pos[Y]>slider_bot) directions.pos[Y] = slider_bot; 

    unsigned num_directions = 2 + 15*(slider_bot-directions.pos[Y])/(slider_bot-slider_top);

    cairo_move_to(cr,Geom::Point(slider_margin,slider_bot));
    cairo_line_to(cr,Geom::Point(slider_margin,slider_top));
    cairo_set_line_width(cr,.5);
    cairo_set_source_rgba (cr, 0., 0.3, 0., 1.);
    cairo_stroke(cr);

    cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
    cairo_set_line_width (cr, 1);
    cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
    cairo_set_line_width (cr, 1);
    
    unsigned N=  paths.size();
    for(unsigned i=0;i<N;i++) {
      double R,G,B;
      convertHSVtoRGB(360.*double(i)/double(N),1,0.75,R,G,B);
      metro_lines[i].rgb[0] = R;
      metro_lines[i].rgb[1] = G;
      metro_lines[i].rgb[2] = B;
      cairo_set_source_rgba (cr, R, G, B, 0.8);
      fit f(metro_lines[i].pts);
      f.schematised_merging(num_directions);
      f.draw(cr);
      cairo_stroke(cr);
    }
    cairo_set_source_rgba (cr, 0., 0., 0, 1);
    {
      PangoLayout* layout = pango_cairo_create_layout (cr);
      pango_layout_set_text(layout, 
			    notify->str().c_str(), -1);

      PangoFontDescription *font_desc = pango_font_description_new();
      pango_font_description_set_family(font_desc, "Sans");
      const unsigned size_px = 10;
      pango_font_description_set_absolute_size(font_desc, size_px * 1024.0);
      pango_layout_set_font_description(layout, font_desc);
      PangoRectangle logical_extent;
      pango_layout_get_pixel_extents(layout,
				     NULL,
				     &logical_extent);
      cairo_move_to(cr, 0, height-logical_extent.height);
      pango_cairo_show_layout(cr, layout);
    }
    Toy::draw(cr, notify, width, height, save,timer_stream);
  }

    void first_time(int argc, char** argv) {
        string location_file_name("data/london-locations.csv");
        string path_file_name("data/london.txt");
        if(argc > 2) {
            location_file_name = argv[1];
            path_file_name = argv[2];
        }
        cout << location_file_name << ", " << path_file_name << endl;
        parse_data(paths, location_file_name, path_file_name);
        for(unsigned i=0;i<paths.size();i++) {
            metro_lines.push_back(PointSetHandle());
            for(unsigned j=0;j<paths[i].size();j++) {
                metro_lines.back().push_back(paths[i][j]);
            }
        }
        for(unsigned i=0;i<metro_lines.size();i++) {
            handles.push_back(&metro_lines[i]);
        }
        handles.push_back(&directions);
    }

    MetroMap() {
  }

};





int main(int argc, char **argv) {
    init(argc, argv, new MetroMap());

    return 0;
}

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

