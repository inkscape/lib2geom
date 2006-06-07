#include <vector>
#include <algorithm>

#ifndef SEEN_GEOM_FLOATING_H
#define SEEN_GEOM_FLOATING_H

namespace Geom {

class FloatingObject;

class FloatingFrame {
public:
    FloatingFrame() {
        _parent = _top_frame;
        _top_frame = this;
    }
    ~FloatingFrame() {
        _top_frame = _parent;
        std::vector<FloatingObject *>::iterator iter;
        for ( iter = _floating.begin() ; iter != _floating.end() ; ++iter ) {
            delete *iter;
        }
    }

private:
    static FloatingFrame *_top_frame=NULL;
    FloatingFrame *_parent;
    std::vector<FloatingObject *> _floating;

    static void _float(FloatingObject *object) {
        _top_frame->_floating.push_back(object);
    }
    static void _unfloat(FloatingObject *object) {
        FloatingFrame *frame;
        for ( frame = top_frame ; frame ; frame = frame->_parent ) {
            std::vector<FloatingObject *>::iterator found;
            found = std::find(frame->_floating.begin(),
                              frame->_floating.end(),
                              object);
            if ( found != frame->_floating.end() ) {
                frame->_floating.erase(found);
                break;
            }
        }
    }

friend class FloatingObject;
};

class FloatingObject {
public:
    FloatingObject() {
        FloatingFrame::_float(this);
    }
    virtual ~FloatingObject() {
        /* it is an error if a floating object is manually
           deleted while still floating */
    }

    void unfloat() {
        FloatingFrame::_unfloat(this);
    }
};

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

