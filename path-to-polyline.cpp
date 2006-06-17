#include "path-to-polyline.h"

path_to_polyline::path_to_polyline(const Geom::SubPath &p, double tol) 
:tol(tol)
{
    for(Geom::SubPath::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        switch(iter.cmd()) {
        case Geom::moveto:
            handles.push_back((*iter)[0]);
            break;
        case Geom::lineto:
            line_to_polyline(*iter);
            break;
        case Geom::quadto:
            quad_to_polyline(*iter);
            break;
        case Geom::cubicto:
            cubic_to_polyline(*iter);
            break;
        default:
            //Geom::Point pt(*(e-1));
            //handles.push_back(pt);
            break;
        }
    }
        
}
    
void path_to_polyline::line_to_polyline(Geom::SubPath::Elem e) {
    handles.push_back(e.begin()[1]);
}

void path_to_polyline::cubic_to_polyline(Geom::SubPath::Elem e) {
    Geom::Point v[3];
    for(int i = 0; i < 3; i++)
        v[i] = e[i+1] - e[0];
    Geom::Point orth = v[2]; // unit normal to path line
    //printf("(%g, %g) :  ", orth[0], orth[1]);
    rot90(orth);
    orth.normalize();
    double err = fabs(dot(orth, v[1])) + fabs(dot(orth, v[0]));
    const double thresh = tol;
    
    // better rule, apparently *
    err = L1(e[0] + e[2] - 2*e[1]) +
        L1(e[1] + e[3] - 2*e[2]);

    //printf("(%g)", err);

    if(err < thresh) {
        handles.push_back(e[3]); // approximately a line
        //printf("*\n");
    } else {
        // ideally we break at the two extreme points.  Instead we're going to just cut in half
        //printf("\n");
        Geom::Point mid[3];
        for(int i = 0; i < 3; i++)
            mid[i] = Lerp(0.5, e[i], e[i+1]);
        Geom::Point midmid[2];
        for(int i = 0; i < 2; i++)
            midmid[i] = Lerp(0.5, mid[i], mid[i+1]);
        Geom::Point midmidmid = Lerp(0.5, midmid[0], midmid[1]);
        {
            Geom::Point curve[4] = {e[0], mid[0], midmid[0], midmidmid};
            Geom::SubPath::Elem e0(Geom::cubicto, std::vector<Geom::Point>::const_iterator(curve), std::vector<Geom::Point>::const_iterator(curve) + 4);
            cubic_to_polyline(e0);
        } {
            Geom::Point curve[4] = {midmidmid, midmid[1], mid[2], e[3]};
            Geom::SubPath::Elem e1(Geom::cubicto, std::vector<Geom::Point>::const_iterator(curve), std::vector<Geom::Point>::const_iterator(curve) + 4);
            cubic_to_polyline(e1);
        }
    }
}
    
void path_to_polyline::quad_to_polyline(Geom::SubPath::Elem e) {
    Geom::Point v[2];
    for(int i = 0; i < 2; i++)
        v[i] = e[i+1] - e[0];
    Geom::Point orth = v[1]; // unit normal to path line
    //printf("(%g, %g) :  ", orth[0], orth[1]);
    rot90(orth);
    orth.normalize();
    double err = fabs(dot(orth, v[0]));
    const double thresh = tol;
    
    // better rule, apparently *
    err = L1(e[0] + e[2] - 2*e[1]);

    if(err < thresh) {
        handles.push_back(e[2]); // approximately a line
        //printf("*\n");
    } else {
        // ideally we break at the two extreme points.  Instead we're going to just cut in half
        //printf("\n");
        Geom::Point mid[2];
        for(int i = 0; i < 2; i++)
            mid[i] = Lerp(0.5, e[i], e[i+1]);
        Geom::Point midmid = Lerp(0.5, mid[0], mid[1]);
        {
            Geom::Point curve[3] = {e[0], mid[0], midmid};
            Geom::SubPath::Elem e0(Geom::quadto, std::vector<Geom::Point>::const_iterator(curve), std::vector<Geom::Point>::const_iterator(curve) + 3);
            quad_to_polyline(e0);
        } {
            Geom::Point curve[4] = {midmid, mid[1], e[2]};
            Geom::SubPath::Elem e1(Geom::quadto, std::vector<Geom::Point>::const_iterator(curve), std::vector<Geom::Point>::const_iterator(curve) + 3);
            quad_to_polyline(e1);
        }
    }
}
   
path_to_polyline::operator Geom::SubPath() {
    // make a polyline path
    Geom::SubPath p;
    p.cmd.push_back(Geom::moveto);
    p.handles.push_back(handles[0]);
    for(std::vector<Geom::Point>::const_iterator h(handles.begin()+1), e(handles.end());
        h != e; ++h) {
        p.cmd.push_back(Geom::lineto);
        p.handles.push_back(*h);
    }
    return p;
}


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

    
