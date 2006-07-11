#ifndef GEOM_PATH_BUILDER_H
#define GEOM_PATH_BUILDER_H

#include "path.h"

namespace Geom {

class PathBuilder {
public:
    PathBuilder() : _current_subpath(NULL) {}

    void start_subpath(Point const &p) {
        _path.subpaths.push_back(SubPath());
        _current_subpath = &_path.subpaths.back();
        _current_subpath->handles.push_back(p);
        _initial_point = _current_point = p;
    }

    void push_line(Point const &p) {
        if (!_current_subpath) start_subpath(_current_point);
        _current_subpath->cmd.push_back(lineto);
        _current_subpath->handles.push_back(p);
    }

    void push_quad(Point const &p0, Point const &p1) {
        if (!_current_subpath) start_subpath(_current_point);
        _current_subpath->cmd.push_back(quadto);
        _current_subpath->handles.push_back(p0);
        _current_subpath->handles.push_back(p1);
    }

    void push_cubic(Point const &p0, Point const &p1, Point const &p2) {
        if (!_current_subpath) start_subpath(_current_point);
        _current_subpath->cmd.push_back(cubicto);
        _current_subpath->handles.push_back(p0);
        _current_subpath->handles.push_back(p1);
        _current_subpath->handles.push_back(p2);
    }

    void push_ellipse(Point const &p0, Point const &p1, Point const &p2, Point const &p3) {
        if (!_current_subpath) start_subpath(_current_point);
        _current_subpath->cmd.push_back(cubicto);
        _current_subpath->handles.push_back(p0);
        _current_subpath->handles.push_back(p1);
        _current_subpath->handles.push_back(p2);
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
    Point _current_point;
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
