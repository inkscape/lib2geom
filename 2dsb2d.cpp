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

void draw_sb2d(cairo_t* cr, vector<SBasis2d> const &sb2, Geom::Point dir, double width) {
    multidim_sbasis<2> B;
    for(int ui = 0; ui <= 10; ui++) {
        double u = ui/10.;
        B[0] = extract_u(sb2[0], u);// + BezOrd(u);
        B[1] = extract_u(sb2[1], u);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = (width/2)*B[i] + BezOrd(width/4);
        }
        cairo_md_sb(cr, B);
    }
    for(int vi = 0; vi <= 10; vi++) {
        double v = vi/10.;
        B[1] = extract_v(sb2[1], v);// + BezOrd(v);
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
                        sb2[dim][i][corner] = dl/(width/2)*pow(4,ui+vi);
                        }
        }
        draw_sb2d(cr, sb2, dir*0.1, width);
        cairo_set_source_rgba (cr, 0., 0., 0, 0.5);
        cairo_stroke(cr);
        multidim_sbasis<2> B = bezier_to_sbasis<2, 3>(handles.begin() + surface_handles);
        cairo_md_sb(cr, B);
        for(int dim = 0; dim < 2; dim++) {
            std::vector<double> r = roots(B[dim]);
            for(int i = 0; i < r.size(); i++)
                draw_cross(cr, point_at(B, r[i]));
            r = roots(B[dim] - BezOrd(width/4));
            for(int i = 0; i < r.size(); i++)
                draw_cross(cr, point_at(B, r[i]));
        }
        cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
        cairo_stroke(cr);
        B *= (4./width);
        multidim_sbasis<2> tB = compose(sb2, B);
        B = (width/2)*B + Geom::Point(width/4, width/4);
        //cairo_md_sb(cr, B);
        tB = (width/2)*tB + Geom::Point(width/4, width/4);
        
        cairo_md_sb(cr, tB);
        
        //*notify << "bo = " << sb2.index(0,0);
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
