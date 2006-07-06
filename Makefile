EXTRA_CPPFLAGS:=$(shell pkg-config --cflags gtk+-2.0) \
		$(shell pkg-config --cflags cairo) 
		
EXTRA_LDFLAGS:=$(shell pkg-config --libs gtk+-2.0) -lgsl -lblas


CXX=g++
CXXFLAGS=-O3
CPPFLAGS=-I.


TARGETOBJS=arc-bez.o conic.o one-D.o poly-test.o rat-bez.o s-bez.o \
	toy-cairo.o unit-test-sbasis.o
BADTARGETOBJS=bspline.o conic-2.o ode-toy-cairo.o tensor-reparam.o toy.o

LIBOBJS=arc-length.o centroid.o geom.o matrix-rotate-ops.o \
	matrix-translate-ops.o matrix.o path-find-points-of-interest.o \
	path-intersect.o path-metric.o path-to-polyline.o path-to-svgd.o \
	path.o point-fns.o poly.o read-svgd.o rect.o rotate-fns.o s-basis.o \
	sbasis-poly.o types.o

EXTRAOBJS=interactive-bits.o path-cairo.o $(TARGETOBJS)

TARGETS=$(TARGETOBJS:.o=)

ALLOBJS=$(LIBOBJS) $(EXTRAOBJS)

# Dependency magic.
%o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $*.cpp -o $*.o
	@$(CXX) -MM $(CPPFLAGS) $*.cpp > $*.d
	@cp -f $*.d $*.d.tmp 
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

# Grab dependencies where they exist.
-include $(ALLOBJS:.o=.d)

$(EXTRAOBJS): CPPFLAGS:=$(EXTRA_CPPFLAGS) $(CPPFLAGS)


all: $(TARGETS)

sbasis: one-D s-bez rat-bez arc-bez unit-test-sbasis


lib2geom.a: $(LIBOBJS)
	ar cr lib2geom.a $(LIBOBJS)
	ranlib lib2geom.a


# TODO: There are problems if the unit test code is in the main .o file
#       since that will be in the library.  So these targets may not work.
#       Best to pull out the test code.

path: path.cpp lib2geom.a
	$(CXX) $(CXXFLAGS) -o $@ $^ -DUNIT_TEST $(extra_cppflags)

path-to-svgd: path-to-svgd.cpp lib2geom.a
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ -DUNIT_TEST $(extra_cppflags)


# Targets;

$(TARGETS): %: %.o interactive-bits.o path-cairo.o lib2geom.a 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -I. $^ $(LDFLAGS) $(EXTRA_LDFLAGS)


# test stuff.

2geom-tolua.cpp: 2geom-tolua.pkg

%.cpp: %.pkg
	tolua++5.1 -o $@ $^ 

lib2geomlua: 2geom-tolua.pkg 2geom-tolua.cpp
	tolua++5.1 -n $@ -o 2geom-tolua.cpp  2geom-tolua.pkg


conjugate_gradient.o: conjugate_gradient.cpp conjugate_gradient.h

test_cg: conjugate_gradient.o test_cg.cpp
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ -lgsl -lblas

heat_diffusion: heat_diffusion.cpp
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 


.PHONY: all clean clobber
clean:
	rm -f $(ALLOBJS) *~

clobber: clean
	rm -f $(TARGETS)

