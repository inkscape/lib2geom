#include "point.h"
#include "point-ops.h"
#include "point-fns.h"

#include <boost/python.hpp>
#include <boost/python/implicit.hpp>
using namespace boost::python;

tuple point_to_tuple(Geom::Point const& p)
{
    return make_tuple(p[0], p[1]);
}

Geom::Point tuple_to_point(boost::python::tuple t)
{
    return Geom::Point(extract<double>(t[0]), extract<double>(t[1]));
}

BOOST_PYTHON_MODULE(lib2geom_py)
{
    def("point_to_tuple", point_to_tuple);
    def("tuple_to_point", tuple_to_point);

    //point-fns.h
    def("L1", Geom::L1);
    def("L2", Geom::L2);
    def("L2sq", Geom::L2sq);
    def("LInfty", Geom::LInfty);
    def("is_zero", Geom::is_zero);
    def("is_unit_vector", Geom::is_unit_vector);
    def("atan2", Geom::atan2);
    def("angle_between", Geom::angle_between);
    def("point_equalp", Geom::point_equalp);
    def("rot90", Geom::rot90);
    def("Lerp", Geom::Lerp);
    def("unit_vector", Geom::unit_vector);
    def("dot", Geom::dot);
    def("distance", Geom::distance);
    def("dist_sq", Geom::dist_sq);
    def("cross", Geom::cross);
    def("abs", Geom::abs);
    
    class_<Geom::Point>("Point", init<double, double>())
        .def(self_ns::str(self))
        .def("tuple", point_to_tuple)
    
        .def("from_tuple", tuple_to_point)
        .staticmethod("from_tuple")
        
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
    implicitly_convertible<Geom::Point,tuple>();
// TODO: explain why this gives a compile time error
//    implicitly_convertible<tuple,Geom::Point>();

}

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
