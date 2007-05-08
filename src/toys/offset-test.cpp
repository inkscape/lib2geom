#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "solver.h"
#include "d2.h"
#include "s-basis-2d.h"
#include "sb-geometric.h"

#include "path-cairo.h"

#include "toy-framework.h"

#include <time.h>

using std::vector;
using namespace Geom;
using namespace std;

// TODO: 
// use path2
// replace Ray stuff with path2 line segments.

//-----------------------------------------------

static void 
plot_offset(cairo_t* cr, D2<SBasis> const &M,
            Coord offset = 10,
            int NbPts = 10){
    D2<SBasis> dM = derivative(M);
    for (int i = 0;i < NbPts;i++){
        double t = i*1./NbPts;
        Geom::Point V = dM(t);
        V = offset*rot90(unit_vector(V));
        draw_handle(cr, M(t)+V);
    }
}



class OffsetTester: public Toy {

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        D2<SBasis> B = handles_to_sbasis<3>(handles.begin());
        *notify << "Curve offset:" << endl;
        *notify << " -blue: pointwise plotted offset," << endl;
        *notify << " -red:  sbasis approximation," << endl;
        *notify << "Rays are drawn where the curve has been splitted" << endl;

        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
        cairo_md_sb(cr, B);
        cairo_stroke(cr);

        Coord offset = -100;
        plot_offset(cr,B,offset,11);
        cairo_set_source_rgba (cr, 0, 0, 1, 1);
        cairo_stroke(cr);

        cairo_set_source_rgba (cr, 0.5, 0.2, 0., 0.8);
        Piecewise<D2<SBasis> > n = rot90(unitVector(derivative(B)));
        cairo_pw_d2(cr,Piecewise<D2<SBasis> >(B)+n*offset);
        cairo_stroke(cr);

        for(int i = 0; i < n.size()+1;i++){
            Point ptA=B(n.cuts[i]), ptB;
            if (i==n.size()) 
                ptB=ptA+n.segs[i-1].at1()*offset;
            else 
                ptB=ptA+n.segs[i].at0()*offset;
            cairo_move_to(cr,ptA);
            cairo_line_to(cr,ptB);
            cairo_set_source_rgba (cr, 1, 0, 0, 1);
            cairo_stroke(cr);
        }

//         Piecewise<SBasis> cV = curvature(B);
//         for(int i = 0; i < cV.size();i++){
//             subB = compose(B, Linear(cV.cuts[i], cV.cuts[i+1]));
//             N = multiply(cV[i]*(-offset), rot90(V[i])) + subB;
//             cairo_md_sb(cr, N);
//             cairo_set_source_rgba (cr, 1, 0, 0.6, 0.5);
//             cairo_stroke(cr);

//             t0 = t1;
//         }
        *notify << "Total length: " << length(B) << endl;
        *notify << "(nb of cuts: " << n.size()-1 << ")" << endl;
    
        Toy::draw(cr, notify, width, height, save);
    }        
  
public:
    OffsetTester(){
        if(handles.empty()) {
            for(int i = 0; i < 4; i++)
                handles.push_back(Geom::Point(200+50*i,300+70*uniform()));
        }
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "offset-test", new OffsetTester);
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
//vim:filetype = cpp:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :

 	  	 
