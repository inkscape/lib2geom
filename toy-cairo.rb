#!/usr/bin/env ruby
require 'gtk2'
require 'cairo'
Gtk.init

accel_group = Gtk::AccelGroup.new

window = Gtk::Window.new("toy cairo")

window.signal_connect("destroy") {
  Gtk.main_quit
}

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

canvas = Gtk::DrawingArea.new
canvas.set_size_request(400,400)
box.pack_start(canvas)

code = Gtk::TextView.new
box.pack_start(code)

canvas.signal_connect("expose_event") {
  %q{
  cr = Cairo::Context.new(canvas.window) # this bit hasn't been written yet.
  cr.move_to(50, 50)
  cr.curve_to(100, 25, 100, 75, 150, 50)
  cr.line_to(150, 150)
  cr.line_to(50, 150)
  cr.close_path

  cr.set_source_rgb(0.0, 0.0, 0.0)
  cr.fill_preserve
}
}

%q{

    g_signal_connect ((gpointer) open, "activate",
                    G_CALLBACK (on_open_activate),
                    NULL);
    g_signal_connect ((gpointer) quit, "activate",
                    gtk_main_quit,
                    NULL);
    g_signal_connect ((gpointer) about, "activate",
                    G_CALLBACK (on_about_activate),
                    NULL);

    gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);


    gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, TRUE);

    gtk_signal_connect(GTK_OBJECT(window),
                       "delete_event",
                       GTK_SIGNAL_FUNC(delete_event_cb),
                       NULL);

    gtk_widget_push_visual(gdk_rgb_get_visual());
    gtk_widget_push_colormap(gdk_rgb_get_cmap());
    canvas = gtk_drawing_area_new();

    gtk_signal_connect(GTK_OBJECT (canvas),
                       "expose_event",
                       GTK_SIGNAL_FUNC(expose_event),
                       0);
    gtk_widget_add_events(canvas, (GDK_BUTTON_PRESS_MASK |
                                   GDK_BUTTON_RELEASE_MASK |
                                   GDK_KEY_PRESS_MASK    |
                                   GDK_POINTER_MOTION_MASK));
    gtk_signal_connect(GTK_OBJECT (canvas),
                       "button_press_event",
                       GTK_SIGNAL_FUNC(mouse_event),
                       0);
    gtk_signal_connect(GTK_OBJECT (canvas),
                       "button_release_event",
                       GTK_SIGNAL_FUNC(mouse_release_event),
                       0);
    gtk_signal_connect(GTK_OBJECT (canvas),
                       "motion_notify_event",
                       GTK_SIGNAL_FUNC(mouse_motion_event),
                       0);
    gtk_signal_connect(GTK_OBJECT(canvas),
                       "key_press_event",
                       GTK_SIGNAL_FUNC(key_release_event),
                       0);

    gtk_widget_pop_colormap();
    gtk_widget_pop_visual();

    //GtkWidget *vb = gtk_vbox_new(0, 0);


    //gtk_container_add(GTK_CONTAINER(window), vb);

    gtk_box_pack_start(GTK_BOX(menubox), canvas, TRUE, TRUE, 0);

    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);

    gtk_widget_show_all(window);

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
