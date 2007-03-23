#include <cassert>
#include <algorithm>
#include <vector>
#include "s-basis.h"
#include "point-ops.h"
#include "point-fns.h"
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

using std::vector;

unsigned total_pieces_sub;
unsigned total_pieces_inc;

void draw_sb2d(cairo_t* cr, SBasis2d const &sb2, Geom::Point dir, double width) {
    multidim_sbasis<2> B;
    for(int ui = 0; ui <= 10; ui++) {
        double u = ui/10.;
        B[0] = dir[0]*extract_u(sb2, u) + Linear(u);
        B[1] = SBasis(Linear(0,1))+dir[1]*extract_u(sb2, u);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = (width/2)*B[i] + Linear(width/4);
        }
        cairo_md_sb(cr, B);
    }
    for(int vi = 0; vi <= 10; vi++) {
        double v = vi/10.;
        B[1] = dir[1]*extract_v(sb2, v) + Linear(v);
        B[0] = SBasis(Linear(0,1)) + dir[0]*extract_v(sb2, v);
        for(unsigned i = 0; i < 2; i ++) {
            B[i] = (width/2)*B[i] + Linear(width/4);
        }
        cairo_md_sb(cr, B);
    }
}

class Sb2d: public Toy {
public:
    cairo_surface_t* imagedata;
    void render_mg_patch(cairo_t* cr, 
                         double x0, double y0, double x1, double y1, 
                         SBasis2d & sb2) {
        cairo_save(cr);
        cairo_rectangle(cr, x0, y0, x1 - x0, y1 - y0);
        
        unsigned char* row = cairo_image_surface_get_data(imagedata);
        unsigned char a[4][4];
        for(int corner = 0; corner < 4; corner++) {
            //assert(sb2[0][corner] >= 0);
            //assert(sb2[0][corner] <= 1);
            double v = sb2[0][corner];
            if(v < 0) v = 0;
            if(v > 1) v = 1;
            for(int comp = 0; comp < 3; comp++)
                a[corner][comp] = v*255;
            a[corner][3] = 255;
        }
        const unsigned w = x1 - x0;
        const unsigned h = y1 - y0;
        for(unsigned long y = 0; y < h; y++) {
            unsigned char* p = row;
            unsigned long ay = (h-y);
            for(unsigned long x = 0; x < 256; x++) {
                for(int comp = 0; comp < 4; comp++) {
                    unsigned long ax = (w-x);
                    unsigned long val = ax*ay*(unsigned long)(a[0][comp]);
                    val += x*ay*(unsigned long)(a[1][comp]) +
                        ax*y*(unsigned long)(a[2][comp]) +
                        x*y*(unsigned long)(a[3][comp]);
                    p[comp] = val / (w*h);
                }
                p[3] = 255;
                p += 4;
            }
            row += cairo_image_surface_get_stride(imagedata);
        }
        cairo_pattern_t* c_pat = cairo_pattern_create_for_surface(imagedata);
        cairo_matrix_t matrix;
        cairo_matrix_init_translate (&matrix, -x0, -y0);
        cairo_pattern_set_matrix (c_pat, &matrix);
        //cairo_pattern_set_extend(c_pat, CAIRO_EXTEND_REPEAT);
        cairo_set_source(cr, c_pat);
        cairo_fill(cr);
        cairo_restore(cr);
    }

    void subdivide_mg_patch(cairo_t* cr, 
                            double x0, double y0, double x1, double y1, 
                            SBasis2d &sb2) {
        double tail_error = 0, st = 1;
        for(int vi = 0; vi < sb2.vs; vi++) {
            double ss = st;
            for(int ui = 0; ui < sb2.us; ui++) {
                unsigned i = ui + vi*sb2.us;
                if(vi + ui > 0)
                    tail_error += ss*sb2[i].abs();
                ss *= 0.5;
            }
            st *= 0.5;
        }
        double w = x1 - x0;
        double h = y1 - y0;
        if((w < 256) && (h < 256) && tail_error < 1./255)
            render_mg_patch(cr, x0, y0, x1, y1, sb2);
        else {
            for(unsigned corner = 0; corner < 4; corner++) {
                Linear split_s((corner % 2)/ 2., 0.5 + (corner % 2)/ 2.);
                Linear split_t((corner / 2)/ 2., 0.5 + (corner / 2)/ 2.);
                double mid_x = (x0 + x1) / 2;
                double mid_y = (y0 + y1) / 2;
                SBasis2d subd2(sb2);
                double st = 1;
                for(int vi = 0; vi < subd2.vs; vi++) {
                    double ss = st;
                    for(int ui = 0; ui < subd2.us; ui++) {
                        unsigned i = ui + vi*subd2.us;
                        for(int iu = 0; iu < 2; iu++) {
                        for(int iv = 0; iv < 2; iv++) {
                            subd2[i][iu + iv*2] = (1 - split_t[iv])*(1 - split_s[iu])*sb2[i][0] +
                                (1 - split_t[iv])*split_s[iu]*sb2[i][1] +
                                split_t[iv]*(1 - split_s[iu])*sb2[i][2] +
                                split_t[iv]*split_s[iu]*sb2[i][3];
                        }
                        }
                        subd2[i] *= ss;
                        ss *= 0.25;
                    }
                    st *= 0.25;
                }
                subdivide_mg_patch(cr, (1 - split_s[0])*x0 + split_s[0]*x1, 
                                   (1 - split_t[0])*y0 + split_t[0]*y1,
                                   (1 - split_s[1])*x0 + split_s[1]*x1, 
                                   (1 - split_t[1])*y0 + split_t[1]*y1,
                                   subd2);
            }
        }
    }
    
    
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        SBasis2d sb2;
        sb2.us = 2;
        sb2.vs = 2;
        const int depth = sb2.us*sb2.vs;
        const int surface_handles = 4*depth;
        sb2.resize(depth, Linear2d(0));
        vector<Geom::Point> display_handles(surface_handles);
        Geom::Point dir(1,-2);
        if(handles.empty()) {
            for(int vi = 0; vi < sb2.vs; vi++)
             for(int ui = 0; ui < sb2.us; ui++)
              for(int iv = 0; iv < 2; iv++)
               for(int iu = 0; iu < 2; iu++)
                   handles.push_back(Geom::Point((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
                                                 (2*(iv+vi)/(2.*vi+1)+1)*width/4.));
        
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
               if(sb2[i][corner] > 1)
                   sb2[i][corner] = 1;
               if(sb2[i][corner] < 0)
                   sb2[i][corner] = 0;
           }
        draw_sb2d(cr, sb2, dir*0.1, width);
        
        cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
        cairo_stroke(cr);

        subdivide_mg_patch(cr, width/4., width/4.,
                        3*width/4., 3*width/4., sb2);
        
        if(!save)
            for(int i = 0; i < display_handles.size(); i++)
                draw_circ(cr, display_handles[i]);
    }
    
    Sb2d() {
        imagedata = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
                                                 256,
                                                 256);
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
