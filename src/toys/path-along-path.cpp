#include <2geom/d2.h>
#include <2geom/piecewise.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#include <algorithm>
using std::vector;
using namespace Geom;

class PathAlongPathToy: public Toy {
    PointSetHandle skel_handles, pat_handles;
    PointHandle origin_handle;
    bool should_draw_numbers() override{return false;}

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        D2<SBasis> skeleton = skel_handles.asBezier();
        D2<SBasis> pattern  = pat_handles.asBezier();


        cairo_set_line_width(cr,1.);
        cairo_pw_d2_sb(cr, Piecewise<D2<SBasis> >(skeleton));
        cairo_set_source_rgba(cr,0.0,0.0,1.0,1.0);
        cairo_stroke(cr);

        cairo_pw_d2_sb(cr, Piecewise<D2<SBasis> >(pattern));
        cairo_set_source_rgba(cr,1.0,0.0,1.0,1.0);
        cairo_stroke(cr);

        origin_handle.pos[0]=150;
        Geom::Point origin = origin_handle.pos;

        Piecewise<D2<SBasis> > uskeleton = arc_length_parametrization(Piecewise<D2<SBasis> >(skeleton),2,.1);
        uskeleton = remove_short_cuts(uskeleton,.01);
        Piecewise<D2<SBasis> > n = rot90(derivative(uskeleton));
        n = force_continuity(remove_short_cuts(n,.1));
        
        Piecewise<SBasis> x=Piecewise<SBasis>(pattern[0]-origin[0]);
        Piecewise<SBasis> y=Piecewise<SBasis>(pattern[1]-origin[1]);
        Interval pattBnds = *bounds_exact(x);
        int nbCopies = int(uskeleton.cuts.back()/pattBnds.extent());

        //double pattWidth = uskeleton.cuts.back()/nbCopies;
        double pattWidth = pattBnds.extent();
        
        double offs = 0;
        x-=pattBnds.min();
        //x*=pattWidth/pattBnds.extent();
        
        Piecewise<D2<SBasis> >output;
        for (int i=0; i<nbCopies; i++){
            output.concat(compose(uskeleton,x+offs)+y*compose(n,x+offs));
            offs+=pattWidth;
        }

        //Perform cut for last segment
        double tt = uskeleton.cuts.back() - offs;
        if(tt > 0.) {
            vector<double> rs = roots(x - tt);
            rs.push_back(0); rs.push_back(1);  //regard endpoints
            std::sort(rs.begin(), rs.end());
            std::unique(rs.begin(), rs.end());
            //enumerate indices of sections to the left of the line
            for(unsigned i = (x[0].at0()>tt ? 1 : 0); i < rs.size()-1; i+=2) {
                Piecewise<SBasis> port = portion(x+offs, rs[i], rs[i+1]);
                output.concat(compose(uskeleton,port)+portion(y, rs[i], rs[i+1])*compose(n,port));
            }
        }

        cairo_pw_d2_sb(cr, output);
        cairo_set_source_rgba(cr,1.0,0.0,1.0,1.0);
        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }        

public:
    PathAlongPathToy() : origin_handle(150,150) {
        if(handles.empty()) {
            handles.push_back(&skel_handles);
            handles.push_back(&pat_handles);
            for(int i = 0; i < 8; i++)
                skel_handles.push_back(200+50*i,400);
            for(int i = 0; i < 4; i++)
                pat_handles.push_back(100+uniform()*400,
                                      150+uniform()*100);

            handles.push_back(&origin_handle);
        }
    }
};


int main(int argc, char **argv) {
    init(argc, argv, new PathAlongPathToy);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

