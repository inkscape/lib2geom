#!/usr/bin/python

import gtk,math
import pangocairo,cairo

# def draw_text(cr, pos, txt, bottom = False):
# def draw_number(cr, pos, num):

def draw_circ(cr, (x, y)):
    cr.new_sub_path()
    cr.arc(x, y, 3, 0, math.pi*2)
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

class Toy:
    def __init__(self):
        self.handles = []
        self.mouse_down = False
        self.old_mouse = None
        self.selected = None
        self.notify = "notify"

    def draw(self, cr, (width, height), save):
        bounds = self.should_draw_bounds()
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
            draw_text(cr, (0, height), self.notify, True)
        
    def mouse_moved(self, e):
        mouse = (e.x, e.y)
    
        if e.state & (gtk.gdk.BUTTON1_MASK | gtk.gdk.BUTTON3_MASK):
            if self.selected:
                self.handles[self.selected[0]].move_to(self.selected, self.old_mouse, mouse)
        self.old_mouse = mouse
        self.redraw()
    def mouse_pressed(self, e):
        mouse = (e.x, e.y)
        if e.button == 1:
            for i,h in enumerate(self.handles):
                hit = h.hit(mouse)
                if hit != None:
                    self.selected = (i, hit)
            self.mouse_down = True
        self.old_mouse = mouse
        self.redraw()

    def mouse_released(self, e):
        self.selected = None
        if e.button == 1:
            self.mouse_down = False
        self.redraw()

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


window = None
canvas = None
current_toy = None

def draw_text(cr, loc, txt, bottom = False):
    layout = pangocairo.CairoContext.create_layout (cr)
    layout.set_text(txt)
    bounds = layout.get_pixel_extents()
    cr.move_to(loc[0], loc[1])
    if bottom:
        cr.move_to(loc[0], loc[1] - bounds[1][3])
    cr.show_layout(layout)

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
        left, top, width, height = current_toy.get_save_size()
        
        if filename[-4:] == ".pdf":
            cr_s = cairo.PDFSurface(filename, width, height)
        elif filename[-4:] == ".png":
            cr_s = cairo.ImageSurface(cairo.FORMAT_ARGB32, width, height )
        else:
            cr_s = cairo.SVGSurface(filename, width, height)
        cr = cairo.Context(cr_s)
        cr.translate(-left, -top)
        current_toy.draw(cr, (width, height), True)
            
        cr.show_page()
        del cr
        del cr_s
    d.destroy()
        
def delete_event(window, e):
    gtk.main_quit()
    return False
                        
resized = False

def expose_event(widget, event):
    cr = widget.window.cairo_create()
    
    width, height = widget.window.get_size()
    global resized

    if not resized:
        alloc_size = ((0, width), (0, height))
        if current_toy:
            current_toy.resize_canvas(alloc_size)
        resized = True
    if current_toy:
        current_toy.draw(cr, (width, height), False)

    return True

def mouse_motion_event(widget, e):
    if current_toy:
        current_toy.mouse_moved(e)

    return False

def mouse_event(widget, e):
    if current_toy:
        current_toy.mouse_pressed(e)
        
    return False

def mouse_release_event(widget, e):
    if current_toy:
        current_toy.mouse_released(e)

    return False

def key_release_event(widget, e):
    if current_toy:
        current_toy.key_hit(e)

    return False

def size_allocate_event(widget, allocation):
    alloc_size = ((allocation.x, allocation.x + allocation.width),
                  (allocation.y, allocation.y+allocation.height))
    if current_toy:
        current_toy.resize_canvas(alloc_size)

    return False

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

window = None
vbox = None
canvas = None
def init(argv, t, width, height):
    global current_toy,window
    current_toy = t

    t.first_time(argv)
    
    global window, vbox, canvas
    window = gtk.Window(gtk.WINDOW_TOPLEVEL)
    window.set_title("title")

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

    window.connect("delete_event", delete_event)

    canvas = gtk.DrawingArea()

    canvas.add_events((gtk.gdk.BUTTON_PRESS_MASK | gtk.gdk.BUTTON_RELEASE_MASK | gtk.gdk.KEY_PRESS_MASK | gtk.gdk.POINTER_MOTION_MASK))

    canvas.connect("expose_event", expose_event)
    canvas.connect("button_press_event", mouse_event)
    canvas.connect("button_release_event", mouse_release_event)
    canvas.connect("motion_notify_event", mouse_motion_event)
    canvas.connect("key_press_event", key_release_event)
    canvas.connect("size-allocate", size_allocate_event)

    vbox = gtk.VBox(False, 0)
    window.add(vbox)

    vbox.pack_start (menu, False, False, 0)

    pain = gtk.VPaned()
    vbox.pack_start(pain, True, True, 0)
    pain.add1(canvas)

    canvas.set_size_request(width, height)
    window.show_all()
    t.window = window
    t.canvas = canvas

    # Make sure the canvas can receive key press events.
    canvas.set_flags(gtk.CAN_FOCUS)
    canvas.grab_focus()
    assert(canvas.is_focus())

    t.gtk_ready()
    
    gtk.main()

def get_vbox():
    global vbox
    return vbox
