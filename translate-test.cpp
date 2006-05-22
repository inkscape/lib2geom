#include <utest/utest.h>
#include <libnr/point-ops.h>
#include <libnr/matrix.h>
#include <libnr/matrix-fns.h>
#include <libnr/matrix-ops.h>
#include <libnr/point-matrix-ops.h>
#include <libnr/translate.h>
#include <libnr/translate-ops.h>
using Geom::X;
using Geom::Y;


int main(int argc, char *argv[])
{
    utest_start("translate");

    Geom::Point const b(-2.0, 3.0);
    Geom::translate const tb(b);
    Geom::translate const tc(-3.0, -2.0);
    UTEST_TEST("constructors, operator[]") {
        UTEST_ASSERT( tc[X] == -3.0 && tc[Y] == -2.0 );
        UTEST_ASSERT( tb[0] == b[X] && tb[1] == b[Y] );
    }

    UTEST_TEST("operator=") {
        Geom::translate tb_eq(tc);
        tb_eq = tb;
        UTEST_ASSERT( tb == tb_eq );
        UTEST_ASSERT( tb_eq != tc );
    }

    Geom::translate const tbc( tb * tc );
    UTEST_TEST("operator*(translate, translate)") {
        UTEST_ASSERT( tbc.offset == Geom::Point(-5.0, 1.0) );
        UTEST_ASSERT( tbc.offset == ( tc * tb ).offset );
        UTEST_ASSERT( Geom::Matrix(tbc) == Geom::Matrix(tb) * Geom::Matrix(tc) );
    }

    UTEST_TEST("operator*(Point, translate)") {
        UTEST_ASSERT( tbc.offset == b * tc );
        UTEST_ASSERT( b * tc == b * Geom::Matrix(tc) );
    }

    Geom::translate const t_id(0.0, 0.0);
    Geom::Matrix const m_id(Geom::identity());
    UTEST_TEST("identity") {
        UTEST_ASSERT( b * t_id == b );
        UTEST_ASSERT( Geom::Matrix(t_id) == m_id );
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
