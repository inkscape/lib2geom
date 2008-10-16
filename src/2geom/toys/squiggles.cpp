#include <2geom/piecewise.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-math.h>
#include <2geom/sbasis-geometric.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <vector>

using namespace Geom;
using namespace std;

void cairo_pw(cairo_t *cr, Piecewise<SBasis> p) {
    for(unsigned i = 0; i < p.size(); i++) {
        D2<SBasis> B;
        B[0] = Linear(p.cuts[i], p.cuts[i+1]);
        B[1] = Linear(150) + p[i];
        cairo_md_sb(cr, B);
    }
}

void cairo_horiz(cairo_t *cr, double y, vector<double> p) {
    for(unsigned i = 0; i < p.size(); i++) {
        cairo_move_to(cr, p[i], y);
        cairo_rel_line_to(cr, 0, 10);
    }
}

void cairo_vert(cairo_t *cr, double x, vector<double> p) {
    for(unsigned i = 0; i < p.size(); i++) {
        cairo_move_to(cr, x, p[i]);
        cairo_rel_line_to(cr, 10, 0);
    }
}

#include <2geom/toys/pwsbhandle.cpp>  // FIXME: This looks like it may give problems later, (including a .cpp file)

class Squiggles: public Toy {
    unsigned segs, handles_per_curve, curves;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0., 0., 1);
        cairo_set_line_width (cr, 1);
        
        Piecewise<SBasis> pws[curves];
        for(unsigned a = 0; a < curves; a++) {
	    pws[a] = dynamic_cast<PWSBHandle*>(handles[a])->value();
            assert(pws[a].invariants());
            
            cairo_pw(cr, pws[a]);
        }
        cairo_set_source_rgba (cr, 0.5, 0.5, 0.5, 1);
        cairo_move_to(cr,Point(150,200));
        cairo_line_to(cr,Point(450,200));
        cairo_stroke(cr);
        cairo_set_source_rgba (cr, 0., 0., 0., 1);
#if 1
        //curvature to curve:
        Piecewise<SBasis> alpha = integral(pws[0]-50)/1000;
        Piecewise<D2<SBasis> > v = sectionize(tan2(alpha));
        Piecewise<D2<SBasis> > pwc = integral(v);	
#else
        //direction of speed to curve
        Piecewise<SBasis> acpw = (pws[0] - pws[0].valueAt(pws[0].cuts[0]))/10;
	Piecewise< D2<SBasis> > pwc = sectionize(integral(tan2(acpw)));
	pwc -= pwc.valueAt(pwc.cuts[0]);
	pwc += Point(width/2, height/2);
	D2<Interval> r = bounds_exact(pwc);
	pwc -= Geom::Point(r[0][0], r[1][0]);
	r = bounds_exact(pwc);
	cairo_rectangle(cr, r[0][0], r[1][0], r[0].extent(), r[1].extent());
#endif        

#if 1
        // transform to fix end points:
        Point start = pwc.firstValue();
        Point end = pwc.lastValue();
        Point u = end - start;
        Matrix mat1 = Matrix(u[X],u[Y],-u[Y],u[X],start[X],start[Y]);
        Matrix mat2 = Matrix(width/2,0,0,width/2,width/4,200);
        mat1 = mat1.inverse()*mat2;
        pwc = pwc*mat1;
#endif
	cairo_pw_d2(cr, pwc);
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_stroke(cr);
/*
	cairo_pw_d2(cr, pwc);
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_stroke(cr);
*/	

        Toy::draw(cr, notify, width, height, save);
    }

    bool should_draw_numbers() { return false; }
        
public:
    Squiggles () {
        curves = 1;
        for(unsigned a = 0; a < curves; a++) {
	    PWSBHandle*psh = new PWSBHandle(5, 1);
	    handles.push_back(psh);
	    for(unsigned i = 0; i < psh->handles_per_curve; i++) {
	    
		psh->push_back(150 + 300*i/(psh->curve_size*psh->segs), 
			       200);
                //uniform() * 150 + 150 - 50 * a);
	    }
	}
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new Squiggles());
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
