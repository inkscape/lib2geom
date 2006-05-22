#include "path.h"

Geom::Path read_svgd(FILE* f);
/** read a file containing an SVG style path data giving a Path. Reads to either eof or a '"'.*/

void write_svgd(FILE* f, Geom::Path const &p);
/** write a file SVG pth data style from p. */


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

