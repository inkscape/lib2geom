#include <cxxtest/TestSuite.h>

#include <libnr/scale.h>
#include <libnr/scale-ops.h>
using Geom::X;
using Geom::Y;

class NrScaleTest : public CxxTest::TestSuite
{
public:

    NrScaleTest() :
        sa( 1.5, 2.0 ),
        b( -2.0, 3.0 ),
        sb( b )
    {
    }
    virtual ~NrScaleTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static NrScaleTest *createSuite() { return new NrScaleTest(); }
    static void destroySuite( NrScaleTest *suite ) { delete suite; }

    Geom::scale const sa;
    Geom::Point const b;
    Geom::scale const sb;



    void testXY_CtorArrayOperator(void)
    {
        TS_ASSERT_EQUALS( sa[X], 1.5 );
        TS_ASSERT_EQUALS( sa[Y], 2.0 );
        TS_ASSERT_EQUALS( sa[0u], 1.5 );
        TS_ASSERT_EQUALS( sa[1u], 2.0 );
    }


    void testCopyCtor_AssignmentOp_NotEquals(void)
    {
        Geom::scale const sa_copy(sa);
        TS_ASSERT_EQUALS( sa, sa_copy );
        TS_ASSERT(!( sa != sa_copy ));
        TS_ASSERT( sa != sb );
    }

    void testAssignmentOp(void)
    {
        Geom::scale sa_eq(sb);
        sa_eq = sa;
        TS_ASSERT_EQUALS( sa, sa_eq );
    }

    void testPointCtor(void)
    {
        TS_ASSERT_EQUALS( sb[X], b[X] );
        TS_ASSERT_EQUALS( sb[Y], b[Y] );
    }

    void testOpStarPointScale(void)
    {
        Geom::Point const ab( b * sa );
        TS_ASSERT_EQUALS( ab, Geom::Point(-3.0, 6.0) );
    }

    void testOpStarScaleScale(void)
    {
        Geom::scale const sab( sa * sb );
        TS_ASSERT_EQUALS( sab, Geom::scale(-3.0, 6.0) );
    }

    void testOpDivScaleScale(void)
    {
        Geom::scale const sa_b( sa / sb );
        Geom::scale const exp_sa_b(-0.75, 2./3.);
        TS_ASSERT_EQUALS( sa_b[0], exp_sa_b[0] );
//      TS_ASSERT_EQUALS( fabs( sa_b[1] - exp_sa_b[1] ) < 1e-10 );
        TS_ASSERT_DELTA( sa_b[1], exp_sa_b[1], 1e-10 );
    }
};

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
