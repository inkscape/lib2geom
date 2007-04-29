#include "s-basis.h"
#include "d2.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "s-basis-2d.h"

#include "path-cairo.h"

#include <iterator>

#include "toy-framework.h"

using std::string;
using std::vector;
using namespace Geom;

unsigned total_pieces_sub;
unsigned total_pieces_inc;

class Sb2d2: public Toy {
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        D2<SBasis2d> sb2;
        for(int dim = 0; dim < 2; dim++) {
            sb2[dim].us = 2;
            sb2[dim].vs = 2;
            const int depth = sb2[dim].us*sb2[dim].vs;
            const int surface_handles = 4*depth;
            sb2[dim].resize(depth, Linear2d(0));
        }
        const int depth = sb2[0].us*sb2[0].vs;
        const int surface_handles = 4*depth;
        Geom::Point dir(1,-2);
        if(handles.empty()) {
        for(int vi = 0; vi < sb2[0].vs; vi++)
                for(int ui = 0; ui < sb2[0].us; ui++)
                for(int iv = 0; iv < 2; iv++)
                        for(int iu = 0; iu < 2; iu++)
                        handles.push_back(Geom::Point((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
                                                        (2*(iv+vi)/(2.*vi+1)+1)*width/4.));
        
        for(int i = 0; i < 4; i++)
                handles.push_back(Geom::Point(uniform()*width/4.,
                                                uniform()*width/4.));
        
        }
        
        for(int dim = 0; dim < 2; dim++) {
        Geom::Point dir(0,0);
        dir[dim] = 1;
        for(int vi = 0; vi < sb2[dim].vs; vi++)
            for(int ui = 0; ui < sb2[dim].us; ui++)
                for(int iv = 0; iv < 2; iv++)
                    for(int iu = 0; iu < 2; iu++) {
                        unsigned corner = iu + 2*iv;
                        unsigned i = ui + vi*sb2[dim].us;
                        Geom::Point base((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
                                                (2*(iv+vi)/(2.*vi+1)+1)*width/4.);
                        if(vi == 0 && ui == 0) {
                                base = Geom::Point(width/4., width/4.);
                        }
                        double dl = dot((handles[corner+4*i] - base), dir)/dot(dir,dir);
                        sb2[dim][i][corner] = dl/(width/2)*pow(4.0,ui+vi);
                    }
        }
        cairo_2dsb2d(cr, sb2, dir*0.1, width);
        cairo_set_source_rgba (cr, 0., 0., 0, 0.5);
        cairo_stroke(cr);
        D2<SBasis> B = handles_to_sbasis<3>(handles.begin() + surface_handles);
        cairo_md_sb(cr, B);
        for(int dim = 0; dim < 2; dim++) {
            std::vector<double> r = roots(B[dim]);
            for(int i = 0; i < r.size(); i++)
                draw_cross(cr, B(r[i]));
            r = roots(B[dim] - Linear(width/4));
            for(int i = 0; i < r.size(); i++)
                draw_cross(cr, B(r[i]));
        }
        cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
        cairo_stroke(cr);
        B *= (4./width);
        D2<SBasis> tB = compose(sb2, B);
        B = B*(width/2) + Geom::Point(width/4, width/4);
        //cairo_md_sb(cr, B);
        tB = tB*(width/2) + Geom::Point(width/4, width/4);
        
        cairo_md_sb(cr, tB);
        
        //*notify << "bo = " << sb2.index(0,0);

        Toy::draw(cr, notify, width, height, save);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "2dsb2d", new Sb2d2);
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
