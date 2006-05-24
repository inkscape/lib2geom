extra_cppflags := $(shell pkg-config --cflags --libs gtk+-2.0) -lgsl -lblas

all: path path-to-svgd toy-cairo conic

CXX=g++
CXXFLAGS = -g

path.o: path.cpp nearestpoint.cpp path.h
arc-length.o: arc-length.cpp arc-length.h path.h

path-to-svgd.o: path-to-svgd.cpp path-to-svgd.h path.h

path: path.cpp rect.o point-fns.o types.o path.h
	$(CXX) -o $@ $^ -DUNIT_TEST


path-to-svgd: path.o path-to-svgd.cpp read-svgd.o path-to-polyline.o types.o rect.o point-fns.o types.o path.h
	$(CXX) -o $@ -I . $^ -DUNIT_TEST

toy: toy.cpp path.o path-to-svgd.o read-svgd.o path-to-polyline.o types.o rect.o geom.o read-svgd.o path-find-points-of-interest.o point-fns.o types.o rotate-fns.o matrix.o arc-length.o path-intersect.o
	$(CXX) -o $@ -I . $^ $(extra_cppflags) 

toy-cairo: toy-cairo.cpp path.o path-to-svgd.o read-svgd.o path-to-polyline.o types.o rect.o geom.o read-svgd.o path-find-points-of-interest.o point-fns.o types.o rotate-fns.o matrix.o arc-length.o path-intersect.o
	$(CXX) -o $@ -I . $^ $(extra_cppflags) 

conic: conic.cpp path.o path-to-svgd.cpp read-svgd.o path-to-polyline.o point-fns.o types.o rect.o geom.o path.h
	$(CXX) -o $@ -I . $^ $(extra_cppflags) 

clean:
	rm -f *.o path path-to-svgd toy conic toy-cairo *~

.cpp.o:
	$(CXX)  $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean mkdirs
