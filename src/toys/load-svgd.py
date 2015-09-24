#!/usr/bin/python

import py2geom
import toyframework
import random,gtk
from py2geom_glue import *

def cairo_region(cr, r):
    cr.save()
    cr.set_source_rgba(0, 0, 0, 1);
    if not r.isFill():
        pass#cr.set_dash([]1, 0)
    cairo_path(cr, r)
    cr.fill()
    cr.restore()

def cairo_regions(cr, p):
    for j in p:
        cairo_region(cr, j)

def cairo_shape(cr, s):
    cairo_regions(cr, s.getContent())



def cleanup(ps):
    pw = py2geom.paths_to_pw(ps)
    centre, area = py2geom.centroid(pw)
    if(area > 1):
        return py2geom.sanitize(ps) * py2geom.Translate(-centre)
    else:
        return py2geom.sanitize(ps)

class LoadSVGD(toyframework.Toy):
    def __init__(self):
        toyframework.Toy.__init__(self)
        self.bs = []
        self.offset_handle = toyframework.PointHandle(0,0)
        self.handles.append(self.offset_handle)
    def draw(self, cr, pos, save):
        t = py2geom.Translate(*self.offset_handle.pos)
        #self.paths_b[0] * t
        m = py2geom.Matrix()
        m.setIdentity()
        bst = self.bs * (m * t)
        #bt = Region(b * t, b.isFill())
        
        cr.set_line_width(1)
        
        cairo_shape(cr, bst)
        
        toyframework.Toy.draw(self, cr, pos, save)
        
    def first_time(self, argv):
        path_b_name="star.svgd"
        if len(argv) > 1:
            path_b_name = argv[1]
        self.paths_b = py2geom.read_svgd(path_b_name)
        
	bounds = py2geom.bounds_exact(self.paths_b)
        self.offset_handle.pos = bounds.midpoint() - bounds.corner(0)

        self.bs = cleanup(self.paths_b)
t = LoadSVGD()
import sys

toyframework.init(sys.argv, t, 500, 500)

