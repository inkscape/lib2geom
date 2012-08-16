#State of cython Bindings 

This report summarizes work done on [lib2geom][2g] cython bindings during 
GSoC 2012 mostly from technical standpoint. It should serve for anyone
doing any further work on these bindings, and for me to help my memory.

Prior to the project, there were bindings using boost.python. These
bindigs, however, uncovered only part of the lib2geom functionality and 
were not actively developed. I decided to go for a fresh start using 
[cython] [cy]. Cython is programming language capable of calling C and C++
methods and making so-called "extension types", which act as python 
classes. Reasons for choosing python include bit of experience I had with
it, its cleanliness and activity in development.

##Overview of Bindings

All work was done in [trunk][tr], since it mostly added new functionality 
without altering existing code. Whole bindings are located in directory 

    2geom_dir/src/2geom/cython-bindings

Code is located in files ending with .pxd and .pyx, roughly cython's 
equivalent of header and implementation files. Classes in cython are 
divided into logical groups, which are wrapped in corresponding classes.
cy2geom.pyx imports all classes and methods, and it's what one sees after

    >>> import cy2geom
    
In addition to this, there are some C++ files, necessary to wrap some of
the code for cython, unit-tests covering most of the functionality and
wrapper.py script, used to skip tedious part of creating extension types.

Building of bindings was integrated with cmake, which is used to build 
the bindings, using  [thewext's code] [cm].

I will now describe files/logical groups of them, their significance and
state. I will try to write down most of the technical non-trivialities that
I encountered.

Pattern used for wrapping all classes is taken from cython docs and various
other cython bindings. In .pxd file C++ classes, methods and functions are
declared. In corresponding .pyx file there is an extension type which holds
pointer (called `thisptr`) to it's underlying C++ class. Instance of this 
class is created with cython's `__cinit__`, and deleted with `__dealloc__`. 
Methods of class are called with `self.thisptr.methodname`, which cython
translates to `thisptr->methodname`. Argument are stripped of their python
wrapping using `deref(arg.thisptr)`, or they are translated automatically 
(these are basic numerical types). 

There is a function for each class, named `wrap_classname()`, which takes
C++ instance and creates new python object around it.

##`_common_decl`
These files (.pxd/.pyx) include basic declarations, common to all files.
Functions for creating and wrapping `vector[double]` should become  reduntant
in next cython release (0.17), since it should do this conversion 
automatically. 

All extension types (cdef classes) have prefix `_cy_`, to avoid clash with 
C++ classes. This is removed when importing to `_common_decl.pyx`, but is still
somewhat visible to the user of bindings, for example in traces. Other 
option would be renaming the C++ classes.

##`_cy_primitives`
Geometric primitives are wrapped here. These are relatively simple, few 
things that are worth mentioning are:

* Some of the methods for Line and Ray could be rewritten using properties.
* General points about exceptions, docstrings and classmethod apply here.

##`_cy_rectangle`
These files include intervals and rectangles, which all inherit from
`GenericInterval[C]` or `GenericRect[C]`, respectively. 

For intervals, classes Interval and IntInterval are wrapped, with added 
methods for interval too. Opt variants are supported by rewriting 
`__getattr__` method. When calling the method that Interval provides, 
OptInterval checks whether it's not empty and passes arguments to Interval's
method. If it is empty, it will raise ValueError. Due to this, Interval's
method do not appear in OptInterval's namespace, which might be not ideal.

GenericInterval is supported by setting the template type to WrappedPyObject,
defined in `wrapped-pyobject.h`. This thin wrapper of `PyObject *` overloads
operators to support arithmetic operations, comparisons and similar. When 
adding two WrappedPyObjects, these objects call corresponding addition using
Python's C-API. This is a bit unstable feature, because it still leaks these
python objects, and propagating error from Python, through C-API to cython
back to Python is not done yet correctly. However, constraints on
parameter of GenericInterval's type are pretty tight - it has to support
arithmetic operations within itself, multiplication by reals and has to be 
ordered (ordered linear space with multiplication and division between its
elements), so having GenericInterval only with reals and integers covers most
cases.

For rectangles, situations is almost identical as for intervals. GenericRect,
return 2-tuples as a points, which is a bit of disadvantage, because adding 
tuples doesn't add element-wise, but append them one after another. It might
be convenient to implement PyPoint overloading this funtionality.

A bit problematic region is in comparing all the intervals and rectangles.
Should every interval be comparable to every other type, Interval with 
OptIntInterval? This would probably require writing the logic in cython.

GenericOptRect is not there yet, because I got GenericRect only working
only recently. It should be, however, quick to add.


to be continued.


##General Issues:
Exceptions coming from C++ code are not handled yet, they actually crash 
the program. Simplest solution is adding `except +` after every method that
possibly raises exceptions (I hope to do this before GSoC ends), but trace's
won't look that nice.

cython has some kind of problem with docstrings, it doesn't write function's
argument to them, just ellipsis. This can be addresed by specifying all 
arguments in the docstring.

Design decision has been made to put most of function not belonging to any 
class to extension type's namespace using classmethods. Other options are
making them check for all possible types (this is ugly and error-prone) or
differentiating using different names, which is more or less the same as 
classmethods, just not that systematic

[2g]: http://lib2geom.sourceforge.net/
[cy]: http://www.cython.org/
[cm]: https://github.com/thewtex/cython-cmake-example
[tr]: https://code.launchpad.net/~lib2geom-hackers/lib2geom/trunk
