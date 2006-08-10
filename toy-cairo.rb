#!/usr/bin/env ruby
require 'gtk2'
require 'cairo'
Gtk.init

accel_group = Gtk::AccelGroup.new

window = Gtk::Window.new("toy cairo")
window.set_default_size(600,600)
window.add_accel_group(accel_group)
window.signal_connect("destroy") {Gtk.main_quit}

box = Gtk::VBox.new
window.add(box)

menubar = Gtk::MenuBar.new
box.pack_start(menubar, false, false, 0)

# File menu
fileitem = Gtk::MenuItem.new("_File",true)
menubar.append(fileitem)

filemenu = Gtk::Menu.new
fileitem.submenu = filemenu

openitem = Gtk::ImageMenuItem.new(Gtk::Stock::OPEN, accel_group)
filemenu.append(openitem)
openitem.signal_connect("activate") do
    dialog = Gtk::FileChooserDialog.new("Open File",
                                     window,
                                     Gtk::FileChooser::ACTION_OPEN,
                                     nil,
                                     [Gtk::Stock::CANCEL, Gtk::Dialog::RESPONSE_CANCEL],
                                     [Gtk::Stock::OPEN, Gtk::Dialog::RESPONSE_ACCEPT])
    [["SVG Path (*.svgd)","*.svgd"],["All files (*)","*"]].each do |x|
        filter = Gtk::FileFilter.new
        filter.name = x[0]
        filter.add_pattern(x[1])
        dialog.add_filter(filter)
    end
    
    if dialog.run == Gtk::Dialog::RESPONSE_ACCEPT
        #TODO: do something with this file
        puts "#{dialog.filename} was selected"
    end
    dialog.destroy
end

filemenu.append(Gtk::SeparatorMenuItem.new)

quititem = Gtk::ImageMenuItem.new(Gtk::Stock::QUIT, accel_group)
filemenu.append(quititem)
quititem.signal_connect("activate") {Gtk::main_quit()}

# Help menu
helpitem = Gtk::MenuItem.new("_Help",true)
menubar.append(helpitem)

helpmenu = Gtk::Menu.new
helpitem.submenu = helpmenu

aboutitem = Gtk::ImageMenuItem.new("_About", true)
helpmenu.append(aboutitem)

aboutbox = Gtk::AboutDialog.new
aboutbox.name = "2geom Toy Cairo"
#Pixbuf doesn't load SVGs
#aboutbox.logo = Gdk::Pixbuf.new("2geom.svg")
aboutitem.signal_connect("activate") do
    aboutbox.show()    
end



canvas = Gtk::DrawingArea.new
canvas.add_events(Gdk::Event::BUTTON_PRESS_MASK|
    Gdk::Event::BUTTON_RELEASE_MASK|
    Gdk::Event::KEY_PRESS_MASK|
    Gdk::Event::POINTER_MOTION_MASK)
box.pack_start(canvas)

canvas.signal_connect("expose_event") do
  cr = canvas.window.create_cairo_context
  cr.move_to(50, 50)
  cr.curve_to(100, 25, 100, 75, 150, 50)
  cr.line_to(150, 150)
  cr.line_to(50, 150)
  cr.close_path

  cr.set_source_rgb(0.0, 0.0, 0.0)
  cr.fill_preserve
end

canvas.signal_connect("button_press_event") do
end
canvas.signal_connect("button_release_event") do
end
canvas.signal_connect("key_press_event") do
end
canvas.signal_connect("motion_notify_event") do
end



code = Gtk::TextView.new
box.pack_start(code)

%q{
    dash_gc = gdk_gc_new(canvas->window);
    gint8 dash_list[] = {4, 4};
    gdk_gc_set_dashes(dash_gc, 0, dash_list, 2);
    GdkColor colour;
    colour.red = 0xffff;
    colour.green = 0xffff;
    colour.blue = 0xffff;

    plain_gc = gdk_gc_new(canvas->window);
    
    //gdk_gc_set_rgb_fg_color(dash_gc, &colour);
    gdk_rgb_find_color(gtk_widget_get_colormap(canvas), &colour);
    gdk_window_set_background(canvas->window, &colour);
    gdk_gc_set_line_attributes(dash_gc, 1, GDK_LINE_ON_OFF_DASH,
                               GDK_CAP_BUTT,GDK_JOIN_MITER);

    /* Make sure the canvas can receive key press events. */
    GTK_WIDGET_SET_FLAGS(canvas, GTK_CAN_FOCUS);
    
    
    assert(GTK_WIDGET_CAN_FOCUS(canvas));
    gtk_widget_grab_focus(canvas);
    assert(gtk_widget_is_focus(canvas));
    
    g_idle_add((GSourceFunc)idler, canvas);
}
window.show_all
Gtk.main
