#include "nrbpath.h"
#include "path2.h"
#include "libnr/nr-path.h"

/* a better and tested version of this method can be found in Inkscape's source: /src/libnr/n-art-bpath-2geom.cpp */

Geom::Path NR2Geom(NRBPath) {
    Geom::Path ret = Geom::Path();
    int i = 0;
    Geom::Point cur;
    while (s->path[i].code != NR_END) {
        switch(s->path[i].code) {
            //TODO: differentiate these two 
            case NR_MOVETO:
            case NR_MOVETO_OPEN:
                cur = Geom::Point(s->path[i].x3, s->path[i].y3);
            break;

            case NR_LINETO:
                Geom::Point p = Geom::Point(s->path[i].x3, s->path[i].y3);
                ret.append(LineSegment(cur, p));
                cur = p
            break;

            case NR_CURVETO:
                Geom::Point p = Geom::Point(s->path[i].x3, s->path[i].y3);
                ret.append(CubicBezier(cur, Geom::Point(s->path[i].x1, s->path[i].y1), Geom::Point(s->path[i].x2, s->path[i].y2) p));
                cur = p
            break;
		}
		i += 1;
	}
}
