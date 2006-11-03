//"http://jgt.akpeters.com/papers/Vincent02/BezType.html"
/*
 * Implementation of the algorithm described in:
 *
 *      Stephen Vincent.
 *      Fast Detection of the Geometric Form of Two-Dimensional Cubic B&eacute;zier Curves.
 *      Journal of Graphics Tools, 7(3):43-51, 2002
 *
 * See the paper for discussion of the algorithm.
 */

#include <math.h>

#define	kBezTypeArch                    0
#define	kBezTypeSingleInflection        1
#define	kBezTypeDoubleInflection        2
#define	kBezTypeCusp                    3
#define	kBezTypeSelfIntersecting        4
#define	kBezTypeStraightLine            5

#define	kEpsilon		1e-10

#define	Sqr(x)			((x)*(x))
#define	Cube(x)			((x)*(x)*(x))
#define	Abs(x)			((x) > 0 ? (x) : -(x))

typedef struct 
{
    double	x;
    double	y;
} TPoint2;

static bool PtEqual ( TPoint2 pt0 , TPoint2 pt1 )

{
    if ( ( Abs ( pt0.x - pt1.x ) < kEpsilon ) &&
         ( Abs ( pt0.y - pt1.y ) < kEpsilon ) )
        return true;
    else
        return false;
}

static int Sgn ( double a )


{
    if ( a > kEpsilon )
        return 1;
    else if ( a < -kEpsilon )
        return -1;
    else
        return 0;
}

static double Determinant ( TPoint2 pt0 , TPoint2 pt1 , TPoint2 pt2 )

{

    double		det;

    det = ( pt1.x - pt0.x ) * ( pt2.y - pt0.y ) -
        ( pt1.y - pt0.y ) * ( pt2.x - pt0.x );

    return ( det );
}

static void EvaluatePt ( double t , TPoint2 *cpt , TPoint2 *pt )

    //	Evaluates the point on a cubic Bezier whose control points are in the array cpt

    //	at parameter value t and returns the resuls in pt


{

    pt->x = cpt [0].x * Cube( 1 - t ) + 
        cpt [1].x * 3*t * Sqr( 1 - t ) +
        cpt [2].x * 3*Sqr ( t ) * ( 1 - t ) +
        cpt [3].x * Cube ( t );

    pt->y = cpt [0].y * Cube( 1 - t ) + 
        cpt [1].y * 3*t * Sqr( 1 - t ) +
        cpt [2].y * 3*Sqr ( t ) * ( 1 - t ) +
        cpt [3].y * Cube ( t );  

}

static short PointRelativeToLine ( TPoint2 p0 , TPoint2 p1 , TPoint2 p2 )

    //	Determine the position of a point p2 with respect to a line

    //	defined by p0 and p1. 'Find' the closest point on the line to p2.

    //	If it lies before p0 return -1 , if it lies after p1 return 1 :

    //	otherwise return 0


{

    double		a;
    double		b;

    a = - ( p1.x - p0.x ) * ( p2.x - p0.x );
    b = ( p1.y - p0.y ) * ( p2.y - p0.y );

    if ( ( a - b ) >= 0 )
        return ( -1 );
    else
    {

        a = - ( p0.x - p1.x ) * ( p2.x - p1.x );
        b = ( p0.y - p1.y ) * ( p2.y - p1.y );

        if ( ( a - b ) >= 0.0 )
            return ( 1 );
        else
            return ( 0 );

    }

}


static short CuspValue ( double det_012 , double det_013 , double det_023 )

//	Distinguish between loop, cusp, and 2 inflection point curves


{
    double		a;
    double		b;
    double		c;
    double		d;

    a = 3*det_012 + det_023 - 2*det_013;
    b = -3*det_012 + det_013;
    c = det_012;

    d = b*b - 4*a*c;

    //	Rather than test against kEpsilon, you could test against sqrt(d)/2*a for

    //	a better approximation to a cusp.


    if ( d > kEpsilon )
        return kBezTypeDoubleInflection;
    else
        if ( d < -kEpsilon )
            return kBezTypeSelfIntersecting;
        else
            return kBezTypeCusp;

}

int	GetBezierType ( TPoint2 *pt )

{

    double		det_012;
    double		det_123;
    double		det_013;
    double		det_023;
    int			sign_012;
    int			sign_123;
    int			sign_013;
    int			sign_023;
    double		t;
    TPoint2		pt_t;

    det_012 = Determinant ( pt [0] , pt [1] , pt [2] );
    det_123 = Determinant ( pt [1] , pt [2] , pt [3] );
    det_013 = Determinant ( pt [0] , pt [1] , pt [3] );
    det_023 = Determinant ( pt [0] , pt [2] , pt [3] );

    sign_012 = Sgn ( det_012 );
    sign_123 = Sgn ( det_123 );
    sign_013 = Sgn ( det_013 );
    sign_023 = Sgn ( det_023 );

    //	First address the cases where 3 or more consecutive control points

    //	are colinear.


    if ( sign_012 == 0 && sign_123 == 0 )
    {

        if ( sign_013 == 0 )
            return kBezTypeStraightLine;	//	Case E : all 4 points are colinear. Could test for a single point here if necessary

        else	
            return kBezTypeArch;			//	Points 1 and 2 coincident


    }
    else if ( sign_012 == 0 )
    {

        //	Case F : first 3 control points are colinear


        if ( sign_013 == sign_123 )
            return kBezTypeArch;
        else
            return kBezTypeSingleInflection;

    }
    else if ( sign_123 == 0 )
    {

        //	Case F : second 3 control points are colinear


        if ( sign_023 == sign_012 )
            return kBezTypeArch;
        else
            return kBezTypeSingleInflection;

    }
    else if ( sign_013 == 0 )
    {

        //	Case G : points 0,1,3 are colinear


        short k = PointRelativeToLine ( pt [0] , pt [3] , pt [1] );

        if ( k == -1 )
            return kBezTypeArch;
        else
            if ( k == 0 )
                return kBezTypeSingleInflection;
            else
                return CuspValue ( det_012 , det_013 , det_023 );			

    }
    else if ( sign_023 == 0 )
    {

        //	Case G : points 0,2,3 are colinear


        short k = PointRelativeToLine ( pt [0] , pt [3] , pt [2] );

        if ( k == 1 )
            return kBezTypeArch;
        else
            if ( k == 0 )
                return kBezTypeSingleInflection;
            else
                return CuspValue ( det_012 , det_013 , det_023 );			

    }

    //	OK : on to the more interesting stuff. At this point it's known that

    //	no 3 of the control points are colinear


    else if ( sign_012 != sign_123 )
    {

        //	Case A : the control points zig-zag


        return kBezTypeSingleInflection;	
    }
    else if ( sign_012 == sign_013 && sign_012 == sign_023 )
    {

        //	Case B : Convex control polygon


        return kBezTypeArch;
    }	
    else if ( sign_012 != sign_013 && sign_012 != sign_023 )
    {

        //	Case C : Self-intersecting control polygon


        return CuspValue ( det_012 , det_013 , det_023 );		
    }
    else
    {

        //	Case D : Concave control polygon


        t = det_013 / ( det_013 - det_023 );

        EvaluatePt ( t , pt , &pt_t );

        if ( PointRelativeToLine ( pt [0] , pt [3] , pt_t ) == 0 )
        {
            return CuspValue ( det_012 , det_013 , det_023 );			
        }
        else
            return kBezTypeArch;

    }

}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

