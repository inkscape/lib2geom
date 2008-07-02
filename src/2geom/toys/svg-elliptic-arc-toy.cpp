#include <memory>
#include <2geom/path.h>
#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>


using namespace Geom;


class EllipticToy: public Toy 
{
  public:
    virtual void draw(
    			cairo_t *cr, 
    			std::ostringstream *notify, 
    			int width, int height, 
    			bool save
    		) 
    {
        start_point = extremes.pts[0];
        end_point = extremes.pts[1];
        large_arc = toggles[0].on;
        sweep = toggles[1].on;
        
        rx = 2 * (cursors.pts[0][X] - slider_x_min);
        ry = 2 * (cursors.pts[1][X] - slider_x_min);
        rot_angle = deg_to_rad( 2 * (cursors.pts[2][X] - slider_x_min) );
        from_t = (cursors.pts[3][X] - slider_x_min) / 100.0;
        to_t = (cursors.pts[4][X] - slider_x_min) / 100.0;
        
        if ( are_near(start_point, end_point) )
        {
        	draw_controls(cr, width, height, notify);
        	draw_text(cr, Point(10,10), "there are infinite eclipses passing for one point");
        	Toy::draw(cr, notify, width, height, save);
        	return;
        }
        
        // calculate the center of the two possible ellipse supporting the arc if any
        std::auto_ptr< std::pair<Point,Point> > 
        centers( calculate_ellipse_centers( start_point, end_point, rx, ry, rot_angle ) );
        if ( centers.get() == NULL )
        {
            //*notify << "< Ellipse Toy >" 
            //        << " No Ellipse Arc satisfies the given geometric constraints \n";
        	draw_controls(cr, width, height, notify);
        	draw_text(cr, Point(10,10), "there is no eclipse satisfying the given constraints");
        	Toy::draw(cr, notify, width, height, save);
        	return;
        }
        
        // init elliptical arc
        EllipticalArc ea( start_point, rx, ry, rot_angle, large_arc, sweep, end_point );
        EllipticalArc* eap = NULL;
        if ( toggles[2].on )
        {
        	eap = static_cast<EllipticalArc*>(ea.portion(from_t, to_t));
        }
        EllipticalArc& elliptic_arc = ea;
        start_angle = elliptic_arc.start_angle();
        end_angle = elliptic_arc.end_angle();
        cx = elliptic_arc.center(X);
        cy = elliptic_arc.center(Y);
        

        // draw axes passing through the center of the ellipse supporting the arc
        cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
        cairo_set_line_width(cr, 0.2);
        draw_axes(cr);
        
        
        // draw the the 2 ellipse with rays rx, ry passing through 
        // the 2 given point and with the x-axis inclined of rot_angle
        draw_elliptical_arc_with_cairo( cr,	centers->first[X], centers->first[Y],	
        							    rx, ry, 0, 2*M_PI, rot_angle );
        cairo_stroke(cr);
        draw_elliptical_arc_with_cairo( cr,	centers->second[X], centers->second[Y],	
        							    rx, ry, 0, 2*M_PI, rot_angle );
        cairo_stroke(cr);
        
        
        // draw the arc with cairo in order to make a visual comparison
//        cairo_set_line_width(cr, 0.3);
//        cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
//        if ( elliptic_arc.sweep_flag() )
//        {
//        	draw_elliptical_arc_with_cairo(	cr,	cx, cy,	rx, ry, 
//        									start_angle, end_angle,	rot_angle );
//    	}
//    	else
//    	{
//        	draw_elliptical_arc_with_cairo(	cr,	cx, cy,	rx, ry, 
//        									end_angle, start_angle,	rot_angle );   		
//    	}
//        cairo_stroke(cr);
        
        
        // convert the elliptical arc to a sbasis path and draw it
        D2<SBasis> arc = elliptic_arc.toSBasis();
        cairo_set_line_width(cr, 0.2);
        cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
        cairo_md_sb(cr, arc);
        cairo_stroke(cr);
        
        if ( toggles[2].on and eap != NULL )
        {
        	
        	cairo_set_line_width(cr, 0.4);
        	cairo_set_source_rgba(cr, 0.0, 1.0, 1.0, 1.0);
        	cairo_move_to(cr, eap->center(X), eap->center(Y));
        	cairo_line_to(cr, eap->initialPoint()[X], eap->initialPoint()[Y]);
        	cairo_move_to(cr, eap->center(X), eap->center(Y));
        	cairo_line_to(cr, eap->finalPoint()[X], eap->finalPoint()[Y]);
        	cairo_stroke(cr);
        	D2<SBasis> sub_arc = eap->toSBasis();
            cairo_md_sb(cr, sub_arc);
            cairo_stroke(cr);
        }
        
        draw_controls(cr, width, height, notify);
        
        Toy::draw(cr, notify, width, height, save);
        
        if ( toggles[2].on and eap != NULL )
        {
        	delete eap;
        }
    }


  public:
    EllipticToy() 
    {
        // change these parameters to get all the possible elliptical arc variants
        start_angle = (11.0/6.0) * M_PI;
        sweep_angle = (2.0/6.0) * M_PI;
        end_angle = start_angle + sweep_angle;
        rot_angle = (0.0/6.0) * M_PI;
        rx = 200;
        ry = 150;
        cx = 300;
        cy = 300;
    
        start_point = Point(cx + rx * std::cos(start_angle), cy + ry * std::sin(start_angle) );
        end_point = Point( cx + rx * std::cos(end_angle), cy + ry * std::sin(end_angle) );

        large_arc = false;
        sweep = true;
        
        // parameters for portion method
        from_t = 0;
        to_t = 1;
        
        
        slider_x_min = 130;
        double rot_angle_deg = decimal_round(rad_to_deg(rot_angle),2);
        
        handles.push_back(&extremes);
        handles.push_back(&cursors);
        
        extremes.pts.push_back(start_point);
        extremes.pts.push_back(end_point);
        cursors.pts.push_back(Point(rx/2 + slider_x_min , 0));
        cursors.pts.push_back(Point(ry/2 + slider_x_min, 0));
        cursors.pts.push_back(Point(rot_angle_deg/2 + slider_x_min, 0));
        cursors.pts.push_back(Point(100 * from_t + slider_x_min, 0));
        cursors.pts.push_back(Point(100 * to_t + slider_x_min, 0));
        
        toggles.push_back( Toggle("Large Arc", large_arc) );
        toggles.push_back( Toggle("Clockwise", sweep) );
        toggles.push_back( Toggle("Enable Portion",false) );
    }

  private:
	  
    void mouse_pressed(GdkEventButton* e) 
    {
        toggle_events(toggles, e);
        Toy::mouse_pressed(e);
    }
    
    void draw_axes(cairo_t* cr) const
  	{
    	Point D(std::cos(rot_angle), std::sin(rot_angle));
    	Point Dx = (rx + 20) * D;
    	Point Dy = (ry + 20) * D.cw();
    	Point C(cx,cy);
    	Point LP = C - Dx;
    	Point RP = C + Dx;
    	Point UP = C - Dy;
    	Point DP = C + Dy;
    	
    	cairo_move_to(cr, LP[X], LP[Y]);
    	cairo_line_to(cr, RP[X], RP[Y]);
    	cairo_move_to(cr, UP[X], UP[Y]);
    	cairo_line_to(cr, DP[X], DP[Y]);
    	cairo_move_to(cr, 0, 0);
    	cairo_stroke(cr);
  	}
    
    void draw_elliptical_arc_with_cairo(
    		cairo_t *cr, 
    		double _cx, double _cy,
    		double _rx, double _ry,
    		double _sa, double _ea,
    		double _ra = 0
    		) const
    {
        double cos_rot_angle = std::cos(_ra);
        double sin_rot_angle = std::sin(_ra);
        cairo_matrix_t transform_matrix;
        cairo_matrix_init( &transform_matrix, 
                           _rx * cos_rot_angle, _rx * sin_rot_angle,
                          -_ry * sin_rot_angle, _ry * cos_rot_angle,
                           _cx,                 _cy 
                         );
        cairo_save(cr);
        cairo_transform(cr, &transform_matrix);
        cairo_arc(cr, 0, 0, 1, _sa, _ea);
        cairo_restore(cr);
    }
    
    void draw_controls(cairo_t* cr, int width, int height, std::ostringstream *notify)
    {
        double rx_slider_y = height - 110;
        double ry_slider_y = rx_slider_y + 20;
        double angle_slider_y = ry_slider_y + 20;
        double from_slider_y = angle_slider_y + 20;
        double to_slider_y = from_slider_y + 20;
        double ray_slider_len = 250;
        double angle_slider_len = 180;
        double from_to_slider_len = 100;
        double ray_slider_x_max = slider_x_min + ray_slider_len;
        double angle_slider_x_max = slider_x_min + angle_slider_len;
        double from_to_slider_x_max = slider_x_min + from_to_slider_len;

        
        cursors.pts[0][Y] = rx_slider_y;
        if ( cursors.pts[0][X] < slider_x_min) cursors.pts[0][X] = slider_x_min;
        if ( cursors.pts[0][X] > ray_slider_x_max) cursors.pts[0][X] = ray_slider_x_max;
        cursors.pts[1][Y] = ry_slider_y;
        if ( cursors.pts[1][X] < slider_x_min) cursors.pts[1][X] = slider_x_min;
        if ( cursors.pts[1][X] > ray_slider_x_max) cursors.pts[1][X] = ray_slider_x_max;
        cursors.pts[2][Y] = angle_slider_y;
        if ( cursors.pts[2][X] < slider_x_min) cursors.pts[2][X] = slider_x_min;
        if ( cursors.pts[2][X] > angle_slider_x_max) cursors.pts[2][X] = angle_slider_x_max;
        cursors.pts[3][Y] = from_slider_y;
        if ( cursors.pts[3][X] < slider_x_min) cursors.pts[3][X] = slider_x_min;
        if ( cursors.pts[3][X] > from_to_slider_x_max) cursors.pts[3][X] = from_to_slider_x_max;
        cursors.pts[4][Y] = to_slider_y;
        if ( cursors.pts[4][X] < slider_x_min) cursors.pts[4][X] = slider_x_min;
        if ( cursors.pts[4][X] > from_to_slider_x_max) cursors.pts[4][X] = from_to_slider_x_max;
       
        
        cairo_set_source_rgba(cr, 0.1, 0.1, 0.1, 1.0);
        cairo_set_line_width(cr, 0.4);
        cairo_move_to(cr, slider_x_min, rx_slider_y);
        cairo_rel_line_to(cr, ray_slider_len, 0);
        cairo_move_to(cr, slider_x_min, ry_slider_y);
        cairo_rel_line_to(cr, ray_slider_len, 0);
        cairo_move_to(cr, slider_x_min, angle_slider_y);
        cairo_rel_line_to(cr, angle_slider_len, 0);
        cairo_move_to(cr, slider_x_min, from_slider_y);
        cairo_rel_line_to(cr, from_to_slider_len, 0);
        cairo_move_to(cr, slider_x_min, to_slider_y);
        cairo_rel_line_to(cr, from_to_slider_len, 0);
        cairo_stroke(cr);
        
        
        Point T(width - 300, height - 60);
        
        toggles[0].bounds = Rect( T, T + Point(100,25) );
        toggles[1].bounds = Rect( T + Point(105,0), T + Point(205,25) );
        toggles[2].bounds = Rect( T + Point(0,30), T + Point(160,55) );
        draw_toggles(cr, toggles);
        
        *notify << std::endl << "  rx: " << rx << std::endl
                << "  ry: " << ry << std::endl
                << "  angle: " << decimal_round(rad_to_deg(rot_angle),2) << std::endl
                << "  from: " << decimal_round(from_t,2) << std::endl
                << "  to: " << decimal_round(to_t,2) << std::endl; 

    }
    
    std::pair<Point,Point>*
    calculate_ellipse_centers( Point _initial_point, Point _final_point,
                               double _rx,           double _ry,
                               double _rot_angle = 0
                             ) const
    {
        double sin_rot_angle = std::sin(_rot_angle);
        double cos_rot_angle = std::cos(_rot_angle);

        Point sp = _initial_point;
        Point ep = _final_point;
        
        Matrix m( _rx * cos_rot_angle, _rx * sin_rot_angle,
                 -_ry * sin_rot_angle, _ry * cos_rot_angle,
                  0,                   0 );
        Matrix im = m.inverse();
        Point sol = (ep - sp) * im;
        double half_sum_angle = std::atan2(-sol[X], sol[Y]);
        double half_diff_angle;
        if ( are_near(std::fabs(half_sum_angle), M_PI/2) )
        {
            double anti_sgn_hsa = (half_sum_angle > 0) ? -1 : 1;
            double arg = anti_sgn_hsa * sol[X] / 2;
            // if |arg| is a little bit > 1 acos returns nan
            if ( are_near(arg, 1) )
            half_diff_angle = 0;
            else if ( are_near(arg, -1) )
                half_diff_angle = M_PI;
            else
            {
                if ( !(-1 < arg && arg < 1) ) return NULL; 
                //assert( -1 < arg && arg < 1 );
                //  if it fails 
                // => there is no ellipse that satisfies the given constraints
                half_diff_angle = std::acos( arg );
            }
            half_diff_angle = M_PI/2 - half_diff_angle;
        }
        else
        {
            double  arg = sol[Y] / ( 2 * std::cos(half_sum_angle) );
            // if |arg| is a little bit > 1 asin returns nan
            if ( are_near(arg, 1) ) 
                half_diff_angle = M_PI/2;
            else if ( are_near(arg, -1) )
                half_diff_angle = -M_PI/2;
            else
            {
            	if ( !(-1 < arg && arg < 1) ) return NULL;
                //assert( -1 < arg && arg < 1 );  
                // if it fails 
                // => there is no ellipse that satisfies the given constraints
                half_diff_angle = std::asin( arg );
            }
        }

        std::pair<Point,Point>* centers = new std::pair<Point,Point>();

        if ( half_sum_angle < 0 ) half_sum_angle += 2*M_PI;
        
        double hda = half_diff_angle;
        if ( hda < 0 ) hda += M_PI;
        double sa = half_sum_angle - hda;
        // 0 <= start_angle < 2PI
        if ( sa < 0 ) sa += 2*M_PI;
        sol[0] = std::cos(sa);
        sol[1] = std::sin(sa);
        centers->first = sp - sol * m;

        hda = -half_diff_angle;
        if ( hda < 0 ) hda +=  M_PI;
        sa = half_sum_angle - hda;
        if ( sa < 0 ) sa += 2*M_PI;
        sol[0] = std::cos(sa);
        sol[1] = std::sin(sa);
        centers->second = sp - sol * m;

        return centers;
    }

  private:
    PointSetHandle extremes;
    PointSetHandle cursors;
	std::vector<Toggle> toggles;
	double slider_x_min;
    double start_angle, sweep_angle, end_angle, rot_angle;
    double rx, ry, cx, cy;
    Point start_point, end_point;
    bool large_arc, sweep;
    double from_t, to_t;
    
    
};


int main(int argc, char **argv) 
{	
    init( argc, argv, new EllipticToy(), 850, 780 );
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
