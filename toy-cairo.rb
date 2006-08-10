#!/usr/bin/env ruby
require 'gtk2'
require 'cairo'
$LOAD_PATH.unshift "../packages/cairo/ext/"
$LOAD_PATH.unshift "../packages/cairo/lib/"

require 'cairo'
require 'stringio'
#require "rsvg2"

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

saveitem = Gtk::ImageMenuItem.new(Gtk::Stock::SAVE, accel_group)
filemenu.append(saveitem)

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


canvas.signal_connect("button_press_event") do
end
canvas.signal_connect("button_release_event") do
end
canvas.signal_connect("key_press_event") do
end
canvas.signal_connect("motion_notify_event") do
end



code = Gtk::TextView.new
sw = Gtk::ScrolledWindow.new
box.pack_start(sw)
sw.add(code)

tb = code.buffer
tb.set_text("  cr.move_to(50, 50)
  cr.curve_to(100, 25, 100, 75, 150, 50)
  cr.line_to(150, 150)
  cr.line_to(50, 150)
  cr.close_path

  cr.set_source_rgb(0.0, 0.0, 0.0)
  cr.fill_preserve
  cr.set_source_rgb(1.0, 0.0, 0.0)
  cr.set_line_join(Cairo::LINE_JOIN_MITER)
  cr.set_line_width(4)
  cr.stroke
")
tb.set_text("cr.move_to(186.58,0.0)
cr.curve_to(182.01,-0.01,177.49,1.64,174.21,5.0)
cr.line_to(119.64,60.84)
cr.curve_to(101.21,83.67,132.18,80.99,145.46,87.56)
cr.curve_to(150.22,92.43,127.19,96.03,131.96,100.91)
cr.curve_to(136.72,105.78,160.78,110.29,165.55,115.16)
cr.curve_to(170.31,120.03,155.79,125.19,160.55,130.06)
cr.curve_to(165.31,134.93,176.33,130.32,178.39,141.56)
cr.curve_to(179.86,149.6,198.25,145.02,207.24,138.44)
cr.curve_to(212.0,133.56,198.13,134.03,202.89,129.16)
cr.curve_to(214.74,117.04,225.78,124.76,229.83,112.63)
cr.curve_to(231.83,106.63,212.4,103.37,217.17,98.5)
cr.curve_to(230.88,90.5,278.25,85.29,255.77,62.81)
cr.line_to(199.21,5.0)
cr.curve_to(195.75,1.68,191.15,0.01,186.58,0.0)
cr.close_path()
cr.move_to(187.05,5.31)
cr.curve_to(190.31,5.33,193.58,6.53,195.92,8.91)
cr.line_to(217.49,30.84)
cr.curve_to(219.53,32.94,219.98,37.93,218.36,38.16)
cr.curve_to(216.07,38.48,213.54,31.88,210.21,31.88)
cr.curve_to(206.53,31.88,207.6,43.88,204.61,43.88)
cr.curve_to(201.45,43.88,200.41,37.53,196.58,37.53)
cr.curve_to(193.49,37.53,191.4,46.13,187.05,46.13)
cr.curve_to(183.19,46.13,179.85,27.5,177.52,27.5)
cr.curve_to(175.48,27.5,174.33,44.03,169.8,44.03)
cr.curve_to(162.25,44.03,150.64,44.0,150.64,44.0)
cr.curve_to(146.81,43.99,147.44,40.25,151.14,36.25)
cr.curve_to(158.76,28.03,173.39,13.76,178.14,8.91)
cr.curve_to(180.53,6.47,183.79,5.3,187.05,5.31)
cr.close_path()
cr.move_to(161.14,90.59)
cr.curve_to(162.37,90.59,201.96,95.88,186.67,100.06)
cr.curve_to(180.92,101.64,154.05,90.59,161.14,90.59)
cr.close_path()
cr.move_to(246.89,105.94)
cr.curve_to(242.46,106.09,238.07,108.37,236.86,112.63)
cr.curve_to(236.86,115.39,257.27,117.21,257.27,111.97)
cr.curve_to(255.81,107.76,251.33,105.78,246.89,105.94)
cr.close_path()
cr.move_to(153.3,117.94)
cr.curve_to(146.9,117.88,139.41,122.57,144.92,127.34)
cr.curve_to(149.76,131.52,157.22,126.3,159.46,120.47)
cr.curve_to(158.14,118.72,155.8,117.96,153.3,117.94)
cr.close_path()
cr.move_to(234.3,118.31)
cr.curve_to(228.07,123.9,235.0,129.58,241.14,125.97)
cr.curve_to(242.51,124.58,241.1,119.7,234.3,118.31)
cr.close_path()
cr.move_to(21.38,211.73)
cr.curve_to(27.65,212.1,44.11,210.92,54.07,210.92)
cr.curve_to(64.41,210.92,65.27,224.85,54.42,224.85)
cr.curve_to(41.02,224.85,19.65,224.85,6.64,224.85)
cr.curve_to(-2.24,224.85,-2.19,211.04,6.66,205.62)
cr.curve_to(29.53,191.61,47.88,185.14,47.88,168.51)
cr.curve_to(47.88,151.61,29.25,147.75,9.63,161.34)
cr.curve_to(1.59,166.91,-4.54,155.19,4.94,148.65)
cr.curve_to(59.51,110.96,92.55,186.63,21.38,211.73)
cr.close_path()
cr.move_to(112.98,171.13)
cr.curve_to(96.59,157.03,65.85,165.87,65.85,197.27)
cr.curve_to(65.85,228.46,104.86,229.44,112.98,216.71)
cr.curve_to(112.98,239.71,95.91,241.77,78.56,241.77)
cr.curve_to(69.78,241.77,69.74,255.49,78.56,255.49)
cr.curve_to(117.66,255.49,126.29,241.51,126.29,203.05)
cr.line_to(126.29,184.15)
cr.curve_to(126.29,150.75,133.39,144.43,139.62,136.31)
cr.curve_to(145.58,128.53,135.09,122.36,128.52,128.92)
cr.curve_to(121.0,136.44,112.98,156.44,112.98,171.13)
cr.close_path()
cr.move_to(113.88,194.57)
cr.curve_to(113.88,217.56,79.72,221.19,79.72,196.15)
cr.curve_to(79.72,171.11,113.88,171.16,113.88,194.57)
cr.close_path()
cr.move_to(190.71,190.21)
cr.curve_to(190.71,195.21,189.09,200.5,180.21,200.5)
cr.line_to(147.97,200.5)
cr.curve_to(151.28,217.76,168.18,217.1,183.42,208.87)
cr.curve_to(188.62,206.07,194.23,215.61,186.52,220.09)
cr.curve_to(180.12,223.81,172.94,225.38,166.03,225.38)
cr.curve_to(148.1,225.38,135.41,214.47,135.41,195.13)
cr.curve_to(135.41,175.34,149.97,165.54,164.15,165.32)
cr.curve_to(177.6,165.11,190.71,175.55,190.71,190.21)
cr.close_path()
cr.move_to(179.59,190.34)
cr.curve_to(179.59,182.43,173.91,176.32,164.01,176.32)
cr.curve_to(154.31,176.32,148.21,181.52,148.21,190.73)
cr.line_to(179.59,190.34)
cr.close_path()
cr.move_to(210.92,195.38)
cr.curve_to(210.92,206.82,216.45,215.44,226.06,215.44)
cr.curve_to(236.41,215.44,241.2,207.01,241.2,195.38)
cr.curve_to(241.2,183.62,236.13,176.25,226.06,176.25)
cr.curve_to(215.99,176.25,210.92,184.09,210.92,195.38)
cr.close_path()
cr.move_to(226.07,165.37)
cr.curve_to(244.59,165.37,253.16,178.85,253.16,195.38)
cr.curve_to(253.16,211.92,244.24,225.4,226.07,225.4)
cr.curve_to(208.09,225.4,199.49,211.42,199.49,195.38)
cr.curve_to(199.49,179.35,207.38,165.37,226.07,165.37)
cr.close_path()
cr.move_to(307.11,177.79)
cr.curve_to(322.76,155.04,350.06,168.57,350.06,188.19)
cr.curve_to(350.06,188.19,350.06,211.55,350.06,219.96)
cr.curve_to(350.06,227.27,338.58,227.54,338.58,219.96)
cr.curve_to(338.58,211.62,338.58,192.61,338.58,192.61)
cr.curve_to(338.58,173.06,313.48,172.6,313.48,192.61)
cr.curve_to(313.48,192.61,313.48,212.09,313.48,219.96)
cr.curve_to(313.48,227.28,300.75,227.56,300.75,219.96)
cr.curve_to(300.75,211.36,300.75,192.61,300.75,192.61)
cr.curve_to(300.75,171.32,275.64,171.79,275.64,192.61)
cr.curve_to(275.64,192.61,275.64,210.58,275.64,219.96)
cr.curve_to(275.64,227.29,262.54,227.43,262.54,219.96)
cr.curve_to(262.54,211.02,262.93,189.23,262.93,189.23)
cr.curve_to(262.93,167.07,293.18,155.09,307.11,177.79)
cr.close_path()
cr.set_source_rgb(0.0, 0.0, 0.0)
cr.fill_preserve
")

canvas.signal_connect("expose_event") do
  cr = canvas.window.create_cairo_context
  tb = code.buffer
  Kernel.eval(tb.get_text())
  #handle = RSVG::Handle.new_from_file("branding/2geom/svg")
  #cr.render_rsvg_handle(handle)

end

saveitem.signal_connect("activate") {
  surface = Cairo::ImageSurface.new(Cairo::FORMAT_ARGB32, 200, 200)
  cr = Cairo::Context.new(surface)

  tb = code.buffer
  Kernel.eval(tb.get_text())

  cr.target.write_to_png("screenshot.png")

  cr.target.finish
  surface = Cairo::SVGSurface.new("screenshot.svg", 200, 200)
  cr = Cairo::Context.new(surface)

  tb = code.buffer
  Kernel.eval(tb.get_text())
  
  cr.show_page
  
  cr.target.finish
}



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
