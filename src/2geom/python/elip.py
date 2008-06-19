#!/usr/bin/python

import gtk,gtk.gdk,math,pango  #Numeric,

templayout = None

def draw_spot(w, h):
    x,y = h.x, h.y
    g = gtk.gdk.GC(w)
    w.draw_line(g, int(x), int(y), int(x), int(y))

def draw_handle(w, h, name = ""):
    x,y = h.x, h.y
    g = gtk.gdk.GC(w)
    w.draw_line(g, int(x-3), int(y), int(x+3), int(y))
    w.draw_line(g, int(x), int(y-3), int(x), int(y+3))
    templayout.set_text(name)
    w.draw_layout(g, x, y, templayout)

def draw_ray(w, h, d):
    x,y = h.x, h.y
    g = gtk.gdk.GC(w)
    w.draw_line(g, int(h.x), int(h.y), int(3*d.x-2*h.x), int(3*d.y-2*h.y))

def intersect(n0, d0, n1, d1):
    denominator = n0.x*n1.y - n0.y*n1.x
    X = n1.y * d0 - n0.y * d1
    if denominator == 0:
        return None;

    Y = n0.x * d1 - n1.x * d0

    return handle(X / denominator, Y / denominator)

def seg(a0, a1, b0, b1):
    n0 = handle(a1.y - a0.y, -a1.x + a0.x)
    d0 = n0.x*a0.x + n0.y *a0.y
    n1 = handle(b1.y - b0.y, -b1.x + b0.x)
    d1 = n1.x*b0.x + n1.y *b0.y

    return intersect(n0, d0, n1, d1)

def draw_elip(w, h):
    g = gtk.gdk.GC(w)
    w.draw_line(g, h[0].x, h[0].y, h[1].x, h[1].y)
    w.draw_line(g, h[3].x, h[3].y, h[4].x, h[4].y)
    w.draw_line(g, h[3].x, h[3].y, h[2].x, h[2].y)
    w.draw_line(g, h[2].x, h[2].y, h[1].x, h[1].y)

    c = seg(h[0], h[1], h[3], h[4])
    draw_handle(w, c)
    
    if 0:
        for i in range(6):
            w.draw_line(g, h[i].x, h[i].y, h[(i+1)%6].x, h[(i+1)%6].y)
        
    
    cx,cy = c.x, c.y

    ox, oy = None, None
    for i in range(0, 101):
        t = i/100.0
        
        
        nx = (1-t)*h[0].x + t*h[3].x
        ny = (1-t)*h[0].y + t*h[3].y
        #w.draw_line(g, 2*cx-nx, 2*cy-ny, nx, ny)
        c1 = seg(handle(2*cx-nx, 2*cy-ny), handle(nx, ny), h[0], h[2])
        #draw_handle(w, c1)
        c2 = seg(handle(2*cx-nx, 2*cy-ny), handle(nx, ny), h[4], h[2])
        #draw_handle(w, c2)
        #draw_ray(w, h[3], c1)
        #draw_ray(w, h[1], c2)
        six = seg(c1, h[3], c2, h[1])
        #draw_spot(w, six)
        if ox:
            w.draw_line(g, ox, oy, six.x, six.y)
        ox, oy = six.x, six.y
    return
    
    r = math.hypot(h[0].x - cx, h[0].y - cy)
    s = math.atan2(h[0].y - h[3].y, h[0].x - h[3].x)
    e = math.atan2(h[1].y - h[4].y, h[1].x - h[4].x)
    for i in range(0, 101):
        t = (e-s)*i/100.0 + s
        nx, ny = r*math.cos(t)+cx, r*math.sin(t)+cy
        sx, sy = r*math.cos(t+math.pi)+cx, r*math.sin(t+math.pi)+cy
        w.draw_line(g, sx, sy, nx, ny)
    

class handle:
    def __init__(self, x, y):
        self.x = x
        self.y = y
    def __repr__(self):
        return "handle(%f, %f)" % (self.x, self.y)

handles = [handle(145.000000, 50.000000), handle(43.000000, 69.000000), handle(26.000000, 135.000000), handle(48.000000, 189.000000), handle(248.000000, 188.000000)]

selected_handle = None

def display(da, ev):
    g = gtk.gdk.GC(da.window)
    i = 0

    
    
    for h in handles:
        draw_handle(da.window, h, str(i))
        i += 1
    draw_elip(da.window, handles)

def mouse_event(w, e):
    global selected_handle
    if e.button == 1:
        for h in handles:
            if math.hypot(e.x-h.x, e.y - h.y) < 5:
                selected_handle = (h, (e.x-h.x, e.y-h.y))

def mouse_release_event(w, e):
    global selected_handle
    selected_handle = None

def mouse_motion_event(w, e):
    global selected_handle
    if selected_handle:
        h, (ox, oy) = selected_handle
        if(e.state & gtk.gdk.BUTTON1_MASK):
            h.x = e.x - ox
            h.y = e.y - oy
            w.queue_draw()


win = gtk.Window()
win.set_default_size(400,400)
vb = gtk.VBox(False)

da = gtk.DrawingArea()
templayout = da.create_pango_layout("")
da.connect("expose_event", display)
da.add_events(gtk.gdk.BUTTON_PRESS_MASK | gtk.gdk.BUTTON_RELEASE_MASK | gtk.gdk.KEY_PRESS_MASK | gtk.gdk.POINTER_MOTION_MASK)
da.connect("button-press-event", mouse_event)
da.connect("button-release-event", mouse_release_event)
da.connect("motion-notify-event", mouse_motion_event)
#da.connect("key_press_event", key_event)
win.add(vb)
vb.pack_start(da)
win.connect("destroy", gtk.main_quit)


win.show_all()

gtk.main()

print handles
