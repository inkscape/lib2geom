#include "path-to-svgd.h"
#include "path-to-polyline.h"
#include <ctype.h>
#include <vector>
#include <cassert>

#ifdef UNIT_TEST
int main(int argc, char **argv) {
    FILE* f = fopen("banana.svg", "r");
    assert(f);
    Geom::Path p = read_svgd(f);
    
    path_to_polyline pl(p);
    
    write_svgd(f, Geom::Path(pl));
}
#endif

//} // namespace Geom

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

