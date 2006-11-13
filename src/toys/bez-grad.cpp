#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include <gtk/gtk.h>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <vector>
#include "s-basis.h"
#include "interactive-bits.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "path.h"
#include "path-cairo.h"
#include <iterator>
#include "multidim-sbasis.h"
#include "path-builder.h"
#include "translate.h"
#include "translate-ops.h"
#include "s-basis-2d.h"
#include "path-builder.h"

#include "toy-framework.cpp"

using std::string;
using std::vector;

unsigned total_pieces_sub;
unsigned total_pieces_inc;

const double u_subs = 5,
             v_subs = 5,
             fudge = .01;

const double inv_u_subs = 1 / u_subs,
             inv_v_subs = 1 / v_subs;

void draw_sb2d(cairo_t* cr, vector<SBasis2d> const &sb2, Geom::Point dir, double width) {
    multidim_sbasis<2> B;
    for(int ui = 0; ui <= 10; ui++) {
        double u = ui/10.;
        B[0] = extract_u(sb2[0], u);
        B[1] = extract_u(sb2[1], u);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = (width/2)*B[i] + BezOrd(width/4);
        }
        cairo_md_sb(cr, B);
    }
    for(int vi = 0; vi <= 10; vi++) {
        double v = vi/10.;
        B[1] = extract_v(sb2[1], v);
        B[0] = extract_v(sb2[0], v);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = (width/2)*B[i] + BezOrd(width/4);
        }
        cairo_md_sb(cr, B);
    }
}

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
            double corner[4][2] = {{0,0}, {1,0},{1,1},{0,1}}; // grey code?
            for(int side = 0; side < 4; side++) {
                Geom::Point dstart((corner[(side+1)%4][0] - corner[side][0])*width/6.,
                                   (corner[(side+1)%4][1] - corner[side][1])*width/6.);
                Geom::Point start((1+2*corner[side][0])*width/4.,
                                  (1+2*corner[side][1])*width/4.);
                
                for(int hand = 0; hand < 3; hand++) {
                    handles.push_back(start + dstart*hand);
                }
            }
        }
        std::vector<Geom::Point> circulator_hack(handles);
        circulator_hack.push_back(handles[0]);
        
        multidim_sbasis<2> side_B[4];
        
        for(int side = 0; side < 4; side++) {
            side_B[side] = bezier_to_sbasis<2, 3>(circulator_hack.begin() + 3*side);
            side_B[side][0].resize(2);
            side_B[side][1].resize(2); // fix overzealous pruning
            std::cout << side_B[side][0].size() << ", " 
                      << side_B[side][1].size() << std::endl;
            
        }
        
        for(int dim = 0; dim < 2; dim++) {
            for(int vi = 0; vi < sb2[dim].vs; vi++)
                for(int ui = 0; ui < sb2[dim].us; ui++)
                    for(int iv = 0; iv < 2; iv++)
                        for(int iu = 0; iu < 2; iu++) {
                            unsigned corner = iu + 2*iv;
                            unsigned i = ui + vi*sb2[dim].us;
                            sb2[dim][i][corner] = 0;
                        }
        }
        for(int dim = 0; dim < 2; dim++) {
            for(int iv = 0; iv < 2; iv++)
                for(int iu = 0; iu < 2; iu++) {
                    unsigned corner = iu + 2*iv;
                    unsigned cor_map[4] = {0, 1, 3, 2};
                    sb2[dim][0][corner] = side_B[cor_map[corner]][dim][0][0];
                    //std::cout << side_B[cor_map[corner]][dim].size() << std::endl;
                    sb2[dim][1][corner] = side_B[corner][dim][1][0];
                    //sb2[dim][2][corner] = side_B[cor_map[corner]][dim][1][1];
                }
        }
        
        draw_sb2d(cr, sb2, dir*0.1, width);
        cairo_set_source_rgba (cr, 0., 0., 0, 0.5);
        cairo_stroke(cr);
        for(int vi = 0; vi < 10; vi++) {
            double tv = vi/10.;
            for(int ui = 0; ui < 10; ui++) {
                double tu = ui/10.;
                
                Geom::PathSetBuilder pb;
                multidim_sbasis<2> B;
                multidim_sbasis<2> tB;
                
                B[0] = BezOrd(tu-fudge, tu+fudge+inv_u_subs);
                B[1] = BezOrd(tv-fudge, tv-fudge);
                tB = compose(sb2, B);
                subpath_from_sbasis(pb, tB, 0.1);
                
                B[0] = BezOrd(tu+fudge+inv_u_subs, tu+fudge+inv_u_subs);
                B[1] = BezOrd(tv-fudge,            tv+fudge+inv_v_subs);
                tB = compose(sb2, B);
                subpath_from_sbasis(pb, tB, 0.1, false);
                
                B[0] = BezOrd(tu+fudge+inv_u_subs, tu-fudge);
                B[1] = BezOrd(tv+fudge+inv_v_subs, tv+fudge+inv_v_subs);
                tB = compose(sb2, B);
                subpath_from_sbasis(pb, tB, 0.1, false);
                
                B[0] = BezOrd(tu-fudge,            tu-fudge);
                B[1] = BezOrd(tv+fudge+inv_v_subs, tv+fudge);
                tB = compose(sb2, B);
                subpath_from_sbasis(pb, tB, 0.1, false);
                
                if(1) {
                    cairo_PathSet(cr, pb.peek());
                
                    cairo_set_source_rgba (cr, tu, tv, 0, 1);
                    cairo_fill(cr);
                }
            }
        }
        //*notify << "bo = " << sb2.index(0,0);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "bez-grad", new Sb2d2);
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
