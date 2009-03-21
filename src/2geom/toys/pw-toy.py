#!/usr/bin/python

import py2geom
import toyframework
import random,gtk
from py2geom_glue import *

def cairo_pw(cr, p):
    for i in range(p.size()):
        a,b = p.cuts[i], p.cuts[i+1]
        bez = sbasis_to_bezier(p[i], 0)
        cr.move_to(a, bez[0])
        cr.curve_to(lerp(a, b, 1./3), bez[1],
                    lerp(a,b, 2./3), bez[2],
                    b, bez[3])

def cairo_horiz(cr, y, ps):
    for p in ps:
        cr.move_to(p, y);
        cr.rel_line_to(0, 10)

def cairo_vert(cr, x, ps):
    for p in ps:
        cr.move_to(x, p)
        cr.rel_line_to(10, 0)

def l2s(l):
    sb = py2geom.SBasis(l)
    return sb

def constant(l):
    pws = py2geom.PiecewiseSBasis()
    
    pws.push_cut(0)
    pws.push_seg(l2s(py2geom.Linear(l,l)) )
    pws.push_cut(1000)
    return pws

X = l2s(py2geom.Linear(0, 1))
OmX = l2s(py2geom.Linear(1, 0))
def bezier_to_sbasis(handles, order):
    if(order == 0):
        return l2s(py2geom.Linear(handles[0]))
    elif(order == 1):
        return l2s(py2geom.Linear(handles[0], handles[1]))
    else:
        return (py2geom.multiply(OmX, bezier_to_sbasis(handles[:-1], order-1)) +
                py2geom.multiply(X, bezier_to_sbasis(handles[1:], order-1)))


def bez_to_sbasis(handles, order):
    return bezier_to_sbasis([h[1] for h in handles], order)

class PWSBHandle(toyframework.Handle):
    def __init__(self, cs, segs):
        self.handles_per_curve = cs*segs
        self.curve_size = cs
        self.segs = segs
        self.pts = []
    def append(self, x, y):
        self.pts.append(py2geom.Point(x,y))
    def value(self, y_0=0):
        pws = py2geom.PiecewiseSBasis()
        for i in range(0, self.handles_per_curve, self.curve_size):
	    pws.push_cut(self.pts[i][0]);
	    for j in range(i, i + self.curve_size):
		self.pts[j] = py2geom.Point(self.pts[j][0], self.pts[j][1] - y_0)
            hnd = self.pts[i:i+self.curve_size]
            pws.push_seg( bez_to_sbasis(hnd, self.curve_size-1));
	    for j in range(i, i + self.curve_size):
		self.pts[j] = py2geom.Point(self.pts[j][0], self.pts[j][1] + y_0);

	pws.push_cut(self.pts[self.handles_per_curve - 1][0]);
	assert(pws.invariants());
	return pws

    def draw(self, cr, annotes):
        for p in self.pts:
            toyframework.draw_circ(cr, p)

    def hit(self, mouse):
        for i,p in enumerate(self.pts):
            if(py2geom.distance(py2geom.Point(*mouse), p) < 5):
                return i
        return None

    def move_to(self, hit, om, m):
        om = py2geom.Point(*om)
        m = py2geom.Point(*m)
        if hit != None:
            i,hand = hit
            self.pts[hand] = m
            for i in range(self.curve_size, self.handles_per_curve, self.curve_size):
                self.pts[i-1] = py2geom.Point(self.pts[i][0],self.pts[i-1][1])

            for i in range(0, self.handles_per_curve, self.curve_size):
                for j in range(1, (self.curve_size-1)):
                    t = float(j)/(self.curve_size-1)
                    x = lerp(self.pts[i][0], self.pts[i+self.curve_size-1][0],t)
                    self.pts[i+j] = py2geom.Point(x, self.pts[i+j][1])



class PwToy(toyframework.Toy):
    def __init__(self):
        toyframework.Toy.__init__(self)
        self.segs = 2
        self.handles_per_curve = 4 * self.segs
        self.curves = 2
        self.pwsbh = []
        self.interval_test = []
        for a in range(self.curves):
            self.pwsbh.append(PWSBHandle(4, self.curves))
            self.handles.append(self.pwsbh[a])
            for i in range(self.handles_per_curve):
                t = 150 + 300*i/(4*self.segs)
                self.pwsbh[a].append(t, random.uniform(0,1) * 150 + 150 - 50 * a)
        self.interval_test.append(toyframework.PointHandle(150, 400))
        self.interval_test.append(toyframework.PointHandle(300, 400))
        self.handles.append(self.interval_test[0])
        self.handles.append(self.interval_test[1])
        self.func = "pws[0] + pws[1]"
    def gtk_ready(self):
        import gtk
        self.sb_entry = gtk.Entry()
        self.sb_entry.connect("changed", self.sb_changed)
        toyframework.get_vbox().add(self.sb_entry)
        toyframework.get_vbox().show_all()
        self.sb_entry.set_text(self.func)
    def sb_changed(self, sb):
        self.func = sb.get_text()
        self.redraw()
    def draw(self, cr, pos, save):
        cr.set_source_rgba (0., 0., 0., 1)
        cr.set_line_width (1)
        
        pws = [self.pwsbh[i].value() for i in range(self.curves)]
        for p in pws:
            cairo_pw(cr, p)
        cr.stroke()
        
        d = locals().copy()
        for i in dir(py2geom):
            d[i] = py2geom.__dict__[i]
        d['l2s'] = l2s
        d['constant'] = constant
        pw_out = eval(self.func, d)

        bs = py2geom.bounds_local(pw_out, py2geom.OptInterval(
                          py2geom.Interval(self.interval_test[0].pos[0], 
                                           self.interval_test[1].pos[0])));
        if not bs.isEmpty():
            bs = bs.toInterval()
            for ph in self.interval_test:
                ph.pos= py2geom.Point(ph.pos[0], bs.middle())
            cr.save()
            cr.set_source_rgba (.0, 0.25, 0.5, 1.)
            cr.rectangle(self.interval_test[0].pos[0], bs.min(),
                         self.interval_test[1].pos[0]-self.interval_test[0].pos[0], bs.extent())
            cr.stroke()
        bs = py2geom.bounds_exact(pw_out);
        cr.set_source_rgba (0.25, 0.25, .5, 1.);
        if not bs.isEmpty():
            bs = bs.toInterval()
            cairo_horiz(cr, bs.middle(), pw_out.cuts);
            cr.stroke()
        cr.restore()

        cr.set_source_rgba (0., 0., .5, 1.);
        cairo_pw(cr, pw_out)
        cr.stroke()


        self.notify = str(bs)
        toyframework.Toy.draw(self, cr, pos, save)
        
t = PwToy()
import sys

toyframework.init(sys.argv, t, 500, 500)
