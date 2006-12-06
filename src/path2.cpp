#ifndef SEEN_GEOM_PATH_H
#define SEEN_GEOM_PATH_H

#include "path2.h"

namespace Geom {

namespace Path {

class Path {

Path::~Path() {
  delete_range(curves_.begin(), curves_.end()-1);
}

void Path::swap(Path &other) {
  std::swap(curves_, other.curves_);
  std::swap(closed_, other.closed_);
  std::swap(final_, other.final_);
  curves_[curves_.size()-1] = &final_;
  other.curves_[other.curves_.size()-1] = &other.final_;
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
  Rect bounds=front().boundsFast();
  const_iterator iter=begin();
  for ( ++iter ; iter != end() ; ++iter ) {
    bounds.expandTo(iter->boundsExact());
  }
  return bounds;
}

void Path::append(Curve const &curve) {
  if ( curves_.front() != &final_ && curve.initialPoint() != final_[0] ) {
    throw ContinuityError();
  }
  do_append(curve.duplicate());
}

void Path::do_update(Sequence::iterator first_replaced,
                     Sequence::iterator last_replaced,
                     Sequence::iterator first,
                    Sequence::iterator last)
{
  // note: modifies the contents of [first,last)

  check_continuity(first_replaced, last_replaced, first, last);
  duplicate_in_place(first, last);
  delete_range(first_replaced, last_replaced);

  if ( ( last - first ) == ( last_replaced - first_replaced ) ) {
    std::copy(first, last, first_replaced);
  } else {
    // this approach depends on std::vector's behavior WRT iterator stability
    curves_.erase(first_replaced, last_replaced);
    curves_.insert(first_replaced, first, last);
  }

  if ( curves_.front() != &final_ ) {
    final_[0] = back().finalPoint();
    final_[1] = front().initialPoint();
  }

void Path::do_append(Curve *curve) {
  if ( curves_.front() == &final_ ) {
    final_[1] = curve->initialPoint();
  }
  curves_.insert(curves_.end()-1, curve);
  final_[0] = curve->finalPoint();
}

void Path::duplicate_in_place(Sequence::iterator first,
                              Sequence::iterator last)
{
  for ( Sequence::iterator iter=first ; iter != last ; ++iter ) {
    *iter = (*iter)->duplicate();
  }
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

}

}

#endif // SEEN_GEOM_PATH_H

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

