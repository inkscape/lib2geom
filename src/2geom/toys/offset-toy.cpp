#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/sbasis-math.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/path-intersection.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <sstream>

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
            unsigned NbPts = 10){
    D2<SBasis> dM = derivative(M);
    for (unsigned i = 0;i < NbPts;i++){
        double t = i*1./NbPts;
        Point V = dM(t);
        V = offset*rot90(unit_vector(V));
        draw_handle(cr, M(t)+V);
    }
}

static void plot(cairo_t* cr, Piecewise<SBasis> const &f,double vscale=1){
    D2<Piecewise<SBasis> > plot;
    plot[1]=-f*vscale;
    plot[1]+=450;

    plot[0].cuts.push_back(f.cuts.front());
    plot[0].cuts.push_back(f.cuts.back());
    plot[0].segs.push_back(Linear(150,450));

    for (unsigned i=1; i<f.size(); i++){
        double t=f.cuts[i],ft=f.segs[i].at0();
        cairo_move_to(cr, Point(150+t*300, 450));
        cairo_line_to(cr, Point(150+t*300, 450-ft*vscale));
    }
    cairo_d2_pw_sb(cr, plot);
}



class OffsetTester: public Toy {
    PointSetHandle psh;

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        D2<SBasis> B = psh.asBezier();
        *notify << "Curve offset:" << endl;
        *notify << " -blue: pointwise plotted offset," << endl;
        *notify << " -red:  rot90(unitVector(derivative(.)))+rays at cut" << endl;
        *notify << " -gray: cos(atan2),sin(atan2)" << endl;

        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
        cairo_d2_sb(cr, B);
        cairo_stroke(cr);

        Coord offset = -100;
        plot_offset(cr,B,offset,11);
        cairo_set_source_rgba (cr, 0, 0, 1, 1);
        cairo_stroke(cr);

        cairo_set_source_rgba (cr, 0.5, 0.2, 0., 0.8);
        Piecewise<D2<SBasis> > n = rot90(unitVector(derivative(B)));
        Piecewise<D2<SBasis> > offset_curve = Piecewise<D2<SBasis> >(B)+n*offset;
        std::vector<Path> offset_path = path_from_piecewise(offset_curve, 0.1);
        
        cairo_path(cr, offset_path);
        cairo_stroke(cr);
        for(unsigned int pi = 0; pi < offset_path.size(); pi++) {
            Crossings cs = self_crossings(offset_path[pi]);
            for(unsigned int i = 0; i < cs.size(); i++) {
                draw_cross(cr, offset_path[pi].pointAt(cs[i].ta));
                std::stringstream s;
                Point Pa = offset_path[pi].pointAt(cs[i].ta);
                Point Pb = offset_path[pi].pointAt(cs[i].tb);
                s << L1(Pa - Pb) << std::endl;
                std::string ss = s.str();
                draw_text(cr, Pa+Point(3,3), ss.c_str(), false, "Serif 6");
                
            }
        }

        for(unsigned i = 0; i < n.size()+1;i++){
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

        Piecewise<SBasis> alpha = atan2(derivative(B),1e-2,3);
        plot(cr,alpha,75/M_PI);

        Piecewise<D2<SBasis> >n2 = sectionize(D2<Piecewise<SBasis> >(sin(alpha),cos(alpha)));
        cairo_pw_d2_sb(cr,Piecewise<D2<SBasis> >(B)+n2*offset*.9);
        cairo_set_source_rgba (cr, 0.5, 0.2, 0.5, 0.8);
        cairo_stroke(cr);

        Piecewise<SBasis> k = curvature(B);
        cairo_pw_d2_sb(cr,Piecewise<D2<SBasis> >(B)+k*n*100);
        cairo_set_source_rgba (cr, 0.5, 0.2, 0.5, 0.8);
        cairo_stroke(cr);

        *notify << "Total length: " << length(B) << endl;
        *notify << "(nb of cuts of unitVector: " << n.size()-1 << ")" << endl;
        *notify << "(nb of cuts of cos,sin(atan2): " << n2.size()-1 << ")" << endl;
    
        Toy::draw(cr, notify, width, height, save);
    }        
  
public:
    OffsetTester() {
        handles.push_back(&psh);
        for(unsigned i = 0; i < 6; i++)
            psh.push_back(200+50*i,300+70*uniform());
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new OffsetTester);
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

 	  	 
