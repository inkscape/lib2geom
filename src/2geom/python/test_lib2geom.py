#!/usr/bin/python

import py2geom as g

a = g.Point(1,2)
b = g.Point(31,2)
print a, b

point_fns_1 = ["L1", "L2", "L2sq", "LInfty", "is_zero", "is_unit_vector",
             "atan2", "rot90", 
             "unit_vector", "abs"]
point_fns_2 = ["dot", "angle_between", "distance", "distanceSq", "cross"]

for i in point_fns_1:
    print "%s:" % i, g.__dict__[i](a)
for i in point_fns_2:
    print "%s:" % i, g.__dict__[i](a,b)
print "a == b", a == b
print "Lerp:", g.lerp(0.3, a,b)

bo = g.BezOrd(2,3)
print bo
print bo.point_at(0.3)

print bo.reverse()

sn = g.sin(g.BezOrd(0.0,8.0),5)
print sn
print g.inverse(sn,10)
print list(sn)

r_sn = g.roots(sn)
print len(r_sn)
print list(r_sn)

bo = g.BezOrd(-1,1)
sb = g.SBasis()
print sb
print list(g.roots(sb))
sb.append(bo)
print list(g.roots(sb))
