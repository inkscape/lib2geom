#include <cmath>
#include <utest/utest.h>
#include <libnr/matrix.h>
#include <libnr/matrix-fns.h>        /* identity, matrix_equalp */
#include <libnr/matrix-ops.h>
#include <libnr/point-fns.h>
#include <libnr/point-matrix-ops.h>
#include <libnr/rotate.h>
#include <libnr/rotate-fns.h>
#include <libnr/rotate-ops.h>
using Geom::X;
using Geom::Y;

int main(int argc, char *argv[])
{
    utest_start("rotate");

    Geom::Affine const m_id(Geom::identity());
    Geom::Rotate const r_id(0.0);
    Geom::Rotate const rot234(.234);
    UTEST_TEST("constructors, comparisons") {
        UTEST_ASSERT( r_id == r_id );
        UTEST_ASSERT( rot234 == rot234 );
        UTEST_ASSERT( rot234 != r_id );
        UTEST_ASSERT( r_id == Geom::Rotate(Geom::Point(1.0, 0.0)) );
        UTEST_ASSERT( Geom::Affine(r_id) == m_id );
        UTEST_ASSERT( Geom::Affine(r_id).test_identity() );

        UTEST_ASSERT(rotate_equalp(rot234, Geom::Rotate(Geom::Point(cos(.234), sin(.234))), 1e-12));
    }

    UTEST_TEST("operator=") {
        Geom::Rotate rot234_eq(r_id);
        rot234_eq = rot234;
        UTEST_ASSERT( rot234 == rot234_eq );
        UTEST_ASSERT( rot234_eq != r_id );
    }

    UTEST_TEST("inverse") {
        UTEST_ASSERT( r_id.inverse() == r_id );
        UTEST_ASSERT( rot234.inverse() == Geom::Rotate(-.234) );
    }

    Geom::Point const b(-2.0, 3.0);
    Geom::Rotate const rot180(Geom::Point(-1.0, 0.0));
    UTEST_TEST("operator*(Point, rotate)") {
        UTEST_ASSERT( b * r_id == b );
        UTEST_ASSERT( b * rot180 == -b );
        UTEST_ASSERT( b * rot234 == b * Geom::Affine(rot234) );
        UTEST_ASSERT(point_equalp(b * Geom::Rotate(M_PI / 2),
                                  Geom::rot90(b),
                                  1e-14));
        UTEST_ASSERT( b * rotate_degrees(90.) == Geom::rot90(b) );
    }

    UTEST_TEST("operator*(rotate, rotate)") {
        UTEST_ASSERT( r_id * r_id == r_id );
        UTEST_ASSERT( rot180 * rot180 == r_id );
        UTEST_ASSERT( rot234 * r_id == rot234 );
        UTEST_ASSERT( r_id * rot234 == rot234 );
        UTEST_ASSERT(rotate_equalp(rot234 * rot234.inverse(), r_id, 1e-14));
        UTEST_ASSERT(rotate_equalp(rot234.inverse() * rot234, r_id, 1e-14));
        UTEST_ASSERT(rotate_equalp(( Geom::Rotate(0.25) * Geom::Rotate(.5) ),
                                   Geom::Rotate(.75),
                                   1e-10));
    }

    UTEST_TEST("operator/(rotate, rotate)") {
        UTEST_ASSERT( rot234 / r_id == rot234 );
        UTEST_ASSERT( rot234 / rot180 == rot234 * rot180 );
        UTEST_ASSERT(rotate_equalp(rot234 / rot234, r_id, 1e-14));
        UTEST_ASSERT(rotate_equalp(r_id / rot234, rot234.inverse(), 1e-14));
    }

    return ( utest_end()
             ? EXIT_SUCCESS
             : EXIT_FAILURE );
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
