#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis.h"

#include "path2.h"
#include "path-cairo.h"

#include "toy-framework.h"

using std::vector;
using namespace Geom;

class Gear {
public:
    // pitch circles touch on two properly meshed gears
    // all measurements are taken from the pitch circle
    double pitch_diameter() {return (_number_of_teeth * _module) / M_PI;}
    double pitch_radius() {return pitch_diameter() / 2.0;}
    void pitch_radius(double R) {_module = (2 * M_PI * R) / _number_of_teeth;}
    
    // base circle serves as the basis for the involute toothe profile
    double base_diameter() {return pitch_diameter() * cos(_pressure_angle);}
    double base_radius() {return base_diameter() / 2.0;}
    
    // diametrical pitch
    double diametrical_pitch() {return _number_of_teeth / pitch_diameter();}
    
    // height of the tooth above the pitch circle
    double addendum() {return 1.0 / diametrical_pitch();}
    // depth of the tooth below the pitch circle
    double dedendum() {return addendum() + _clearance;}
    
    // root circle specifies the bottom of the fillet between teeth
    double root_radius() {return pitch_radius() - dedendum();}
    double root_diameter() {return root_radius() * 2.0;}
    
    // outer circle is the outside diameter of the gear
    double outer_radius() {return pitch_radius() + addendum();}
    double outer_diameter() {return outer_radius() * 2.0;}
    
    // angle covered by the tooth on the pitch circle
    double tooth_thickness_angle() {return M_PI / _number_of_teeth;}
    
    Geom::Point centre() {return _centre;}
    void centre(Geom::Point c) {_centre = c;}
    
    double angle() {return _angle;}
    void angle(double a) {_angle = a;}
    
    int number_of_teeth() {return _number_of_teeth;}
    
    Geom::Path2::Path path();
    Gear spawn(int N, double a);
    
    Gear(int n, double m, double phi) {
        _number_of_teeth = n;
        _module = m;
        _pressure_angle = phi;
        _clearance = 0.0;
        _angle = 0.0;
        _centre = Geom::Point(0.0,0.0);
    }
private:
    int _number_of_teeth;
    double _pressure_angle;
    double _module;
    double _clearance;
    double _angle;
    Geom::Point _centre;
    MultidimSBasis<2> _involute(double start, double stop) {
        MultidimSBasis<2> B;
        MultidimSBasis<2> I;
        BezOrd bo = BezOrd(start,stop);
        
        B[0] = cos(bo,2);
        B[1] = sin(bo,2);
        
        I = B - BezOrd(0,1) * derivative(B);
        I = base_radius()*I + _centre;
        return I;
    }
    MultidimSBasis<2> _arc(double start, double stop, double R) {
        MultidimSBasis<2> B;
        BezOrd bo = BezOrd(start,stop);
        
        B[0] = cos(bo,2);
        B[1] = sin(bo,2);
        
        B = R*B + _centre;
        return B;
    }
    // angle of the base circle used to create the involute to a certain radius
    double involute_swath_angle(double R) {
        if (R <= base_radius()) return 0.0;
        return sqrt(R*R - base_radius()*base_radius())/base_radius();
    }

    // angle of the base circle between the origin of the involute and the intersection on another radius
    double involute_intersect_angle(double R) {
        if (R <= base_radius()) return 0.0;
        return (sqrt(R*R - base_radius()*base_radius())/base_radius()) - acos(base_radius()/R);
    }
};
Geom::Path2::Path Gear::path() {
    Geom::Path2::Path pb;
    
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
    double first_tooth_angle = _angle - ((0.5 * tip_advance) + involute_advance);
    bool first = true;
    Geom::Point last_point(0,0);
    for (int i=0; i < _number_of_teeth; i++)
    {
        double cursor = first_tooth_angle + (i * tooth_rotation);

        MultidimSBasis<2> leading_I = compose(_involute(cursor, cursor + involute_swath_angle(outer_radius())), BezOrd(involute_t,1));
        if (!first) {
            leading_I[X][0][0] = last_point[X];
            leading_I[Y][0][0] = last_point[Y];
        }        
        first = false;
        pb.append(leading_I);
        cursor += involute_advance;
        
        MultidimSBasis<2> tip = _arc(cursor, cursor+tip_advance, outer_radius());
        tip[X][0][0] = leading_I[X][0][1];
        tip[Y][0][0] = leading_I[Y][0][1];
        pb.append(tip);
        cursor += tip_advance;
        
        cursor += involute_advance;
        MultidimSBasis<2> trailing_I = compose(_involute(cursor, cursor - involute_swath_angle(outer_radius())), BezOrd(1,involute_t));
        trailing_I[X][0][0] = tip[X][0][1];
        trailing_I[Y][0][0] = tip[Y][0][1];
        pb.append(trailing_I);
       
        Geom::Point leading_start;
        Geom::Point leading_end;
        if (base_radius() > root_radius()) {
            leading_start = point_at(trailing_I,1);
            leading_end = (root_radius() * unit_vector(leading_start - _centre)) + _centre;
            pb.append(Geom::Path2::LineSegment(leading_start, leading_end));
        }
        
        MultidimSBasis<2> root = _arc(cursor, cursor+root_advance, root_radius());
        if (base_radius() > root_radius()) {
            root[X][0][0] = leading_end[X];
            root[Y][0][0] = leading_end[Y];
        } else {
            root[X][0][0] = trailing_I[X][0][1];
            root[Y][0][0] = trailing_I[Y][0][1];
            last_point[X] = root[X][0][1];
            last_point[Y] = root[Y][0][1];
        }
        pb.append(root);
        cursor += root_advance;
        
        if (base_radius() > root_radius()) {
            Geom::Point trailing_start = point_at(root,1);
            Geom::Point trailing_end = (base_radius() * unit_vector(trailing_start - _centre)) + _centre;
            pb.append(Geom::Path2::LineSegment(trailing_start, trailing_end));
            last_point = trailing_end;
        }
    }
    pb.close();
    
    return pb;
}
Gear Gear::spawn(int N, double a) {
    Gear gear(N, _module, _pressure_angle);
    double dist = gear.pitch_radius() + pitch_radius();
    gear.centre(Geom::Point::polar(a, dist) + _centre);
    double new_angle = 0.0;
    if (gear.number_of_teeth() % 2 == 0)
        new_angle -= gear.tooth_thickness_angle();
    new_angle -= (_angle) * (pitch_radius() / gear.pitch_radius());
    new_angle += (a) * (pitch_radius() / gear.pitch_radius());
    gear.angle(new_angle + a);
    return gear;
}

class GearToy: public Toy {
    public:
    GearToy () {
        for(int i = 0; i < 2; i++)
            handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    }
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
        /* draw cross hairs
        for(int i = 1; i < 2; i++) {
            cairo_move_to(cr, centre[0]-minor_dim/4, centre[1]);
            cairo_line_to(cr, centre[0]+minor_dim/4, centre[1]);
            cairo_move_to(cr, centre[0], centre[1]-minor_dim/4);
            cairo_line_to(cr, centre[0], centre[1]+minor_dim/4);
        }
        cairo_stroke(cr);*/
        
        double pressure_angle = 20.0 * M_PI / 180;
        Gear gear(15,200.0,pressure_angle);
        Geom::Point gear_centre = handles[1];
        gear.pitch_radius(L2(handles[0] - gear_centre));
        gear.angle(atan2(handles[0] - gear_centre));
        gear.centre(gear_centre);
        
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
        Geom::Path2::Path p = gear.path();
        cairo_path(cr, p);
        cairo_set_source_rgba (cr, 0., 0., 0., 0.5);
        cairo_set_line_width (cr, 2.0);
        cairo_stroke(cr);
        
        Gear gear2 = gear.spawn(5, -2.0 * M_PI / 8.0);
        Geom::Path2::Path p2 = gear2.path();
        cairo_path(cr, p2);
        cairo_set_source_rgba (cr, 0., 0., 0., 0.5);
        cairo_set_line_width (cr, 2.0);
        cairo_stroke(cr);
        
        Gear gear3 = gear2.spawn(8, 0.0 * M_PI / 8.0);
        Geom::Path2::Path p3 = gear3.path();
        cairo_path(cr, p3);
        cairo_set_source_rgba (cr, 0., 0., 0., 0.5);
        cairo_set_line_width (cr, 2.0);
        cairo_stroke(cr);
        
        Gear gear4 = gear.spawn(6, 3.0 * M_PI / 4.0);
        Geom::Path2::Path p4 = gear4.path();
        cairo_path(cr, p4);
        cairo_set_source_rgba (cr, 0., 0., 0., 0.5);
        cairo_set_line_width (cr, 2.0);
        cairo_stroke(cr);
        
        *notify << "angle = " << gear.angle();

        Toy::draw(cr, notify, width, height, save);
    }
};

int main(int argc, char **argv) {
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
