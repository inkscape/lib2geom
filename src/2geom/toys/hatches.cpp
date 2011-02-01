#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-geometric.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <cstdlib>
#include <vector>
using std::vector;
using namespace Geom;

#define SIZE 4
#define NB_SLIDER 8

//------------------------------------------------
// Some goodies to navigate through curve's levels.
//------------------------------------------------
struct LevelCrossing{
    Point pt;
    double t;
    bool sign;
    bool used;
};
struct LevelCrossingOrder {
    bool operator()(LevelCrossing a, LevelCrossing b) {
        return a.pt[Y] < b.pt[Y];
    }
};

typedef std::vector<LevelCrossing> LevelCrossings;

class LevelsCrossings: public std::vector<LevelCrossings>{
public:
    LevelsCrossings():std::vector<LevelCrossings>(){};
    LevelsCrossings(std::vector<std::vector<double> > const &times,
                    Piecewise<D2<SBasis> > const &f,
                    Piecewise<SBasis> const &dx){
        for (unsigned i=0; i<times.size(); i++){
            LevelCrossings lcs;
            for (unsigned j=0; j<times[i].size(); j++){
                LevelCrossing lc;
                lc.pt = f.valueAt(times[i][j]);
                lc.t = times[i][j];
                lc.sign = ( dx.valueAt(times[i][j])>0 );
                lc.used = false;
                lcs.push_back(lc);
            }
            std::sort(lcs.begin(), lcs.end(), LevelCrossingOrder());
            //TODO: reverse all "in" flag if we had the wrong orientation!
            push_back(lcs);
        }
    }
    void flipInOut(){
        for (unsigned i=0; i<size(); i++){
            for (unsigned j=0; j<(*this)[i].size(); j++){
                (*this)[i][j].sign = !(*this)[i][j].sign;
            }
        }
    }
    void findFirstUnused(unsigned &level, unsigned &idx){
        level = size();
        idx = 0;
        for (unsigned i=0; i<size(); i++){
            for (unsigned j=0; j<(*this)[i].size(); j++){
                if (!(*this)[i][j].used){
                    level = i;
                    idx = j;
                    return;
                }
            }
        }
    }
    //set indexes to point to the next point in the "snake walk"
    //follow_level's meaning: 
    //  0=yes upward
    //  1=no, last move was upward,
    //  2=yes downward
    //  3=no, last move was downward.
    void step(unsigned &level, unsigned &idx, int &direction){
        std::cout << "Entering step: "<<level<<","<<idx<<", dir="<< direction<<"\n";

        if ( direction % 2 == 0 ){
            if (direction == 0) {
                if ( idx >= (*this)[level].size()-1 || (*this)[level][idx+1].used ) {
                    level = size();
                    std::cout << "max end of level reached...\n";
                    return;
                }
                idx += 1;
            }else{
                if ( idx <= 0  || (*this)[level][idx-1].used ) {
                    level = size();
                    std::cout << "min end of level reached...\n";
                    return;
                }
                idx -= 1;
            }
            direction += 1;
            std::cout << "exit with: "<<level<<","<<idx<<", dir="<< direction<<"\n";
            return;
        }
        double t = (*this)[level][idx].t;
        double sign = ((*this)[level][idx].sign ? 1 : -1);
        double next_t = t;
        level += 1;
        direction = (direction + 1)%4;
        if (level == size()){
            std::cout << "max level reached\n";
            return;
        }
        for (unsigned j=0; j<(*this)[level].size(); j++){
            double tj = (*this)[level][j].t;
            if ( sign*(tj-t) > 0 ){
                if( next_t == t ||  sign*(tj-next_t)<0 ){
                    next_t = tj;
                    idx = j;
                }
            }
        }
        if ( next_t == t ){//not found.
            level = size();
            std::cout << "no next time found\n";
            return;
        }
        //TODO: time is periodic!!!
        //TODO: allow several components.
        if ( (*this)[level][idx].used ) {
            level = size();
            std::cout << " reached a point already used\n";
            return;
        }
        std::cout << "exit with: "<<level<<","<<idx<<"\n";
        return;
    }
};


//------------------------------------------------
// Generate the levels with random, growth...
//------------------------------------------------
std::vector<double>generateLevels(Interval const &domain, 
                                  double const width, 
                                  double const growth,
                                  double randomness){
    std::vector<double> result;
    std::srand(0);
    double x = domain.min() + width/2;
    double step = width;
    while (x<domain.max()){
        result.push_back(x);
        double rdm = 1+ ( (rand() % 100) - 50) /100.*randomness;
        x+= step*growth*rdm;
        step*=growth;
    }
    return result;
}


//-------------------------------------------------------
// Walk through the intersections to creat linear hatches
//-------------------------------------------------------
std::vector<Point> linearSnake(Piecewise<D2<SBasis> > const &f, double dy,double growth, double rdmness){

    std::vector<Point> result;

    Piecewise<SBasis> x = make_cuts_independent(f)[X];
    //Rque: derivative is computed twice in the 2 lines below!!
    Piecewise<SBasis> dx = derivative(x);
    OptInterval range = bounds_exact(x);
    //TODO: test range non emptyness!!
    std::vector<double> levels = generateLevels((*range), dy, growth, rdmness);
    std::vector<std::vector<double> > times;
    times = multi_roots(x,levels);

//TODO: fix multi_roots!!!*****************************************
//remove doubles :-(
    std::vector<std::vector<double> > cleaned_times(levels.size(),std::vector<double>());
    for (unsigned i=0; i<times.size(); i++){
        if ( times[i].size()>0 ){
            double last_t = times[i][0]-1;//ugly hack!!
            for (unsigned j=0; j<times[i].size(); j++){
                if (times[i][j]-last_t >0.000001){
                    last_t = times[i][j];
                    cleaned_times[i].push_back(last_t);
                }
            }
        }
    }
    times = cleaned_times;
    for (unsigned i=0; i<times.size(); i++){
        std::cout << "roots on level "<<i<<": ";
        for (unsigned j=0; j<times[i].size(); j++){
            std::cout << times[i][j] <<" ";
        }
        std::cout <<"\n";
    }
//*******************************************************************
    LevelsCrossings lscs(times,f,dx);
    unsigned i,j;
    lscs.findFirstUnused(i,j);
    while ( i < lscs.size() ){ 
        int dir = 0;
        while ( i < lscs.size() ){
            result.push_back(lscs[i][j].pt);
            lscs[i][j].used = true;
            lscs.step(i,j, dir);
        }
        //TODO: handle "non convex cases" where hatches have to be restarted at some point.
        //This needs some care in linearSnake->smoothSnake.
        //
        lscs.findFirstUnused(i,j);
    }
    return result;
}

//-------------------------------------------------------
// Smooth the linear hatches according to params...
//-------------------------------------------------------
Piecewise<D2<SBasis> > smoothSnake(std::vector<Point> const &linearSnake,
                                   double scale_bf = 1, double scale_bb = 1,
                                   double scale_tf = 1, double scale_tb = 1){

    if (linearSnake.size()<2) return Piecewise<D2<SBasis> >();
    bool is_top = true;
    Point last_pt = linearSnake[0];
    Point last_hdle = linearSnake[0];
    Path result(last_pt);
    unsigned i=1;
    while( i+1<linearSnake.size() ){
        Point pt0 = linearSnake[i];
        Point pt1 = linearSnake[i+1];
        Point new_pt = (pt0+pt1)/2;
        double scale = (is_top ? scale_tf : scale_bf );
        Point new_hdle = new_pt+(pt0-new_pt)*scale;
       
        result.appendNew<CubicBezier>(last_hdle,new_hdle,new_pt);
        
        last_pt = new_pt;
        scale = (is_top ? scale_tb : scale_bb );
        last_hdle = new_pt+(pt1-new_pt)*scale;
        i+=2;
        is_top = !is_top;
    }
    if ( i<linearSnake.size() )
        result.appendNew<CubicBezier>(last_hdle,linearSnake[i],linearSnake[i]);
    return result.toPwSb();
}

//-------------------------------------------------------
// Bend a path...
//-------------------------------------------------------

Piecewise<D2<SBasis> > bend(Piecewise<D2<SBasis> > const &f, Piecewise<SBasis> bending){
    D2<Piecewise<SBasis> > ff = make_cuts_independent(f);
    ff[X] += compose(bending, ff[Y]);
    return sectionize(ff);
}

//-------------------------------------------------------
// The toy!
//-------------------------------------------------------
class HatchesToy: public Toy {

    PointHandle adjuster[NB_SLIDER];

public:
    PointSetHandle b1_handle;
    PointSetHandle b2_handle;
    void draw(cairo_t *cr,
	      std::ostringstream *notify,
	      int width, int height, bool save, std::ostringstream *timer_stream) {
        for(unsigned i=0; i<NB_SLIDER; i++){
            adjuster[i].pos[X] = 30+i*20;
            if (adjuster[i].pos[Y]<100) adjuster[i].pos[Y] = 100;
            if (adjuster[i].pos[Y]>400) adjuster[i].pos[Y] = 400;
            cairo_move_to(cr, Point(30+i*20,100));
            cairo_line_to(cr, Point(30+i*20,400));
            cairo_set_line_width (cr, .5);
            cairo_set_source_rgba (cr, 0., 0., 0., 1);
            cairo_stroke(cr);
        }
        double hatch_width    = (400-adjuster[0].pos[Y])/300.*50;
        double scale_topfront = (250-adjuster[1].pos[Y])/150.*5;
        double scale_topback  = (250-adjuster[2].pos[Y])/150.*5;
        double scale_botfront = (250-adjuster[3].pos[Y])/150.*5;
        double scale_botback  = (250-adjuster[4].pos[Y])/150.*5;
        double growth =       1+(250-adjuster[5].pos[Y])/150.*.1;
        double rdmness =      1+(400-adjuster[6].pos[Y])/300.*.9;
        double bend_amount    = (250-adjuster[7].pos[Y])/300.*100.;

        b1_handle.pts.back() = b2_handle.pts.front(); 
        b1_handle.pts.front() = b2_handle.pts.back(); 
        D2<SBasis> B1 = b1_handle.asBezier();
        D2<SBasis> B2 = b2_handle.asBezier();
        
        {
            cairo_save(cr);
            cairo_set_line_width(cr, 0.3);
            cairo_set_source_rgb(cr, 0, 0, 0);
            cairo_d2_sb(cr, B1);
            cairo_d2_sb(cr, B2);
            cairo_restore(cr);
        }
        
        Piecewise<D2<SBasis> >B;
        B.concat(Piecewise<D2<SBasis> >(B1));
        B.continuousConcat(Piecewise<D2<SBasis> >(B2));

        Piecewise<SBasis> bending = Piecewise<SBasis>(shift(Linear(bend_amount),1));
        //TODO: test optrect non empty!!
        bending.setDomain((*bounds_exact(B))[Y]);
        Piecewise<D2<SBasis> >bentB = bend(B, bending);
        
        std::vector<Point> snakePoints;
        snakePoints = linearSnake(bentB, hatch_width, growth, rdmness);
        Piecewise<D2<SBasis> >smthSnake = smoothSnake(snakePoints,
                                                       scale_topfront,
                                                       scale_topback,
                                                       scale_botfront, 
                                                       scale_botback); 

        smthSnake = bend(smthSnake, -bending);
        cairo_pw_d2_sb(cr, smthSnake);
        cairo_set_line_width (cr, 1.5);
        cairo_set_source_rgba (cr, 0., 0., 0., 1);
        cairo_stroke(cr);
        
        if ( snakePoints.size() > 0 ){
            Path snake(snakePoints.front());
            for (unsigned i=1; i<snakePoints.size(); i++){
                snake.appendNew<LineSegment>(snakePoints[i]);
            }
            //cairo_pw_d2_sb(cr, snake.toPwSb() );
        }

        //cairo_pw_d2_sb(cr, B);
        cairo_set_line_width (cr, .5);
        cairo_set_source_rgba (cr, 0.7, 0.2, 0., 1);
        cairo_stroke(cr);


        Toy::draw(cr, notify, width, height, save,timer_stream);
    }        
  
public:
    HatchesToy(){
        for(int i = 0; i < SIZE; i++) {
            b1_handle.push_back(150+uniform()*300,150+uniform()*300);
            b2_handle.push_back(150+uniform()*300,150+uniform()*300);
        }
        b1_handle.pts[0] = Geom::Point(400,300);
        b1_handle.pts[1] = Geom::Point(400,400);
        b1_handle.pts[2] = Geom::Point(100,400);
        b1_handle.pts[3] = Geom::Point(100,300);

        b2_handle.pts[0] = Geom::Point(100,300);
        b2_handle.pts[1] = Geom::Point(100,200);
        b2_handle.pts[2] = Geom::Point(400,200);
        b2_handle.pts[3] = Geom::Point(400,300);
        handles.push_back(&b1_handle);
        handles.push_back(&b2_handle);

        for(unsigned i = 0; i < NB_SLIDER; i++) {
            adjuster[i].pos = Geom::Point(30+i*20,250);
            handles.push_back(&(adjuster[i]));
        }
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new HatchesToy);
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
//vim:filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99:
