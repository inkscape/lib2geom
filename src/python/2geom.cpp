#include "point.h"

#include <boost/python.hpp>
using namespace boost::python;

BOOST_PYTHON_MODULE(lib2geom_py)
{
    class_<Geom::Point>("Point", init<double, double>())
        .def(self_ns::str(self))
    ;
}
