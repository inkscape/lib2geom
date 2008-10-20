#!/usr/bin/python

import py2geom
import toyframework
import random,gtk
from py2geom_glue import *

class PathToy(toyframework.Toy):
    def __init__(self):
        toyframework.Toy.__init__(self)
        self.handles.append(toyframework.PointHandle(200, 200))
        self.path_b_name="star.svgd"
        self.pv = py2geom.read_svgd(self.path_b_name);
        centr = py2geom.Point()
        for p in self.pv:
            c,area = py2geom.centroid(p.toPwSb())
            centr += c
        self.pv = self.pv*py2geom.Matrix(py2geom.Translate(-centr))
    def draw(self, cr, pos, save):
        cr.set_source_rgba (0., 0., 0., 1)
        cr.set_line_width (1)
        
        
        B = (self.pv[0]*py2geom.Matrix(py2geom.Translate(*self.handles[0].pos))).toPwSb();
        n = py2geom.rot90(py2geom.unit_vector(py2geom.derivative(B), 0.01, 3));
        al = py2geom.arcLengthSb(B, 0.1);
        offset = 10.
        
        offset_curve = B+py2geom.sin(al*0.1, 0.01, 2)*n*10.
        offset_path = py2geom.path_from_piecewise(offset_curve, 0.1, True)
        
        py2geom.cairo_path(cr, offset_path)
        cr.stroke()
        
        self.notify = ''
        toyframework.Toy.draw(self, cr, pos, save)
        
t = PathToy()
import sys

toyframework.init(sys.argv, t, 500, 500)
