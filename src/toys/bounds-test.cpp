#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "d2.h"

#include "path-cairo.h"

#include "toy-framework.h"

#include <time.h>

using std::vector;
using namespace Geom;
using namespace std;

static void plot(cairo_t* cr, SBasis const &B,double vscale=1,double a=0,double b=1){
    D2<SBasis> plot;
    plot[0]=SBasis(Linear(150+a*300,150+b*300));
    plot[1]=B*(-vscale);
    plot[1]+=300;
    cairo_md_sb(cr, plot);
    cairo_stroke(cr);
}
static void plot_bar(cairo_t* cr, double height, double vscale=1,double a=0,double b=1){
    cairo_move_to(cr, Geom::Point(150+300*a,-height*vscale+300));
    cairo_line_to(cr, Geom::Point(150+300*b,-height*vscale+300));
    cairo_stroke(cr);
}

class BoundsTester: public Toy {
    unsigned size;
    
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        
        SBasis B;
        for (unsigned i=0;i<size;i++){
            handles[i    ][0]=150+15*(i-size);
            handles[i+size][0]=450+15*(i+1);
            cairo_move_to(cr, Geom::Point(handles[i    ][0],150));
            cairo_line_to(cr, Geom::Point(handles[i    ][0],450));
            cairo_move_to(cr, Geom::Point(handles[i+size][0],150));
            cairo_line_to(cr, Geom::Point(handles[i+size][0],450));
        }
        cairo_move_to(cr, Geom::Point(0,300));
        cairo_line_to(cr, Geom::Point(600,300));
        
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 1);
        cairo_stroke(cr);
        
        for (unsigned i=0;i<size;i++){
            B.push_back(Linear(-(handles[i     ][1]-300)*pow(4.,(int)i),
                               -(handles[i+size][1]-300)*pow(4.,(int)i) ));
        }
        B.normalize();
        plot(cr,B,1);   
        cairo_set_source_rgba (cr, 0., 0., 0.8, 1);
        cairo_stroke(cr);

        Interval bnds = bounds_local(B,Interval(0.,.5));
        plot_bar(cr,bnds.min(),1,.0,.5);
        plot_bar(cr,bnds.max(),1,.0,.5);
        cairo_set_source_rgba (cr, 0.4, 0., 0., 1);
        cairo_stroke(cr);
        bnds = bounds_exact(B);
        plot_bar(cr,bnds.min());
        plot_bar(cr,bnds.max());
        cairo_set_source_rgba (cr, 0.9, 0., 0., 1);
        cairo_stroke(cr);
        
/*
This is a multi-root test...
*/
        handles[2*size  ][0]=150;
        handles[2*size+1][0]=150;
        handles[2*size+2][0]=150;
        handles[2*size  ][1]=std::max(handles[2*size  ][1],handles[2*size+1][1]);
        handles[2*size+1][1]=std::max(handles[2*size+1][1],handles[2*size+2][1]);
        vector<double> levels;
        levels.push_back((300-handles[2*size  ][1]));
        levels.push_back((300-handles[2*size+1][1]));
        levels.push_back((300-handles[2*size+2][1]));
        for (unsigned i=0;i<levels.size();i++) plot_bar(cr,levels[i]);
        
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
        
        *notify<<"Use handles to set the coefficients of the s-basis."<<std::endl;
        
        vector<double>my_roots;
        
//         cairo_set_source_rgba (cr, 0.9, 0., 0.8, 1);
//         for (unsigned i=0;i<levels.size();i++){
//             my_roots.clear();
//             my_roots=roots(B-Linear(levels[i]));
//             for(unsigned j=0;j<my_roots.size();j++){
//                 draw_cross(cr,Point(150+300*my_roots[j],300-levels[i]));
//             }
//         }

//         cairo_set_source_rgba (cr, 0.9, 0., 0.8, 1);

        vector<vector<double> > sols=multi_roots(B,levels,.001,.001);
        //map<double,unsigned> sols=multi_roots(B,levels);
        //for(map<double,unsigned>::iterator sol=sols.begin();sol!=sols.end();sol++){
        //    draw_handle(cr,Point(150+300*(*sol).first,300-levels[(*sol).second]));
        //}

        for (unsigned i=0;i<sols.size();i++){
            for (unsigned j=0;j<sols[i].size();j++){
                draw_handle(cr,Point(150+300*sols[i][j],300-levels[i]));
            }
        }
        cairo_set_source_rgba (cr, 0.9, 0., 0.8, 1);

/*
        
        clock_t end_t;
        unsigned iterations = 0;
        
        my_roots.clear();
        end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            my_roots.clear();
            for (unsigned i=0;i<levels.size();i++){
                my_roots=roots(B-Linear(levels[i]));
            }
            iterations++;
        }
        *notify << 1000*0.1/iterations <<" ms = roots time"<< std::endl;
        
        sols.clear();
        end_t = clock()+clock_t(0.1*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            sols.clear();
            sols=multi_roots(B,levels);
            iterations++;
        }
        *notify << 1000*0.1/iterations <<" ms = multi roots time"<< std::endl;
*/               
        Toy::draw(cr, notify, width, height, save);
    }        
    
public:
    BoundsTester(){
        size=5;
        if(handles.empty()) {
            for(unsigned i = 0; i < 2*size; i++)
                handles.push_back(Geom::Point(0,150+150+uniform()*300*0));
        }
        handles.push_back(Geom::Point(150,300+ 50+uniform()*100));
        handles.push_back(Geom::Point(150,300- 50+uniform()*100));
        handles.push_back(Geom::Point(150,300-150+uniform()*100));
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "bounds-test", new BoundsTester);
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
