import cy2geom

p = cy2geom.Point(3, 4)

print (p.length())

q = cy2geom.Point.polar(0.786, 1.414)

print (q.length())
print (q)

print q.ccw()
print q.ccw().cw()
print -q
print q*100
print (p+q)/2
print p > q, p < q, p == q, p != q
print q.isNormalized(0.1)
print cy2geom.Point.L2sq(q)
print cy2geom.Point.cross(q, p)
print cy2geom.Point.distance(p, q)
print cy2geom.Point.atan2(p)
print cy2geom.Point.unit_vector(q)
