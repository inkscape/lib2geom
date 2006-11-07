#include "path.h"
#include "cubic_bez_util.h"
#include "s-basis.h"
#include "multidim-sbasis.h"
#include "bezier-to-sbasis.h"

namespace Geom{

static LineTo _lineto;
LineTo * const lineto=&_lineto;
static QuadTo _quadto;
QuadTo * const quadto=&_quadto;
static CubicTo _cubicto;
CubicTo * const cubicto=&_cubicto;

multidim_sbasis<2>
LineTo::to_sbasis(Geom::Path::Elem const & elm) {
    return bezier_to_sbasis<2, 1>(elm.begin());
}

multidim_sbasis<2>
QuadTo::to_sbasis(Geom::Path::Elem const & elm) {
    return bezier_to_sbasis<2, 2>(elm.begin());
}

multidim_sbasis<2>
CubicTo::to_sbasis(Geom::Path::Elem const & elm) {
    return bezier_to_sbasis<2, 3>(elm.begin());
}

Point
LineTo::point_at(Geom::Path::Elem const & elm, double t) {
    return Lerp(t, elm[0], elm[1]);
}

Point
QuadTo::point_at(Geom::Path::Elem const & elm, double t) {
    Geom::Point mid[2];
    for(int i = 0; i < 2; i++)
        mid[i] = Lerp(t, elm[i], elm[i+1]);
    return Lerp(t, mid[0], mid[1]);
}

Point
CubicTo::point_at(Geom::Path::Elem const & elm, double t) {
    Geom::Point mid[3];
    for(int i = 0; i < 3; i++)
        mid[i] = Lerp(t, elm[i], elm[i+1]);
    Geom::Point midmid[2];
    for(int i = 0; i < 2; i++)
        midmid[i] = Lerp(t, mid[i], mid[i+1]);
    return Lerp(t, midmid[0], midmid[1]);
}

Maybe<Rect> Path::bbox() const {
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

void Path::push_back(Elem e) {
    assert(e.begin() != e.end());
    if(!handles.empty() && *e.begin() != handles.back()) {
        handles.push_back(*e.begin());
    }
    cmd.push_back(e.op);
    handles.insert(handles.end(), e.begin()+1, e.end());
}


/*** Path::insert
 * copy elements from [s,e) to before before (as per vector.insert)
 * note that this operation modifies the path.
 * 
 */
void Path::insert(ConstIter before, ConstIter s, ConstIter e) {
    assert(0);
/*
    if((*s).begin()[0] != ) {
        handles.push_back(*e.begin());
    }
    cmd.push_back(e.op);
    handles.insert(handles.end(), e.begin(), e.end());
*/
}


/*Path Path::insert_node(Location at) {
    Path p;
    
    p.insert(p.end(), begin(), at.it); // begining of path
    }*/

Geom::Point Geom::Path::Elem::point_at(double t) const {
    return (*op).point_at(*this, t);
}

void
Geom::Path::Elem::point_tangent_acc_at(double t, 
                                           Geom::Point &pos, 
                                           Geom::Point &tgt,
                                           Geom::Point &acc) const {
/*
        Poly Qx = get_parametric_poly(*this, X);
        Poly Qy = get_parametric_poly(*this, Y);
        pos = Point(Qx(t), Qy(t));
        Qx = derivative(Qx);
        Qy = derivative(Qy);
        tgt = Point(Qx(t), Qy(t));
        Qx = derivative(Qx);
        Qy = derivative(Qy);
        acc = Point(Qx(t), Qy(t));
*/
}


Point Path::point_at(Location at) const {
    ptrdiff_t offset = at.it - begin();
    
    assert(offset >= 0);
    assert(offset < size());
    return (*at.it).point_at(at.t);
}

void Path::point_tangent_acc_at(Location at, Point &pos, Point & tgt, Point &acc) const {
    ptrdiff_t offset = at.it - begin();
    
    assert(offset >= 0);
    assert(offset < size());
    (*at.it).point_tangent_acc_at(at.t, pos, tgt, acc);
}

#include "nearestpoint.cpp"

bool Path::Elem::nearest_location(Point p, double& dist, double& tt) const {
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

Path::Location Path::nearest_location(Point p, double &dist) const {
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

Path Path::subpath(ConstIter begin, ConstIter end) {
    Path result;
    for(ConstIter iter(begin); iter != end; ++iter) {
        result.push_back(*iter);
    }
    return result;
}


// Perhaps this hash is not strong enough
Path::operator HashCookie() {
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
Path::total_segments() const {
    return cmd.size();
}

unsigned
PathSet::total_segments() const {
    unsigned segs = 0;
    for(PathSet::const_iterator it = begin(); it != end(); it++) {
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

