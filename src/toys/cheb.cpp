#include "d2.h"
#include "sbasis.h"
#include "sbasis-poly.h"

#include "path-cairo.h"
#include "toy-framework.h"
#include "chebyshev.h"

#include <vector>
using std::vector;
using namespace Geom;

#include <gsl/gsl_math.h>
#include <gsl/gsl_chebyshev.h>

vector<double> cheb_coeff;

double
f (double x, void *p)
{
	if (x < 0.5)
		return 1;
	else
		return -1;
}

class Sb1d: public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
        
        D2<SBasis> B;
        B[0] = Linear(width/4, 3*width/4);
        for(unsigned i = 0; i < 40; i+=5) {
            B[1] = compose(chebyshev_approximant(f, i, Interval(-1,1)),
                           Linear(-1,1));
            
            Geom::Path pb;
            B[1] = SBasis(Linear(2*width/4)) - B[1]*(width/4);
            pb.append(B);
            cairo_path(cr, pb);
            
            cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
            cairo_stroke(cr);
        }
        Toy::draw(cr, notify, width, height, save);
    }

    virtual void first_time(int argc, char **argv) {
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new Sb1d());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
