#ifndef GEOM_PATH_BUILDER_H
#define GEOM_PATH_BUILDER_H

#include "path.h"

namespace Geom {

class PathBuilder {
public:
    PathBuilder() : _current_subpath(NULL) {}

    void start_subpath(Point const &p0) {
        _path.subpaths.push_back(SubPath());
        _current_subpath = &_path.subpaths.back();
        _current_subpath->closed = false;
        _current_subpath->handles.push_back(p0);
        _initial_point = _current_point = p0;
    }

    void push_line(Point const &p1) {
        if (!_current_subpath) start_subpath(_current_point);
        _current_subpath->cmd.push_back(lineto);
        _current_subpath->handles.push_back(p1);
        _current_point = p1;
    }

    void push_quad(Point const &p1, Point const &p2) {
        if (!_current_subpath) start_subpath(_current_point);
        _current_subpath->cmd.push_back(quadto);
        _current_subpath->handles.push_back(p1);
        _current_subpath->handles.push_back(p2);
        _current_point = p2;
    }

    void push_cubic(Point const &p1, Point const &p2, Point const &p3) {
        if (!_current_subpath) start_subpath(_current_point);
        _current_subpath->cmd.push_back(cubicto);
        _current_subpath->handles.push_back(p1);
        _current_subpath->handles.push_back(p2);
        _current_subpath->handles.push_back(p3);
        _current_point = p3;
    }

    void 
    push_cubic(Point const &p0, Point const &p1, Point const &p2, Point const &p3) {
        if(p0 != _current_point)
            start_subpath(p0);
        push_cubic(p1, p2, p3);
    }

    void push_ellipse(Point const &p1, Point const &p2, Point const &p3, Point const &p4) {
        if (!_current_subpath) start_subpath(_current_point);
        _current_subpath->cmd.push_back(cubicto);
        _current_subpath->handles.push_back(p1);
        _current_subpath->handles.push_back(p2);
        _current_subpath->handles.push_back(p3);
        _current_subpath->handles.push_back(p4);
    }

    void close_subpath() {
        if (_current_subpath) {
            push_line(_initial_point);
            _current_subpath->closed = true;
            _current_subpath = NULL;
        }
    }

    Path const &peek() const { return _path; }

private:
    Path _path;
    SubPath *_current_subpath;
    Point _current_point; // perhaps this is just _current_subpath->handles.back()
    Point _initial_point;
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
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
