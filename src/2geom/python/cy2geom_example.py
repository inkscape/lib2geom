#!/usr/bin/python

import cy2geom
from cy2geom import *

a = Point(1,2)
b = Point(31,2)
print a, b
print Point.dot(a,b)
print Point.unit_vector(a)
