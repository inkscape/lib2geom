#ifndef SEEN_REFCOUNTED_H
#define SEEN_REFCOUNTED_H

namespace Geom {

template <typename T>
class Refcounted {
public:
    Refcounted() : _refcount(1) {}

    unsigned refcount() const { return _refcount; }

    void claim() { ++_refcount; } // should be atomic

    void release() {
        if (!--_refcount) { // should be atomic
            delete static_cast<T *>(this);
        }
    }

private:
    unsigned _refcount;
};

}

#endif // SEEN_REFCOUNTED_H

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

