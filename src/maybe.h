/*
 * Nullable values for C++
 *
 * Copyright 2004, 2007  MenTaLguY <mental@rydia.net>
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

#ifndef SEEN_GEOM_MAYBE_H
#define SEEN_GEOM_MAYBE_H

#include <stdexcept>
#include <string>

namespace Geom {

class IsNothing : public std::domain_error {
public:
    IsNothing() : domain_error(std::string("Is nothing")) {}
};

struct Nothing {};

template <typename T>
class MaybeStorage {
public:
    MaybeStorage() : _is_nothing(true) {}
    MaybeStorage(T const &value)
    : _value(value), _is_nothing(false) {}

    bool is_nothing() const { return _is_nothing; }
    T &value() { return _value; }
    T const &value() const { return _value; }

private:
    T _value;
    bool _is_nothing;
};

template <typename T>
class Maybe {
public:
    Maybe() {}
    Maybe(Nothing) {}
    Maybe(T const &t) : _storage(t) {}
    Maybe(Maybe const &m) : _storage(m._storage) {}

    template <typename T2>
    Maybe(Maybe<T2> const &m) {
        if (m) {
            _storage = *m;
        }
    }

    template <typename T2>
    Maybe(Maybe<T2 const &> m) {
        if (m) {
            _storage = *m;
        }
    }

    operator bool() const { return !_storage.is_nothing(); }

    T const &operator*() const throw(IsNothing) {
        if (_storage.is_nothing()) {
            throw IsNothing();
        } else {
            return _storage.value();
        }
    }
    T &operator*() throw(IsNothing) {
        if (_storage.is_nothing()) {
            throw IsNothing();
        } else {
            return _storage.value();
        }
    }

    T const *operator->() const throw(IsNothing) {
        if (_storage.is_nothing()) {
            throw IsNothing();
        } else {
            return &_storage.value();
        }
    }
    T *operator->() throw(IsNothing) {
        if (_storage.is_nothing()) {
            throw IsNothing();
        } else {
            return &_storage.value();
        }
    }

    template <typename T2>
    bool operator==(Maybe<T2> const &other) const {
        bool is_nothing = _storage.is_nothing();
        if ( is_nothing || !other ) {
            return is_nothing && !other;
        } else {
            return _storage.value() == *other;
        }
    }
    template <typename T2>
    bool operator!=(Maybe<T2> const &other) const {
        bool is_nothing = _storage.is_nothing();
        if ( is_nothing || !other ) {
            return !is_nothing || other;
        } else {
            return _storage.value() != *other;
        }
    }

private:
    MaybeStorage<T> _storage;
};

template <typename T>
class Maybe<T &> {
public:
    Maybe() : _ref(NULL) {}
    Maybe(Nothing) : _ref(NULL) {}
    Maybe(T &t) : _ref(&t) {}

    template <typename T2>
    Maybe(Maybe<T2> const &m) {
        if (m) {
            _ref = &*m;
        } 
    }

    template <typename T2>
    Maybe(Maybe<T2 &> m) {
        if (m) {
            _ref = *m;
        }
    }

    template <typename T2>
    Maybe(Maybe<T2 const &> m) {
        if (m) {
            _ref = *m;
        }
    }

    operator bool() const { return _ref; }

    T &operator*() const throw(IsNothing) {
        if (!_ref) {
            throw IsNothing();
        } else {
            return *_ref;
        }
    }
    T *operator->() const throw(IsNothing) {
        if (!_ref) {
            throw IsNothing();
        } else {
            return _ref;
        }
    }

    template <typename T2>
    bool operator==(Maybe<T2> const &other) const {
        if ( !_ref || !other ) {
            return !_ref && !other;
        } else {
            return *_ref == *other;
        }
    }
    template <typename T2>
    bool operator!=(Maybe <T2> const &other) const {
        if ( !_ref || !other ) {
            return _ref || other;
        } else {
            return *_ref != *other;
        }
    }

private:
    T *_ref;
};

}

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
