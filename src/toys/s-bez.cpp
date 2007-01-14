#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis.h"
#include "s-basis-2d.h"

#include "path2.h"
#include "path-cairo.h"
#include "path-builder.h"

#include <iterator>
#include "translate.h"
#include "translate-ops.h"

#include "toy-framework.h"

using std::vector;
using namespace Geom;

unsigned total_pieces_sub;
unsigned total_pieces_inc;

void draw_offset(cairo_t *cr, MultidimSBasis<2> const &B, double dist, double tol=0.1) {
    draw_handle(cr, Geom::Point(B[0].point_at(1), B[1].point_at(1)));
    
    MultidimSBasis<2> dB;
    dB = derivative(B);
    SBasis arc = dot(dB, dB);
    
    /*** A rather weak effort at estimating the offset curve error.  Here we assume that the
     * biggest error occurs where the derivative vanishes.  This code tries to bound the smallest
     * magnitude of the derivative.
     */
    double err = 0;
    double ss = 0.25;
    for(int i = 1; i < arc.size(); i++) {
        err += fabs(Hat(arc[i]))*ss;
        ss *= 0.25;
    }
    double le = fabs(arc[0][0]) - err;
    double re = fabs(arc[0][1]) - err;
    err /= std::max(arc[0][0], arc[0][1]);
    if(err > tol) {
        const int N = 2;
        for(int subdivi = 0; subdivi < N; subdivi++) {
            double dsubu = 1./N;
            double subu = dsubu*subdivi;
            MultidimSBasis<2> Bp;
            for(int dim = 0; dim < 2; dim++) {
                Bp[dim] = compose(B[dim], BezOrd(subu, dsubu+subu));
            }
            draw_offset(cr, Bp, dist);
        }
    } else {
        arc = sqrt(arc, 2);
    
        MultidimSBasis<2> offset;
    
        for(int dim = 0; dim < 2; dim++) {
            double sgn = dim?-1:1;
            offset[dim] = B[dim] + divide(dist*sgn*dB[1-dim],arc, 2);
        }
            cairo_set_source_rgba (cr, 0., 0.5, 0, 0.5);
        cairo_stroke(cr);
        cairo_md_sb_handles(cr, offset);
        cairo_set_source_rgba (cr, 0., 0, 0, 1);
        cairo_stroke(cr);
        
    }
}

class SBez: public Toy {
virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
    cairo_set_line_width (cr, 0.5);
    
    MultidimSBasis<2> B = bezier_to_sbasis<2, 3>(handles.begin());
    cairo_md_sb(cr, B);
    total_pieces_sub = 0;
    total_pieces_inc = 0;
    for(int i = 4; i < 5; i++) {
        draw_offset(cr, B, 10*i);
        draw_offset(cr, B, -10*i);
    }
    *notify << "total pieces subdivision = " << total_pieces_sub << std::endl; 
    *notify << "total pieces inc = " << total_pieces_inc;
    Toy::draw(cr, notify, width, height, save);
}
public:
SBez () {
    for(unsigned i = 0; i < 4; i++) handles.push_back(Geom::Point(uniform()*400, uniform()*400));
}
};

int main(int argc, char **argv) {   
    init(argc, argv, "s-bez", new SBez());

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
