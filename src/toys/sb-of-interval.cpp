#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#include <time.h>
#include <vector>

#include <2geom/orphan-code/sbasis-of.h>

using std::vector;
using namespace Geom;
using namespace std;

#define SIZE 4

static void plot(cairo_t* cr, SBasis const &B,double vscale=1,double a=0,double b=1){
    D2<SBasis> plot;
    plot[0]=SBasis(Linear(150+a*300,150+b*300));
    plot[1]=B*(-vscale);
    plot[1]+=300;
    cairo_d2_sb(cr, plot);
//    cairo_stroke(cr);
}
static void plot(cairo_t* cr, SBasisOf<Interval> const &f, double vscale=1,double /*dx*/=.05, double a=0,double b=1){
    cairo_save(cr);
#if 0
    double t = a;
    while (t<=b){
        Interval i = f.valueAt(t);
        std::cout<<i.min()<<","<<i.max()<<"\n";
        draw_cross(cr, Geom::Point( 150+t*300, 300-i.min()*vscale ) );
        draw_cross(cr, Geom::Point( 150+t*300, 300-i.max()*vscale ) );
        t+=dx;
    }
#endif
    D2<SBasis> plot;
    Path pth;
    pth.setStitching(true);
    SBasis fmin(f.size(), Linear());
    SBasis fmax(f.size(), Linear());
    for(unsigned i = 0; i < f.size(); i++) {
        for(unsigned j = 0; j < 2; j++) {
            fmin[i][j] = f[i][j].min();
            fmax[i][j] = f[i][j].max();
        }
    }
    plot[0]=SBasis(Linear(150+a*300,150+b*300));
    plot[1]=fmin*(-vscale);
    plot[1]+=300;
    pth.append(plot);
    plot[1]=fmax*(-vscale);
    plot[1]+=300;
    pth.append(reverse(plot));
    cairo_path(cr, pth);
    
    cairo_set_source_rgba(cr, 0, 0, 0, 0.1);
    cairo_fill(cr);
    cairo_restore(cr);
}



class SbOfInterval: public Toy {

    unsigned size;
    PointHandle adjuster_a[3*SIZE];
    PointHandle adjuster_b[3*SIZE];

    void drawSliders(cairo_t *cr, PointHandle adjuster[], double y_min, double y_max){
        for (unsigned i=0; i < size; i++){
            cairo_move_to(cr, Geom::Point(adjuster[3*i].pos[X],y_min));
            cairo_line_to(cr, Geom::Point(adjuster[3*i].pos[X],y_max));
        }
    }
    void drawIntervals(cairo_t *cr, PointHandle adjuster[], double /*x*/, double /*dx*/, double /*y_min*/, double /*y_max*/){
        for (unsigned i=0; i < size; i++){
            cairo_move_to(cr, adjuster[3*i+1].pos);
            cairo_line_to(cr, adjuster[3*i+2].pos);
        }
    }
    void setupSliders(PointHandle adjuster[], double x, double dx, double y_min, double y_max){
        for (unsigned i=0; i < size; i++){
            for (unsigned j=0; j < 3; j++){
                adjuster[3*i+j].pos[X] = x + dx*i;
                if (adjuster[3*i+j].pos[Y] < y_min) adjuster[3*i+j].pos[Y] = y_min;
                if (adjuster[3*i+j].pos[Y] > y_max) adjuster[3*i+j].pos[Y] = y_max;
            }
            if (adjuster[3*i+1].pos[Y] < adjuster[3*i+2].pos[Y]) adjuster[3*i+1].pos[Y] = adjuster[3*i+2].pos[Y];
            if (adjuster[3*i  ].pos[Y] < adjuster[3*i+2].pos[Y]) adjuster[3*i  ].pos[Y] = adjuster[3*i+2].pos[Y];
            if (adjuster[3*i  ].pos[Y] > adjuster[3*i+1].pos[Y]) adjuster[3*i  ].pos[Y] = adjuster[3*i+1].pos[Y];
        }
  }

    PointSetHandle hand;
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        
        SBasisOf<Interval> f;
        double min=150, max=450;
        setupSliders(adjuster_a, 100, 15, min, max);
        setupSliders(adjuster_b, 500, 15, min, max);
        drawSliders(cr, adjuster_a, min, max);
        drawSliders(cr, adjuster_b, min, max);
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 1);
        cairo_stroke(cr);
        drawIntervals(cr, adjuster_a, 100, 15, min, max);
        drawIntervals(cr, adjuster_b, 500, 15, min, max);
        cairo_set_line_width (cr, 3);
        cairo_set_source_rgba (cr, 0.8, 0.2, 0.2, 1);
        cairo_stroke(cr);

        cairo_move_to(cr, Geom::Point(150,300));
        cairo_line_to(cr, Geom::Point(450,300));
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 1);
        cairo_stroke(cr);

        for (unsigned i=0; i < size; i++){
            double amin = (max+min)/2 - adjuster_a[3*i+1].pos[Y];
            double amax = (max+min)/2 - adjuster_a[3*i+2].pos[Y];
            Interval ai(amin*std::pow(4.,(int)i), amax*std::pow(4.,(int)i));
            double bmin = (max+min)/2 - adjuster_b[3*i+1].pos[Y];
            double bmax = (max+min)/2 - adjuster_b[3*i+2].pos[Y];
            Interval bi(bmin*std::pow(4.,(int)i), bmax*std::pow(4.,(int)i));
            f.push_back(LinearOf<Interval>(ai,bi));
        }
        SBasis f_dble(size, Linear());
        for (unsigned i=0; i < size; i++){
            double ai = (max+min)/2 - adjuster_a[3*i].pos[Y];
            double bi = (max+min)/2 - adjuster_b[3*i].pos[Y];
            f_dble[i] = Linear(ai*std::pow(4.,(int)i),bi*std::pow(4.,(int)i));
        }

        plot(cr,f_dble);   
        plot(cr,f);
        cairo_set_source_rgba (cr, 0., 0., 0.8, 1);
        cairo_stroke(cr);
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }        
    
public:
    SbOfInterval(){
        size=SIZE;
        for(unsigned i=0; i < size; i++){
            adjuster_a[3*i].pos[X] = 0;
            adjuster_a[3*i].pos[Y] = 300;
            adjuster_a[3*i+1].pos[X] = 0;
            adjuster_a[3*i+1].pos[Y] = 350;
            adjuster_a[3*i+2].pos[X] = 0;
            adjuster_a[3*i+2].pos[Y] = 250;
            adjuster_b[3*i].pos[X] = 0;
            adjuster_b[3*i].pos[Y] = 300;
            adjuster_b[3*i+1].pos[X] = 0;
            adjuster_b[3*i+1].pos[Y] = 350;
            adjuster_b[3*i+2].pos[X] = 0;
            adjuster_b[3*i+2].pos[Y] = 250;
        }

        for(unsigned i = 0; i < size; i++) {
            handles.push_back(&(adjuster_a[3*i]));
            handles.push_back(&(adjuster_a[3*i+1]));
            handles.push_back(&(adjuster_a[3*i+2]));
            handles.push_back(&(adjuster_b[3*i]));
            handles.push_back(&(adjuster_b[3*i+1]));
            handles.push_back(&(adjuster_b[3*i+2]));
        }
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new SbOfInterval);
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
