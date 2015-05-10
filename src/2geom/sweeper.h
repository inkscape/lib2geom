/** @file
 * @brief Class for implementing sweepline algorithms
 *//*
  * Authors:
  *   Krzysztof Kosiński <tweenk.pl@gmail.com>
  *
  * Copyright 2015 Authors
  *
  * This library is free software; you can redistribute it and/or
  * modify it either under the terms of the GNU Lesser General Public
  * License version 2.1 as published by the Free Software Foundation
  * (the "LGPL") or, at your option, under the terms of the Mozilla
  * Public License Version 1.1 (the "MPL"). If you do not alter this
  * notice, a recipient may use your version of this file under either
  * the MPL or the LGPL.
  *
  * You should have received a copy of the LGPL along with this library
  * in the file COPYING-LGPL-2.1; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
  * You should have received a copy of the MPL along with this library
  * in the file COPYING-MPL-1.1
  *
  * The contents of this file are subject to the Mozilla Public License
  * Version 1.1 (the "License"); you may not use this file except in
  * compliance with the License. You may obtain a copy of the License at
  * http://www.mozilla.org/MPL/
  *
  * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
  * OF ANY KIND, either express or implied. See the LGPL or the MPL for
  * the specific language governing rights and limitations.
  */

#ifndef LIB2GEOM_SEEN_SWEEPER_H
#define LIB2GEOM_SEEN_SWEEPER_H

#include <2geom/coord.h>
#include <algorithm>
#include <vector>
#include <boost/intrusive/list.hpp>
#include <boost/range/algorithm/heap_algorithm.hpp>

namespace Geom {

struct IntervalSweepTraits {
    typedef Interval Bound;
    typedef std::less<Coord> Compare;
    inline static Coord entry_value(Bound const &b) { return b.min(); }
    inline static Coord exit_value(Bound const &b) { return b.max(); }
};

template <Dim2 d>
struct RectSweepTraits {
    typedef Rect Bound;
    typedef std::less<Coord> Compare;
    inline static Coord entry_value(Bound const &b) { return b[d].min(); }
    inline static Coord exit_value(Bound const &b) { return b[d].max(); }
};

/** @brief Generic sweepline algorithm.
 *
 * This class encapsulates an algorithm that sorts the objects according
 * to their bounds, then moves an imaginary line (sweepline) over those
 * bounds from left to right. Objects are added to the active list when
 * the line starts intersecting their bounds, and removed when it completely
 * passes over them.
 *
 * To use this, create a derived class and reimplement the _enter()
 * and/or _leave() virtual functions, insert all the objects,
 * and finally call process(). You can specify the bound type
 * and how it should be accessed by defining a custom SweepTraits class.
 *
 * Look in path.cpp for example usage.
 */
template <typename Item, typename SweepTraits = IntervalSweepTraits>
class Sweeper {
public:
    typedef typename SweepTraits::Bound Bound;

    Sweeper() {}

    void insert(Bound const &bound, Item const &item) {
        assert(!(typename SweepTraits::Compare()(
            SweepTraits::exit_value(bound),
            SweepTraits::entry_value(bound))));
        _items.push_back(Record(bound, item));
    }

    template <typename Iter, typename BoundFunc>
    void insert(Iter first, Iter last, BoundFunc f = BoundFunc()) {
        for (; first != last; ++first) {
            Bound b = f(*first);
            assert(!(typename SweepTraits::Compare()(
                SweepTraits::exit_value(b),
                SweepTraits::entry_value(b))));
            _items.push_back(Record(b, *first));
        }
    }

    /** @brief Process entry and exit events.
     * This will iterate over all inserted items, calling the virtual protected
     * functions _enter() and _leave() according to the order of the boundaries
     * of each item. */
    void process() {
        if (_items.empty()) return;

        typename SweepTraits::Compare cmp;

        for (RecordIter i = _items.begin(); i != _items.end(); ++i) {
            _entry_events.push_back(i);
            _exit_events.push_back(i);
        }
        boost::make_heap(_entry_events, _entry_heap_compare);
        boost::make_heap(_exit_events, _exit_heap_compare);
        boost::pop_heap(_entry_events, _entry_heap_compare);
        boost::pop_heap(_exit_events, _exit_heap_compare);

        RecordIter next_entry = _entry_events.back();
        RecordIter next_exit = _exit_events.back();
        _entry_events.pop_back();
        _exit_events.pop_back();

        while (next_entry != _items.end() || next_exit != _items.end()) {
            assert(next_exit != _items.end());

            if (next_entry == _items.end() ||
                cmp(SweepTraits::exit_value(next_exit->bound),
                    SweepTraits::entry_value(next_entry->bound)))
            {
                // exit event - remove record from active list
                _leave(*next_exit);
                _active_items.erase(_active_items.iterator_to(*next_exit));
                if (!_exit_events.empty()) {
                    boost::pop_heap(_exit_events, _exit_heap_compare);
                    next_exit = _exit_events.back();
                    _exit_events.pop_back();
                } else {
                    next_exit = _items.end();
                    // we should end the loop after this happens
                }
            } else {
                // entry event - add record to active list
                _enter(*next_entry);
                _active_items.push_back(*next_entry);
                if (!_entry_events.empty()) {
                    boost::pop_heap(_entry_events, _entry_heap_compare);
                    next_entry = _entry_events.back();
                    _entry_events.pop_back();
                } else {
                    next_entry = _items.end();
                }
            }
        }

        assert(_active_items.empty());
    }

protected:
    /// The item and its sweepline boundary.
    struct Record {
        boost::intrusive::list_member_hook<> _hook;
        Bound bound;
        Item item;

        Record(Bound const &b, Item const &i)
            : bound(b), item(i)
        {}
    };
    typedef typename std::vector<Record>::iterator RecordIter;

    typedef boost::intrusive::list
    < Record
    , boost::intrusive::member_hook
        < Record
        , boost::intrusive::list_member_hook<>
        , &Record::_hook
        >
    > RecordList;

    /** @brief Enter an item record.
     * Override this to process an item as it is about to enter the active list.
     * When called, the passed record will not be part of the active list. */
    virtual void _enter(Record const &) {}
    /** @brief Leave an item record.
     * Override this to process an item as it is about to leave the active list.
     * When called, the passed record will be part of the active list. */
    virtual void _leave(Record const &) {}

    /// The list of all item records undergoing sweeping.
    std::vector<Record> _items;
    /// The list of active item records.
    RecordList _active_items;

private:
    inline static bool _entry_heap_compare(RecordIter a, RecordIter b) {
        typename SweepTraits::Compare cmp;
        return cmp(SweepTraits::entry_value(b->bound), SweepTraits::entry_value(a->bound));
    }
    inline static bool _exit_heap_compare(RecordIter a, RecordIter b) {
        typename SweepTraits::Compare cmp;
        return cmp(SweepTraits::exit_value(b->bound), SweepTraits::exit_value(a->bound));
    }

    std::vector<RecordIter> _entry_events;
    std::vector<RecordIter> _exit_events;
};

} // namespace Geom

#endif // !LIB2GEOM_SEEN_SWEEPER_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
