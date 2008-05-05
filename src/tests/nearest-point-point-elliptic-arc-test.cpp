

#include "d2.h"
#include "sbasis.h"
#include "path.h"



double fake_fun( double x, unsigned int i)
{
	return x + i;
}



void ea_np_test()
{
	static const unsigned int N = 3000000;
	std::vector<SVGEllipticalArc> eas;
	//std::vector< D2<SBasis> > easb;
	SVGEllipticalArc ea;
	double rx, ry, rot;
	bool large_flag, sweep_flag;
	Point p1, p2;
	bool constraints_error;
	unsigned int iter = 0;
	while ( eas.size() < N )
	{
		++iter;
		rx = 400 * uniform();
		ry = 400 * uniform();
		rot = 2*M_PI * uniform();
		large_flag = ( uniform() < 0.5 );
		sweep_flag = ( uniform() < 0.5 );
		p1 = Point(400 * uniform(), 400 * uniform());
		p2 = Point(400 * uniform(), 400 * uniform());
		try
		{
			ea = SVGEllipticalArc(p1, rx, ry, rot, large_flag, sweep_flag, p2);
			
		}
		catch ( RangeError e )
		{
			constraints_error = true;
		}
		if( constraints_error )
		{
			constraints_error = false;
			continue;
		}
		eas.push_back( ea );
		//easb.push_back( ea.toSBasis() );
	}
	std::cerr << eas.size() << std::endl;
	std::cerr << iter << std::endl;
	
	std::vector<Point> points;
	while ( points.size() < N )
	{
		points.push_back( Point( 400 * uniform(), 400 * uniform() ) );
	}
	
	time_t t1, t2;
	
	time(&t1);
	double t;
	for ( unsigned int i = 0; i < N; ++i )
	{
		t = eas[i].allNearestPoints(points[i], 1.0/3, 2.0/3).front();
		//t = ea_all_nearest_points(points[i], eas[i]).front();
		fake_fun( t, i );
	}
	time(&t2);
	
//	time(&t1);
//	for ( unsigned int i = 0; i < N; ++i )
//	{
//		fake_fun( Geom::nearest_point( points[i], easb[i] ), i );
//	}
//	time(&t2);
	
	
	std::cerr << "time: " << difftime(t2, t1) << std::endl;
}

int main(int argc, char **argv) 
{	
    ea_np_test();
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
