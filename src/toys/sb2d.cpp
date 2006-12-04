#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis.h"
#include "s-basis-2d.h"

#include "path.h"
#include "path-cairo.h"
#include "path-builder.h"

#include <iterator>
#include "translate.h"
#include "translate-ops.h"

#include "toy-framework.h"

using std::vector;

unsigned total_pieces_sub;
unsigned total_pieces_inc;

class Sb2d: public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        SBasis2d sb2;
        sb2.us = 2;
        sb2.vs = 2;
        const int depth = sb2.us*sb2.vs;
        const int surface_handles = 4*depth;
        sb2.resize(depth, BezOrd2d(0));
        vector<Geom::Point> display_handles(surface_handles);
        Geom::Point dir(1,-2);
        if(handles.empty()) {
            for(int vi = 0; vi < sb2.vs; vi++)
             for(int ui = 0; ui < sb2.us; ui++)
              for(int iv = 0; iv < 2; iv++)
               for(int iu = 0; iu < 2; iu++)
                   handles.push_back(Geom::Point((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
                                                 (2*(iv+vi)/(2.*vi+1)+1)*width/4.));
        
            handles.push_back(Geom::Point(3*width/4., width/4.) + 30*dir);
            for(int i = 0; i < 4; i++)
                handles.push_back(Geom::Point(uniform()*width/4.,
                                              uniform()*width/4.));
        }
        dir = (handles[surface_handles] - Geom::Point(3*width/4., width/4.)) / 30;
        if(!save) {
            cairo_move_to(cr, 3*width/4., width/4.);
            cairo_line_to(cr, handles[surface_handles]);
        }
        for(int vi = 0; vi < sb2.vs; vi++)
         for(int ui = 0; ui < sb2.us; ui++)
          for(int iv = 0; iv < 2; iv++)
           for(int iu = 0; iu < 2; iu++) {
               unsigned corner = iu + 2*iv;
               unsigned i = ui + vi*sb2.us;
               Geom::Point base((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
               (2*(iv+vi)/(2.*vi+1)+1)*width/4.);
               double dl = dot((handles[corner+4*i] - base), dir)/dot(dir,dir);
               display_handles[corner+4*i] = dl*dir + base;
               sb2[i][corner] = dl*10/(width/2)*pow(4,ui+vi);
           }
        draw_sb2d(cr, sb2, dir*0.1, width);
        multidim_sbasis<2> B = bezier_to_sbasis<2, 3>(handles.begin() + surface_handles+1);
        cairo_md_sb(cr, B);
        B *= 1./(width/4);
        B = (width/2)*B + Geom::Point(width/4, width/4);
        cairo_md_sb(cr, B);
    
        *notify << "bo = " << sb2.index(0,0); 
    
        cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
        cairo_stroke(cr);
        if(!save)
            for(int i = 0; i < display_handles.size(); i++)
                draw_circ(cr, display_handles[i]);
        Toy::draw(cr, notify, width, height, save);
    }
};

int main(int argc, char **argv) {
        init(argc, argv, "sb2d", new Sb2d());
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
