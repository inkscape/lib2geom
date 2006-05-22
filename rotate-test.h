#include <cxxtest/TestSuite.h>

#include <cmath>
#include <libnr/matrix.h>
#include <libnr/matrix-ops.h>
#include <libnr/matrix-fns.h>        /* identity, matrix_equalp */
#include <libnr/point-fns.h>
#include <libnr/rotate.h>
#include <libnr/rotate-fns.h>
#include <libnr/rotate-ops.h>
using Geom::X;
using Geom::Y;


class NrRotateTest : public CxxTest::TestSuite
{
public:

    NrRotateTest() :
        m_id( Geom::identity() ),
        r_id( 0.0 ),
        rot234( .234 ),
        b( -2.0, 3.0 ),
        rot180( Geom::Point(-1.0, 0.0) )
    {
    }
    virtual ~NrRotateTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static NrRotateTest *createSuite() { return new NrRotateTest(); }
    static void destroySuite( NrRotateTest *suite ) { delete suite; }

    Geom::Matrix const m_id;
    Geom::rotate const r_id;
    Geom::rotate const rot234;
    Geom::Point const b;
    Geom::rotate const rot180;




    void testCtorsCompares(void)
    {
        TS_ASSERT_EQUALS( r_id, r_id );
        TS_ASSERT_EQUALS( rot234, rot234 );
        TS_ASSERT_DIFFERS( rot234, r_id );
        TS_ASSERT_EQUALS( r_id, Geom::rotate(Geom::Point(1.0, 0.0)) );
        TS_ASSERT_EQUALS( Geom::Matrix(r_id), m_id );
        TS_ASSERT( Geom::Matrix(r_id).test_identity() );

        TS_ASSERT(rotate_equalp(rot234, Geom::rotate(Geom::Point(cos(.234), sin(.234))), 1e-12));
    }

    void testAssignmentOp(void)
    {
        Geom::rotate rot234_eq(r_id);
        rot234_eq = rot234;
        TS_ASSERT_EQUALS( rot234, rot234_eq );
        TS_ASSERT_DIFFERS( rot234_eq, r_id );
    }

    void testInverse(void)
    {
        TS_ASSERT_EQUALS( r_id.inverse(), r_id );
        TS_ASSERT_EQUALS( rot234.inverse(), Geom::rotate(-.234) );
    }

    void testOpStarPointRotate(void)
    {
        TS_ASSERT_EQUALS( b * r_id, b );
        TS_ASSERT_EQUALS( b * rot180, -b );
        TS_ASSERT_EQUALS( b * rot234, b * Geom::Matrix(rot234) );
        TS_ASSERT(point_equalp(b * Geom::rotate(M_PI / 2),
                               Geom::rot90(b),
                               1e-14));
        TS_ASSERT_EQUALS( b * rotate_degrees(90.), Geom::rot90(b) );
    }

    void testOpStarRotateRotate(void)
    {
        TS_ASSERT_EQUALS( r_id * r_id, r_id );
        TS_ASSERT_EQUALS( rot180 * rot180, r_id );
        TS_ASSERT_EQUALS( rot234 * r_id, rot234 );
        TS_ASSERT_EQUALS( r_id * rot234, rot234 );
        TS_ASSERT( rotate_equalp(rot234 * rot234.inverse(), r_id, 1e-14) );
        TS_ASSERT( rotate_equalp(rot234.inverse() * rot234, r_id, 1e-14) );
        TS_ASSERT( rotate_equalp(( Geom::rotate(0.25) * Geom::rotate(.5) ),
                                 Geom::rotate(.75),
                                 1e-10) );
    }

    void testOpDivRotateRotate(void)
    {
        TS_ASSERT_EQUALS( rot234 / r_id, rot234 );
        TS_ASSERT_EQUALS( rot234 / rot180, rot234 * rot180 );
        TS_ASSERT( rotate_equalp(rot234 / rot234, r_id, 1e-14) );
        TS_ASSERT( rotate_equalp(r_id / rot234, rot234.inverse(), 1e-14) );
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
