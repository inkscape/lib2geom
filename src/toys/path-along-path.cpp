#include "s-basis.h"
#include "sb-geometric.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "d2.h"

#include "path-cairo.h"

#include "toy-framework.h"

using std::vector;
using namespace Geom;

class NormalBundleToy: public Toy {

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        D2<SBasis> skeleton = handles_to_sbasis<3>(handles.begin());
        D2<SBasis> pattern  = handles_to_sbasis<3>(handles.begin()+4);
        Geom::Point O = *(handles.begin()+8);
    
        printf("ici\n");
        Piecewise<D2<SBasis> > uskeleton = arc_length_parametrization(Piecewise<D2<SBasis> >(skeleton));
        //TODO: add linear/cutoff extension...
        Piecewise<D2<SBasis> > n = rot90(derivative(uskeleton));

        Piecewise<SBasis> x=Piecewise<SBasis>(pattern[0]-O[0]), y=Piecewise<SBasis>(pattern[1]-O[1]);
        printf("la\n");
        Piecewise<D2<SBasis> >output = compose(uskeleton,x)+y*compose(n,x);
        
        cairo_set_line_width(cr,1.);

        cairo_pw_d2(cr, Piecewise<D2<SBasis> >(skeleton));
        cairo_set_source_rgba(cr,0.0,0.0,1.0,1.0);
        cairo_stroke(cr);

        cairo_pw_d2(cr, Piecewise<D2<SBasis> >(pattern));
        cairo_set_source_rgba(cr,1.0,0.0,1.0,1.0);
        cairo_stroke(cr);

        cairo_pw_d2(cr, output);
        cairo_set_source_rgba(cr,1.0,0.0,1.0,1.0);
        cairo_stroke(cr);


        Toy::draw(cr, notify, width, height, save);
    }        

public:
    NormalBundleToy(){
        if(handles.empty()) {
            for(int i = 0; i < 4; i++)
                handles.push_back(Geom::Point(200+50*i,400));
            for(int i = 0; i < 4; i++)
                handles.push_back(Geom::Point(100+uniform()*400,
                                              150+uniform()*100));
            handles.push_back(Geom::Point(200,200));
        }
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "normal-bundle", new NormalBundleToy);
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

 	  	 
