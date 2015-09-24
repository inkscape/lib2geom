#!/usr/bin/python

import sys
sys.path.append(os.path.join(os.path.dirname(__file__), "..", "py2geom"))

from  py2geom import *
import py2geom

P = Point(1,2)
Q = Point(3,5)
print P, Q, P+Q

print L2(P)
print cross(P,Q)
#print dir(py2geom)

import numpy

ply = numpy.poly1d([1,0,2])

print ply(0.5)
t = numpy.poly1d([1,0])
q,r = ply / t
print q,r

print ply 
print q*t + r
print ply

def poly_to_sbasis(p):
    sb = SBasis()
    s = numpy.poly1d([-1, 1, 0])
    while True:
        q,r = p / s
        #print "r:", r
        print "r:", repr(r)
        x = Linear(r[0],r[1]+r[0])
        sb.append(x)
        p = q
        print "q:", repr(p), len(list(p))
        if len(list(p)) <= 1 and p[0] == 0:
            return sb

def sbasis_to_poly(sb):
    p = numpy.poly1d([0])
    s = numpy.poly1d([-1, 1, 0])
    sp = numpy.poly1d([1])
    for sbt in sb:
        p += sp*(sbt[0]*(numpy.poly1d([-1,1])) + sbt[1]*(numpy.poly1d([1,0])))
        sp *= s
    return p

trial = numpy.poly1d([1,0,2])
sb = poly_to_sbasis(trial)
print repr(trial),"p2sb:", sb
print "and back again:", repr(sbasis_to_poly(sb))
print repr(sbasis_to_poly(derivative(sb))), repr(trial.deriv())

print "unit tests:"
x = Linear(0,1)
sb = SBasis()
sb.append(x)
print sb
sb = sb*sb
print sb
print sb[0]

print "terms"
for i in range(6):
    sb = SBasis()
    for j in range(3):
        sb.append(Linear(i==2*j,i==2*j+1))
    print sb
    
    print sbasis_to_poly(sb)
