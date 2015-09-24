#!/usr/bin/python

import gtk,math
import pangocairo,cairo
import gobject

# def draw_text(cr, pos, txt, bottom = False):
# def draw_number(cr, pos, num):

def draw_circ(cr, (x, y)):
    cr.new_sub_path()
    cr.arc(x, y, 3, 0, math.pi*2)
    cr.stroke()
def draw_cross(cr, (x, y)):
    cr.move_to(x-3, y-3)
    cr.line_to(x+3, y+3)
    cr.move_to(x-3, y+3)
    cr.line_to(x+3, y-3)
    cr.stroke()

class Handle:
    def __init__(self):
        pass
    def draw(self, cr, annotes):
        pass
    def hit(self, (x, y)):
        return None
    def move_to(self, hit, om, m):
        pass
    def scroll(self, pos, dir):
        pass

class PointHandle(Handle):
    def __init__(self, x, y, name=""):
        Handle.__init__(self)
        self.pos = (x,y)
        self.name = name
    def draw(self, cr, annotes):
        draw_circ(cr, self.pos)
        if annotes:
            draw_text(cr, self.pos, str(self.name))
    def hit(self, mouse):
        if math.hypot(mouse[0] - self.pos[0], mouse[1] - self.pos[1]) < 5:
            return 0
        return None
    def move_to(self, hit, om, m):
        self.pos = m

class PointSetHandle(Handle):
    def __init__(self, pts=None, name=""):
        Handle.__init__(self)
        self.pts = pts or []
        self.name = name
    def draw(self, cr, annotes):
        for p in self.pts:
            draw_circ(cr, p)
            if annotes:
                draw_text(cr, p, str(self.name))
    def hit(self, mouse):
        for i,p in enumerate(self.pts):
            if math.hypot(mouse[0] - p[0], mouse[1] - p[1]) < 5:
                return i
        return None
    def move_to(self, hit, om, m):
        self.pts[hit[1]] = m
    def append(self, x,y):
        self.pts.append((x,y))

class Toy:
    def __init__(self):
        self.handles = []
        self.mouse_down = False
        self.old_mouse = None
        self.selected = None
        self.notify = "notify"
        self.origin = [0,0]
        self.interactive_level = 0
        self.transform = None

    def draw(self, cr, (width, height), save):
        bounds = self.should_draw_bounds()
        self.transform = cr.get_matrix()
        self.transform.invert()
        if bounds == 1:
            cr.set_source_rgba(0., 0., 0, 0.8)
            cr.set_line_width (0.5)
            for i in [1,3]:
                cr.move_to(0, i*width/4)
                cr.line_to(width, i*width/4)
                cr.move_to(i*width/4, 0)
                cr.line_to(i*width/4, height)
        elif bounds == 2:
            cr.set_source_rgba (0., 0., 0, 0.8)
            cr.set_line_width (0.5)
            cr.move_to(0, width/2)
            cr.line_to(width, width/2)
            cr.move_to(width/2, 0)
            cr.line_to(width/2, height)

        cr.set_source_rgba (0., 0.5, 0, 1)
        cr.set_line_width (1)
        annotes = self.should_draw_numbers()
        for i,h in enumerate(self.handles):
            cr.save()
            if self.selected and i == self.selected[0]:
                cr.set_source_rgba (0.5, 0, 0, 1)
            h.draw(cr, annotes)
            cr.restore()

        cr.set_source_rgba (0., 0.5, 0, 0.8)
        if self.notify:
            cr.save()
            cr.identity_matrix()
            bnds = draw_text(cr, (0, height), self.notify, True)
            l,t,w,h = bnds[1]
            cr.set_source_rgba(1,1,1,0.9)
            cr.rectangle(l,t+height-h, w, h)
            cr.fill()
            cr.set_source_rgba(0,0,0,1)
            draw_text(cr, (0, height), self.notify, True)
            cr.restore()
        
    def mouse_moved(self, e):
        mouse = (e.x, e.y)
        if self.transform != None:
            mouse = self.transform.transform_point(e.x, e.y)
    
        if e.state & (gtk.gdk.BUTTON1_MASK | gtk.gdk.BUTTON3_MASK):
            self.interactive_level = 2
            if self.selected:
                self.handles[self.selected[0]].move_to(self.selected, self.old_mouse, mouse)
        self.old_mouse = mouse
        self.canvas.queue_draw()
        #self.redraw()
    def mouse_pressed(self, e):
        self.interactive_level = 1
        mouse = (e.x, e.y)
        if self.transform != None:
            mouse = self.transform.transform_point(e.x, e.y)
        if e.button == 1:
            for i,h in enumerate(self.handles):
                hit = h.hit(mouse)
                if hit != None:
                    self.selected = (i, hit)
            self.mouse_down = True
        self.old_mouse = mouse
        self.canvas.queue_draw()
        #self.redraw()

    def mouse_released(self, e):
        self.interactive_level = 1
        self.selected = None
        if e.button == 1:
            self.mouse_down = False
        self.canvas.queue_draw()
        #self.redraw()

    def scroll_event(self, da, ev):
        self.interactive_level = 1
        #print 'scroll:'+'\n'.join([str((x, getattr(ev, x))) for x in dir(ev)])
        #print 'end'
        da.queue_draw()
        hit = None
        for i,h in enumerate(self.handles):
            hit = h.scroll((ev.x,ev.y), ev.direction)
            if hit != None:
                break
        return hit != None
        
    def key_hit(self, e):
        pass

    def should_draw_numbers(self):
        return True
    def should_draw_bounds(self):
        return 0
    
    def first_time(self, argv):
        pass
    
    def gtk_ready(self):
        pass
    
    def resize_canvas(self, s):
        pass
    def redraw(self):
        self.window.queue_draw()
    def get_save_size(self):
        return (0,0)+self.window.window.get_size()
    def delete_event(self, window, e):
        gtk.main_quit()
        return False
    
    def expose_event(self, widget, event):
        cr = widget.window.cairo_create()

        width, height = widget.window.get_size()
        global resized

        if not resized:
            alloc_size = ((0, width), (0, height))
            self.resize_canvas(alloc_size)
            resized = True
        cr.translate(self.origin[0], self.origin[1])
        self.draw(cr, (width, height), False)

        return True

    def mouse_motion_event(self, widget, e):
        e.x -= self.origin[0]
        e.y -= self.origin[1]
        self.mouse_moved(e)

        return False

    def mouse_event(self, widget, e):
        e.x -= self.origin[0]
        e.y -= self.origin[1]
        self.mouse_pressed(e)

        return False

    def mouse_release_event(self, widget, e):
        e.x -= self.origin[0]
        e.y -= self.origin[1]
        self.mouse_released(e)

        return False

    def key_release_event(self, widget, e):
        self.key_hit(e)

        return False

    def size_allocate_event(self, widget, allocation):
        alloc_size = ((allocation.x, allocation.x + allocation.width),
                      (allocation.y, allocation.y+allocation.height))
        self.resize_canvas(alloc_size)

        return False

    def relax_interaction_timeout(self):
        if self.interactive_level > 0:
            self.interactive_level -= 1
            self.canvas.queue_draw()
        return True
                        

class Toggle:
    def __init__(self):
        self.bounds = (0,0,0,0)
        self.text = ""
        self.on = False
    def draw(self, cr):
        cr.set_source_rgba(0,0,0,1)
        cr.rectangle(bounds.left(), bounds.top(),
                     bounds.width(), bounds.height())
        if(on):
            cr.fill()
            cr.set_source_rgba(1,1,1,1)
        else:
            cr.stroke()
        draw_text(cr, bounds.corner(0) + (5,2), text)
        
    def toggle(self):
        self.on = not self.on
    def set(self, state):
        self.on = state
    def handle_click(self, e):
        if bounds.contains((e.x, e,y)) and e.button == 1:
            toggle()

def toggle_events(toggles, e):
    for t in toggles:
        t.handle_click(e)

def draw_toggles(cr, toggles):
    for t in toggles:
        t.draw(cr)


current_toys = []

def draw_text(cr, loc, txt, bottom = False, font="Sans 12"):
    import pango
    layout = pangocairo.CairoContext.create_layout (cr)
    layout.set_font_description (pango.FontDescription(font))
    
    layout.set_text(txt)
    bounds = layout.get_pixel_extents()
    cr.move_to(loc[0], loc[1])
    if bottom:
        if bottom == "topright":
            cr.move_to(loc[0] - bounds[1][2],
                       loc[1])
        elif bottom == "bottomright":
            cr.move_to(loc[0] - bounds[1][2],
                       loc[1] - bounds[1][3])
        elif bottom == "topleft":
            cr.move_to(loc[0],
                       loc[1])
        else:
            cr.move_to(loc[0], loc[1] - bounds[1][3])
    cr.show_layout(layout)
    return bounds

# Framework Accessors

# Gui Event Callbacks

def make_about(evt):
    about_window = gtk.Window(gtk.WINDOW_TOPLEVEL)
    about_window.set_title("About")
    about_window.set_resizable(True)
    
    about_text = gtk.TextView()
    buf = about_text.get_buffer()
    buf.set_text("Toy lib2geom application", -1)
    about_window.add(about_text)

    about_window.show_all()

def save_cairo(evt):
    d = gtk.FileChooserDialog("Save file as svg, png or pdf",
                              None,
                              gtk.FILE_CHOOSER_ACTION_SAVE,
                              (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                               gtk.STOCK_SAVE, gtk.RESPONSE_ACCEPT))

    if(d.run() == gtk.RESPONSE_ACCEPT):
        filename = d.get_filename()
        cr_s = None
        left, top, width, height = current_toys[0].get_save_size()
        
        if filename[-4:] == ".pdf":
            cr_s = cairo.PDFSurface(filename, width, height)
        elif filename[-4:] == ".png":
            cr_s = cairo.ImageSurface(cairo.FORMAT_ARGB32, width, height )
        else:
            cr_s = cairo.SVGSurface(filename, width, height)
        cr = cairo.Context(cr_s)
        cr = pangocairo.CairoContext(cairo.Context(cr_s))
        cr.translate(-left, -top)
        current_toys[0].draw(cr, (width, height), True)
            
        cr.show_page()
        del cr
        del cr_s
    d.destroy()
        
resized = False


def FileMenuAction():
    pass

ui = """
<ui>
  <menubar>
    <menu name="FileMenu" action="FileMenuAction">
      <menuitem name="Save" action="save_cairo" />
      <menuitem name="Quit" action="gtk.main_quit" />
      <placeholder name="FileMenuAdditions" />
    </menu>
    <menu name="HelpMenu" action="HelpMenuAction">
      <menuitem name="About" action="make_about" />
    </menu>
  </menubar>
</ui>
"""

def init(argv, t, width, height):
    global current_toys
    current_toys.append(t)

    t.first_time(argv)
    
    t.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
    t.window.set_title("title")

# Creates the menu from the menu data above
    uim = gtk.UIManager()
    ag = gtk.ActionGroup('menu_actiongroup')
    ag.add_actions([('FileMenuAction', None, 'File', None, None, None),
                    ('save_cairo', None, 'Save screenshot', None, None, save_cairo),
                    ('gtk.main_quit', None, 'Quit', None, None, gtk.main_quit),
                    ('HelpMenuAction', None, 'Help', None, None, None),
                    ('make_about', None, 'About', None, None, make_about)
                   ])
    uim.insert_action_group(ag, 0)
    uim.add_ui_from_string(ui)
    menu = uim.get_widget("/ui/menubar")
    # Creates the menu from the menu data above
    #uim = gtk.UIManager()
    #uim.add_ui_from_string(ui)
    #menu = uim.get_widget("ui/menubar")

    t.window.connect("delete_event", t.delete_event)

    t.canvas = gtk.DrawingArea()

    t.canvas.add_events((gtk.gdk.BUTTON_PRESS_MASK | gtk.gdk.BUTTON_RELEASE_MASK | gtk.gdk.KEY_PRESS_MASK | gtk.gdk.POINTER_MOTION_MASK | gtk.gdk.SCROLL_MASK))

    t.canvas.connect("expose_event", t.expose_event)
    t.canvas.connect("button_press_event", t.mouse_event)
    t.canvas.connect("button_release_event", t.mouse_release_event)
    t.canvas.connect("motion_notify_event", t.mouse_motion_event)
    t.canvas.connect("key_press_event", t.key_release_event)
    t.canvas.connect("size-allocate", t.size_allocate_event)
    t.canvas.connect("scroll_event", t.scroll_event)
    
    t.vbox = gtk.VBox(False, 0)
    t.window.add(t.vbox)

    t.vbox.pack_start (menu, False, False, 0)

    pain = gtk.VPaned()
    t.vbox.pack_start(pain, True, True, 0)
    pain.add1(t.canvas)

    t.canvas.set_size_request(width, height)
    t.window.show_all()

    # Make sure the canvas can receive key press events.
    t.canvas.set_flags(gtk.CAN_FOCUS)
    t.canvas.grab_focus()
    assert(t.canvas.is_focus())

    t.gtk_ready()
    gobject.timeout_add(1000, t.relax_interaction_timeout)
    
    gtk.main()

def get_vbox():
    return current_toys[0].vbox
