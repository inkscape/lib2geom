#include "path2.h"

namespace Geom {

namespace Path2 {

namespace {

enum Cmp {
  LESS_THAN=-1,
  GREATER_THAN=1,
  EQUAL_TO=0
};

template <typename T1, typename T2>
inline Cmp cmp(T1 const &a, T2 const &b) {
  if ( a < b ) {
    return LESS_THAN;
  } else if ( b < a ) {
    return GREATER_THAN;
  } else {
    return EQUAL_TO;
  }
}

}

int CurveHelpers::sbasis_winding(MultidimSBasis<2> const &sb, Point p) {
  double minx, maxx;
  bounds(sb[X], minx, maxx);

  if ( p[X] > maxx ) { /* ray does not intersect bbox */
    return 0;
  }

  SBasis fy = sb[Y];
  fy -= p[Y];

  if (fy.empty()) { /* ignore horizontal segment */
    return 0;
  }

  if ( p[X] < minx ) { /* ray does not originate in bbox */
    double y = p[Y];
    /* winding determined by position of endpoints */
    Cmp initial_to_ray = cmp(fy[0][0], y);
    Cmp final_to_ray = cmp(fy[0][1], y);
    switch (cmp(final_to_ray, initial_to_ray)) {
    case GREATER_THAN:
      /* exclude lowermost endpoint */
      return ( initial_to_ray != EQUAL_TO );
    case LESS_THAN:
      /* exclude lowermost endpoint */
      return -( final_to_ray != EQUAL_TO );
    default:
      /* any intersections cancel out */
      return 0;
    }
  } else { /* ray originates in bbox */
    std::vector<double> ts = roots(fy);

    static const unsigned MAX_DERIVATIVES=8;
    Maybe<SBasis> ds[MAX_DERIVATIVES];
    ds[0] = derivative(fy);

    /* winding determined by summing signs of derivatives at intersections */
    int winding=0;
    for ( std::vector<double>::iterator ti = ts.begin()
        ; ti != ts.end()
        ; ++ti )
    { 
      double t = *ti;
      if ( sb[X](t) >= p[X] ) { /* root is ray intersection */
        for ( Maybe<SBasis> *di = ds
            ; di != ( ds + MAX_DERIVATIVES )
            ; ++di )
        {
          if (!*di) {
            *di = derivative(**(di-1));
          }
          switch (cmp((**di)(t), 0)) {
          case GREATER_THAN:
            if ( t > 0 ) { /* exclude lowermost endpoint */
              winding += 1;
            }
            goto next_root;
          case LESS_THAN:
            if ( t < 1 ) { /* exclude lowermost endpoint */
              winding -= 1;
            }
            goto next_root;
          default: (void)0;
            /* give up */
          };
        }
      } 
next_root: (void)0;
    }
    
    return winding;
  }
}

Rect BezierHelpers::bounds(unsigned degree, Point const *points) {
  Point min=points[0];
  Point max=points[0];
  for ( unsigned i = 1 ; i <= degree ; ++i ) {
    for ( unsigned axis = 0 ; axis < 2 ; ++axis ) {
      min[axis] = std::min(min[axis], points[i][axis]);
      max[axis] = std::max(max[axis], points[i][axis]);
    }
  }
  return Rect(min, max);
}

Point BezierHelpers::point_and_derivatives_at(Coord t,
                                              unsigned degree,
                                              Point const *points,
                                              unsigned n_derivs,
                                              Point *derivs)
{
  return Point(0,0); // TODO
}

Geom::Point
BezierHelpers::subdivideArr(Coord t,              // Parameter value
                            unsigned degree,      // Degree of bezier curve
                            Geom::Point const *V, // Control pts
                            Geom::Point *Left,    // RETURN left half ctl pts
                            Geom::Point *Right)   // RETURN right half ctl pts
{
    Geom::Point Vtemp[degree+1][degree+1];

    /* Copy control points	*/
    std::copy(V, V+degree+1, Vtemp[0]);

    /* Triangle computation	*/
    for (unsigned i = 1; i <= degree; i++) {	
        for (unsigned j = 0; j <= degree - i; j++) {
            Vtemp[i][j] = Lerp(t, Vtemp[i-1][j], Vtemp[i-1][j+1]);
        }
    }
    
    for (unsigned j = 0; j <= degree; j++)
        Left[j]  = Vtemp[j][0];
    for (unsigned j = 0; j <= degree; j++)
        Right[j] = Vtemp[degree-j][j];

    return (Vtemp[degree][0]);
}

Path::~Path() {
  delete_range(curves_.begin(), curves_.end()-1);
  delete final_;
}

void Path::swap(Path &other) {
  std::swap(curves_, other.curves_);
  std::swap(closed_, other.closed_);
  std::swap(*final_, *other.final_);
  curves_[curves_.size()-1] = final_;
  other.curves_[other.curves_.size()-1] = other.final_;
}

Rect Path::boundsFast() const {
  Rect bounds=front().boundsFast();
  const_iterator iter=begin();
  for ( ++iter ; iter != end() ; ++iter ) {
    bounds.expandTo(iter->boundsFast());
  }
  return bounds;
}

Rect Path::boundsExact() const {
  Rect bounds=front().boundsExact();
  const_iterator iter=begin();
  for ( ++iter ; iter != end() ; ++iter ) {
    bounds.expandTo(iter->boundsExact());
  }
  return bounds;
}

int Path::winding(Point p) const {
  int winding;
  for ( const_iterator iter = begin()
      ; iter != end_closed()
      ; ++iter )
  {
    winding += iter->winding(p);
  }
  return winding;
}

void Path::append(Curve const &curve) {
  if ( curves_.front() != final_ && curve.initialPoint() != (*final_)[0] ) {
    throw ContinuityError();
  }
  do_append(curve.duplicate());
}

void Path::append(MultidimSBasis<2> const &curve) {
  if ( curves_.front() != final_ ) {
    for ( int i = 0 ; i < 2 ; ++i ) {
      if ( curve[i][0][0] != (*final_)[0][i] ) {
        throw ContinuityError();
      }
    }
  }
  do_append(new SBasisCurve(curve));
}

void Path::do_update(Sequence::iterator first_replaced,
                     Sequence::iterator last_replaced,
                     Sequence::iterator first,
                    Sequence::iterator last)
{
  // note: modifies the contents of [first,last)

  check_continuity(first_replaced, last_replaced, first, last);
  delete_range(first_replaced, last_replaced);

  if ( ( last - first ) == ( last_replaced - first_replaced ) ) {
    std::copy(first, last, first_replaced);
  } else {
    // this approach depends on std::vector's behavior WRT iterator stability
    curves_.erase(first_replaced, last_replaced);
    curves_.insert(first_replaced, first, last);
  }

  if ( curves_.front() != final_ ) {
    (*final_)[0] = back().finalPoint();
    (*final_)[1] = front().initialPoint();
  }
}

void Path::do_append(Curve *curve) {
  if ( curves_.front() == final_ ) {
    (*final_)[1] = curve->initialPoint();
  }
  curves_.insert(curves_.end()-1, curve);
  (*final_)[0] = curve->finalPoint();
}

void Path::delete_range(Sequence::iterator first, Sequence::iterator last) {
  for ( Sequence::iterator iter=first ; iter != last ; ++iter ) {
    delete *iter;
  }
}

void Path::check_continuity(Sequence::iterator first_replaced,
                            Sequence::iterator last_replaced,
                            Sequence::iterator first,
                            Sequence::iterator last)
{
  if ( first != last ) {
    if ( first_replaced != curves_.begin() ) {
      if ( (*first_replaced)->initialPoint() != (*first)->initialPoint() ) {
        throw ContinuityError();
      }
    }
    if ( last_replaced != (curves_.end()-1) ) {
      if ( (*(last_replaced-1))->finalPoint() != (*(last-1))->finalPoint() ) {
        throw ContinuityError();
      }
    }
  } else if ( first_replaced != last_replaced ) {
    if ( (*first_replaced)->initialPoint() !=
         (*(last_replaced-1))->finalPoint() )
    {
      throw ContinuityError();
    }
  }
}

Rect SBasisCurve::boundsFast() const {
  throw NotImplemented();
  return Rect(Point(0,0), Point(0,0));
}

Rect SBasisCurve::boundsExact() const {
  throw NotImplemented();
  return Rect(Point(0,0), Point(0,0));
}

Point SBasisCurve::pointAndDerivativesAt(Coord t, unsigned n_derivs, Point *derivs) const {
  throw NotImplemented();
  return Point(0,0);
}

Path const &SBasisCurve::subdivide(Coord t, Path &out) const {
  throw NotImplemented();
}

Rect SVGEllipticalArc::boundsFast() const {
    throw NotImplemented();
}
Rect SVGEllipticalArc::boundsExact() const {
    throw NotImplemented();
}

Point SVGEllipticalArc::pointAndDerivativesAt(Coord t, unsigned n_derivs, Point *derivs) const {
    throw NotImplemented();
}

MultidimSBasis<2> SVGEllipticalArc::sbasis() const {
    throw NotImplemented();
}


}

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
  vim: filetype=cpp:expandtab:shiftwidth=2:tabstop=8:softtabstop=2 :
*/

