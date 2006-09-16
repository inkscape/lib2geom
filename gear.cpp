#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include <gtk/gtk.h>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <vector>
#include "s-basis.h"
#include "interactive-bits.h"
#include "path-builder.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "path-cairo.h"
#include "multidim-sbasis.h"

#include "toy-framework.cpp"

using std::string;
using std::vector;

void draw_md_sb(cairo_t *cr, multidim_sbasis<2> const &B) {
    Geom::PathBuilder pb;
    subpath_from_sbasis(pb, B, 0.1);
    cairo_path(cr, pb.peek());
}

class Gear {
public:
    double pitch_radius() {return (number_of_teeth * module) / (2.0 * M_PI);};
    double pitch_radius(double R) {module = (2 * M_PI * R) / number_of_teeth;};
    double base_radius() {return 2.0 * pitch_radius() * cos(pressure_angle);};
    double diametrical_pitch() {return number_of_teeth / (2.0 * pitch_radius());};
    double addendum() {return 1.0 / diametrical_pitch();};
    double dedendum() {return addendum() + clearance;};
    double root_radius() {return pitch_radius() - dedendum();};
    double outer_radius() {return pitch_radius() + addendum();};
    double tooth_thickness_angle() {M_PI / number_of_teeth;};
    
    // angle of the base circle used to create the involute to a certain radius
    double involute_swath_angle(double R) {
        if (R <= base_radius()) return 0.0;
        return sqrt(R*R - base_radius()*base_radius())/base_radius();
    };

    // angle of the base circle between the origin of the involute and the intersection on another radius
    double involute_intersect_angle(double R) {
        if (R <= base_radius()) return 0.0;
        return (sqrt(R*R - base_radius()*base_radius())/base_radius()) - acos(base_radius()/R);
    };
    
    Geom::Path path(Geom::Point centre, double angle);
    
    Gear(int n, double m, double phi) {
        number_of_teeth = n;
        module = m;
        pressure_angle = phi;
        clearance = 0.0;
    }
private:
    int number_of_teeth;
    double pressure_angle;
    double module;
    double clearance;
    multidim_sbasis<2> involute(double start, double stop, BezOrd swath, Geom::Point centre) {
        multidim_sbasis<2> B;
        multidim_sbasis<2> I;
        BezOrd bo = BezOrd(start,stop);
        
        B[0] = cos(bo,2);
        B[1] = sin(bo,2);
        
        I = B - swath * derivative(B);
        I = base_radius()*I + centre;
        return I;
    }
    multidim_sbasis<2> arc(double start, double stop, double R, Geom::Point centre) {
        multidim_sbasis<2> B;
        BezOrd bo = BezOrd(start,stop);
        
        B[0] = cos(bo,2);
        B[1] = sin(bo,2);
        
        B = R*B + centre;
        return B;
    }
};
Geom::Path Gear::path(Geom::Point centre, double angle) {
    Geom::PathBuilder pb;
    // begin with the angle at the center of each tooth
    double involute_advance = involute_intersect_angle(outer_radius()) - involute_intersect_angle(root_radius());
    double tip_advance = tooth_thickness_angle() - (2 * (involute_intersect_angle(outer_radius()) - involute_intersect_angle(pitch_radius())));
    double root_advance = tooth_thickness_angle() - (2 * (involute_intersect_angle(pitch_radius()) - involute_intersect_angle(root_radius())));
    double involute_t = involute_swath_angle(root_radius())/involute_swath_angle(outer_radius());
    
    angle -= involute_intersect_angle(pitch_radius()) + involute_intersect_angle(root_radius());;
    double full_rotation = angle + (2.0 * M_PI);
    double tooth_rotation = 2.0*tooth_thickness_angle();
    for (double a=angle; a < full_rotation; a+=tooth_rotation)
    {
        double cursor = a;
        
        multidim_sbasis<2> leading_I = involute(cursor, cursor + involute_swath_angle(outer_radius()), BezOrd(involute_t,1), centre);        
        subpath_from_sbasis(pb, leading_I, 0.1);
        cursor += involute_advance;
        
        multidim_sbasis<2> tip = arc(cursor, cursor+tip_advance, outer_radius(), centre);        
        subpath_from_sbasis(pb, tip, 0.1);
        cursor += tip_advance;
        
        multidim_sbasis<2> trailing_I = involute(cursor + involute_swath_angle(outer_radius()), cursor, BezOrd(1-involute_t,1), centre);        
        subpath_from_sbasis(pb, trailing_I, 0.1);
        cursor += involute_advance;
        
        /*
        if (base_radius() > root_radius()) {
            //add line
        }
        */
        
        multidim_sbasis<2> root = arc(cursor, cursor+root_advance, root_radius(), centre);        
        subpath_from_sbasis(pb, root, 0.1);
        cursor += root_advance;
        
        /*
        if (base_radius() > root_radius()) {
            //add line
        }
        */
    }
    
    return pb.peek();
};

class GearToy: public Toy {
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
        for(int i = 0; i < handles.size(); i++) {
            draw_circ(cr, handles[i]);
        }
        cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
        cairo_set_line_width (cr, 0.5);
        double dominant_dim = std::max(width,height);
        double minor_dim = std::min(width,height);
        
        Geom::Point centre = Geom::Point(width/2,height/2);
        // draw cross hairs
        for(int i = 1; i < 2; i++) {
            cairo_move_to(cr, centre[0]-minor_dim/4, centre[1]);
            cairo_line_to(cr, centre[0]+minor_dim/4, centre[1]);
            cairo_move_to(cr, centre[0], centre[1]-minor_dim/4);
            cairo_line_to(cr, centre[0], centre[1]+minor_dim/4);
        }
        cairo_stroke(cr);
        
        Gear gear = Gear(20,20.0,20.0);
        Geom::Point gear_centre = handles[1];
        gear.pitch_radius(L2(handles[0] - gear_centre));
        double angle = atan2(handles[0] - gear_centre);
        
        Geom::Path p = gear.path(gear_centre, angle);
        cairo_path(cr, p);
        cairo_stroke(cr);
        
        // draw base radius
        cairo_new_sub_path(cr);
        cairo_arc(cr, gear_centre[0], gear_centre[1], gear.base_radius(), 0, M_PI*2);
        cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
        cairo_stroke(cr);
        
        cairo_new_sub_path(cr);
        cairo_arc(cr, gear_centre[0], gear_centre[1], gear.pitch_radius(), 0, M_PI*2);
        cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
        cairo_stroke(cr);
        
        cairo_new_sub_path(cr);
        cairo_arc(cr, gear_centre[0], gear_centre[1], gear.outer_radius(), 0, M_PI*2);
        cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
        cairo_stroke(cr);
        
        cairo_new_sub_path(cr);
        cairo_arc(cr, gear_centre[0], gear_centre[1], gear.root_radius(), 0, M_PI*2);
        cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
        cairo_stroke(cr);
        
        *notify << "pitch radius = " << gear.pitch_radius();
    }
};

int main(int argc, char **argv) {
    for(int i = 0; i < 3; i++)
        handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    
    screen_lines = false;

    init(argc, argv, "gear", new GearToy());

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
