#include "path.h"
#include "cubic_bez_util.h"

namespace Geom{

Maybe<Rect> SubPath::bbox() const {
// needs work for other elements.
    if(handles.size() > 0) {
        Rect r(handles[0], handles[0]);
        
        for(std::vector<Geom::Point>::const_iterator h(handles.begin()), e(handles.end());
            h != e; ++h) {
            r.expandTo(*h);
        }
        return Maybe<Rect>(r);
    }
    return Nothing();
}

/*** push_back 
 * append e to this
 * note that this operation modifies the path.
 */

void SubPath::push_back(SubPathElem e) {
    assert(e.begin() != e.end());
    if(!handles.empty() && *e.begin() != handles.back()) {
        cmd.push_back(Geom::moveto);
        handles.push_back(*e.begin());
    }
    cmd.push_back(e.op);
    // inelegant
    if(e.op == moveto)
        handles.insert(handles.end(), e.begin(), e.end());
    else
        handles.insert(handles.end(), e.begin()+1, e.end());
}


/*** SubPath::insert
 * copy elements from [s,e) to before before (as per vector.insert)
 * note that this operation modifies the path.
 * 
 */
void SubPath::insert(SubPathConstIter before, SubPathConstIter s, SubPathConstIter e) {
    assert(0);
/*
    if((*s).begin()[0] != ) {
        cmd.push_back(Geom::moveto);
        handles.push_back(*e.begin());
    }
    cmd.push_back(e.op);
    handles.insert(handles.end(), e.begin(), e.end());
*/
}


/*SubPath SubPath::insert_node(SubPathLocation at) {
    SubPath p;
    
    p.insert(p.end(), begin(), at.it); // begining of path
    }*/

Geom::Point Geom::SubPath::SubPathElem::point_at(double t) {
    switch(op) {
    case Geom::moveto: // these four could be merged by a smarter person
        return s[0];
    case Geom::lineto:
        return Lerp(t, s[0], s[1]);
    case Geom::quadto:
    {
        Geom::Point mid[2];
        for(int i = 0; i < 2; i++)
            mid[i] = Lerp(t, s[i], s[i+1]);
        return Lerp(t, mid[0], mid[1]);
    }
    case Geom::cubicto:
    {
        Geom::Point mid[3];
        for(int i = 0; i < 3; i++)
            mid[i] = Lerp(t, s[i], s[i+1]);
        Geom::Point midmid[2];
        for(int i = 0; i < 2; i++)
            midmid[i] = Lerp(t, mid[i], mid[i+1]);
        return Lerp(t, midmid[0], midmid[1]);
    }
    default:
        break;
    }
}

void
Geom::SubPath::SubPathElem::point_tangent_acc_at(double t, 
                                           Geom::Point &pos, 
                                           Geom::Point &tgt,
                                           Geom::Point &acc) {
    switch(op) {
    case Geom::moveto: // these four could be merged by a smarter person
        pos = s[0];
        break;
    case Geom::lineto:
        pos = Lerp(t, s[0], s[1]);
        tgt = s[1] - s[0];
        acc = Point(0,0);
        break;
    case Geom::quadto:
    {
        Geom::Point mid[2];
        for(int i = 0; i < 2; i++)
            mid[i] = Lerp(t, s[i], s[i+1]);
        pos = Lerp(t, mid[0], mid[1]);
        tgt = 2*t*(mid[0]-mid[1]) -2*mid[0];
        acc = 2*(mid[0]-mid[1]);
        break;
    }
    case Geom::cubicto:
    {
        Geom::Point pc[4];
        for(int i = 0; i < 4; i++)
            pc[i] = Point(0,0);
        
        cubic_bezier_poly_coeff(s, pc);
        pos = t*(t*(t*pc[3] + pc[2]) + pc[1]) + pc[0];
        tgt = t*(3*t*pc[3] + 2*pc[2]) + pc[1];
        acc = 6*t*pc[3] + 2*pc[2];
        break;
    }
    default:
        break;
    }
}


Point SubPath::point_at(SubPathLocation at) {
    return (*at.it).point_at(at.t);
}

void SubPath::point_tangent_acc_at(SubPathLocation at, Point &pos, Point & tgt, Point &acc) {
    (*at.it).point_tangent_acc_at(at.t, pos, tgt, acc);
}

#include "nearestpoint.cpp"

bool SubPath::SubPathElem::nearest_location(Point p, double& dist, double& tt) {
    double new_dist, new_t;
    switch(op) {
    case Geom::moveto:
        new_dist = L2(p - s[0]);
        new_t = 0; // not meaningful
        break;
  // For the rest we can assume that start has already been tried.
    case Geom::lineto:
    {
        new_dist = L2(p - s[1]);
        new_t = 1;
        double t = dot(p - s[0], (s[1] - s[0])) / dot(s[1] - s[0], s[1] - s[0]);
        if((t > 0) && (t < 1)) {
            new_t = t;
            new_dist = fabs(dot(p - s[0], unit_vector(rot90(s[1] - s[0]))));
        }
        break;
    }
    case Geom::quadto:
    {
        new_dist = L2(p - s[2]);
        new_t = 1;
        double t = dot(p - s[0], (s[2] - s[0])) / dot(s[2] - s[0], s[2] - s[0]);
        if((t > 0) && (t < 1)) {
            new_t = t;
            new_dist = fabs(dot(p - s[0], unit_vector(rot90(s[2] - s[0]))));
        }
        break;
    }
    case Geom::cubicto:
    {
        Point hnd[4];
        for(int i = 0; i < 4; i++) 
            hnd[i] = (*this)[i];// fixme
        new_t = NearestPointOnCurve(p, hnd);
        new_dist = L2(p - point_at(new_t));
        break;
    }
    default:
        return false;
    }
    if(new_dist < dist) {
        dist = new_dist;
        tt = new_t;
        return true;
    }
    return false;
}

SubPath::SubPathLocation SubPath::nearest_location(Point p, double &dist) {
    SubPathLocation pl(begin(), 0);
    dist = INFINITY;
    double t = 0;
    int i = 0;
    for(SubPathConstIter elm = begin();
        elm != end();
       ) {
        ++elm;
        if((*elm).nearest_location(p, dist, t)) {
            pl = SubPathLocation(elm, t);
        }
        i++;
    }
    return pl;
}

SubPath SubPath::subpath(SubPathConstIter begin, SubPathConstIter end) {
    SubPath result;
    for(SubPathConstIter iter(begin); iter != end; ++iter) {
        result.push_back(*iter);
    }
    return result;
}



};

#ifdef UNIT_TEST
int main(int argc, char **argv) {
    
}
#endif


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

