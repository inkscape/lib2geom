extra_cppflags := $(shell pkg-config --cflags --libs gtk+-2.0) -lgsl -lblas

all: path path-to-svgd toy-cairo conic

CXX=g++-4.1
CXXFLAGS = -g

path.o: path.cpp nearestpoint.cpp path.h
arc-length.o: arc-length.cpp arc-length.h path.h
poly.o: poly.cpp poly.h

path-to-svgd.o: path-to-svgd.cpp path-to-svgd.h path.h

path: path.cpp rect.o point-fns.o types.o path.h poly.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -DUNIT_TEST $(extra_cppflags) 


path-to-svgd: path.o path-to-svgd.cpp read-svgd.o path-to-polyline.o types.o rect.o point-fns.o types.o path.h poly.o
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ -DUNIT_TEST $(extra_cppflags) 

toy: toy.cpp path.o path-to-svgd.o read-svgd.o path-to-polyline.o types.o rect.o geom.o read-svgd.o path-find-points-of-interest.o point-fns.o types.o rotate-fns.o matrix.o arc-length.o path-intersect.o poly.o
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags)

toy-cairo: toy-cairo.cpp path.o path-to-svgd.o read-svgd.o path-to-polyline.o types.o rect.o geom.o read-svgd.o path-find-points-of-interest.o point-fns.o types.o rotate-fns.o matrix.o arc-length.o path-intersect.o centroid.o poly.o matrix-rotate-ops.o matrix-translate-ops.o 
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 

conic: conic.cpp path.o path-to-svgd.cpp read-svgd.o path-to-polyline.o point-fns.o types.o rect.o geom.o path.h poly.o
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 

ode-toy-cairo: ode-toy-cairo.cpp path.o path-to-svgd.o read-svgd.o path-to-polyline.o types.o rect.o geom.o read-svgd.o path-find-points-of-interest.o point-fns.o types.o rotate-fns.o matrix.o arc-length.o path-intersect.o centroid.o poly.o
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 

poly-test: poly-test.cpp poly.cpp poly.h
	$(CXX) $(CXXFLAGS) -o $@ -I . $^ $(extra_cppflags) 

clean:
	rm -f *.o path path-to-svgd toy conic toy-cairo ode-toy-cairo *~

.cpp.o:
	$(CXX)  $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean mkdirs
