#include <cxxtest/TestSuite.h>

#include <libnr/point-ops.h>
#include <libnr/matrix.h>
#include <libnr/matrix-fns.h>
#include <libnr/matrix-ops.h>
#include <libnr/point-matrix-ops.h>
#include <libnr/translate.h>
#include <libnr/translate-ops.h>
using Geom::X;
using Geom::Y;

class NrTranslateTest : public CxxTest::TestSuite
{
public:

    NrTranslateTest() :
        b( -2.0, 3.0 ),
        tb( b ),
        tc( -3.0, -2.0 ),
        tbc( tb * tc ),
        t_id( 0.0, 0.0 ),
        m_id( Geom::identity() )
    {
    }
    virtual ~NrTranslateTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static NrTranslateTest *createSuite() { return new NrTranslateTest(); }
    static void destroySuite( NrTranslateTest *suite ) { delete suite; }

    Geom::Point const b;
    Geom::translate const tb;
    Geom::translate const tc;
    Geom::translate const tbc;
    Geom::translate const t_id;
    Geom::Affine const m_id;


    void testCtorsArrayOperator(void)
    {
        TS_ASSERT_EQUALS( tc[X], -3.0 );
        TS_ASSERT_EQUALS( tc[Y], -2.0 );

        TS_ASSERT_EQUALS( tb[0], b[X] );
        TS_ASSERT_EQUALS( tb[1], b[Y] );
    }

    void testAssignmentOperator(void)
    {
        Geom::translate tb_eq(tc);
        tb_eq = tb;
        TS_ASSERT_EQUALS( tb, tb_eq );
        TS_ASSERT_DIFFERS( tb_eq, tc );
    }

    void testOpStarTranslateTranslate(void)
    {
        TS_ASSERT_EQUALS( tbc.offset, Geom::Point(-5.0, 1.0) );
        TS_ASSERT_EQUALS( tbc.offset, ( tc * tb ).offset );
        TS_ASSERT_EQUALS( Geom::Affine(tbc), Geom::Affine(tb) * Geom::Affine(tc) );
    }

    void testOpStarPointTranslate(void)
    {
        TS_ASSERT_EQUALS( tbc.offset, b * tc );
        TS_ASSERT_EQUALS( b * tc, b * Geom::Affine(tc) );
    }

    void testIdentity(void)
    {
        TS_ASSERT_EQUALS( b * t_id, b );
        TS_ASSERT_EQUALS( Geom::Affine(t_id), m_id );
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
