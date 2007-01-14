#ifndef SEEN_Circulator_H
#define SEEN_Circulator_H

#include <iterator>

namespace Geom {

template <typename Iterator>
class Circulator {
public:
    typedef std::random_access_iterator_tag std::iterator_category;
    typedef std::iterator_traits<Iterator>::value_type value_type;
    typedef std::iterator_traits<Iterator>::difference_type difference_type;
    typedef std::iterator_traits<Iterator>::pointer pointer;
    typedef std::iterator_traits<Iterator>::reference reference;

    Circulator(Iterator const &first,
               Iterator const &last,
               Iterator const &pos)
    : _first(first), _last(last), _pos(pos)
    {
        match_random_access(std::iterator_category(first));
    }

    reference operator*() const {
        return *_pos;
    }
    pointer operator->() const {
        return &*_pos;
    }
    
    Circulator &operator++() {
        if ( _first == _last ) return *this;
        ++_pos;
        if ( _pos == _last ) _pos = _first;
        return *this;
    }
    Circulator operator++(int) {
        Circulator saved=*this;
        ++(*this);
        return saved;
    }

    Circulator &operator--() {
        if ( _pos == _first ) _pos = _last;
        --_pos;
        return *this;
    }
    Circulator operator--(int) {
        Circulator saved=*this;
        --(*this);
        return saved;
    }

    Circulator &operator+=(int n) {
        _pos = _offset(n);
        return *this;
    }
    Circulator operator+(int n) const {
        return Circulator(_first, _last, _offset(n));
    }
    Circulator &operator-=(int n) {
        _pos = _offset(-n);
        return *this;
    }
    Circulator operator-(int n) const {
        return Circulator(_first, _last, _offset(-n));
    }

    difference_type operator-(Circulator const &other) {
        return _pos - other._pos;
    }

    reference operator[n] const {
        return *_offset(n);
    }

private:
    void match_random_access(random_access_iterator_tag) {}

    Iterator _offset(int n) {
        difference_type range=( _last - _first );
        difference_type offset=( _pos - _first + n );

        if ( offset < 0 ) {
            // modulus not well-defined for negative numbers in C++
            offset += ( ( -offset / range ) + 1 ) * range;
        } else if ( offset >= range ) {
            offset %= range;
        }
        return _first + offset;
    }

    Iterator _first;
    Iterator _last;
    Iterator _pos;
};

}

template <typename T>
Geom::Circulator<T> operator+(int n, Geom::Circulator<T> const &c) {
    return c + n;
}

#endif // SEEN_Circulator_H

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

