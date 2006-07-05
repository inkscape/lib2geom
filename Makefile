extra_cppflags := $(shell pkg-config --cflags --libs gtk+-2.0) -lgsl -lblas

all: path path-to-svgd toy-cairo conic sbasis

CXX=g++
CXXFLAGS = -g -O0

path.o: path.cpp nearestpoint.cpp path.h
arc-length.o: arc-length.cpp arc-length.h path.h
poly.o: poly.cpp poly.h
path-cairo.o: path-cairo.cpp path-cairo.h
	$(CXX) $(CXXFLAGS) -c $^ $(shell pkg-config --cflags cairo) 
interactive-bits.o: interactive-bits.cpp interactive-bits.h
	$(CXX) $(CXXFLAGS) -c $^ $(shell pkg-config --cflags cairo) 

path-to-svgd.o: path-to-svgd.cpp path-to-svgd.h path.h

path: path.cpp rect.o point-fns.o types.o path.h poly.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -DUNIT_TEST $(extra_cppflags) 


path-to-svgd: path.o path-to-svgd.cpp read-svgd.o path-to-polyline.o types.o rect.o point-fns.o types.o path.h poly.o
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ -DUNIT_TEST $(extra_cppflags) 

toy: toy.cpp path.o path-to-svgd.o read-svgd.o path-to-polyline.o types.o rect.o geom.o read-svgd.o path-find-points-of-interest.o point-fns.o types.o rotate-fns.o matrix.o arc-length.o path-intersect.o poly.o
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags)

toy-cairo: toy-cairo.cpp path.o path-to-svgd.o read-svgd.o path-to-polyline.o types.o rect.o geom.o read-svgd.o path-find-points-of-interest.o point-fns.o types.o rotate-fns.o matrix.o arc-length.o path-intersect.o centroid.o poly.o matrix-rotate-ops.o matrix-translate-ops.o path-cairo.o path-metric.o interactive-bits.o
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 

tensor-reparam: tensor-reparam.cpp path.o path-to-svgd.o read-svgd.o path-to-polyline.o types.o rect.o geom.o read-svgd.o path-find-points-of-interest.o point-fns.o types.o rotate-fns.o matrix.o arc-length.o path-intersect.o centroid.o poly.o matrix-rotate-ops.o matrix-translate-ops.o path-cairo.o path-metric.o
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 

conic: conic.cpp path.o path-to-svgd.cpp read-svgd.o path-to-polyline.o point-fns.o types.o rect.o geom.o path.h poly.o
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 

conic-2: conic-2.cpp path.o path-to-svgd.cpp read-svgd.o path-to-polyline.o point-fns.o types.o rect.o geom.o path.h poly.o
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 

b-spline: b-spline.cpp path.o path-to-svgd.cpp read-svgd.o path-to-polyline.o point-fns.o types.o rect.o geom.o path.h poly.o
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 

ode-toy-cairo: ode-toy-cairo.cpp path.o path-to-svgd.o read-svgd.o path-to-polyline.o types.o rect.o geom.o read-svgd.o path-find-points-of-interest.o point-fns.o types.o rotate-fns.o matrix.o arc-length.o path-intersect.o centroid.o poly.o
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 

sbasis: one-D s-bez rat-bez arc-bez

one-D: one-D.cpp s-basis.h s-basis.cpp
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 

s-bez: s-bez.cpp s-basis.h s-basis.cpp multidim-sbasis.h interactive-bits.o
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 

rat-bez: rat-bez.cpp s-basis.h s-basis.cpp multidim-sbasis.h interactive-bits.o
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 

arc-bez: arc-bez.cpp s-basis.h s-basis.cpp multidim-sbasis.h interactive-bits.o
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 

poly-test: poly-test.cpp poly.cpp poly.h
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 

2geom-tolua.cpp: 2geom-tolua.pkg


%.cpp: %.pkg
	tolua++5.1 -o $@ $^ 

lib2geomlua: 2geom-tolua.pkg 2geom-tolua.cpp
	tolua++5.1 -n $@ -o 2geom-tolua.cpp  2geom-tolua.pkg

clean:
	rm -f *.o path path-to-svgd toy conic toy-cairo ode-toy-cairo *~

.cpp.o:
	$(CXX)  $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean mkdirs
