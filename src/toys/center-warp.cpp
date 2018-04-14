#include <2geom/bezier-to-sbasis.h>
#include <2geom/d2.h>
#include <2geom/path.h>
#include <2geom/sbasis-2d.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-math.h>
#include <2geom/svg-path-parser.h>
#include <2geom/transforms.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#include <vector>
using std::vector;
using namespace Geom;

unsigned total_pieces_sub;
unsigned total_pieces_inc;

void cairo_pw(cairo_t *cr, Piecewise<SBasis> p) {
    for(unsigned i = 0; i < p.size(); i++) {
        D2<SBasis> B;
        B[0] = Linear(p.cuts[i], p.cuts[i+1]);
        B[1] = p[i];
        cairo_d2_sb(cr, B);
    }
}

class CentreWarp: public Toy {
    Path path_a;
    D2<SBasis2d> sb2;
    Piecewise<D2<SBasis> >  path_a_pw;
    PointHandle brush_handle;
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        Geom::Point dir(1,-2);

	D2<Piecewise<SBasis> > B = make_cuts_independent(path_a_pw);

	cairo_set_source_rgba (cr, 0., 0.125, 0, 1);

        if(0) {
            D2<Piecewise<SBasis> > tB(cos(B[0]*0.1)*(brush_handle.pos[0]/100) + B[0], 
                                      cos(B[1]*0.1)*(brush_handle.pos[1]/100) + B[1]);
	
            cairo_d2_pw_sb(cr, tB);
        } else  {
            Piecewise<SBasis> r2 = (dot(path_a_pw - brush_handle.pos, path_a_pw - brush_handle.pos));
            Piecewise<SBasis> rc;
            rc.push_cut(0);
            rc.push(SBasis(Linear(1, 1)), 2);
            rc.push(SBasis(Linear(1, 0)), 4);
            rc.push(SBasis(Linear(0, 0)), 30);
            rc *= 10;
            rc.scaleDomain(1000);
            Piecewise<SBasis> swr;
            swr.push_cut(0);
            swr.push(SBasis(Linear(0, 1)), 2);
            swr.push(SBasis(Linear(1, 0)), 4);
            swr.push(SBasis(Linear(0, 0)), 30);
            swr *= 10;
            swr.scaleDomain(1000);
            cairo_pw(cr, swr);// + (height - 100));
            D2<Piecewise<SBasis> >  uB = make_cuts_independent(unitVector(path_a_pw - brush_handle.pos));
        
            D2<Piecewise<SBasis> > tB(compose(rc, (r2))*uB[0] + B[0], 
                                      compose(rc, (r2))*uB[1] + B[1]);
            cairo_d2_pw_sb(cr, tB);
            //path_a_pw = sectionize(tB);
	}
	cairo_stroke(cr);
        
        *notify << path_a_pw.size();
        
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
    void first_time(int argc, char** argv) override {
        const char *path_a_name="star.svgd";
        if(argc > 1)
            path_a_name = argv[1];
        PathVector paths_a = read_svgd(path_a_name);
        assert(!paths_a.empty());
        path_a = paths_a[0];
        
	path_a.close(true);
        path_a_pw = path_a.toPwSb();
        for(unsigned dim = 0; dim < 2; dim++) {
            sb2[dim].us = 2;
            sb2[dim].vs = 2;
            const int depth = sb2[dim].us*sb2[dim].vs;
            sb2[dim].resize(depth, Linear2d(0));
        }
        
        handles.push_back(&brush_handle);
        brush_handle.pos = Point(100,100);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new CentreWarp);
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
