#!/usr/bin/python

import sys
sys.path.append(os.path.join(os.path.dirname(__file__), "..", "py2geom"))

from  py2geom import *
import py2geom
import numpy
import random
from py2geom_glue import *

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
pwsb.push_cut(0)
pwsb.push_cut(1)
print pwsb.size()
print "invariants:", pwsb.invariants()
print pwsb(0)

def l2s(l):
    sb = py2geom.SBasis()
    sb.append(l)
    return sb

X = l2s(py2geom.Linear(0, 1))
OmX = l2s(py2geom.Linear(1, 0))
def bezier_to_sbasis(handles, order):
    print "b2s:", handles, order
    if(order == 0):
        return l2s(py2geom.Linear(handles[0]))
    elif(order == 1):
        return l2s(py2geom.Linear(handles[0], handles[1]))
    else:
        return (py2geom.multiply(OmX, bezier_to_sbasis(handles[:-1], order-1)) +
                py2geom.multiply(X, bezier_to_sbasis(handles[1:], order-1)))


for bz in [[0,1,0], [0,1,2,3]]:
    sb = bezier_to_sbasis(bz, len(bz)-1)
    print bz
    print sb
    print sbasis_to_bezier(sb,0)
