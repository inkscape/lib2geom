#!/usr/bin/python

import py2geom
import toyframework
import random,gtk
import numpy
from py2geom_glue import *

class Box3d(toyframework.Toy):
    def __init__(self):
        toyframework.Toy.__init__(self)
        self.tmat = numpy.zeros([3,4])
        # plane origin
        self.origin_handle = toyframework.PointHandle(180,65)
        self.handles.append(self.origin_handle)
        self.vanishing_points_handles = toyframework.PointSetHandle()
        path_a_name="ptitle.svgd"
        import sys
        if len(sys.argv) > 1:
            path_a_name = sys.argv[1]
        self.paths_a = py2geom.read_svgd(path_a_name)
    

        # Finite images of the three vanishing points and the origin
        self.handles.append(self.vanishing_points_handles)
        self.vanishing_points_handles.append(550,350)
        self.vanishing_points_handles.append(150,300)
        self.vanishing_points_handles.append(380,40)
        self.vanishing_points_handles.append(340,450)
    def draw(self, cr, pos, save):
        orig = self.origin_handle.pos;
	cr.set_source_rgba (0., 0.125, 0, 1)

        # create the transformation matrix for the map  P^3 --> P^2 that has the following effect:
        #      (1 : 0 : 0 : 0) --> vanishing point in x direction (= handle #0)
        #      (0 : 1 : 0 : 0) --> vanishing point in y direction (= handle #1)
        #      (0 : 0 : 1 : 0) --> vanishing point in z direction (= handle #2)
        #      (0 : 0 : 0 : 1) --> origin (= handle #3)
        
        tmat = numpy.zeros([3,4])
        for j in range(4):
            tmat[0][j] = self.vanishing_points_handles.pts[j][0]
            tmat[1][j] = self.vanishing_points_handles.pts[j][1]
            tmat[2][j] = 1

        self.notify = "Projection matrix:\n"
        for i in range(3):
            for j in range(4):
                self.notify += str(tmat[i][j]) + " "
            self.notify += '\n'

        for p in self.paths_a:
            B = py2geom.make_cuts_independant(p.toPwSb())
            preimage = [None]*4
                
            preimage[0] =  (B[0] - orig[0]) / 100;
            preimage[1] = -(B[1] - orig[1]) / 100;
            Piecewise<SBasis> res[3];
            for j in range(3):
                res[j] = (preimage[0] * tmat[j][0] +
                          preimage[1] * tmat[j][1] +
                          + tmat[j][3])
            
            result = D2PiecewiseSBasis(divide(res[0],res[2], 2), 
                                       divide(res[1],res[2], 2))
            
            toyframework.cairo_d2_pw(cr, result)
            cr.set_source_rgba (0., 0.125, 0, 1)
            cr.stroke()
        
        toyframework.Toy.draw(self, cr, pos, save)
    
t = Box3d()
import sys

toyframework.init(sys.argv, t, 500, 500)


