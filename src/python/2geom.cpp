#include "point.h"
#include "point-ops.h"
#include "point-fns.h"
#include "matrix.h"
#include "scale.h"
#include "translate.h"
#include "rotate.h"
#include "s-basis.h"

#include <boost/python.hpp>
#include <boost/python/implicit.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
using namespace boost::python;

int python_index(int const index)
{
    if (index < 0) 
    {
        return -index - 1;
    }
    return index;
}


// helpers for point
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
    int i = python_index(index);
    if (i > 1) {
        PyErr_SetString(PyExc_StopIteration, "No more data.");
        boost::python::throw_error_already_set();
    }
    return p[i];
}

// helpers for bezord
tuple bezord_to_tuple(Geom::BezOrd const& b)
{
    return make_tuple(b[0], b[1]);
}

Geom::BezOrd tuple_to_bezord(boost::python::tuple const& t)
{
    return Geom::BezOrd(extract<double>(t[0]), extract<double>(t[1]));
}

double bezord_getitem(Geom::BezOrd const& b, int const index)
{
    int i = python_index(index);
    if (i > 1) {
        PyErr_SetString(PyExc_StopIteration, "No more data.");
        boost::python::throw_error_already_set();
    }
    return b[i];
}

Geom::BezOrd bezord_reverse(Geom::BezOrd const& b) {
    return reverse(b);
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
// TODO: explain why this gives a load time time error
    def("abs", (Geom::Point (*)(Geom::Point const&))&Geom::abs);
    
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
    
    //s-basis.h
    class_<Geom::BezOrd>("BezOrd", init<double, double>())
        .def(self_ns::str(self))
        .def("__getitem__", bezord_getitem)
        .def("tuple", bezord_to_tuple)
    
        .def("from_tuple", tuple_to_bezord)
        .staticmethod("from_tuple")
        
        .def("point_at", &Geom::BezOrd::point_at)
        .def("apply", &Geom::BezOrd::apply)
        .def("zero", &Geom::BezOrd::zero)
        .def("is_finite", &Geom::BezOrd::is_finite)

        .def(-self)
        .def(self + self)
        .def(self - self)
        .def(self += self)
        .def(self -= self)
        .def(self == self)
        .def(self != self)
        .def(self * self)
        .def("reverse", ((Geom::BezOrd (*)(Geom::BezOrd const &b))&bezord_reverse))
    ;
    implicitly_convertible<Geom::BezOrd,tuple>();
// TODO: explain why this gives a compile time error
//    implicitly_convertible<tuple,Geom::BezOrd>();

    // TODO: some of these don't compile
    //def("shift", &Geom::shift);
    def("truncate", &Geom::truncate);
    def("multiply", &Geom::multiply);
    //def("compose", &Geom::compose);
    def("integral", &Geom::integral);
    def("derivative", &Geom::derivative);
    def("sqrt", &Geom::sqrt);
    def("reciprocal", &Geom::reciprocal);
    def("divide", &Geom::divide);
    def("inverse", &Geom::inverse);
    def("sin", &Geom::sin);
    def("cos", &Geom::cos);
    //def("reverse", &Geom::reverse);
    def("bounds", &Geom::bounds);
    def("roots", &Geom::roots);

    class_<Geom::SBasis>("SBasis")
        .def(self_ns::str(self))
//        .def(vector_indexing_suite<Geom::SBasis>())
        .def(self + self)
        .def(self - self)
        .def("clear", &Geom::SBasis::clear)
        .def("normalize", &Geom::SBasis::normalize)
        .def("tail_eror", &Geom::SBasis::tail_error)
        .def("truncate", &Geom::SBasis::truncate)
        .def("is_finite", &Geom::SBasis::is_finite)
        .def(Geom::BezOrd() - self)
        .def(self += self)
        .def(self -= self)
        .def(self += Geom::BezOrd())
        .def(self -= Geom::BezOrd())
        .def(self += float())
        .def(self -= float())
        .def(Geom::BezOrd() + self)
        .def(float() + self)
        .def(self * self)
        .def(float() * self)
        .def(self *= self)
        .def(self *= float())
        .def(self /= float())
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
