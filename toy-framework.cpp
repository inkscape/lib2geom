#include "toy-framework.h"

#include <sstream>
#include <iostream>

#include "point.h"
#include "point-ops.h"
#include "point-fns.h"
#include "geom.h"

#include "cairo-pdf.h"
#include "cairo-svg.h"

GtkWindow* window;

double uniform() {
    return double(rand()) / RAND_MAX;
}

void make_about() {
    GtkWidget* about_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(about_window), "About");
    gtk_window_set_policy(GTK_WINDOW(about_window), FALSE, FALSE, TRUE);
    
    GtkWidget* about_text = gtk_text_view_new();
    GtkTextBuffer* buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(about_text));
    gtk_text_buffer_set_text(buf, "Toy lib2geom application", -1);
    gtk_container_add(GTK_CONTAINER(about_window), about_text);

    gtk_widget_show_all(about_window);
}

void save() {
    GtkWidget* d = gtk_file_chooser_dialog_new("Save file as svg or pdf", window, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
        const char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));
        cairo_surface_t* cr_s;

        if (strcmp(filename + strlen(filename) - 4, ".pdf") == 0)
            cr_s = cairo_pdf_surface_create(filename, 600., 600.);
        else
            cr_s = cairo_svg_surface_create(filename, 600., 600.);

        cairo_t* cr = cairo_create(cr_s);
        
        if(current_toy != NULL)
            current_toy->draw(cr, new std::ostringstream, 600, 600, true);

        cairo_show_page(cr);
        cairo_destroy (cr);
        cairo_surface_destroy (cr_s);
    }
    gtk_widget_destroy(d);
}

GtkItemFactory* item_factory;

GtkItemFactoryEntry menu_items[] = {
    { "/_File",             NULL,           NULL,           0,  "<Branch>" },
    { "/File/_Save",        "<CTRL>S",      save,           0,  "<StockItem>", GTK_STOCK_SAVE },
    { "/File/_Quit",        "<CTRL>Q",      gtk_main_quit,  0,  "<StockItem>", GTK_STOCK_QUIT },
/*    { "/_Settings",         NULL,           NULL,           0,  "<Branch>" },
    { "/Settings/Colors",   NULL,           NULL,           0,  "<Item>" },
    { "/Settings/Fonts",    NULL,           NULL,           0,  "<Item>" },
    { "/Settings/Grid",     NULL,           NULL,           0,  "<Item>" },
    { "/Settings/sep",      NULL,           NULL,           0,  "<Separator>" },
    { "/Settings/Show Statusbar", NULL,     show_status,    0,  "<CheckItem>" },*/
    { "/_Help",             NULL,           NULL,           0,  "<LastBranch>" },
    { "/Help/About",        NULL,           make_about,     0,  "<Item>" }
};
gint nmenu_items = 5;

GtkItemFactory* get_menu_factory(GtkWindow* window, GtkItemFactoryEntry items[], gint num) {
    GtkAccelGroup* accel_group = gtk_accel_group_new();

    GtkItemFactory* item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);
    gtk_item_factory_create_items(item_factory, num, items, NULL);

    gtk_window_add_accel_group(window, accel_group); 

    return item_factory;
}

GtkWidget* get_menubar(GtkItemFactory* item_factory) {
    return gtk_item_factory_get_widget(item_factory, "<main>");
}

static gint delete_event(GtkWidget* window, GdkEventAny* e, gpointer data) {
    (void)( window);
    (void)( e);
    (void)( data);

    gtk_main_quit();
    return FALSE;
}

static gboolean
expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    cairo_t *cr = gdk_cairo_create (widget->window);
    
    int width = 256;
    int height = 256;
    gdk_drawable_get_size(widget->window, &width, &height);

    std::ostringstream notify;

    if(screen_lines) {    
        cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
        cairo_set_line_width (cr, 0.5);
        for(int i = 1; i < 4; i+=2) {
            cairo_move_to(cr, 0, i*width/4);
            cairo_line_to(cr, width, i*width/4);
            cairo_move_to(cr, i*width/4, 0);
            cairo_line_to(cr, i*width/4, height);
        }
    }

    if(current_toy != NULL)
        current_toy->draw(cr, &notify, width, height, false);
    
    cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
    cairo_set_line_width (cr, 1);
    for(int i = 0; i < handles.size(); i++) {
        std::ostringstream number;
        number << i;
        draw_circ(cr, handles[i]);
        cairo_move_to(cr, handles[i]);
        PangoLayout* layout = pango_cairo_create_layout (cr);
        pango_layout_set_text(layout, 
                              number.str().c_str(), -1);

        PangoFontDescription *font_desc = pango_font_description_new();
        pango_font_description_set_family(font_desc, "Sans");
        const int size_px = 10;
        pango_font_description_set_absolute_size(font_desc, size_px * 1024.0);
        pango_layout_set_font_description(layout, font_desc);
        PangoRectangle logical_extent;
        pango_layout_get_pixel_extents(layout,
                                       NULL,
                                       &logical_extent);
        pango_cairo_show_layout(cr, layout);
    }
    
    cairo_set_source_rgba (cr, 0.5, 0, 0, 1);
    if(selected_handle != NULL)
        draw_circ(cr, *selected_handle);

    cairo_set_source_rgba (cr, 0.5, 0.25, 0, 1);
    cairo_stroke(cr);

    cairo_set_source_rgba (cr, 0., 0.5, 0, 0.8);
    {
        notify << std::ends;
        PangoLayout *layout = gtk_widget_create_pango_layout(widget, notify.str().c_str());
        PangoRectangle logical_extent;
        pango_layout_get_pixel_extents(layout,
                                       NULL,
                                       &logical_extent);
        cairo_move_to(cr, 0, height-logical_extent.height);
        pango_cairo_show_layout(cr, layout);
    }
    cairo_destroy (cr);

    return TRUE;
}

static void handle_mouse(GtkWidget* widget) {
    gtk_widget_queue_draw (widget);
}

static gint mouse_motion_event(GtkWidget* widget, GdkEventMotion* e, gpointer data) {
    (void)(data);
    Geom::Point mouse(e->x, e->y);
    
    if(e->state & (GDK_BUTTON1_MASK | GDK_BUTTON3_MASK)) {
        if(selected_handle)
            *selected_handle = mouse;
        handle_mouse(widget);
    }

    if(e->state & (GDK_BUTTON2_MASK)) {
        gtk_widget_queue_draw(widget);
    }
    
    old_mouse_point = mouse;

    if(current_toy != NULL)
        current_toy->mouse_moved(e);

    return FALSE;
}

static gint mouse_event(GtkWidget* window, GdkEventButton* e, gpointer data) {
    (void)(data);
    Geom::Point mouse(e->x, e->y);
    if(e->button == 1 || e->button == 3) {
        for(int i = 0; i < handles.size(); i++) {
            if(Geom::L2(mouse - handles[i]) < 5)
                selected_handle = &handles[i];
        }
        handle_mouse(window);
    } else if(e->button == 2) {
        gtk_widget_queue_draw(window);
    }
    old_mouse_point = mouse;

    if(current_toy != NULL)
        current_toy->mouse_pressed(e);

    return FALSE;
}

static gint mouse_release_event(GtkWidget* window, GdkEventButton* e, gpointer data) {
    (void)(data);
    (void)(window);
    (void)(e);
    selected_handle = 0;
    Geom::Point mouse(e->x, e->y);

    if(current_toy != NULL)
        current_toy->mouse_released(e);

    return FALSE;
}

static gint key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    if(current_toy != NULL)    
        current_toy->key_pressed(event);
    return FALSE;
}

void init(int argc, char **argv, char *title, Toy* t) {
    current_toy = t;
    gtk_init (&argc, &argv);
    
    gdk_rgb_init();

    window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));

    gtk_window_set_title(GTK_WINDOW(window), title);

    item_factory = get_menu_factory(window, menu_items, nmenu_items);
    GtkWidget* menu = get_menubar(item_factory);

    //gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, TRUE);

    gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(delete_event), NULL);

    gtk_widget_push_visual(gdk_rgb_get_visual());
    gtk_widget_push_colormap(gdk_rgb_get_cmap());
    canvas = gtk_drawing_area_new();

    gtk_widget_add_events(canvas, (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_KEY_PRESS_MASK | GDK_POINTER_MOTION_MASK));

    gtk_signal_connect(GTK_OBJECT (canvas), "expose_event", GTK_SIGNAL_FUNC(expose_event), 0);
    gtk_signal_connect(GTK_OBJECT(canvas), "button_press_event", GTK_SIGNAL_FUNC(mouse_event), 0);
    gtk_signal_connect(GTK_OBJECT (canvas), "button_release_event", GTK_SIGNAL_FUNC(mouse_release_event), 0);
    gtk_signal_connect(GTK_OBJECT (canvas), "motion_notify_event", GTK_SIGNAL_FUNC(mouse_motion_event), 0);
    gtk_signal_connect(GTK_OBJECT(canvas), "key_press_event", GTK_SIGNAL_FUNC(key_release_event), 0);

    gtk_widget_pop_colormap();
    gtk_widget_pop_visual();

    GtkWidget* box = gtk_vbox_new (FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    gtk_box_pack_start (GTK_BOX (box), menu, FALSE, FALSE, 0);

    GtkWidget* pain = gtk_vpaned_new();
    gtk_box_pack_start(GTK_BOX(box), pain, TRUE, TRUE, 0);
    gtk_paned_add1(GTK_PANED(pain), canvas);

    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);

    gtk_widget_show_all(GTK_WIDGET(window));

    /* Make sure the canvas can receive key press events. */
    GTK_WIDGET_SET_FLAGS(canvas, GTK_CAN_FOCUS);
    assert(GTK_WIDGET_CAN_FOCUS(canvas));
    gtk_widget_grab_focus(canvas);
    assert(gtk_widget_is_focus(canvas));

    gtk_main();
}
