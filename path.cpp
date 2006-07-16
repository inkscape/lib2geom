#include "path.h"
#include "cubic_bez_util.h"
#include "poly.h"



static Poly
quadratic_bezier_poly(Geom::SubPath::Elem const & b, int dim) {
    Poly result;
    double c[6] = {1, 
                    -2, 2, 
                    1, -2, 1};

    int cp = 0;
    
    result.coeff.resize(3);
    
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j <= i; j++) {
            result.coeff[2 - j] += (c[cp]*(b[2- i]))[dim];
            cp++;
        }
    }
    return result;
}


static Poly
cubic_bezier_poly(Geom::SubPath::Elem const & b, int dim) {
    Poly result;
    double c[10] = {1, 
                    -3, 3, 
                    3, -6, 3,
                    -1, 3, -3, 1};

    int cp = 0;
    
    result.coeff.resize(4);
    
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j <= i; j++) {
            result.coeff[3 - j] += (c[cp]*(b[3- i]))[dim];
            cp++;
        }
    }
    return result;
}

Poly get_parametric_poly(Geom::SubPath::Elem const & b, int dim) {
    Poly result;
    switch(b.op) {
    case Geom::lineto:
        result.coeff.push_back(b[0][dim]);
        result.coeff.push_back((b[1]-b[0])[dim]);
        return result;
    case Geom::quadto:
        return quadratic_bezier_poly(b, dim);
    case Geom::cubicto:
        return cubic_bezier_poly(b, dim);
    default:
        return result;
    }
}


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

void SubPath::push_back(Elem e) {
    assert(e.begin() != e.end());
    if(!handles.empty() && *e.begin() != handles.back()) {
        handles.push_back(*e.begin());
    }
    cmd.push_back(e.op);
    handles.insert(handles.end(), e.begin()+1, e.end());
}


/*** SubPath::insert
 * copy elements from [s,e) to before before (as per vector.insert)
 * note that this operation modifies the path.
 * 
 */
void SubPath::insert(ConstIter before, ConstIter s, ConstIter e) {
    assert(0);
/*
    if((*s).begin()[0] != ) {
        handles.push_back(*e.begin());
    }
    cmd.push_back(e.op);
    handles.insert(handles.end(), e.begin(), e.end());
*/
}


/*SubPath SubPath::insert_node(Location at) {
    SubPath p;
    
    p.insert(p.end(), begin(), at.it); // begining of path
    }*/

Geom::Point Geom::SubPath::Elem::point_at(double t) const {
    switch(op) {
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
Geom::SubPath::Elem::point_tangent_acc_at(double t, 
                                           Geom::Point &pos, 
                                           Geom::Point &tgt,
                                           Geom::Point &acc) const {
    switch(op) {
    case Geom::lineto:
    case Geom::quadto:
    case Geom::cubicto:
    {
        Poly Qx = get_parametric_poly(*this, X);
        Poly Qy = get_parametric_poly(*this, Y);
        pos = Point(Qx(t), Qy(t));
        Qx = derivative(Qx);
        Qy = derivative(Qy);
        tgt = Point(Qx(t), Qy(t));
        Qx = derivative(Qx);
        Qy = derivative(Qy);
        acc = Point(Qx(t), Qy(t));
        break;
    }
    default:
        break;
    }
}


Point SubPath::point_at(Location at) const {
    ptrdiff_t offset = at.it - begin();
    
    assert(offset >= 0);
    assert(offset < size());
    return (*at.it).point_at(at.t);
}

void SubPath::point_tangent_acc_at(Location at, Point &pos, Point & tgt, Point &acc) const {
    ptrdiff_t offset = at.it - begin();
    
    assert(offset >= 0);
    assert(offset < size());
    (*at.it).point_tangent_acc_at(at.t, pos, tgt, acc);
}

#include "nearestpoint.cpp"

bool SubPath::Elem::nearest_location(Point p, double& dist, double& tt) const {
    double new_dist, new_t;
    switch(op) {
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
        
        Poly distx = quadratic_bezier_poly(*this, X) - p[X];
        distx = distx*distx;
        Poly disty = quadratic_bezier_poly(*this, Y) - p[Y];
        disty = disty*disty;
        
        Poly dist = derivative(distx + disty);
        
        std::vector<double> sol = solve_reals(dist);
        for(int i = 0; i < sol.size(); i++) {
            double t = sol[i];
            if((t > 0) && (t < 1)) {
                new_t = t;
                new_dist = L2(point_at(t) - p);
            }
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

SubPath::Location SubPath::nearest_location(Point p, double &dist) const {
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

SubPath SubPath::subpath(ConstIter begin, ConstIter end) {
    SubPath result;
    for(ConstIter iter(begin); iter != end; ++iter) {
        result.push_back(*iter);
    }
    return result;
}


// Perhaps this hash is not strong enough
SubPath::operator HashCookie() {
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
SubPath::total_segments() const {
    return cmd.size();
}

unsigned
Path::total_segments() const {
    unsigned segs = 0;
    for(Path::const_iterator it = begin(); it != end(); it++) {
        segs += (*it).total_segments();
    }
    return segs;
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
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

