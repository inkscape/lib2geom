#include "piecewise.h"
#include "sbasis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-math.h"

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
    unsigned handles_per_curve;
    PWSBHandle(unsigned handles_per_curve) :handles_per_curve(handles_per_curve) {}
    std::vector<Geom::Point> pts;
    virtual void draw(cairo_t *cr, bool annotes = false);
  
    virtual void* hit(Geom::Point mouse);
    virtual void move_to(void* hit, Geom::Point om, Geom::Point m);
    void push_back(double x, double y) {pts.push_back(Geom::Point(x,y));}
    Piecewise<SBasis> value() {
        Piecewise<SBasis> pws;
	Point* base = &pts[0];
	for(unsigned i = 0; i < handles_per_curve; i+=4) {
	    pws.push_cut(base[i][0]);
	    //Bad hack to move 0 to 150
	    for(unsigned j = i; j < i + 4; j++)
		base[j] = Point(base[j][0], base[j][1] - 150);
	    pws.push_seg( Geom::handles_to_sbasis(base+i, 3)[1]);
	    for(unsigned j = i; j < i + 4; j++)
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
	for(unsigned i = 4; i < handles_per_curve; i+=4) {
	    base[i-1][0] = base[i][0];
	}
	for(unsigned i = 0; i < handles_per_curve; i+=4) {
	    for(unsigned j = 1; j < 3; j++)
		base[i+j][0] = (1 - j*0.25)*base[i][0] + (j*0.25)*base[i+3][0];
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

        Piecewise<SBasis> acpw = (pws[0] - pws[0].valueAt(pws[0].cuts[0]))/10;
	if (0) {
	    acpw.push_cut(0);
	    acpw.push_seg( SBasis(Linear(0,M_PI))*SBasis(Linear(0,M_PI)));
	    acpw.push_cut(1);
	}
	D2<Piecewise<SBasis> > arcurv(cos(acpw),sin(acpw));
	Piecewise< D2<SBasis> > pwc = sectionize(arcurv);
	pwc = integral(pwc);
	pwc -= pwc.valueAt(pwc.cuts[0]);
	pwc += Point(width/2, height/2);
	D2<Interval> r = bounds_exact(pwc);
	pwc -= Geom::Point(r[0][0], r[1][0]);
	r = bounds_exact(pwc);
	cairo_rectangle(cr, r[0][0], r[1][0], r[0].extent(), r[1].extent());
	
	cairo_pw_d2(cr, pwc);
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_stroke(cr);

        *notify << pws[0].segN(dynamic_cast<PointHandle*>(handles[handles.size() - 1])->pos[0]) << "; " << pws[0].segT(dynamic_cast<PointHandle*>(handles[handles.size() - 1])->pos[0]);
        Toy::draw(cr, notify, width, height, save);
    }

    bool should_draw_numbers() { return false; }
        
public:
    PwToy () {
        segs = 3;
        handles_per_curve = 4 * segs;
        curves = 1;
        for(unsigned a = 0; a < curves; a++) {
	    PWSBHandle*psh = new PWSBHandle(handles_per_curve);
	    handles.push_back(psh);
	    for(unsigned i = 0; i < handles_per_curve; i++) {
	    
		psh->push_back(150 + 300*i/(4*segs), 
			       uniform() * 150 + 150 - 50 * a);
	    }
	}
        handles.push_back(new PointHandle(150, 400));
        handles.push_back(new PointHandle(300, 400));
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
