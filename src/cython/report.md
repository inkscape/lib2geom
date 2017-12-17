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

##`_cy_affine`
Affine and specialised transforms. This is in pretty good state. Eigenvalues
seem to be a bit off sometimes.

##`_cy_curves`
All of the curves are wrapped here, together with functions like Linear, 
SBasis and Bezier. 

Curve implements method every curve should implement, but other curve 
classes do not inherit from Curve. This is mainly because of technical
details, this inheritance would only add complexity to code. However, I 
might consider rewriting curves, so they would actually inherit from 
Curve, if this appears to be needed.

State of functions and classmethods is a bit strange. I tried to copy 
2geom's methods, but there are differences for Beziers and SBasis 
polynomials - to name a few, SBasis implements multiplication with SBasis
as a operator, but Bezier only as a function, or bounds are implemented 
as instance methods for Linear, but as a module level functions for Bezier
and SBasis. In case I wasn't sure what to do with a method, I put it, as 
a classmethod, to corresponding class.

BezierCurveN is not implemented. It would be major hurdle, as cython has
problems with templated classes - one would have to declare each type 
separately. First three orders are wrapped, and BezierCurve, with variable
degree, is also wrapped.

`hacks.h` is used to go around cython's problems with integer template 
parameters.

##`_cy_path`
Path is wrapped here. Biggest problem with path was it's heavy use of 
iterators, which are not the same as python's iterators. In most of the
cases, I replaced them with integers marking the position of iterator, 
and go to the position step-by-step. This makes potentially O(1) operations
actually O(n) where n is size of Path. Best way to deal with this would be
to allow iterators to be shifted by arbitrary number. 

Not everything is provided, but there should be enough functionality to 
have total control over Path's curves.

##`_cy_conicsection`
Circle and ellipse classes. ratQuad and xAx from conic-section should also
go there.

##`utils.py`
Only a simple function to draw curve/path to Tk windows and regular
N-agon creating function reside there, useful mainly for debugging

##`wrapper.py`
Script used to cut the most trivial part of creating bindings, writing
.pxd declarations, extension type and other functions. It's pretty tailored
into 2geom, but I guess after some work one could make it more generic.


##General Issues
Exceptions coming from C++ code are not handled yet, they actually crash 
the program. Simplest solution is adding `except +` after every method that
possibly raises exceptions, but traces won't look that nice.

cython has some kind of problem with docstrings, it doesn't write function's
argument to them, just ellipsis. This can be addressed by specifying all 
arguments in the docstring.

Design decision has been made to put most of function not belonging to any 
class to extension type's namespace using classmethods. Most apt extension
type is chosen, distance(Point, Point) goes to Point class. Other options 
are making them check for all possible types (this is ugly and error-prone) 
or differentiating using different names, which is more or less the same as 
classmethods, just not that systematic

##What's missing
Piecewise missing and all of crossings/boolops stuff are biggest things 
that didn't make. Concerning Piecewise, I think best option is to 
implement only Piecewise<SBasis> and Piecewise<D2<SBasis>> as those are 
only ones actually useful (SBasis, Bezier and their D2 versions are AFAIK
only classes implementing required concepts).

Crossings are working, boolops not so much. Work on Shape and Region has
been started.

#Personal Notes
I am also summarizing a bit on my personal experience with GSoC.

##What Did I Learn
I learned a lot, not only new technology, but also gained new perspective
on long-time work. 

Speaking of technology, I am much more confident with cython, and I am not
really scared of python C-API. I got a grasp of bazaar, CMake, python's 
unittest module and various (mainly template) C++ features, heavily used 
in 2geom.

I got a grasp of what a polished product looks like, and I realize how 
hard is to get your product to that state. I can now appreciate testing,
at first seemingly just a boring and not trivial chore, turned out to be
very helpful both for my understanding of code and for making changes more
quickly. I also read a lot about design of python API, what's idiomatic 
and what does various norms (PEPs) say about python.

I think I never did something of this size before (not that monolithic), 
so I learned lot about my work habits and importance of both work ethic 
and taking a rest.

##What I Liked & Disliked
I liked mainly the process of learning new things. I disliked the really
technical scope of this project - I can imagine more exciting things to
work on than doing bindings, but I guess it doesn't hurt to have this kind
of experience. I certainly won't spend my whole life doing only exciting 
things :)

##What Would I Do Differently
I would have planned a bit more. I like to write it on account of my 
inexperience, I felt many times that knowing how should final product 
look like would save me tons of work - this way, I approached state
with which I was more-or-less content at the end of doing most of the tasks.
Luckily, I got time to go through the code at the end, so I ended up 
correcting the largest mistakes.

I would also discuss design of API bit more, since mapping the C++ sometimes
turned out to be cumbersome from python's perspective. Often, I would spend
lots of time thinking through some detail (Iterator for Path, boost::optional,
most of the templated classes), only doing ad-hoc solution at the end in 
order to I to keep progressing.

Jan Pulmann -  jan.pulmann@gmail.com

[2g]: http://lib2geom.sourceforge.net/
[cy]: http://www.cython.org/
[cm]: https://github.com/thewtex/cython-cmake-example
[tr]: https://code.launchpad.net/~lib2geom-hackers/lib2geom/trunk
