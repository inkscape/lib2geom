import math

def q(x, a, b, c):
    sa = math.sqrt(a)
    y = math.sqrt(a*x*x+b*x+c)
    dp = 2*a*x + b
    r = dp*y/(4*a)
    t = abs(dp + 2*y*sa)
    s = (4*a*c-b*b)/(a*sa*8)
    return r+s*math.log(t)

a = 1160390
b = -922658
c = 249477
print q(1, a,b,c) - q(0,a,b,c)

