#!/usr/bin/python

import py2geom as g

a = g.Point(1,2)
b = g.Point(31,2)
print a, b

point_fns_1 = ["L1", "L2", "L2sq", "LInfty", "is_zero",
             "atan2", "rot90",
             "unit_vector", "abs"]
point_fns_2 = ["dot", "angle_between", "distance", "distanceSq", "cross"]
point_fns_3 = [ "is_unit_vector", ]

for i in point_fns_1:
    print "%s:" % i, g.__dict__[i](a)
for i in point_fns_2:
    print "%s:" % i, g.__dict__[i](a,b)
for i in point_fns_3:
    print "%s:" % i, g.__dict__[i](a,0)
print "a == b", a == b
print "Lerp:", g.lerp(0.3, a,b)

"""
# old code:
bo = g.BezOrd(2,3)
print bo
print bo.point_at(0.3)
"""
bo = g.Bezier(2,3)
print bo
print bo.valueAt(0.3)

"""
# old code:
print bo.reverse()
"""
print g.reverse(bo.toSBasis())

"""
# old code:
sn = g.sin(g.BezOrd(0.0,8.0),5)
print sn
print g.inverse(sn,10)
print list(sn)
"""
bezsb = g.Bezier(0.0,8.0).toSBasis()
sn = g.sin(g.PiecewiseSBasis(bezsb), 5, 0)
print sn
print g.inverse(bezsb,10)
print list(sn)

r_sn = g.roots(sn)
print len(r_sn)
print list(r_sn)

"""
# old code:
bo = g.BezOrd(-1,1)
sb = g.SBasis()
print sb
print list(g.roots(sb))
sb.append(bo)
print list(g.roots(sb))
"""
bo = g.Bezier(-1,1)
sb = g.SBasis(0)
print sb
print list(g.roots(sb))
pws = g.PiecewiseSBasis(sb)
pws.concat( g.PiecewiseSBasis( bo.toSBasis() ) ) # in-place replace, returns None
print list(g.roots(pws))
