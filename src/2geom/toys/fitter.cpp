/** Generates approximate metromap lines
 * Copyright njh
 * Copyright Tim Dwyer
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
#include "sbasis.h"
#include "point.h"
#include "interactive-bits.h"
#include "geom.h"

using std::string;
using std::vector;
using std::pair;
using std::make_pair;
using std::ifstream;
using std::map;
using std::cout;
using std::endl;
using namespace Geom;

static GtkWidget *canvas;


Linear z0(0.5,1.);

std::vector<Geom::Point*> handles;
Geom::Point *selected_handle;
Geom::Point old_handle_pos;
Geom::Point old_mouse_point;

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
    void schematised_merging();

    double get_block_line(block& b, Point& d, Point& n, Point& c) {
        n = unit_vector(rot90(d));
        c = Point(b.ss.Sx/b.ss.n,b.ss.Sy/b.ss.n);
        return 0;
    }
};

void parse_data(vector<vector<Point> >& paths) {
    ifstream location_file("data/london-locations.csv"), path_file("data/london.txt");
    string id,sx,sy;
    map<string,Point> idlookup;
    while (getline(location_file,id,','))
    {
        getline(location_file,sx,',');
        getline(location_file,sy,'\n');
        char *e;
        double x = strtod(sx.c_str(),&e), y = strtod(sy.c_str(),&e);
        //cout << id << " (" << x << "," << y << ")"<< endl;
        idlookup[id]=Point(x,y);
    }
    string l;
    while (getline(path_file,l,'\n')) {
        vector<Point> ps;
        if(l.size() < 2) continue; // skip blank lines
        if(l.find(":",0)!=string::npos) continue; // skip labels
        string::size_type p=0,q;
        while((q=l.find(",",p))!=string::npos || p < l.size() && (q = l.size()-1)) {
            id = l.substr(p,q-p);
            //cout << id << ",";
            ps.push_back(2*idlookup[id]);
            p=q+1;
        }
        paths.push_back(ps);
        //cout << "*******************************************" << endl;
    }
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

void fit::schematised_merging() {
    const double link_cost = 0;
    const unsigned N = input.size()-1;
    blocks.resize(N);
    unsigned C=4;
    for(unsigned i = 0; i<C ; i++) {
        double t = M_PI*i/float(C);
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
                if(beg==0) {
                    //start = intersection of b.line and 
                    //        line through input[0] orthogonal to b.line
                    line_intersection(n, dot(c,n), d, dot(d,input[0]), start);
                } else {
                    //start = intersection of b.line and blocks[prev].line
                    block p = blocks[prev];
                    if(b.angle!=p.angle) {
                        get_block_line(p,angles[p.angle],n1,c1);
                        line_intersection(n, dot(c,n), n1, dot(c1,n1), start);
                    }
                }

                if (b.next < N) {
                    //end = intersection of b.line and blocks[b.next].line
                    block next = blocks[b.next];
                    if(b.angle!=next.angle) {
                        get_block_line(next,angles[next.angle],n1,c1);
                        line_intersection(n, dot(c,n), n1, dot(c1,n1), end);
                    }
                } else {
                    //end = intersection of b.line and
                    //      line through input[N-1] orthogonal to b.line
                    line_intersection(n, dot(c,n), d, dot(d,input[N]), end);
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

// H in [0,360)
// S, V, R, G, B in [0,1]
void convertHSVtoRGB(const double H, const double S, const double V,
                     double& R, double& G, double& B) {
    int Hi = int(floor(H/60.)) % 6;
    double f = H/60. - Hi;
    double p = V*(1-S);
    double q = V*(1-f*S);
    double t = V*(1-(1-f)*S);
    switch(Hi) {
        case 0: R=V, G=t, B=p; break;
        case 1: R=q, G=V, B=p; break;
        case 2: R=p, G=V, B=t; break;
        case 3: R=p, G=q, B=V; break;
        case 4: R=t, G=p, B=V; break;
        case 5: R=V, G=p, B=q; break;
    }
}
void draw_everything(cairo_t* cr, int /*width*/, int height) {
    std::ostringstream notify;
    cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
    cairo_set_line_width (cr, 1);
    cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
    cairo_set_line_width (cr, 1);
    
    unsigned N= paths.size();
    for(unsigned i=0;i<N;i++) {
        double R,G,B;
        convertHSVtoRGB(360.*double(i)/double(N),0.7,1.,R,G,B);
        cairo_set_source_rgba (cr, R, G, B, 0.8);
        fit f(paths[i]);
        f.schematised_merging();
        f.draw(cr);
        for(unsigned j = 0; j < paths[i].size(); j++) {
            draw_circ(cr, paths[i][j]);
        }
        cairo_stroke(cr);
    }
    cairo_set_source_rgba (cr, 0., 0., 0, 1);
    {
        PangoLayout* layout = pango_cairo_create_layout (cr);
        pango_layout_set_text(layout, 
                              notify.str().c_str(), -1);

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
}
static gboolean
expose_event(GtkWidget *widget, GdkEventExpose */*event*/, gpointer /*data*/)
{
    cairo_t *cr = gdk_cairo_create (widget->window);
    
    int width = 256;
    int height = 256;
    gdk_drawable_get_size(widget->window, &width, &height);
    draw_everything(cr,width,height);
    
    cairo_destroy (cr);
    
    return TRUE;
}

static void handle_mouse(GtkWidget* widget) {
    gtk_widget_queue_draw (widget);
}

static gint mouse_motion_event(GtkWidget* widget, GdkEventMotion* e, gpointer /*data*/) {
    Geom::Point mouse(e->x, e->y);
    
    if(e->state & (GDK_BUTTON1_MASK | GDK_BUTTON3_MASK)) {
        if(selected_handle) {
            *selected_handle = mouse - old_handle_pos;
            
        }
        handle_mouse(widget);
    }

    if(e->state & (GDK_BUTTON2_MASK)) {
        gtk_widget_queue_draw(widget);
    }
    
    old_mouse_point = mouse;

    return FALSE;
}

static gint mouse_event(GtkWidget* window, GdkEventButton* e, gpointer /*data*/) {
    Geom::Point mouse(e->x, e->y);
    if(e->button == 1 || e->button == 3) {
        for(unsigned i = 0; i < handles.size(); i++) {
            if(Geom::distance(mouse, *handles[i]) < 5) {
                selected_handle = handles[i];
                old_handle_pos = mouse - *handles[i];
            }
        }
        handle_mouse(window);
    } else if(e->button == 2) {
        gtk_widget_queue_draw(window);
    }
    old_mouse_point = mouse;

    return FALSE;
}

static gint mouse_release_event(GtkWidget* /*window*/, GdkEventButton* /*e*/, gpointer /*data*/) {
    selected_handle = 0;
    return FALSE;
}

static gint key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer) {
    gint ret = FALSE;
    if (event->keyval == ' ') {
        ret = TRUE;
    } else if (event->keyval == 'l') {
        ret = TRUE;
    } else if (event->keyval == 'q') {
        exit(0);
    } else if (event->keyval == 's') {
        int width = 256;
        int height = 256;
        gdk_drawable_get_size(widget->window, &width, &height);
        cairo_surface_t* cr_s = cairo_pdf_surface_create("metromap.pdf",
                                                         width,
                                                         height);

        cairo_t* cr = cairo_create(cr_s);
        draw_everything(cr,width,height);
        cairo_show_page(cr);
        cairo_destroy (cr);
        cairo_surface_destroy (cr_s);

        ret = TRUE;
    }

    if (ret) {
        gtk_widget_queue_draw(widget);
    }

    return ret;
}

static gint
delete_event_cb(GtkWidget* /*window*/, GdkEventAny* /*e*/, gpointer /*data*/)
{
    gtk_main_quit();
    return FALSE;
}

static void
on_open_activate(GtkMenuItem */*menuitem*/, gpointer /*user_data*/) {
    //TODO: show open dialog, get filename
    
    char const *const filename = "banana.svgd";

    FILE* f = fopen(filename, "r");
    if (!f) {
        perror(filename);
        return;
    }
    
    gtk_widget_queue_draw(canvas); // globals are probably evil
}

static void
on_about_activate(GtkMenuItem */*menuitem*/, gpointer /*user_data*/) {
    
}

double uniform() {
    return double(rand()) / RAND_MAX;
}

int main(int argc, char **argv) {
    Geom::Point start_point(uniform()*100, uniform()*100);
    parse_data(paths);
/*
    vector<Point> ps;
    double step = 50;
    for(unsigned i = 0; i < 3; i++) {
        ps.push_back(start_point);
        start_point += Geom::Point(uniform()*step, 0.5*uniform()*step);
    }
    paths.push_back(ps);
    for(unsigned i = 0; i < 10; i++) {
        handles.push_back(start_point);
        start_point += Geom::Point(0.5*uniform()*step, uniform()*step);
        }*/
    for(unsigned i=0;i<paths.size();i++) {
        for(unsigned j=0;j<paths[i].size();j++) {
            handles.push_back(&paths[i][j]);
        }
    }
    gtk_init (&argc, &argv);
    
    gdk_rgb_init();
    GtkWidget *menubox;
    GtkWidget *menubar;
    GtkWidget *menuitem;
    GtkWidget *menu;
    GtkWidget *open;
    GtkWidget *separatormenuitem;
    GtkWidget *quit;
    GtkWidget *menuitem2;
    GtkWidget *menu2;
    GtkWidget *about;
    GtkAccelGroup *accel_group;

    accel_group = gtk_accel_group_new ();
 
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(window), "Fitter");

    menubox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (menubox);
    gtk_container_add (GTK_CONTAINER (window), menubox);

    menubar = gtk_menu_bar_new ();
    gtk_widget_show (menubar);
    gtk_box_pack_start (GTK_BOX (menubox), menubar, FALSE, FALSE, 0);

    menuitem = gtk_menu_item_new_with_mnemonic ("_File");
    gtk_widget_show (menuitem);
    gtk_container_add (GTK_CONTAINER (menubar), menuitem);

    menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);

    open = gtk_image_menu_item_new_from_stock ("gtk-open", accel_group);
    gtk_widget_show (open);
    gtk_container_add (GTK_CONTAINER (menu), open);

    separatormenuitem = gtk_separator_menu_item_new ();
    gtk_widget_show (separatormenuitem);
    gtk_container_add (GTK_CONTAINER (menu), separatormenuitem);
    gtk_widget_set_sensitive (separatormenuitem, FALSE);

    quit = gtk_image_menu_item_new_from_stock ("gtk-quit", accel_group);
    gtk_widget_show (quit);
    gtk_container_add (GTK_CONTAINER (menu), quit);

    menuitem2 = gtk_menu_item_new_with_mnemonic ("_Help");
    gtk_widget_show (menuitem2);
    gtk_container_add (GTK_CONTAINER (menubar), menuitem2);

    menu2 = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem2), menu2);

    about = gtk_menu_item_new_with_mnemonic ("_About");
    gtk_widget_show (about);
    gtk_container_add (GTK_CONTAINER (menu2), about);

    g_signal_connect ((gpointer) open, "activate",
                    G_CALLBACK (on_open_activate),
                    NULL);
    g_signal_connect ((gpointer) quit, "activate",
                    gtk_main_quit,
                    NULL);
    g_signal_connect ((gpointer) about, "activate",
                    G_CALLBACK (on_about_activate),
                    NULL);

    gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);


    gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, TRUE);

    gtk_signal_connect(GTK_OBJECT(window),
                       "delete_event",
                       GTK_SIGNAL_FUNC(delete_event_cb),
                       NULL);

    gtk_widget_push_visual(gdk_rgb_get_visual());
    gtk_widget_push_colormap(gdk_rgb_get_cmap());
    canvas = gtk_drawing_area_new();

    gtk_signal_connect(GTK_OBJECT (canvas),
                       "expose_event",
                       GTK_SIGNAL_FUNC(expose_event),
                       0);
    gtk_widget_add_events(canvas, (GDK_BUTTON_PRESS_MASK |
                                   GDK_BUTTON_RELEASE_MASK |
                                   GDK_KEY_PRESS_MASK    |
                                   GDK_POINTER_MOTION_MASK));
    gtk_signal_connect(GTK_OBJECT (canvas),
                       "button_press_event",
                       GTK_SIGNAL_FUNC(mouse_event),
                       0);
    gtk_signal_connect(GTK_OBJECT (canvas),
                       "button_release_event",
                       GTK_SIGNAL_FUNC(mouse_release_event),
                       0);
    gtk_signal_connect(GTK_OBJECT (canvas),
                       "motion_notify_event",
                       GTK_SIGNAL_FUNC(mouse_motion_event),
                       0);
    gtk_signal_connect(GTK_OBJECT(canvas),
                       "key_press_event",
                       GTK_SIGNAL_FUNC(key_release_event),
                       0);

    gtk_widget_pop_colormap();
    gtk_widget_pop_visual();

    //GtkWidget *vb = gtk_vbox_new(0, 0);


    //gtk_container_add(GTK_CONTAINER(window), vb);

    gtk_box_pack_start(GTK_BOX(menubox), canvas, TRUE, TRUE, 0);

    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);

    gtk_widget_show_all(window);

    /* Make sure the canvas can receive key press events. */
    GTK_WIDGET_SET_FLAGS(canvas, GTK_CAN_FOCUS);
    assert(GTK_WIDGET_CAN_FOCUS(canvas));
    gtk_widget_grab_focus(canvas);
    assert(gtk_widget_is_focus(canvas));

    //g_idle_add((GSourceFunc)idler, canvas);

    gtk_main();

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
