#include "path.h"
#include "bezier-utils.h"
#include "s-basis.h"
#include "multidim-sbasis.h"
#include "bezier-to-sbasis.h"
#include "path-sbasis.h"
#include "point.h"

namespace Geom{

static LineTo _lineto;
LineTo * const lineto=&_lineto;
static QuadTo _quadto;
QuadTo * const quadto=&_quadto;
static CubicTo _cubicto;
CubicTo * const cubicto=&_cubicto;

MultidimSBasis<2>
LineTo::to_sbasis(Geom::OldPath::Elem const & elm) {
    return bezier_to_sbasis<2, 1>(elm.begin());
}

MultidimSBasis<2>
QuadTo::to_sbasis(Geom::OldPath::Elem const & elm) {
    return bezier_to_sbasis<2, 2>(elm.begin());
}

MultidimSBasis<2>
CubicTo::to_sbasis(Geom::OldPath::Elem const & elm) {
    return bezier_to_sbasis<2, 3>(elm.begin());
}

Point
LineTo::point_at(Geom::OldPath::Elem const & elm, double t) {
    return Lerp(t, elm[0], elm[1]);
}

Point
QuadTo::point_at(Geom::OldPath::Elem const & elm, double t) {
    Geom::Point mid[2];
    for(int i = 0; i < 2; i++)
        mid[i] = Lerp(t, elm[i], elm[i+1]);
    return Lerp(t, mid[0], mid[1]);
}

Point
CubicTo::point_at(Geom::OldPath::Elem const & elm, double t) {
    Geom::Point mid[3];
    for(int i = 0; i < 3; i++)
        mid[i] = Lerp(t, elm[i], elm[i+1]);
    Geom::Point midmid[2];
    for(int i = 0; i < 2; i++)
        midmid[i] = Lerp(t, mid[i], mid[i+1]);
    return Lerp(t, midmid[0], midmid[1]);
}

Maybe<Rect> OldPath::bbox() const {
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

void OldPath::push_back(Elem e) {
    assert(e.begin() != e.end());
    if(!handles.empty() && *e.begin() != handles.back()) {
        handles.push_back(*e.begin());
    }
    cmd.push_back(e.op);
    handles.insert(handles.end(), e.begin()+1, e.end());
}


/*** OldPath::insert
 * copy elements from [s,e) to before before (as per vector.insert)
 * note that this operation modifies the path.
 * 
 */
void OldPath::insert(ConstIter before, ConstIter s, ConstIter e) {
    assert(0);
/*
    if((*s).begin()[0] != ) {
        handles.push_back(*e.begin());
    }
    cmd.push_back(e.op);
    handles.insert(handles.end(), e.begin(), e.end());
*/
}


/*OldPath OldPath::insert_node(Location at) {
    OldPath p;
    
    p.insert(p.end(), begin(), at.it); // begining of path
    }*/

Geom::Point Geom::OldPath::Elem::point_at(double t) const {
    return (*op).point_at(*this, t);
}

void
Geom::OldPath::Elem::point_tangent_acc_at(double t, 
                                           Geom::Point &pos, 
                                           Geom::Point &tgt,
                                           Geom::Point &acc) const {
    // inelegant
    MultidimSBasis<2> a = elem_to_sbasis(*this);
    pos = Geom::Point(a[0](t), a[1](t));
    a = derivative(a);
    tgt = Geom::Point(a[0](t), a[1](t));
    a = derivative(a);
    acc = Geom::Point(a[0](t), a[1](t));
}


Point OldPath::point_at(Location at) const {
    ptrdiff_t offset = at.it - begin();
    
    assert(offset >= 0);
    assert(offset < size());
    return (*at.it).point_at(at.t);
}

void OldPath::point_tangent_acc_at(Location at, Point &pos, Point & tgt, Point &acc) const {
    ptrdiff_t offset = at.it - begin();
    
    assert(offset >= 0);
    assert(offset < size());
    (*at.it).point_tangent_acc_at(at.t, pos, tgt, acc);
}

#include "nearestpoint.cpp"

bool OldPath::Elem::nearest_location(Point p, double& dist, double& tt) const {
    double new_dist, new_t;
    if(dynamic_cast<Geom::LineTo *>(op)) {
        new_dist = L2(p - s[1]);
        new_t = 1;
        double t = dot(p - s[0], (s[1] - s[0])) / dot(s[1] - s[0], s[1] - s[0]);
        if((t > 0) && (t < 1)) {
            new_t = t;
            new_dist = fabs(dot(p - s[0], unit_vector(rot90(s[1] - s[0]))));
        }
    } else if(dynamic_cast<Geom::QuadTo *>(op)) {
        ;
    } else if(dynamic_cast<Geom::CubicTo *>(op)) {
        Point hnd[4];
        for(int i = 0; i < 4; i++) 
            hnd[i] = (*this)[i];// fixme
        new_t = NearestPointOnCurve(p, hnd);
        new_dist = L2(p - point_at(new_t));
    }
    if(new_dist < dist) {
        dist = new_dist;
        tt = new_t;
        return true;
    }
    return false;
}

OldPath::Location OldPath::nearest_location(Point p, double &dist) const {
    Location pl(begin(), 0);
    dist = INFINITY;
    double t = 0;
    int i = 0;
    for(ConstIter elm = begin();
        elm != end();
        ++elm
       ) {
        if((*elm).nearest_location(p, dist, t)) {
            pl = Location(elm, t);
        }
        i++;
    }
    return pl;
}

OldPath OldPath::subpath(ConstIter begin, ConstIter end) {
    OldPath result;
    for(ConstIter iter(begin); iter != end; ++iter) {
        result.push_back(*iter);
    }
    return result;
}


// Perhaps this hash is not strong enough
OldPath::operator HashCookie() {
    HashCookie hc;
    hc.value = (unsigned long)(&(*cmd.begin()));
    hc.value *= 0x101;
    hc.value += (unsigned long)(&(*cmd.end()));
    hc.value *= 0x101;
    hc.value += (unsigned long)(&(*handles.begin()));
    hc.value *= 0x101;
    hc.value += (unsigned long)(&(*handles.end()));
    return hc;
}

unsigned 
OldPath::total_segments() const {
    return cmd.size();
}

unsigned
OldPathSet::total_segments() const {
    unsigned segs = 0;
    for(OldPathSet::const_iterator it = begin(); it != end(); it++) {
        segs += (*it).total_segments();
    }
    return segs;
}


};

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

