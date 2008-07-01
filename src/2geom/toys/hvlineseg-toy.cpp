

#include "path-cairo.h"
#include "toy-framework-2.h"
#include "hvlinesegment.h"

using namespace Geom;


template<typename T>
void draw_each( cairo_t* cr, std::vector<T>& _controls)
{
	typename std::vector<T>::iterator it = _controls.begin();
	for(; it != _controls.end(); ++it)
	{
		it->draw(cr);
	}
}

class HLineSegToy : public Toy
{
    void draw( cairo_t *cr,	std::ostringstream *notify, 
    		   int width, int height, bool save ) 
    {
        if (first_time)
        {
            first_time = false;
            init_controls(notify, width, height);
        }
    	
    	pts[1][Y] = pts[0][Y];
    	HLineSegment hls(pts[0][X], pts[1][X], pts[0][Y]);
    	if (toggles[0].on)  
    	{
    		Curve* c = hls.reverse();
    		hls = *(static_cast<HLineSegment*>(c));
    		delete c;
    	}
    	
    	HLineSegment hls2;
    	hls2.setInitial(pts[2]);
    	hls2.setFinal(pts[3]);
    	hls2.setY(pts[2][Y]);
    	
    	std::vector<double> xroot = hls.roots(pts[4][X], X);
    	assert( xroot.size() < 2 );
    	
    	try
    	{
    		std::vector<double> yroot = hls.roots(pts[4][Y], Y);
    		assert( yroot.empty() );
    	}
    	catch (InfiniteSolutions e)
    	{
    		std::cerr << e.what() << std::endl;
    	}
    	
    	
    	double t = hls.nearestPoint(pts[5], 1.0/4, 3.0/4);
    	
    	double tt = sliders[0].value();
    	
    	std::pair<HLineSegment, HLineSegment> hls_pair
    		= hls2.subdivide(tt);
    	hls_pair.first.setY( hls2.getY() - 20 );
    	hls_pair.second.setY( hls2.getY() + 20 );

    	HLineSegment hls3;
    	{
    		Curve* c = hls.derivative();
    		hls3 = *(static_cast<HLineSegment*>(c));
    		delete c;
    	}
    	assert( hls3.initialPoint() == hls3.finalPoint() );
    	
    	std::vector<Point> points = hls.pointAndDerivatives(tt, 2);
    	
    	assert(points.size() == 2);
    	assert(points[0] == hls.pointAt(tt));
    	assert(points[1] == hls3.pointAt(tt));
    	
    	HLineSegment hls4;
    	{
    		Curve* c = hls.portion(tt, t);
    		hls4 = *(static_cast<HLineSegment*>(c));
    		delete c;
    	}
    	assert( hls4.initialPoint() == hls.pointAt(tt) 
    			&& hls4.finalPoint() == hls.pointAt(t) );
    	
    	hls4.setY(hls.getY() + 20);
    	
    	
    	Path p, p2, p3, p4, p5;
    	p.append(hls);
    	p2.append(hls2);
     	p3.append(hls_pair.first);
     	p4.append(hls_pair.second);
     	p5.append(hls4);
        cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
        cairo_set_line_width(cr, 0.3);
    	cairo_path(cr, p);
    	cairo_path(cr, p2);
    	cairo_path(cr, p3);
    	cairo_path(cr, p4);
    	cairo_path(cr, p5);
    	if ( xroot.size() == 1 )
    	{
    		Point prj = hls.pointAt(xroot.front());
    		draw_handle(cr, prj);
    	}
    	cairo_move_to(cr, pts[5]);
    	cairo_line_to(cr, hls.valueAt(t,X), hls.valueAt(t,Y));
    	draw_circ(cr, hls3.initialPoint());
    	cairo_stroke(cr);
    	
    	*notify
    		<< " initial point: " << hls.initialPoint() 
    		<< "   final point: " << hls.finalPoint() << std::endl
    		<< "   is degenerate: " << (hls.isDegenerate() ? "true" : "false")
    	;

    	Toy::draw(cr, notify, width, height, save);
    }

    void init_controls(std::ostringstream */*notify*/, int /*width*/, int height)
    {
    	Point time_sp = Point(10, height - 80);
    	sliders[0].geometry(time_sp, 200);
    	
        Point toggle_sp( 300, height - 80); 
        toggles[0].bounds = Rect( toggle_sp, toggle_sp + Point(100,25) );        
    }
    

  public:
	HLineSegToy()
	{	    
	    first_time = true;
		psh.push_back(Point(300, 50));
		psh.push_back(Point(300, 250));
		psh.push_back(Point(50, 100));
		psh.push_back(Point(150, 200));
		psh.push_back(Point(50, 150));
		psh.push_back(Point(150, 250));
		psh.push_back(Point(100, 100));
		
		pts = & (psh.pts[0]);
		
		sliders.push_back(Slider(0.0, 1.0, 0.1, 0.0, "t"));
		
		toggles.push_back(Toggle("Reverse", false));
		
        handles.push_back(&psh);
        handles.push_back(&(sliders[0]));
        handles.push_back(&(toggles[0]));
	}
	
  private:
    bool first_time;
    PointSetHandle psh;
    Point* pts;
	std::vector< Slider > sliders;
	std::vector<Toggle> toggles;
};

class VLineSegToy : public Toy
{
    void draw( cairo_t *cr,	std::ostringstream *notify, 
    		   int width, int height, bool save ) 
    {
        if (first_time)
        {
            first_time = false;
            init_controls(notify, width, height);
        }
    	
    	pts[1][X] = pts[0][X];
    	VLineSegment hls(pts[0][X], pts[0][Y], pts[1][Y]);
    	if (toggles[0].on)  
    	{
    		Curve* c = hls.reverse();
    		hls = *(static_cast<VLineSegment*>(c));
    		delete c;
    	}
    	
    	VLineSegment hls2;
    	hls2.setInitial(pts[2]);
    	hls2.setFinal(pts[3]);
    	hls2.setX(pts[2][X]);
    	
    	std::vector<double> yroot = hls.roots(pts[4][Y], Y);
    	assert( yroot.size() < 2 );
    	
    	try
    	{
    		std::vector<double> xroot = hls.roots(pts[4][X], X);
    		assert( xroot.empty() );
    	}
    	catch (InfiniteSolutions e)
    	{
    		std::cerr << e.what() << std::endl;
    	}
    	
    	
    	double t = hls.nearestPoint(pts[5], 1.0/4, 3.0/4);
    	
    	double tt = sliders[0].value();
    	
    	std::pair<VLineSegment, VLineSegment> hls_pair
    		= hls2.subdivide(tt);
    	hls_pair.first.setX( hls2.getX() - 20 );
    	hls_pair.second.setX( hls2.getX() + 20 );

    	VLineSegment hls3;
    	{
    		Curve* c = hls.derivative();
    		hls3 = *(static_cast<VLineSegment*>(c));
    		delete c;
    	}
    	assert( hls3.initialPoint() == hls3.finalPoint() );
    	
    	std::vector<Point> points = hls.pointAndDerivatives(tt, 2);
    	
    	assert(points.size() == 2);
    	assert(points[0] == hls.pointAt(tt));
    	assert(points[1] == hls3.pointAt(tt));
    	
    	VLineSegment hls4;
    	{
    		Curve* c = hls.portion(tt, t);
    		hls4 = *(static_cast<VLineSegment*>(c));
    		delete c;
    	}
    	assert( hls4.initialPoint() == hls.pointAt(tt) 
    			&& hls4.finalPoint() == hls.pointAt(t) );
    	
    	hls4.setX(hls.getX() + 20);
    	
    	
    	Path p, p2, p3, p4, p5;
    	p.append(hls);
    	p2.append(hls2);
     	p3.append(hls_pair.first);
     	p4.append(hls_pair.second);
     	p5.append(hls4);
        cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
        cairo_set_line_width(cr, 0.3);
    	cairo_path(cr, p);
    	cairo_path(cr, p2);
    	cairo_path(cr, p3);
    	cairo_path(cr, p4);
    	cairo_path(cr, p5);
    	if ( yroot.size() == 1 )
    	{
    		Point prj = hls.pointAt(yroot.front());
    		draw_handle(cr, prj);
    	}
    	cairo_move_to(cr, pts[5]);
    	cairo_line_to(cr, hls.valueAt(t,X), hls.valueAt(t,Y));
    	draw_circ(cr, hls3.initialPoint());
    	cairo_stroke(cr);
    	
    	*notify
    		<< " initial point: " << hls.initialPoint() 
    		<< "   final point: " << hls.finalPoint() << std::endl
    		<< "   is degenerate: " << (hls.isDegenerate() ? "true" : "false")
    	;

    	Toy::draw(cr, notify, width, height, save);
    }

    void init_controls(std::ostringstream */*notify*/, int /*width*/, int height)
    {
    	Point time_sp = Point(10, height - 80);
    	sliders[0].geometry(time_sp, 200);
    	
        Point toggle_sp( 300, height - 80); 
        toggles[0].bounds = Rect( toggle_sp, toggle_sp + Point(100,25) );
    }
    

  public:
	VLineSegToy()
	{
	    first_time = true;
	    
		psh.push_back(Point(300, 50));
		psh.push_back(Point(300, 250));
		psh.push_back(Point(50, 100));
		psh.push_back(Point(150, 200));
		psh.push_back(Point(50, 150));
		psh.push_back(Point(150, 250));
		psh.push_back(Point(100, 100));
		
		pts = & (psh.pts[0]);
		
		sliders.push_back(Slider(0.0, 1.0, 0.0, 0.0, "t"));
		
		toggles.push_back(Toggle("Reverse", false));
		
	    handles.push_back(&psh);
	    handles.push_back(&(sliders[0]));
	    handles.push_back(&(toggles[0]));
	}
	
  private:
    bool first_time;
    PointSetHandle psh;
    Point* pts;
    std::vector< Slider  > sliders;
	std::vector<Toggle> toggles;
};

int main(int argc, char **argv) 
{	
	char choice = 'h';
	if (argc > 1)
		choice = argv[1][0];
	Toy* toy;
	if ( choice == 'v' )
		toy = new VLineSegToy();
	else
		toy = new HLineSegToy();
    init( argc, argv, toy);
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

