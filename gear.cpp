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
    // pitch circles touch on two properly meshed gears
    // all measurements are taken from the pitch circle
    double pitch_diameter() {return 2.0 * m_pitch_radius;};
    double pitch_radius() {return m_pitch_radius;};
    
    double pitch_radius(double R) {m_pitch_radius = R;};
    
    // base circle serves as the basis for the involute toothe profile
    double base_diameter() {return 2.2 * pitch_diameter() * cos(pressure_angle);};
    double base_radius() {return base_diameter() / 2.0;};
    
    // diametrical pitch
    double diametrical_pitch() {return number_of_teeth / pitch_diameter();};
    
    // height of the tooth above the pitch circle
    double addendum() {return 1.0 / diametrical_pitch();};
    // depth of the tooth below the pitch circle
    double dedendum() {return addendum() + clearance;};
    
    // root circle specifies the bottom of the fillet between teeth
    double root_radius() {return pitch_radius() - dedendum();};
    double root_diameter() {return root_radius() * 2.0;};
    
    // outer circle is the outside diameter of the gear
    double outer_radius() {return pitch_radius() + addendum();};
    double outer_diameter() {return outer_radius() * 2.0;};
    
    // angle covered by the tooth on the pitch circle
    double tooth_thickness_angle() {return M_PI / number_of_teeth;};
    
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
    
    Gear(int n, double R, double phi) {
        number_of_teeth = n;
        m_pitch_radius = R;
        pressure_angle = phi;
        clearance = 0.0;
    }
private:
    int number_of_teeth;
    double pressure_angle;
    double m_pitch_radius;
    double clearance;
    multidim_sbasis<2> involute(double start, double stop, Geom::Point centre) {
        multidim_sbasis<2> B;
        multidim_sbasis<2> I;
        BezOrd bo = BezOrd(start,stop);
        
        B[0] = cos(bo,2);
        B[1] = sin(bo,2);
        
        I = B - BezOrd(0,1) * derivative(B);
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
Geom::Path Gear::path(Geom::Point centre, double first_tooth_angle) {
    Geom::PathBuilder pb;
    
    // angle covered by a full tooth and fillet
    double tooth_rotation = 2.0 * tooth_thickness_angle();
    // angle covered by an involute
    double involute_advance = involute_intersect_angle(outer_radius()) - involute_intersect_angle(root_radius());
    // angle covered by the tooth tip
    double tip_advance = tooth_thickness_angle() - (2 * (involute_intersect_angle(outer_radius()) - involute_intersect_angle(pitch_radius())));
    // angle covered by the toothe root
    double root_advance = (tooth_rotation - tip_advance) - (2.0 * involute_advance);
    // begin drawing the involute at t if the root circle is larger than the base circle
    double involute_t = involute_swath_angle(root_radius())/involute_swath_angle(outer_radius());
    
    //rewind angle to start drawing from the leading edge of the tooth
    first_tooth_angle = first_tooth_angle - ((0.5 * tip_advance) + involute_advance);
    
    for (int i=0; i < number_of_teeth; i++)
    {
        double cursor = first_tooth_angle + (i * tooth_rotation);
        
        multidim_sbasis<2> leading_I = compose(involute(cursor, cursor + involute_swath_angle(outer_radius()), centre), BezOrd(involute_t,1));        
        subpath_from_sbasis(pb, leading_I, 0.1);
        cursor += involute_advance;
        
        multidim_sbasis<2> tip = arc(cursor, cursor+tip_advance, outer_radius(), centre);        
        subpath_from_sbasis(pb, tip, 0.1);
        cursor += tip_advance;
        
        cursor += involute_advance;
        multidim_sbasis<2> trailing_I = compose(involute(cursor, cursor - involute_swath_angle(outer_radius()), centre), BezOrd(1,involute_t));        
        subpath_from_sbasis(pb, trailing_I, 0.1);
        
        if (base_radius() > root_radius()) {
            Geom::Point leading_start = point_at(trailing_I,1);
            Geom::Point leading_end = (root_radius() * unit_vector(leading_start - centre)) + centre;
            multidim_sbasis<2> leading_fillet;
            for (int dim=0; dim < 2; dim++)
                leading_fillet[dim] = BezOrd(leading_start[dim], leading_end[dim]);
            subpath_from_sbasis(pb, leading_fillet, 0.1);
        }
        
        multidim_sbasis<2> root = arc(cursor, cursor+root_advance, root_radius(), centre);
        subpath_from_sbasis(pb, root, 0.1);
        cursor += root_advance;
        
        if (base_radius() > root_radius()) {
            Geom::Point trailing_start = point_at(root,1);
            Geom::Point trailing_end = (base_radius() * unit_vector(trailing_start - centre)) + centre;
            multidim_sbasis<2> trailing_fillet;
            for (int dim=0; dim < 2; dim++)
                trailing_fillet[dim] = BezOrd(trailing_start[dim], trailing_end[dim]);
            subpath_from_sbasis(pb, trailing_fillet, 0.1);
        }
        
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
        
        Gear gear = Gear(15,200.0,20.0);
        Geom::Point gear_centre = handles[1];
        gear.pitch_radius(L2(handles[0] - gear_centre));
        double angle = atan2(handles[0] - gear_centre);
        
        // draw radii
        cairo_new_sub_path(cr);
        cairo_arc(cr, gear_centre[0], gear_centre[1], gear.base_radius(), 0, M_PI*2);
        cairo_set_source_rgba (cr, 0., 0., 0.5, 1);
        cairo_stroke(cr);
        
        cairo_new_sub_path(cr);
        cairo_arc(cr, gear_centre[0], gear_centre[1], gear.pitch_radius(), 0, M_PI*2);
        cairo_set_source_rgba (cr, 0.5, 0., 0., 1);
        cairo_stroke(cr);
        
        cairo_new_sub_path(cr);
        cairo_arc(cr, gear_centre[0], gear_centre[1], gear.outer_radius(), 0, M_PI*2);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
        cairo_stroke(cr);
        
        cairo_new_sub_path(cr);
        cairo_arc(cr, gear_centre[0], gear_centre[1], gear.root_radius(), 0, M_PI*2);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
        cairo_stroke(cr);
        
        //draw gear
        Geom::Path p = gear.path(gear_centre, angle);
        cairo_path(cr, p);
        cairo_set_source_rgba (cr, 0., 0., 0., 0.5);
        cairo_set_line_width (cr, 2.0);
        cairo_stroke(cr);
        
        *notify << "pitch = " << gear.pitch_radius() << " base = " << gear.base_radius();
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
