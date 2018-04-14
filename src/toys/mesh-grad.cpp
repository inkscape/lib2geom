/**
 * Generate approximate mesh gradients for blurring technique suggested by bbyak.
 * (njh)
 */
#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/path.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>


#include <vector>
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
public:
    PointSetHandle hand;
    Sb2d2() {
        handles.push_back(&hand);
    }
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        D2<SBasis2d> sb2;
        for(unsigned dim = 0; dim < 2; dim++) {
            sb2[dim].us = 2;
            sb2[dim].vs = 2;
            const int depth = sb2[dim].us*sb2[dim].vs;
            sb2[dim].resize(depth, Linear2d(0));
        }
        Geom::Point dir(1,-2);
        if(hand.pts.empty()) {
            for(unsigned vi = 0; vi < sb2[0].vs; vi++)
                for(unsigned ui = 0; ui < sb2[0].us; ui++)
                    for(unsigned iv = 0; iv < 2; iv++)
                        for(unsigned iu = 0; iu < 2; iu++)
                            hand.pts.push_back(Geom::Point((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
                                                          (2*(iv+vi)/(2.*vi+1)+1)*width/4.));
        
        }
        
        for(int dim = 0; dim < 2; dim++) {
            Geom::Point dir(0,0);
            dir[dim] = 1;
            for(unsigned vi = 0; vi < sb2[dim].vs; vi++)
                for(unsigned ui = 0; ui < sb2[dim].us; ui++)
                    for(unsigned iv = 0; iv < 2; iv++)
                        for(unsigned iu = 0; iu < 2; iu++) {
                            unsigned corner = iu + 2*iv;
                            unsigned i = ui + vi*sb2[dim].us;
                            Geom::Point base((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
                                             (2*(iv+vi)/(2.*vi+1)+1)*width/4.);
                            if(vi == 0 && ui == 0) {
                                base = Geom::Point(width/4., width/4.);
                            }
                            double dl = dot((hand.pts[corner+4*i] - base), dir)/dot(dir,dir);
                            sb2[dim][i][corner] = dl/(width/2)*pow(4.0,(double)ui+vi);
                        }
        }
        cairo_d2_sb2d(cr, sb2, dir*0.1, width);
        cairo_set_source_rgba (cr, 0., 0., 0, 0.5);
        cairo_stroke(cr);
        for(unsigned vi = 0; vi < v_subs; vi++) {
            double tv = vi * inv_v_subs;
            for(unsigned ui = 0; ui < u_subs; ui++) {
                double tu = ui * inv_u_subs;
                
                Geom::Path pb;
                D2<SBasis> B;
                D2<SBasis> tB;
                
                B[0] = Linear(tu-fudge, tu+fudge + inv_u_subs );
                B[1] = Linear(tv-fudge, tv-fudge);
                tB = compose_each(sb2, B);
                tB = tB*(width/2) + Geom::Point(width/4, width/4);
                pb.append(tB);
                
                B[0] = Linear(tu+fudge + inv_u_subs , tu+fudge + inv_u_subs);
                B[1] = Linear(tv-fudge,               tv+fudge + inv_v_subs);
                tB = compose_each(sb2, B);
                tB = tB*(width/2) + Geom::Point(width/4, width/4);
                pb.append(tB);
                
                B[0] = Linear(tu+fudge + inv_u_subs, tu-fudge);
                B[1] = Linear(tv+fudge + inv_v_subs, tv+fudge + inv_v_subs);
                tB = compose_each(sb2, B);
                tB = tB*(width/2) + Geom::Point(width/4, width/4);
                pb.append(tB);
                
                B[0] = Linear(tu-fudge,              tu-fudge);
                B[1] = Linear(tv+fudge + inv_v_subs, tv-fudge);
                tB = compose_each(sb2, B);
                tB = tB*(width/2) + Geom::Point(width/4, width/4);
                pb.append(tB);
                
                cairo_path(cr, pb);
                
                //std::cout <<  pb.peek().end() - pb.peek().begin() << std::endl;
                cairo_set_source_rgba (cr, tu, tv, 0, 1);
                cairo_fill(cr);
            }
        }
        //*notify << "bo = " << sb2.index(0,0);
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new Sb2d2);
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
