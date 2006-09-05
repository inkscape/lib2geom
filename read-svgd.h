#include "path.h"
#include <cstdio>
#include <cassert>

void write_svgd(std::FILE* f, Geom::SubPath const &p);
void write_svgd(std::FILE* f, Geom::Path const &p);

Geom::Path read_svgd(std::FILE* f);


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

