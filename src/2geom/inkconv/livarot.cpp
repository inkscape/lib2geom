//Inkconv
#include "livarot.h"
#include "prim.h"

//livarot (inkscape)
#include "livarot/Path.h"

//lib2geom
#include "path2.h"
#include "point.h"

//std
#include <vector>
using namespace std;

Geom::Path livarot2Geom(Path p) {
    Geom::Path ret = Geom::Path();
    Geom::Point cur;
    for (std::vector<PathDescr*>::iterator i = p.descr_cmd.begin(); i != p.descr_cmd.end(); i++) {
        switch ( descr_cmd[i]->getType() ) {

        case descr_moveto: {
            PathDescrMoveTo *nData = dynamic_cast<PathDescrMoveTo *>(i);
            cur = NR2Geom(nData->p);
        }
        break;

        case descr_lineto: {
            PathDescrLineTo *nData = dynamic_cast<PathDescrLineTo *>(i);
            Geom::Point p = NR2Geom(nData->p);
            ret.append(Geom::LineSegment(cur, p));
            cur = p;
        }
        break;

        case descr_cubicto: {
            PathDescrCubicTo *nData = dynamic_cast<PathDescrCubicTo *>(i);
            Geom::Point p = NR2Geom(nData->end);
            ret.append(Geom::CubicBezier(cur, NR2Geom(nData->p), NR2Geom(nData->start), p));
            cur = p;
        }
        break;

        case descr_bezierto: {
            PathDescrBezierTo *nData = dynamic_cast<PathDescrBezierTo *>(i);
            Geom::Point p = NR2Geom(nData->p);
            if(nData->nb == 2) {
                i++;
                PathDescrIntermBezierTo *nData1 = dynamic_cast<PathDescrIntermBezierTo *>(i);
                Geom::Point p1 = NR2Geom(nData1->p);
                i++;
                PathDescrIntermBezierTo *nData2 = dynamic_cast<PathDescrIntermBezierTo *>(i);
                Geom::Point p2 = NR2Geom(nData2->p);
                ret.append(Geom::QuadraticBezier(cur, p1, p2, p);
            }//TODO: else?
            cur = p;
        }
        break;

        //TODO: Handle intermbezier/forced
        case descr_interm_bezier: {
            PathDescrIntermBezierTo *nData = dynamic_cast<PathDescrIntermBezierTo *>(i);
            cur = NR2Geom(nData->p);
        }
        break;

        case descr_forced: {
            PathDescrForced *nData = dynamic_cast<PathDescrForced *>(i);
            cur = NR2Geom(nData->p);
        }
        break; 

        case descr_arcto: {
            PathDescrArcTo *nData = dynamic_cast<PathDescrArcTo *>(i);
            Geom::Point p = NR2Geom(nData->p);
            ret.append(Geom::SVGEllipticArc(cur, nData->rx, nData->ry, nData->angle, nData->large, nData->clockwise, p));
            cur = p;
        }
        break;

        case descr_close:
            ret.close();
        break;
    }
}
