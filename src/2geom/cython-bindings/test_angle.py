import cy2geom

a = cy2geom.Angle(1.72)
print(a.radians())
print(a.degrees())

b = cy2geom.Angle.from_radians(2)

print(b.radians())

c = cy2geom.Angle.from_degrees_clock(360)

print(c.radians())

print ((a+b).radians0())
print ((b-a).radians0())

print (a==a)
print (a!=a)
print (a==b)
