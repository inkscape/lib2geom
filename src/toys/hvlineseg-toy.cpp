

#include "path-cairo.h"
#include "toy-framework.h"
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


double default_formatter(double x)
{
	return x;
}


class Slider
{
  public:
	  
	typedef  double (*formatter_t) (double );
	
	Slider( Geom::Point& _handle, double _min, double _max,	double _value, const char * _label = "" )
		: m_handle(&_handle), m_pos(Geom::Point(0,0)), m_length(1.0), 
		  m_min(_min), m_max(_max), m_dir(Geom::X), 
		  m_label(_label), m_formatter(&default_formatter)
	{
		value(_value);
	}
	
	double value() const
	{
		return ((m_max - m_min) / m_length) * ((*m_handle)[m_dir] - m_pos[m_dir]) + m_min; 
	}
	
	void value(double _value)
	{
		if ( _value < m_min ) _value = m_min;
		if ( _value > m_max ) _value = m_max;
		(*m_handle)[m_dir] 
		            = (m_length / (m_max - m_min)) * (_value - m_min) + m_pos[m_dir];
	}
	
	void geometry(Geom::Point _pos, double _length, Geom::Dim2 _dir = X)
	{
		double v = value();
		m_pos = _pos;
		m_length = _length;
		m_dir = _dir;
		value(v);
	}
	
	void draw(cairo_t* cr)
	{
		Geom::Dim2 fix_dir = static_cast<Geom::Dim2>( (m_dir + 1) % 2 );
		(*m_handle)[fix_dir] = m_pos[fix_dir];
		double diff = (*m_handle)[m_dir] - m_pos[m_dir];
		if ( diff < 0 )	(*m_handle)[m_dir] = m_pos[m_dir];
		if ( diff > m_length ) (*m_handle)[m_dir] = m_pos[m_dir] + m_length;
		
		std::ostringstream os;
		os << m_label << ": " << (*m_formatter)(value());
		
		cairo_set_source_rgba(cr, 0.1, 0.1, 0.1, 1.0);
		cairo_set_line_width(cr, 0.4);
		cairo_move_to(cr, m_pos[X], m_pos[Y]);
		if ( m_dir == X )
			cairo_rel_line_to(cr, m_length, 0);
		else
			cairo_rel_line_to(cr, m_length, 0);
		//cairo_stroke(cr);
        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
        draw_text(cr, m_pos + Point(0,5), os.str().c_str());
        //cairo_stroke(cr);
	}
	
	void formatter( formatter_t _formatter)
	{
		m_formatter = _formatter;
	}
	
 private:
	Geom::Point* m_handle;
	Geom::Point m_pos;
	double m_length;
	double m_min, m_max;
	int m_dir;
	const char* m_label;
	formatter_t m_formatter;
};


class HLineSegToy : public Toy
{
    void draw( cairo_t *cr,	std::ostringstream *notify, 
    		   int width, int height, bool save ) 
    {
    	draw_controls(cr, notify, width, height);
    	
    	handles[1][Y] = handles[0][Y];
    	HLineSegment hls(handles[0][X], handles[1][X], handles[0][Y]);
    	if (toggles[0].on)  
    	{
    		Curve* c = hls.reverse();
    		hls = *(static_cast<HLineSegment*>(c));
    		delete c;
    	}
    	
    	HLineSegment hls2;
    	hls2.setInitial(handles[2]);
    	hls2.setFinal(handles[3]);
    	hls2.setY(handles[2][Y]);
    	
    	std::vector<double> xroot = hls.roots(handles[4][X], X);
    	assert( xroot.size() < 2 );
    	
    	try
    	{
    		std::vector<double> yroot = hls.roots(handles[4][Y], Y);
    		assert( yroot.empty() );
    	}
    	catch (InfiniteSolutions e)
    	{
    		std::cerr << e.what() << std::endl;
    	}
    	
    	
    	double t = hls.nearestPoint(handles[5], 1.0/4, 3.0/4);
    	
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
    	cairo_move_to(cr, handles[5]);
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

    void draw_controls(cairo_t* cr, std::ostringstream *notify, int width, int height)
    {
    	Point time_sp = Point(10, height - 80);
    	sliders[0].geometry(time_sp, 200);
    	
        Point toggle_sp( 300, height - 80); 
        toggles[0].bounds = Rect( toggle_sp, toggle_sp + Point(100,25) );
        
        draw_each(cr, toggles);
        sliders[0].draw(cr);
        cairo_stroke(cr);
    }
    
    void mouse_pressed(GdkEventButton* e) 
    {
        toggle_events(toggles, e);
        Toy::mouse_pressed(e);
    }

  public:
	HLineSegToy()
	{
		handles.push_back(Point(300, 50));
		handles.push_back(Point(300, 250));
		handles.push_back(Point(50, 100));
		handles.push_back(Point(150, 200));
		handles.push_back(Point(50, 150));
		handles.push_back(Point(150, 250));
		handles.push_back(Point(100, 100));
		
		handles.push_back(Point());
		sliders.push_back(Slider(handles.back(), 0, 1, 0, "t"));
		
		toggles.push_back(Toggle("Reverse", false));
	}
	
  private:
	std::vector<Slider> sliders;
	std::vector<Toggle> toggles;
};

class VLineSegToy : public Toy
{
    void draw( cairo_t *cr,	std::ostringstream *notify, 
    		   int width, int height, bool save ) 
    {
    	draw_controls(cr, notify, width, height);
    	
    	handles[1][X] = handles[0][X];
    	VLineSegment hls(handles[0][X], handles[0][Y], handles[1][Y]);
    	if (toggles[0].on)  
    	{
    		Curve* c = hls.reverse();
    		hls = *(static_cast<VLineSegment*>(c));
    		delete c;
    	}
    	
    	VLineSegment hls2;
    	hls2.setInitial(handles[2]);
    	hls2.setFinal(handles[3]);
    	hls2.setX(handles[2][X]);
    	
    	std::vector<double> yroot = hls.roots(handles[4][Y], Y);
    	assert( yroot.size() < 2 );
    	
    	try
    	{
    		std::vector<double> xroot = hls.roots(handles[4][X], X);
    		assert( xroot.empty() );
    	}
    	catch (InfiniteSolutions e)
    	{
    		std::cerr << e.what() << std::endl;
    	}
    	
    	
    	double t = hls.nearestPoint(handles[5], 1.0/4, 3.0/4);
    	
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
    	cairo_move_to(cr, handles[5]);
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

    void draw_controls(cairo_t* cr, std::ostringstream *notify, int width, int height)
    {
    	Point time_sp = Point(10, height - 80);
    	sliders[0].geometry(time_sp, 200);
    	
        Point toggle_sp( 300, height - 80); 
        toggles[0].bounds = Rect( toggle_sp, toggle_sp + Point(100,25) );
        
        draw_each(cr, toggles);
        sliders[0].draw(cr);
        cairo_stroke(cr);
    }
    
    void mouse_pressed(GdkEventButton* e) 
    {
        toggle_events(toggles, e);
        Toy::mouse_pressed(e);
    }

  public:
	VLineSegToy()
	{
		handles.push_back(Point(300, 50));
		handles.push_back(Point(300, 250));
		handles.push_back(Point(50, 100));
		handles.push_back(Point(150, 200));
		handles.push_back(Point(50, 150));
		handles.push_back(Point(150, 250));
		handles.push_back(Point(100, 100));
		
		handles.push_back(Point());
		sliders.push_back(Slider(handles.back(), 0, 1, 0, "t"));
		
		toggles.push_back(Toggle("Reverse", false));
	}
	
  private:
	std::vector<Slider> sliders;
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

