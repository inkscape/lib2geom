#include "point.h"
#include "point-ops.h"

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

	//point-ops.h
	.def(self + self)
	.def(self - self)
	.def(self ^ self)
	.def(-self)
	.def(float() * self)
	.def(self * float())
	.def(self / float())
	.def(float() / self)
	.def(self != self)
	.def(self <= self)

    ;
}
