#include "Python.h"
#include "2geom/generic-interval.h" 
#include "2geom/generic-rect.h"
#include "2geom/d2.h"


namespace Geom{

//TODO! looks like memory leak
class WrappedPyObject{
public:
    PyObject* self;
    WrappedPyObject(){
        //printf("Created empty WPO\n");
        self = NULL;
    }
    
    WrappedPyObject(WrappedPyObject const &other){
        self = other.getObj();
        //printf("COPY-CONSTRUCTOR %p\n", other.getObj());
        Py_INCREF(self);
        }
    
    WrappedPyObject(PyObject* arg){
        if (arg == NULL){
            //PyErr_Print();
        }
        else{
            //printf("CONSTRUCTOR %p\n", arg);
            Py_INCREF(arg);
            self = arg;
        }
    }
    
    ~WrappedPyObject(){
        //TODO Leaking memory
        //printf("DECREF %p\n", self);
        //Py_DECREF(self);
    }
    
    PyObject* getObj() const {
        return self;
    }
    
    WrappedPyObject operator=(WrappedPyObject other){
        if (this != &other){
            self = other.getObj();
            //printf("OPERATOR= %p\n", self);
            Py_INCREF(self);
        }
        return *this;
    }
    
    WrappedPyObject operator-() const {
        PyObject * ret;
        ret = PyObject_CallMethodObjArgs(self, Py_BuildValue("s", "__neg__"), NULL);
        if (ret == NULL){
            Py_INCREF(Py_None);
            return Py_None;
            }
        //Py_INCREF(ret);
        WrappedPyObject * retw = new WrappedPyObject(ret);
        //Py_DECREF(ret);
        return *retw;
    }
    
    WrappedPyObject arithmetic(PyObject* const other, std::string method, std::string rmethod) const {
        PyObject * ret;
        //printf("%p %p\n", self, other);
        ret = PyObject_CallMethodObjArgs(self, Py_BuildValue("s", method.c_str()), other, NULL);
        if (ret == NULL){
            Py_INCREF(Py_None);
            return Py_None;
            }
        PyObject * isNI = PyObject_RichCompare(ret, Py_NotImplemented, Py_EQ);
        if ( PyInt_AsLong(isNI) ){
            ret = PyObject_CallMethodObjArgs(other, Py_BuildValue("s", rmethod.c_str()), self, NULL);
            }
        if (ret == NULL){
            Py_INCREF(Py_None);
            return Py_None;
            }
        WrappedPyObject * retw = new WrappedPyObject(ret);
        return *retw;
        
        }
    
    WrappedPyObject operator+(WrappedPyObject const other) const {
        return arithmetic(other.getObj(), "__add__", "__radd__");
    }

    WrappedPyObject operator-(WrappedPyObject const other){
        return arithmetic(other.getObj(), "__sub__", "__rsub__");
    }
    
    WrappedPyObject operator*(WrappedPyObject const other){
        return arithmetic(other.getObj(), "__mul__", "__rmul__");
    }

    WrappedPyObject operator*(int other){
        PyObject* other_obj = Py_BuildValue("i", other);
        //Py_INCREF(other_obj);
        WrappedPyObject ret =  arithmetic(other_obj, "__mul__", "__rmul__");
        //Py_DECREF(other_obj);
        return ret;
    }

    WrappedPyObject operator/(int other){
        PyObject* other_obj = Py_BuildValue("i", other);
        Py_INCREF(other_obj);
        WrappedPyObject ret =  arithmetic(other_obj, "__div__", "__rdiv__");
        Py_DECREF(other_obj);
        return ret;
    }
    
    bool richcmp(WrappedPyObject const &other, int op) const {
        PyObject * ret;
        long retv;
        ret = PyObject_RichCompare(self, other.getObj(), op);
        retv = PyInt_AsLong(ret);
        return retv;
    }
    
    bool operator<(WrappedPyObject const &other) const {
        return richcmp(other, Py_LT);
    }
    
    bool operator<=(WrappedPyObject const &other) const {
        return richcmp(other, Py_LE);
    }
    
    bool operator>=(WrappedPyObject const &other) const{
        return richcmp(other, Py_GE);
    }

    bool operator>(WrappedPyObject const &other) const {
        return richcmp(other, Py_GT);
    }
    
    bool operator==(WrappedPyObject const &other) const {
        return richcmp(other, Py_EQ);
    }
    
    WrappedPyObject operator+=(WrappedPyObject other){
        *this = *this + other;
        return *this;

    }

    WrappedPyObject operator-=(WrappedPyObject other){
        *this = *this - other;
        return *this;
    }
};
    
typedef GenericInterval<WrappedPyObject> PyInterval;
typedef GenericOptInterval<WrappedPyObject> PyOptInterval;
    
typedef GenericRect<WrappedPyObject> PyRect;
typedef GenericOptRect<WrappedPyObject> PyOptRect;

typedef D2<WrappedPyObject> PyPoint;

template<>
struct CoordTraits<WrappedPyObject> {
    typedef PyPoint PointType;
    typedef PyInterval IntervalType;
    typedef PyOptInterval OptIntervalType;
    typedef PyRect RectType;
    typedef PyOptRect OptRectType;

    typedef
      boost::equality_comparable< PyInterval
    , boost::additive< PyInterval
    , boost::multipliable< PyInterval
    , boost::orable< PyInterval
    , boost::arithmetic< PyInterval, WrappedPyObject
      > > > > >
        IntervalOps;

    typedef
      boost::equality_comparable< PyRect
    //, boost::equality_comparable< PyRect, IntRect
    , boost::orable< PyRect
    , boost::orable< PyRect, PyOptRect
    , boost::additive< PyRect, PyPoint
    //, boost::multipliable< Rect, Affine
      > > > > //> >
        RectOps;
};
    
}
