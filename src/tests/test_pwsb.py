#!/usr/bin/python

from  py2geom import *
import py2geom
import numpy
import random

def poly_to_sbasis(p):
    sb = SBasis()
    s = numpy.poly1d([-1, 1, 0])
    while True:
        q,r = p / s
        x = Linear(r[0],r[1]+r[0])
        sb.append(x)
        p = q
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

random.seed(1)
trial = numpy.poly1d([random.randrange(0,10) for x in range(6)])

sb = poly_to_sbasis(trial)

pwsb = PiecewiseSBasis()
pwsb.push_seg(sb)
print pwsb.size()
print pwsb(0)
