#include <iostream>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#include <time.h>
#include <vector>

#include <2geom/orphan-code/linearN.h>
#include <2geom/orphan-code/sbasisN.h>

using namespace Geom;
using namespace std;


struct Frame
{
    Geom::Point O;
    Geom::Point x;
    Geom::Point y;
    Geom::Point z;
};

void
plot3d(cairo_t *cr, double x, double y, double z, Frame frame){
    Point p;
    for (unsigned dim=0; dim<2; dim++){
        p[dim] = x*frame.x[dim] + y*frame.y[dim] + z*frame.z[dim];
        p[dim] += frame.O[dim];
    }
    draw_cross(cr, p);
}
void
plot3d(cairo_t *cr, SBasis const &x, SBasis const &y, SBasis const &z, Frame frame){
    D2<SBasis> curve;
    for (unsigned dim=0; dim<2; dim++){
        curve[dim] = x*frame.x[dim] + y*frame.y[dim] + z*frame.z[dim];
        curve[dim] += frame.O[dim];
    }
    cairo_d2_sb(cr, curve);
}

void
plot3d(cairo_t *cr, LinearN<2> const &f, Frame frame){
    int iMax = 5;
    for (int i=0; i<iMax; i++){
        double t = i/(iMax-1.);
        plot3d(cr, Linear(0,1), Linear(t), toLinear(f.partialEval(t, 1)), frame);
        plot3d(cr, Linear(t), Linear(0,1), toLinear(f.partialEval(t, 0)), frame);
    }
}
void
plot3d(cairo_t *cr, SBasisN<2> const &f, Frame frame){
    int iMax = 5;
    for (int i=0; i<iMax; i++){
        double t = i/(iMax-1.);
        plot3d(cr, Linear(0,1), Linear(t), toSBasis(f.partialEval(t, 1)), frame);
        plot3d(cr, Linear(t), Linear(0,1), toSBasis(f.partialEval(t, 0)), frame);
    }
}
void
dot_plot3d(cairo_t *cr, SBasisN<2> const &f, Frame frame){
    int iMax = 15;
    double t[2];
    for (int i=0; i<iMax; i++){
        t[0] = i/(iMax-1.);
        for (int j=0; j<iMax; j++){
            t[1] = j/(iMax-1.);
            plot3d(cr, t[0], t[1], f.valueAt(t), frame);
        }
    }
}


class SBasisDimToy: public Toy {
    PointSetHandle hand;

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {

        double slider_top = width/4.;
        double slider_bot = width*3./4.;
        double slider_margin = width/8.;
        if(hand.pts.empty()) {
            hand.pts.emplace_back(width*3./16., 3*width/4.);
            hand.pts.push_back(hand.pts[0] + Geom::Point(width/2., 0));
            hand.pts.push_back(hand.pts[0] + Geom::Point(width/8., -width/12.));
            hand.pts.push_back(hand.pts[0] + Geom::Point(0,-width/4.));
            hand.pts.emplace_back(slider_margin,slider_bot);
            hand.pts.emplace_back(width-slider_margin,slider_top);
        }
        
        hand.pts[4][X] = slider_margin;
        if (hand.pts[4][Y]<slider_top) hand.pts[4][Y] = slider_top; 
        if (hand.pts[4][Y]>slider_bot) hand.pts[4][Y] = slider_bot; 
        hand.pts[5][X] = width-slider_margin;
        if (hand.pts[5][Y]<slider_top) hand.pts[5][Y] = slider_top; 
        if (hand.pts[5][Y]>slider_bot) hand.pts[5][Y] = slider_bot; 

        double tA = (slider_bot-hand.pts[4][Y])/(slider_bot-slider_top);
        double tB = (slider_bot-hand.pts[5][Y])/(slider_bot-slider_top);

        cairo_move_to(cr,Geom::Point(slider_margin,slider_bot));
        cairo_line_to(cr,Geom::Point(slider_margin,slider_top));
        cairo_move_to(cr,Geom::Point(width-slider_margin,slider_bot));
        cairo_line_to(cr,Geom::Point(width-slider_margin,slider_top));
        cairo_set_line_width(cr,.5);
        cairo_set_source_rgba (cr, 0., 0.3, 0., 1.);
        cairo_stroke(cr);
        
        Frame frame;
        frame.O = hand.pts[0];//
        frame.x = hand.pts[1]-hand.pts[0];//
        frame.y = hand.pts[2]-hand.pts[0];//
        frame.z = hand.pts[3]-hand.pts[0];// 

        plot3d(cr,Linear(0,1),Linear(0,0),Linear(0,0),frame);
        plot3d(cr,Linear(0,1),Linear(1,1),Linear(0,0),frame);
        plot3d(cr,Linear(0,0),Linear(0,1),Linear(0,0),frame);
        plot3d(cr,Linear(1,1),Linear(0,1),Linear(0,0),frame);
        cairo_set_line_width(cr,.2);
        cairo_set_source_rgba (cr, 0., 0., 0., 1.);
        cairo_stroke(cr);

        SBasisN<1> t = LinearN<1>(0,1);

        LinearN<2> u,v;
        setToVariable(u,0);
        setToVariable(v,1);
        SBasisN<2> f, x = u, y = v; //x,y are used for conversion :-(

        
//--------------------
//Basic MultiDegree<2> tests...
//--------------------
#if 0
        unsigned sizes[2];
        sizes[0] = 4;
        sizes[1] = 3;
        MultiDegree<2> d0;
        d0.p[0]=3;
        d0.p[1]=2;
        std::cout<<"(3,2)->"<< d0.asIdx(sizes) <<"\n";
        MultiDegree<2> d1(11,sizes);
        std::cout<<"11->"<< d1.p[0] <<","<<d1.p[1] <<"\n";
#endif
        
//--------------------
//Basic LinearN tests
//--------------------
#if 0
        plot3d(cr, u, frame);
        cairo_set_line_width(cr,1);        
        cairo_set_source_rgba (cr, .75, 0., 0., 1.);
        cairo_stroke(cr);
        plot3d(cr, v, frame);
        cairo_set_line_width(cr,1);        
        cairo_set_source_rgba (cr, 0., 0., 0.75, 1.);
        cairo_stroke(cr);
#endif

//--------------------
//Basic SBasisN tests
//--------------------
#if 1
        f  = x*x + y*y;//(x-one*.5)*(x-one*.5);
        std::cout<<"\nf: "<<f<<"\n";
        std::cout<<"Degrees:\n";
        std::cout<<"quick_deg: "<< f.quick_degree(0)<<", "<<f.quick_degree(1)<<"\n";
        std::cout<<"real s_deg: "<< f.degree(0)<<", "<<f.degree(1)<<"\n";
        std::cout<<"real t_deg: "<< f.real_t_degree(0)<<", "<<f.real_t_degree(1)<<"\n";
        plot3d(cr, f, frame);
        cairo_set_line_width(cr,1);        
        cairo_set_source_rgba (cr, 0., 0.75, 0., 1.);
        cairo_stroke(cr);
#endif

//--------------------
// SBasisOf<SBasisOf<double> > simulation tests
//--------------------
#if 1
        SBasisN<1> y1d = LinearN<1>(0,1);
        SBasisN<2> g,g1;
        g.appendCoef(LinearN<1>(0.), y1d*y1d , 0);
        g.appendCoef(y1d + 1, y1d, 0);
        g1 = x*y*y + x*(-x+1)*( (-x+1)*(y+1) + x*y );
        plot3d(cr, g-g1, frame);
        cairo_set_line_width(cr,1);        
        cairo_set_source_rgba (cr, 0., 0.75, 0., 1.);
        cairo_stroke(cr);
#endif
//--------------------
// SBasisN composition tests
//--------------------
#if 1
        SBasisN<1> z;
        std::vector<SBasisN<1> > var;
        t -=.5;
        var.push_back( t*t + tA);
        var.push_back( (t+.3)*t*(t-.3) + tB);
        z = compose(f,var);
        cairo_set_line_width(cr,1);
        plot3d(cr, toSBasis(var[0]), toSBasis(var[1]), Linear(0.), frame);
        cairo_set_source_rgba (cr, 0., 0., 0.75, 1.);
        cairo_stroke(cr);
        plot3d(cr, toSBasis(var[0]), toSBasis(var[1]), toSBasis(z), frame);
        cairo_set_source_rgba (cr, 0.75, 0., 0., 1.);
        cairo_stroke(cr);
#endif

//--------------------
//Some timing. TODO: Compare to SBasisOf<SBasisOf<double> >
//--------------------
#if 0
        double units = 1e6;
        std::string units_string("us");
        double timer_precision = 0.1;
        clock_t end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        // Base loop to remove overhead
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        long iterations = 0;
        while(end_t > clock()) {
            iterations++;
        }
        double overhead = timer_precision*units/iterations;

        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            f.valueAt(t);
            iterations++;
        }
        *notify << "recursive eval: " 
                << ", time = " << timer_precision*units/iterations-overhead 
                << units_string << std::endl;
#endif

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }        
    
public:
    SBasisDimToy(){
        handles.push_back(&hand);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new SBasisDimToy);
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
// vim: filetype = cpp:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :
