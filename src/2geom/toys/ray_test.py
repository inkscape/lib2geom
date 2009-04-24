# test of 2geom ray bindings

import py2geom as g

# find one point along a ray
a = g.Point(0,0)
b = g.Point(2,2)

r = g.Ray(a,b)
from math import sqrt
print r.pointAt(sqrt(2))

# measure the angle between two rays
c = g.Point(2,-2)
r2 = g.Ray(a,c)
from math import degrees
# FIXME: the third argument (clockwise) ought to be optional, but has to be supplied
print degrees(g.angle_between(r, r2, True))
print degrees(g.angle_between(r, r2))

