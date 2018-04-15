#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-geometric.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#include <time.h>
using std::vector;
using namespace Geom;
using namespace std;

// TODO: 
// use path2
// replace Ray stuff with path2 line segments.

//-----------------------------------------------

class PortionTester: public Toy {
    PointSetHandle curve_handle;
    PointHandle sample_point1, sample_point2;
    std::vector<Toggle> toggles;
    void mouse_pressed(GdkEventButton* e) override {
        toggle_events(toggles, e);
        Toy::mouse_pressed(e);
    }
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
    
        draw_toggles(cr, toggles);
        D2<SBasis> B = curve_handle.asBezier();
        
        cairo_set_line_width (cr, 1);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
        cairo_d2_sb(cr, B);
        cairo_stroke(cr);

	sample_point1.pos[1]=400;
	sample_point1.pos[0]=std::max(150.,sample_point1.pos[0]);
	sample_point1.pos[0]=std::min(450.,sample_point1.pos[0]);
	sample_point2.pos[1]=400;
	sample_point2.pos[0]=std::max(150.,sample_point2.pos[0]);
	sample_point2.pos[0]=std::min(450.,sample_point2.pos[0]);
	cairo_move_to(cr, Geom::Point(150,400));
	cairo_line_to(cr, Geom::Point(450,400));
	cairo_set_source_rgba (cr, 0., 0., 0.5, 0.8);
	cairo_stroke(cr);

	double t0=std::max(0.,std::min(1.,(sample_point1.pos[0]-150)/300.));
	double t1=std::max(0.,std::min(1.,(sample_point2.pos[0]-150)/300.));
        
        Path P;
        P.append(B);
        
        if (toggles[0].on) {
            if (toggles[1].on)
                cairo_curve(cr, P.portion(t0,t1)[0]);
            else
                cairo_path(cr, P.portion(t0,t1));
        } else
            cairo_d2_sb(cr, portion(B,t0,t1));
        
        
	cairo_set_source_rgba (cr, 0.5, 0.2, 0., 0.8);
	cairo_stroke(cr);
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }        
  
public:
    PortionTester(){
        toggles.emplace_back("Path", true);
        toggles[0].bounds = Rect(Point(10,100), Point(100, 130));
        toggles.emplace_back("Curve", true);
        toggles[1].bounds = Rect(Point(10,130), Point(100, 160));
        if(handles.empty()) {
            handles.push_back(&curve_handle);
            handles.push_back(&sample_point1);
            handles.push_back(&sample_point2);
            for(unsigned i = 0; i < 4; i++)
                curve_handle.push_back(150+uniform()*300,150+uniform()*300);
            sample_point1.pos = Geom::Point(250,300);
            sample_point2.pos = Geom::Point(350,300);
        }
    }
};

int main(int argc, char **argv) {
    std::cout << "testing unit_normal(multidim_sbasis) based offset." << std::endl;
    init(argc, argv, new PortionTester);
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
//vim:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :

 	  	 
