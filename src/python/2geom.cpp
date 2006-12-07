#include "point.h"
#include "point-ops.h"
#include "point-fns.h"
#include "matrix.h"
#include "scale.h"
#include "translate.h"
#include "rotate.h"

#include <boost/python.hpp>
#include <boost/python/implicit.hpp>
using namespace boost::python;

tuple point_to_tuple(Geom::Point const& p)
{
    return make_tuple(p[0], p[1]);
}

Geom::Point tuple_to_point(boost::python::tuple const& t)
{
    return Geom::Point(extract<double>(t[0]), extract<double>(t[1]));
}

double point_getitem(Geom::Point const& p, int const index)
{
    int i = index;
    if (i < 0) {
        i = -i - 1;
    }
    if (i > 1) {
        PyErr_SetString(PyExc_StopIteration, "No more data.");
        boost::python::throw_error_already_set();
    }
    return p[i];
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
// TODO: explain why this gives a compile time error
//    def("abs", Geom::abs);
    
    class_<Geom::Point>("Point", init<double, double>())
        .def(self_ns::str(self))
        .def("__getitem__", point_getitem)
        .def("tuple", point_to_tuple)
    
        .def("from_tuple", tuple_to_point)
        .staticmethod("from_tuple")
        
        //point.h
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

    class_<Geom::Matrix>("Matrix", init<double, double, double, double, double, double>())
        .def(self_ns::str(self))
        .add_property("x_axis",&Geom::Matrix::x_axis,&Geom::Matrix::set_x_axis)
        .add_property("y_axis",&Geom::Matrix::y_axis,&Geom::Matrix::set_y_axis)
        .add_property("translation",&Geom::Matrix::translation,&Geom::Matrix::set_translation)
        .def("is_translation", &Geom::Matrix::is_translation)
        .def("is_rotation", &Geom::Matrix::is_rotation)
        .def("is_scale", &Geom::Matrix::is_scale)
        .def("is_uniform_scale", &Geom::Matrix::is_uniform_scale)
        .def("set_identity", &Geom::Matrix::set_identity)
        .def("det", &Geom::Matrix::det)
        .def("descrim2", &Geom::Matrix::descrim2)
        .def("descrim", &Geom::Matrix::descrim)
        .def("expansion", &Geom::Matrix::expansion)
        .def("expansionX", &Geom::Matrix::expansionX)
        .def("expansionY", &Geom::Matrix::expansionY)
    ;

    class_<Geom::scale>("Scale", init<double, double>())
        .def(self == self)
        .def(self != self)
        .def("inverse", &Geom::scale::inverse)
    ;

    class_<Geom::translate>("Translate", init<double, double>());

    class_<Geom::rotate>("Rotate", init<double>())
        .def(self == self)
        .def(self != self)
//TODO: compile reports "not defined"
//        .def(self *= self)
        .def("inverse", &Geom::rotate::inverse)
    ;


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
