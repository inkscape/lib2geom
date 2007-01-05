#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis.h"
#include "s-basis-2d.h"

#include "path2.h"
#include "path-cairo.h"

#include <iterator>
#include "translate.h"
#include "translate-ops.h"

#include "toy-framework.h"

using std::vector;
using namespace Geom;

unsigned total_pieces_sub;
unsigned total_pieces_inc;

const double u_subs = 5,
             v_subs = 5,
             fudge = .01;

const double inv_u_subs = 1 / u_subs,
             inv_v_subs = 1 / v_subs;

class Sb2d2: public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        vector<SBasis2d> sb2(2);
        for(int dim = 0; dim < 2; dim++) {
            sb2[dim].us = 2;
            sb2[dim].vs = 2;
            const int depth = sb2[dim].us*sb2[dim].vs;
            const int surface_handles = 4*depth;
            sb2[dim].resize(depth, BezOrd2d(0));
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
                            sb2[dim][i][corner] = dl/(width/2)*pow(4,ui+vi);
                        }
        }
        cairo_sb2d(cr, sb2, dir*0.1, width);
        cairo_set_source_rgba (cr, 0., 0., 0, 0.5);
        cairo_stroke(cr);
        for(int vi = 0; vi < v_subs; vi++) {
            double tv = vi * inv_v_subs;
            for(int ui = 0; ui < u_subs; ui++) {
                double tu = ui * inv_u_subs;
                
                Geom::Path2::Path pb;
                multidim_sbasis<2> B;
                multidim_sbasis<2> tB;
                
                B[0] = BezOrd(tu-fudge, tu+fudge + inv_u_subs );
                B[1] = BezOrd(tv-fudge, tv-fudge);
                tB = compose(sb2, B);
                tB = (width/2) * tB + Geom::Point(width/4, width/4);
                pb.append(tB);
                
                B[0] = BezOrd(tu+fudge + inv_u_subs , tu+fudge + inv_u_subs);
                B[1] = BezOrd(tv-fudge,               tv+fudge + inv_v_subs);
                tB = compose(sb2, B);
                tB = (width/2) * tB + Geom::Point(width/4, width/4);
                pb.append(tB);
                
                B[0] = BezOrd(tu+fudge + inv_u_subs, tu-fudge);
                B[1] = BezOrd(tv+fudge + inv_v_subs, tv+fudge + inv_v_subs);
                tB = compose(sb2, B);
                tB = (width/2) * tB + Geom::Point(width/4, width/4);
                pb.append(tB);
                
                B[0] = BezOrd(tu-fudge,              tu-fudge);
                B[1] = BezOrd(tv+fudge + inv_v_subs, tv-fudge);
                tB = compose(sb2, B);
                tB = (width/2) * tB + Geom::Point(width/4, width/4);
                pb.append(tB);
                
                cairo_path(cr, pb);
                
                //std::cout <<  pb.peek().end() - pb.peek().begin() << std::endl;
                cairo_set_source_rgba (cr, tu, tv, 0, 1);
                cairo_fill(cr);
            }
        }
        //*notify << "bo = " << sb2.index(0,0);
        Toy::draw(cr, notify, width, height, save);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "mesh-grad", new Sb2d2);
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
