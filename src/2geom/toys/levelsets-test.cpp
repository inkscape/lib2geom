#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

using std::vector;
using namespace Geom;
using namespace std;

//-------------------------------------
//-------------------------------------
//-------------------------------------
//-------------------------------------
//-------------------------------------


//-------------------------------------
//-------------------------------------
//-------------------------------------
//-------------------------------------
//-------------------------------------
//-------------------------------------




static void plot(cairo_t* cr, SBasis const &B,double vscale=1,double a=0,double b=1){
    D2<SBasis> plot;
    plot[0]=SBasis(Linear(150+a*300,150+b*300));
    plot[1]=B*(-vscale);
    plot[1]+=300;
    cairo_d2_sb(cr, plot);
    cairo_stroke(cr);
}
static void plot_bar(cairo_t* cr, double height, double vscale=1,double a=0,double b=1){
    cairo_move_to(cr, Geom::Point(150+300*a,-height*vscale+300));
    cairo_line_to(cr, Geom::Point(150+300*b,-height*vscale+300));
}
static void plot_bar(cairo_t* cr, Interval height, double vscale=1,double a=0,double b=1){
    Point A(150+300*a,-height.max()*vscale+300);
    Point B(150+300*b,-height.min()*vscale+300);
    cairo_rectangle(cr, Rect(A,B) );
}

class BoundsTester: public Toy {
    unsigned size;
    PointSetHandle hand;
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        
        for (unsigned i=0;i<size;i++){
            hand.pts[i    ][0]=150+15*(i-size);
            hand.pts[i+size][0]=450+15*(i+1);
            cairo_move_to(cr, Geom::Point(hand.pts[i    ][0],150));
            cairo_line_to(cr, Geom::Point(hand.pts[i    ][0],450));
            cairo_move_to(cr, Geom::Point(hand.pts[i+size][0],150));
            cairo_line_to(cr, Geom::Point(hand.pts[i+size][0],450));
        }
        cairo_move_to(cr, Geom::Point(0,300));
        cairo_line_to(cr, Geom::Point(600,300));
        
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 1);
        cairo_stroke(cr);
        
        SBasis B(size, Linear());
        for (unsigned i=0;i<size;i++){
            B[i] = Linear(-(hand.pts[i     ][1]-300)*pow(4.,(int)i),
                          -(hand.pts[i+size][1]-300)*pow(4.,(int)i) );
        }
        B.normalize();
        plot(cr,B,1);   
        cairo_set_source_rgba (cr, 0., 0., 0.8, 1);
        cairo_stroke(cr);

        hand.pts[2*size  ][X]=150;
        hand.pts[2*size+1][X]=150;
        hand.pts[2*size+2][X]=150;
        hand.pts[2*size  ][Y]=std::max(hand.pts[2*size  ][Y],hand.pts[2*size+1][Y]);
        hand.pts[2*size+1][Y]=std::max(hand.pts[2*size+1][Y],hand.pts[2*size+2][Y]);
        vector<Interval> levels;
        Interval range(300-hand.pts[2*size  ][Y], 300-hand.pts[2*size+1][Y]);
        levels.push_back( range );
        for (unsigned i=0;i<levels.size();i++) plot_bar(cr,levels[i]);
        cairo_set_source_rgba( cr, 1., 0., 0., .5);
        cairo_stroke(cr);

        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
        
        *notify<<"Use hand.pts to set the coefficients of the s-basis."<<std::endl;
        
        vector<double>my_roots;
        
        vector<vector<Interval> > sols=level_sets(B,levels,0,1);
        for (unsigned i=0;i<sols.size();i++){
            for (unsigned j=0;j<sols[i].size();j++){
                plot_bar(cr,levels[i], 1., sols[i][j].min(), sols[i][j].max());
                cairo_set_source_rgba( cr, 0., 0., 1., .3);
                cairo_fill(cr);
                plot_bar(cr,levels[i], 1., sols[i][j].min(), sols[i][j].max());
                cairo_set_source_rgba( cr, 1., 0., 1., .8);
                cairo_stroke(cr);
                printf("%f,%f\n", sols[i][j].min(), sols[i][j].max());
            }
        }
        printf("\n");

        cairo_set_source_rgba( cr, 0., 0., 1., .5);
        cairo_fill(cr);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }        
    
public:
    BoundsTester(){
        size=5;
        if(hand.pts.empty()) {
            for(unsigned i = 0; i < 2*size; i++)
                hand.pts.push_back(Geom::Point(0,150+150+uniform()*300*0));
        }
        hand.pts.push_back(Geom::Point(150,300+ 50+uniform()*100));
        hand.pts.push_back(Geom::Point(150,300- 50+uniform()*100));
        hand.pts.push_back(Geom::Point(150,300-150+uniform()*100));
        handles.push_back(&hand);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new BoundsTester);
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
