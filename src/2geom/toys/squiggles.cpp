#include <2geom/piecewise.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-math.h>
#include <2geom/sbasis-geometric.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <vector>

#define NB_CTL_PTS 4

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

Piecewise<SBasis> interpolate(std::vector<double> val){
    SBasis bump_in = Linear(0,1);
    bump_in.push_back(Linear(-1,1));
    SBasis bump_out = Linear(1,0);
    bump_out.push_back(Linear(1,-1));
    
    Piecewise<SBasis> result;
    result.cuts.push_back(0);
    for (unsigned i = 0; i<val.size()-1; i++){
        result.push(bump_out*val[i]+bump_in*val[i+1],i+1);
    }
    return result;
}


//#include <2geom/toys/pwsbhandle.cpp>  // FIXME: This looks like it may give problems later, (including a .cpp file)

class Squiggles: public Toy {
    unsigned segs, handles_per_curve, curves;

    PointSetHandle hand;
    unsigned current_ctl_pt;
    Point current_pos;
    Point current_dir;
    std::vector<double> curvatures;
    Piecewise<D2<SBasis> > curve;
    double tot_length;

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0., 0., 1);
        cairo_set_line_width (cr, 1);

        if (mouse_down && selected) {
            for(unsigned i = 0; i < handles.size(); i++) {
                if(selected == handles[i]){
                    *notify << "Selected = " << i; 
                    current_ctl_pt = i;
                    double time = current_ctl_pt*tot_length/(NB_CTL_PTS-1);
                    current_pos = curve.valueAt(time);
                    current_dir = derivative(curve).valueAt(time);
                    Point hdle = dynamic_cast<PointHandle*>(handles[current_ctl_pt])->pos;
                    curvatures[i] = cross(hdle - current_pos,current_dir)/1000;
                }
    	    }
        }

        Piecewise<SBasis> curvature = interpolate(curvatures);
        curvature.setDomain(Interval(0,tot_length));
        Piecewise<SBasis> alpha = integral(curvature);
        Piecewise<D2<SBasis> > v = sectionize(tan2(alpha));
        curve = integral(v)+Point(100,100);	

        Piecewise<SBasis> xxx = Piecewise<SBasis>(Linear(100.,100+tot_length));
        xxx.setDomain(Interval(0,tot_length));
        Piecewise<D2<SBasis> >pwc = sectionize(D2<Piecewise<SBasis> >(xxx, curvature*100+100));
	cairo_pw_d2(cr, pwc);

        //transform to keep current point in place
        double time = current_ctl_pt*tot_length/(NB_CTL_PTS-1);
        Point new_pos = curve.valueAt(time);
        Point new_dir = v.valueAt(time);
        Matrix mat1 = Matrix(    new_dir[X],    new_dir[Y],    -new_dir[Y],    new_dir[X],    new_pos[X],    new_pos[Y]);
        Matrix mat2 = Matrix(current_dir[X],current_dir[Y],-current_dir[Y],current_dir[X],current_pos[X],current_pos[Y]);
        mat1 = mat1.inverse()*mat2;
        curve = curve*mat1;
        v = v*mat1.without_translation();

        //update handles
        cairo_save(cr);
        double dashes[2] = {3, 2};
        cairo_set_dash(cr, dashes, 2, 0);
        cairo_set_line_width(cr, .5);
        cairo_set_source_rgba (cr, 0., 0., 0.5, 1);
        for(unsigned i = 0; i < NB_CTL_PTS; i++) {
            Point m = curve.valueAt(i*tot_length/(NB_CTL_PTS-1));
            *notify << "Curvature[" << i <<"] = "<< curvatures[i] << "\n"; 
            dynamic_cast<PointHandle*>(handles[i])->pos = m + curvatures[i]*1000*rot90(v.valueAt(i*tot_length/(NB_CTL_PTS-1)));
            draw_handle(cr, m);
            cairo_move_to(cr, m);
            cairo_line_to(cr, dynamic_cast<PointHandle*>(handles[i])->pos);
        }
        cairo_stroke(cr);
        cairo_restore(cr);

	cairo_pw_d2(cr, curve);
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_stroke(cr);

/*
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
*/
        Toy::draw(cr, notify, width, height, save);
    }

    bool should_draw_numbers() { return false; }

public:
    Squiggles () {
        current_ctl_pt = 0;
        current_dir = Point(0,1);
        current_pos = Point(100,100);
        tot_length = 200;

        curve = Piecewise<D2<SBasis> >(D2<SBasis>(Linear(100,300),Linear(100,100)));
        for(unsigned i = 0; i < NB_CTL_PTS; i++) {
            curvatures.push_back(0);
            PointHandle *pt_hdle = new PointHandle(Geom::Point(100+i*tot_length/(NB_CTL_PTS-1), 100.));
            handles.push_back(pt_hdle);
        }

/*
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
*/
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
