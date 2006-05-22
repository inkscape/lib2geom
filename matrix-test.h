#include <cxxtest/TestSuite.h>

#include <libnr/matrix.h>
#include <libnr/matrix-fns.h>
#include <libnr/matrix-ops.h>
#include <libnr/matrix-rotate-ops.h>
#include <libnr/matrix-scale-ops.h>
#include <libnr/point-matrix-ops.h>
#include <libnr/rotate.h>
#include <libnr/rotate-ops.h>
#include <libnr/scale-ops.h>
#include <libnr/scale-translate-ops.h>
#include <libnr/translate.h>
#include <libnr/translate-ops.h>
#include <libnr/translate-scale-ops.h>
using Geom::Matrix;
using Geom::X;
using Geom::Y;

inline bool point_equalp(Geom::Point const &a, Geom::Point const &b)
{
    return ( Geom_DF_TEST_CLOSE(a[X], b[X], 1e-5) &&
             Geom_DF_TEST_CLOSE(a[Y], b[Y], 1e-5)   );
}

class NrMatrixTest : public CxxTest::TestSuite
{
public:

    NrMatrixTest() :
        m_id( Geom::identity() ),
        r_id( Geom::Point(1, 0) ),
        t_id( 0, 0 ),
        c16( 1.0, 2.0,
             3.0, 4.0,
             5.0, 6.0),
        r86( Geom::Point(.8, .6) ),
        mr86( r86 ),
        t23( 2.0, 3.0 ),
        s_id( 1.0, 1.0 )
    {
    }
    virtual ~NrMatrixTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static NrMatrixTest *createSuite() { return new NrMatrixTest(); }
    static void destroySuite( NrMatrixTest *suite ) { delete suite; }

    Matrix const m_id;
    Geom::rotate const r_id;
    Geom::translate const t_id;
    Matrix const c16;
    Geom::rotate const r86;
    Geom::Matrix const mr86;
    Geom::translate const t23;
    Geom::scale const s_id;




    void testCtorsAssignmentOp(void)
    {
        Matrix const c16_copy(c16);
        Matrix c16_eq(m_id);
        c16_eq = c16;
        for(unsigned i = 0; i < 6; ++i) {
            TS_ASSERT_EQUALS( c16[i], 1.0 + i );
            TS_ASSERT_EQUALS( c16[i], c16_copy[i] );
            TS_ASSERT_EQUALS( c16[i], c16_eq[i] );
            TS_ASSERT_EQUALS( m_id[i], double( i == 0 || i == 3 ) );
        }
    }

    void testScaleCtor(void)
    {
        Geom::scale const s(2.0, 3.0);
        Geom::Matrix const ms(s);
        Geom::Point const p(5.0, 7.0);
        TS_ASSERT_EQUALS( p * s, Geom::Point(10.0, 21.0) );
        TS_ASSERT_EQUALS( p * ms, Geom::Point(10.0, 21.0) );
    }

    void testRotateCtor(void)
    {
        Geom::Point const p0(1.0, 0.0);
        Geom::Point const p90(0.0, 1.0);
        TS_ASSERT_EQUALS( p0 * r86, Geom::Point(.8, .6) );
        TS_ASSERT_EQUALS( p0 * mr86, Geom::Point(.8, .6) );
        TS_ASSERT_EQUALS( p90 * r86, Geom::Point(-.6, .8) );
        TS_ASSERT_EQUALS( p90 * mr86, Geom::Point(-.6, .8) );
        TS_ASSERT( matrix_equalp(Matrix( r86 * r86 ),
                                 mr86 * mr86,
                                 1e-14) );
    }

    void testTranslateCtor(void)
    {
        Geom::Matrix const mt23(t23);
        Geom::Point const b(-2.0, 3.0);
        TS_ASSERT_EQUALS( b * t23, b * mt23 );
    }

    void testIdentity(void)
    {
        TS_ASSERT( m_id.test_identity() );
        TS_ASSERT( Matrix(t_id).test_identity() );
        TS_ASSERT( !(Matrix(Geom::translate(-2, 3)).test_identity()) );
        TS_ASSERT( Matrix(r_id).test_identity() );
        Geom::rotate const rot180(Geom::Point(-1, 0));
        TS_ASSERT( !(Matrix(rot180).test_identity()) );
        TS_ASSERT( Matrix(s_id).test_identity() );
        TS_ASSERT( !(Matrix(Geom::scale(1.0, 0.0)).test_identity()) );
        TS_ASSERT( !(Matrix(Geom::scale(0.0, 1.0)).test_identity()) );
        TS_ASSERT( !(Matrix(Geom::scale(1.0, -1.0)).test_identity()) );
        TS_ASSERT( !(Matrix(Geom::scale(-1.0, -1.0)).test_identity()) );
    }

    void testInverse(void)
    {
        TS_ASSERT_EQUALS( m_id.inverse(), m_id );
        TS_ASSERT_EQUALS( Matrix(t23).inverse(), Matrix(Geom::translate(-2.0, -3.0)) );
        Geom::scale const s2(-4.0, 2.0);
        Geom::scale const sp5(-.25, .5);
        TS_ASSERT_EQUALS( Matrix(s2).inverse(), Matrix(sp5) );
    }

    void testNrMatrixInvert(void)
    {
        NRMatrix const nr_m_id(m_id);
        Matrix const m_s2(Geom::scale(-4.0, 2.0));
        NRMatrix const nr_s2(m_s2);
        Matrix const m_sp5(Geom::scale(-.25, .5));
        NRMatrix const nr_sp5(m_sp5);
        Matrix const m_t23(t23);
        NRMatrix const nr_t23(m_t23);
        NRMatrix inv;
        nr_matrix_invert(&inv, &nr_m_id);
        TS_ASSERT_EQUALS( Matrix(inv), m_id );
        nr_matrix_invert(&inv, &nr_t23);
        TS_ASSERT_EQUALS( Matrix(inv), Matrix(Geom::translate(-2.0, -3.0)) );
        nr_matrix_invert(&inv, &nr_s2);
        TS_ASSERT_EQUALS( Matrix(inv), Matrix(nr_sp5) );
        nr_matrix_invert(&inv, &nr_sp5);
        TS_ASSERT_EQUALS( Matrix(inv), Matrix(nr_s2) );

        /* Test that nr_matrix_invert handles src == dest. */
        inv = nr_s2;
        nr_matrix_invert(&inv, &inv);
        TS_ASSERT_EQUALS( Matrix(inv), Matrix(nr_sp5) );
        inv = nr_t23;
        nr_matrix_invert(&inv, &inv);
        TS_ASSERT_EQUALS( Matrix(inv), Matrix(Geom::translate(-2.0, -3.0)) );
    }

    void testEllipticQuadraticForm(void)
    {
        Geom::Matrix const aff(1.0, 1.0,
                             0.0, 1.0,
                             5.0, 6.0);
        Geom::Matrix const invaff = aff.inverse();
        TS_ASSERT_EQUALS( invaff[1], -1.0 );
		
        Geom::Matrix const ef(elliptic_quadratic_form(invaff));
        Geom::Matrix const exp_ef(2, -1,
                                -1, 1,
                                0, 0);
        TS_ASSERT_EQUALS( ef, exp_ef );
    }

    void testMatrixStarRotate(void)
    {
        Geom::Matrix const ma(2.0, -1.0,
                            4.0, 4.0,
                            -0.5, 2.0);
        Geom::Matrix const a_r86( ma * r86 );
        Geom::Matrix const ma1( a_r86 * r86.inverse() );
        TS_ASSERT( matrix_equalp(ma1, ma, 1e-12) );
        Geom::Matrix const exp_a_r86( 2*.8 + -1*-.6,  2*.6 + -1*.8,
                                    4*.8 + 4*-.6,   4*.6 + 4*.8,
                                    -.5*.8 + 2*-.6, -.5*.6 + 2*.8 );
        TS_ASSERT( matrix_equalp(a_r86, exp_a_r86, 1e-12) );
    }

    void testTranslateStarScale_ScaleStarTranslate(void)
    {
        Geom::translate const t2n4(2, -4);
        Geom::scale const sn2_8(-2, 8);
        Geom::Matrix const exp_ts(-2, 0,
                                0,  8,
                                -4, -32);
        Geom::Matrix const exp_st(-2, 0,
                                0,  8,
                                2, -4);
        TS_ASSERT_EQUALS( exp_ts, t2n4 * sn2_8 );
        TS_ASSERT_EQUALS( exp_st, sn2_8 * t2n4 );
    }

    void testMatrixStarScale(void)
    {
        Geom::Matrix const ma(2.0, -1.0,
                            4.0, 4.0,
                            -0.5, 2.0);
        Geom::scale const sn2_8(-2, 8);
        Geom::Matrix const exp_as(-4, -8,
                                -8, 32,
                                1,  16);
        TS_ASSERT_EQUALS( ma * sn2_8, exp_as );
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
