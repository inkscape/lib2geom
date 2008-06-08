#include "piecewise.h"
#include "sbasis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-math.h"
#include "sbasis-geometric.h"

#include "path-cairo.h"
#include "toy-framework-2.h"

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

class PWSBHandle : public Handle{
public:
    unsigned handles_per_curve, curve_size, segs;
    PWSBHandle(unsigned cs, unsigned segs) :handles_per_curve(cs*segs),curve_size(cs), segs(segs) {}
    std::vector<Geom::Point> pts;
    virtual void draw(cairo_t *cr, bool annotes = false);
  
    virtual void* hit(Geom::Point mouse);
    virtual void move_to(void* hit, Geom::Point om, Geom::Point m);
    void push_back(double x, double y) {pts.push_back(Geom::Point(x,y));}
    Piecewise<SBasis> value() {
        Piecewise<SBasis> pws;
	Point* base = &pts[0];
	for(unsigned i = 0; i < handles_per_curve; i+=curve_size) {
	    pws.push_cut(base[i][0]);
	    //Bad hack to move 0 to 150
	    for(unsigned j = i; j < i + curve_size; j++)
		base[j] = Point(base[j][0], base[j][1] - 150);
	    pws.push_seg( Geom::handles_to_sbasis(base+i, curve_size-1)[1]);
	    for(unsigned j = i; j < i + curve_size; j++)
		base[j] = Point(base[j][0], base[j][1] + 150);
	}
	pws.push_cut(base[handles_per_curve - 1][0]);
	assert(pws.invariants());
	return pws;
    }
};

void PWSBHandle::draw(cairo_t *cr, bool annotes) {
    for(unsigned i = 0; i < pts.size(); i++) {
	draw_circ(cr, pts[i]);
    }
}

void* PWSBHandle::hit(Geom::Point mouse) {
    for(unsigned i = 0; i < pts.size(); i++) {
	if(Geom::distance(mouse, pts[i]) < 5)
	    return (void*)(&pts[i]);
    }
    return 0;
}

void PWSBHandle::move_to(void* hit, Geom::Point om, Geom::Point m) {
    if(hit) {
	*(Geom::Point*)hit = m;
	Point* base = &pts[0];
	for(unsigned i = curve_size; i < handles_per_curve; i+=curve_size) {
	    base[i-1][0] = base[i][0];
	}
	for(unsigned i = 0; i < handles_per_curve; i+=curve_size) {
	    for(unsigned j = 1; j < (curve_size-1); j++) {
                double t = float(j)/(curve_size-1);
		base[i+j][0] = (1 - t)*base[i][0] + t*base[i+curve_size-1][0];
            }
	}
    }
}

class PwToy: public Toy {
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
        cairo_stroke(cr);
        
        cairo_save(cr);
        cairo_set_source_rgba (cr, 1., 0., 0., 1);
        cairo_pw(cr, reverse(pws[0]));
        cairo_stroke(cr);
        cairo_restore(cr);

        Piecewise<SBasis> acpw = (pws[0] - pws[0].valueAt(pws[0].cuts[0]))/10;
	Piecewise< D2<SBasis> > pwc = sectionize(integral(tan2(acpw)));
	pwc -= pwc.valueAt(pwc.cuts[0]);
	pwc += Point(width/2, height/2);
	D2<Interval> r = bounds_exact(pwc);
	pwc -= Geom::Point(r[0][0], r[1][0]);
	r = bounds_exact(pwc);
	cairo_rectangle(cr, r[0][0], r[1][0], r[0].extent(), r[1].extent());
	
	cairo_pw_d2(cr, pwc);
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save);
    }

    bool should_draw_numbers() { return false; }
        
public:
    PwToy () {
        curves = 1;
        for(unsigned a = 0; a < curves; a++) {
	    PWSBHandle*psh = new PWSBHandle(5, 3);
	    handles.push_back(psh);
	    for(unsigned i = 0; i < psh->handles_per_curve; i++) {
	    
		psh->push_back(150 + 300*i/(psh->curve_size*psh->segs), 
			       uniform() * 150 + 150 - 50 * a);
	    }
	}
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new PwToy());
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
