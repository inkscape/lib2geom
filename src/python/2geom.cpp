#include "point.h"

#include <boost/python.hpp>
using namespace boost::python;

BOOST_PYTHON_MODULE(lib2geom_py)
{
    class_<Geom::Point>("Point", init<double, double>())
        .def(self_ns::str(self))

	.def("polar", &Geom::Point::polar)
	.staticmethod("polar")

	.def("ccw", &Geom::Point::ccw)
	.def("cw", &Geom::Point::cw)
	.def("round", &Geom::Point::round)
	.def("normalize", &Geom::Point::normalize)

	.def(self += self)
	.def(self -= self)
	.def(self /= float())
	.def(self *= float())

	.def(self == self)
    ;
}
