#include <2geom/piecewise.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-math.h>
#include <2geom/sbasis-geometric.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <vector>

#define NB_CTL_PTS 6
#define K_SCALE .002

using namespace Geom;
using namespace std;

void cairo_pw(cairo_t *cr, Piecewise<SBasis> p) {
    for(unsigned i = 0; i < p.size(); i++) {
        D2<SBasis> B;
        B[0] = Linear(p.cuts[i], p.cuts[i+1]);
        B[1] = Linear(150) + p[i];
        cairo_d2_sb(cr, B);
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

/*
Piecewise<SBasis> interpolate(std::vector<double> values, std::vector<double> times){
    assert ( values.size() == times.size() );
    if ( values.size() == 0 ) return Piecewise<SBasis>();
    if ( values.size() == 1 ) return Piecewise<SBasis>(values[0]);//what about time??

    SBasis bump_in = Linear(0,1);//Enough for piecewise linear interpolation.
    //bump_in.push_back(Linear(-1,1));//uncomment for C^1 interpolation
    SBasis bump_out = Linear(1,0);
    //bump_out.push_back(Linear(1,-1));
    
    Piecewise<SBasis> result;
    result.cuts.push_back(times[0]);
    for (unsigned i = 0; i<values.size()-1; i++){
        result.push(bump_out*values[i]+bump_in*values[i+1],times[i+1]);
    }
    return result;
}
*/

//#include <2geom/toys/pwsbhandle.cpp>  // FIXME: This looks like it may give problems later, (including a .cpp file)

class Squiggles: public Toy {
    unsigned segs, handles_per_curve, curves;

    PointSetHandle hand;
    unsigned current_ctl_pt;
    Point current_pos;
    Point current_dir;
    std::vector<double> curvatures;
    std::vector<double> times;
    Piecewise<D2<SBasis> > curve;
    double tot_length;
    int mode; //0=set curvature, 1=set curv.+rotation, 2=translate, 3=slide time.

    virtual void mouse_moved(GdkEventMotion* e){
        mode = 0;
        if((e->state & (GDK_SHIFT_MASK)) && 
           (e->state & (GDK_CONTROL_MASK))) {
            mode = 3;
        }else if(e->state & (GDK_CONTROL_MASK)) {
            mode = 1;
        }else if(e->state & (GDK_SHIFT_MASK)) {
            mode = 2;
        }
        Toy::mouse_moved(e);
    }


    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_source_rgba (cr, 0., 0., 0., 1);
        cairo_set_line_width (cr, 1);
        
        *notify << "Drag to set curvature,\n";
        *notify << "SHIFT-Drag to move curve,\n";
        *notify << "CTRL-Drag to rotate,\n";
        *notify << "SHIFT-CTRL-Drag to slide handles.";
        //Get user input
        if (mouse_down && selected) {
            for(unsigned i = 0; i < handles.size(); i++) {
                if(selected == handles[i]){
                    current_ctl_pt = i;
                    break;
                }
            }
            double time = times[current_ctl_pt];
            current_pos = curve.valueAt(time);
            current_dir = derivative(curve).valueAt(time);//*This should be a unit vector!*
            Point hdle = dynamic_cast<PointHandle*>(handles[current_ctl_pt])->pos;
            if (mode == 0){
                curvatures[current_ctl_pt] = cross(hdle - current_pos,current_dir)*K_SCALE;
            }else if (mode == 1){//Rotate
                double sign = ( curvatures[current_ctl_pt]>=0 ? 1 : -1 ); 
                //curvatures[current_ctl_pt] = sign*L2(hdle - current_pos)*K_SCALE;
                current_dir = -sign*unit_vector(rot90(hdle - current_pos));
            }else if (mode == 2){//Translate
                Point old_pos = current_pos + curvatures[current_ctl_pt]/K_SCALE*rot90(current_dir); 
                current_pos += hdle - old_pos;
                curve += hdle - old_pos;
            }else if (mode == 3){//Slide time
                Point old_pos = current_pos + curvatures[current_ctl_pt]/K_SCALE*rot90(current_dir); 
                double delta = dot(hdle - old_pos,current_dir);
                double epsilon = 2;
                if (current_ctl_pt>0 && times[current_ctl_pt]+delta < times[current_ctl_pt-1]+epsilon){
                    delta = times[current_ctl_pt-1] + epsilon - times[current_ctl_pt];
                }
                if (current_ctl_pt<times.size()-1 && times[current_ctl_pt]+delta > times[current_ctl_pt+1]-epsilon){
                    delta = times[current_ctl_pt+1] - epsilon - times[current_ctl_pt];
                }
                times[current_ctl_pt] += delta;
                current_pos += delta*current_dir;
            }
        }

        //Compute new curve

        Piecewise<SBasis> curvature = interpolate( times, curvatures , 1);
        Piecewise<SBasis> alpha = integral(curvature);
        Piecewise<D2<SBasis> > v = sectionize(tan2(alpha));
        curve = integral(v)+Point(100,100);	

        //transform to keep current point in place
        double time = times[current_ctl_pt];
        Point new_pos = curve.valueAt(time);
        Point new_dir = v.valueAt(time);
        Affine mat1 = Affine(    new_dir[X],    new_dir[Y],    -new_dir[Y],    new_dir[X],    new_pos[X],    new_pos[Y]);
        Affine mat2 = Affine(current_dir[X],current_dir[Y],-current_dir[Y],current_dir[X],current_pos[X],current_pos[Y]);
        mat1 = mat1.inverse()*mat2;
        curve = curve*mat1;
        v = v*mat1.withoutTranslation();

        //update handles
        cairo_save(cr);
        double dashes[2] = {3, 2};
        cairo_set_dash(cr, dashes, 2, 0);
        cairo_set_line_width(cr, .5);
        cairo_set_source_rgba (cr, 0., 0., 0.5, 1);
        for(unsigned i = 0; i < NB_CTL_PTS; i++) {
            Point m = curve.valueAt(times[i]);
            dynamic_cast<PointHandle*>(handles[i])->pos = m +
                curvatures[i]/K_SCALE*rot90(v.valueAt(times[i]));
            draw_handle(cr, m);
            cairo_move_to(cr, m);
            cairo_line_to(cr, dynamic_cast<PointHandle*>(handles[i])->pos);
        }

#if 0
        D2<Piecewise<SBasis> > graphe;
        graphe[X] = Piecewise<SBasis>(Linear(100,300));
        graphe[Y] = -curvature/K_SCALE+400;
        graphe[X].setDomain(graphe[Y].domain());
        cairo_d2_pw_sb(cr, graphe);
#endif

        cairo_stroke(cr);
        cairo_restore(cr);

	cairo_pw_d2_sb(cr, curve);
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    bool should_draw_numbers() { return false; }

public:
    Squiggles () {
        current_ctl_pt = 0;
        current_dir = Point(1,0);
        current_pos = Point(100,100);
        tot_length = 300;

        curve = Piecewise<D2<SBasis> >(D2<SBasis>(Linear(100,300),Linear(100,100)));
        for(unsigned i = 0; i < NB_CTL_PTS; i++) {
            curvatures.push_back(0);
            times.push_back(i*tot_length/(NB_CTL_PTS-1));
            PointHandle *pt_hdle = new PointHandle(Geom::Point(100+i*tot_length/(NB_CTL_PTS-1), 100.));
            handles.push_back(pt_hdle);
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
