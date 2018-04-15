#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/bezier-to-sbasis.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

using std::vector;
using namespace Geom;

unsigned total_pieces_sub;
unsigned total_pieces_inc;

class Sb2d: public Toy {
public:
    PointSetHandle hand;
    Sb2d() {
        handles.push_back(&hand);
    }
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        SBasis2d sb2;
        sb2.us = 2;
        sb2.vs = 2;
        const int depth = sb2.us*sb2.vs;
        const int surface_handles = 4*depth;
        sb2.resize(depth, Linear2d(0));
        vector<Geom::Point> display_handles(surface_handles);
        Geom::Point dir(1,-2);
        if(hand.pts.empty()) {
            for(unsigned vi = 0; vi < sb2.vs; vi++)
             for(unsigned ui = 0; ui < sb2.us; ui++)
              for(unsigned iv = 0; iv < 2; iv++)
               for(unsigned iu = 0; iu < 2; iu++)
                   hand.pts.emplace_back((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
                                                 (2*(iv+vi)/(2.*vi+1)+1)*width/4.);
        
            hand.pts.push_back(Geom::Point(3*width/4., width/4.) + 30*dir);
        }
        dir = (hand.pts[surface_handles] - Geom::Point(3*width/4., width/4.)) / 30;
        if(!save) {
            cairo_move_to(cr, 3*width/4., width/4.);
            cairo_line_to(cr, hand.pts[surface_handles]);
        }
        for(unsigned vi = 0; vi < sb2.vs; vi++)
         for(unsigned ui = 0; ui < sb2.us; ui++)
          for(unsigned iv = 0; iv < 2; iv++)
           for(unsigned iu = 0; iu < 2; iu++) {
               unsigned corner = iu + 2*iv;
               unsigned i = ui + vi*sb2.us;
               Geom::Point base((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
               (2*(iv+vi)/(2.*vi+1)+1)*width/4.);
               double dl = dot((hand.pts[corner+4*i] - base), dir)/dot(dir,dir);
               display_handles[corner+4*i] = dl*dir + base;
               sb2[i][corner] = dl*10/(width/2)*pow(4.,(double)ui+vi);
           }
        cairo_sb2d(cr, sb2, dir*0.1, width);
    
        *notify << "bo = " << sb2.index(0,0); 
    
        cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
        cairo_stroke(cr);
        if(!save)
            for(auto display_handle : display_handles)
                draw_circ(cr, display_handle);
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
};

int main(int argc, char **argv) {
        init(argc, argv, new Sb2d());
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
