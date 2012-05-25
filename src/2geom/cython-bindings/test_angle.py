import cy2geom

a = cy2geom.Angle(1.72)
print(a.radians())
print(a.degrees())
b = cy2geom.Angle.from_radians(2)
print a, b
print(b.radians())
