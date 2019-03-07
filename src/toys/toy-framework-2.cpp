#include <cstring>
#include <cstdint>
#include <typeinfo>
#include <cairo.h>
#include <gtk/gtk.h>
#include <toys/toy-framework-2.h>

#include <cairo-features.h>
#if CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
#endif
#if CAIRO_HAS_SVG_SURFACE
#include <cairo-svg.h>
#endif

GtkApplicationWindow* the_window = nullptr;
static GtkWidget *the_canvas = nullptr;
Toy* the_toy = nullptr;
int the_requested_height = 0;
int the_requested_width = 0;
gchar **the_emulated_argv = nullptr;

gchar *arg_spool_filename = nullptr;
gchar *arg_handles_filename = nullptr;
gchar *arg_screenshot_filename = nullptr;
gchar **arg_extra_files = nullptr;

//Utility functions

double uniform() {
    return double(rand()) / RAND_MAX;
}

colour colour::from_hsv( float H,          // hue shift (in degrees)
                         float S,          // saturation shift (scalar)
                         float V,          // value multiplier (scalar)
                         float A
    )
{
    double inr = 1;
    double ing = 0;
    double inb = 0;
    float k = V/3;
    float a = V*S*cos(H)/3;
    float b = V*S*sin(H)/3;

    return colour(
        (k+2*a)*inr -     2*b*ing +    (k-a-b)*inb,
        (-k+a+3*b)*inr + (3*a-b)*ing + (-k+a+2*b)*inb,
        (2*k-2*a)*inr +     2*b*ing +  (2*k+a+b)*inb,
        A);
}

 // Given H,S,L in range of 0-1

 // Returns a Color (RGB struct) in range of 0-255

colour colour::from_hsl(float h, float sl, float l, float a) {
    h /= M_PI*2;
    colour rgba(l,l,l,a); // default to gray

    double v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);

    if (v > 0) {
        double m;
        double sv;
        int sextant;
        double fract, vsf, mid1, mid2;

        m = l + l - v;
        sv = (v - m ) / v;
        h *= 6.0;
        sextant = (int)h;
        fract = h - sextant;
        vsf = v * sv * fract;
        mid1 = m + vsf;
        mid2 = v - vsf;
        switch (sextant%6) {
            case 0:
                rgba.r = v;
                rgba.g = mid1;
                rgba.b = m;
                break;

            case 1:
                rgba.r = mid2;
                rgba.g = v;
                rgba.b = m;
                break;

            case 2:
                rgba.r = m;
                rgba.g = v;
                rgba.b = mid1;
                break;

            case 3:
                rgba.r = m;
                rgba.g = mid2;
                rgba.b = v;
                break;

            case 4:
                rgba.r = mid1;
                rgba.g = m;
                rgba.b = v;
                break;

            case 5:
                rgba.r = v;
                rgba.g = m;
                rgba.b = mid2;
                break;
        }
    }
    return rgba;
}

void cairo_set_source_rgba(cairo_t* cr, colour c) {
    cairo_set_source_rgba(cr, c.r, c.g, c.b, c.a);
}

void draw_text(cairo_t *cr, Geom::Point loc, const char* txt, bool bottom, const char* fontdesc) {
    PangoLayout* layout = pango_cairo_create_layout (cr);
    pango_layout_set_text(layout, txt, -1);
    PangoFontDescription *font_desc = pango_font_description_from_string(fontdesc);
    pango_layout_set_font_description(layout, font_desc);
    pango_font_description_free (font_desc);
    PangoRectangle logical_extent;
    pango_layout_get_pixel_extents(layout, NULL, &logical_extent);
    cairo_move_to(cr, loc - Geom::Point(0, bottom ? logical_extent.height : 0));
    pango_cairo_show_layout(cr, layout);
}

void draw_text(cairo_t *cr, Geom::Point loc, const std::string& txt, bool bottom, const std::string& fontdesc) {
    draw_text(cr, loc, txt.c_str(), bottom, fontdesc.c_str());
}

void draw_number(cairo_t *cr, Geom::Point pos, int num, std::string name, bool bottom) {
    std::ostringstream number;
    if (name.size())
	number << name;
    number << num;
    draw_text(cr, pos, number.str().c_str(), bottom);
}

void draw_number(cairo_t *cr, Geom::Point pos, unsigned num, std::string name, bool bottom) {
    std::ostringstream number;
    if (name.size())
	number << name;
    number << num;
    draw_text(cr, pos, number.str().c_str(), bottom);
}

void draw_number(cairo_t *cr, Geom::Point pos, double num, std::string name, bool bottom) {
    std::ostringstream number;
    if (name.size())
	number << name;
    number << num;
    draw_text(cr, pos, number.str().c_str(), bottom);
}

//Framework Accessors
void redraw() { gtk_widget_queue_draw(GTK_WIDGET(the_window)); }

void Toy::draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool /*save*/, std::ostringstream *timer_stream)
{
    if(should_draw_bounds() == 1) {
        cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
        cairo_set_line_width (cr, 0.5);
        for(unsigned i = 1; i < 4; i+=2) {
            cairo_move_to(cr, 0, i*width/4);
            cairo_line_to(cr, width, i*width/4);
            cairo_move_to(cr, i*width/4, 0);
            cairo_line_to(cr, i*width/4, height);
        }
    }
    else if(should_draw_bounds() == 2) {
        cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
        cairo_set_line_width (cr, 0.5);
	cairo_move_to(cr, 0, width/2);
	cairo_line_to(cr, width, width/2);
	cairo_move_to(cr, width/2, 0);
	cairo_line_to(cr, width/2, height);
    }

    cairo_set_line_width (cr, 1);
    for(unsigned i = 0; i < handles.size(); i++) {
        cairo_set_source_rgb (cr, handles[i]->rgb[0], handles[i]->rgb[1], handles[i]->rgb[2]);
	handles[i]->draw(cr, should_draw_numbers());
    }

    cairo_set_source_rgba (cr, 0.5, 0, 0, 1);
    if(selected && mouse_down == true)
	selected->draw(cr, should_draw_numbers());

    cairo_set_source_rgba (cr, 0.5, 0.25, 0, 1);
    cairo_stroke(cr);

    cairo_set_source_rgba (cr, 0., 0.5, 0, 0.8);
    {
        *notify << std::ends;
        draw_text(cr, Geom::Point(0, height-notify_offset), notify->str().c_str(), true);
    }
    if(show_timings) {
        *timer_stream << std::ends;
        draw_text(cr, Geom::Point(0, notify_offset), timer_stream->str().c_str(), false);
    }
}

void Toy::mouse_moved(GdkEventMotion* e)
{
    Geom::Point mouse(e->x, e->y);

    if(e->state & (GDK_BUTTON1_MASK | GDK_BUTTON3_MASK)) {
        if(selected)
	    selected->move_to(hit_data, old_mouse_point, mouse);
    }
    old_mouse_point = mouse;
    redraw();
}

void Toy::mouse_pressed(GdkEventButton* e) {
    Geom::Point mouse(e->x, e->y);
    selected = NULL;
    hit_data = NULL;
    canvas_click_button = e->button;
    if(e->button == 1) {
        for(unsigned i = 0; i < handles.size(); i++) {
    	    void * hit = handles[i]->hit(mouse);
    	    if(hit) {
    		selected = handles[i];
    		hit_data = hit;
    	    }
        }
        mouse_down = true;
    }
    old_mouse_point = mouse;
    redraw();
}

void Toy::scroll(GdkEventScroll* /*e*/) {
}

void Toy::canvas_click(Geom::Point at, int button) {
    (void)at;
    (void)button;
}

void Toy::mouse_released(GdkEventButton* e) {
    if(selected == NULL) {
        Geom::Point mouse(e->x, e->y);
        canvas_click(mouse, canvas_click_button);
        canvas_click_button = 0;
    }
    selected = NULL;
    hit_data = NULL;
    if(e->button == 1)
	mouse_down = false;
    redraw();
}

void Toy::load(FILE* f) {
    char data[1024];
    if (fscanf(f, "%1024s", data)) {
        name = data;
    }
    for(unsigned i = 0; i < handles.size(); i++) {
        handles[i]->load(f);
    }
}

void Toy::save(FILE* f) {
	fprintf(f, "%s\n", name.c_str());
    for(unsigned i = 0; i < handles.size(); i++)
	handles[i]->save(f);
}

//Gui Event Callbacks

void show_about_dialog(GSimpleAction *, GVariant *, gpointer) {
    GtkWidget* about_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(about_window), "About");
    gtk_window_set_resizable(GTK_WINDOW(about_window), FALSE);

    GtkWidget* about_text = gtk_text_view_new();
    GtkTextBuffer* buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(about_text));
    gtk_text_buffer_set_text(buf, "Toy lib2geom application", -1);
    gtk_container_add(GTK_CONTAINER(about_window), about_text);

    gtk_widget_show_all(about_window);
}

void quit(GSimpleAction *, GVariant *, gpointer) {
    g_application_quit(g_application_get_default());
}

Geom::Point read_point(FILE* f) {
    Geom::Point p;
    for(unsigned i = 0; i < 2; i++)
        assert(fscanf(f, " %lf ", &p[i]));
    return p;
}

Geom::Interval read_interval(FILE* f) {
    Geom::Interval p;
    Geom::Coord a, b;
    assert(fscanf(f, " %lf ", &a));
    assert(fscanf(f, " %lf ", &b));
    p.setEnds(a, b);
    return p;
}

void open_handles(GSimpleAction *, GVariant *, gpointer) {
    if (!the_toy) return;
	GtkWidget* d = gtk_file_chooser_dialog_new(
	     "Open handle configuration", GTK_WINDOW(the_window), GTK_FILE_CHOOSER_ACTION_OPEN,
	     "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
        const char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));
        FILE* f = fopen(filename, "r");
        the_toy->load(f);
        fclose(f);
    }
    gtk_widget_destroy(d);
}

void save_handles(GSimpleAction *, GVariant *, gpointer) {
    if (!the_toy) return;
    GtkWidget* d = gtk_file_chooser_dialog_new(
         "Save handle configuration", GTK_WINDOW(the_window), GTK_FILE_CHOOSER_ACTION_SAVE,
         "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
        const char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));
        FILE* f = fopen(filename, "w");
        the_toy->save(f);
        fclose(f);
    }
    gtk_widget_destroy(d);
}

void write_image(const char* filename) {
    cairo_surface_t* cr_s;
    unsigned l = strlen(filename);
    int width = gdk_window_get_width(gtk_widget_get_window(the_canvas));
    int height = gdk_window_get_height(gtk_widget_get_window(the_canvas));
    bool save_png = false;

    if (l >= 4 && strcmp(filename + l - 4, ".png") == 0) {
        cr_s = cairo_image_surface_create ( CAIRO_FORMAT_ARGB32, width, height );
        save_png = true;
    }

#if CAIRO_HAS_PDF_SURFACE
    else if (l >= 4 && strcmp(filename + l - 4, ".pdf") == 0)
        cr_s = cairo_pdf_surface_create(filename, width, height);
#endif
#if CAIRO_HAS_SVG_SURFACE
#if CAIRO_HAS_PDF_SURFACE
    else
#endif
        cr_s = cairo_svg_surface_create(filename, width, height);
#endif
    cairo_t* cr = cairo_create(cr_s);

    if(save_png) {
        cairo_save(cr);
        cairo_set_source_rgb(cr, 1,1,1);
        cairo_paint(cr);
        cairo_restore(cr);
    }
    if(the_toy != NULL) {
        std::ostringstream * notify = new std::ostringstream;
        std::ostringstream * timer_stream = new std::ostringstream;
        the_toy->draw(cr, notify, width, height, true, timer_stream);
        delete notify;
        delete timer_stream;
    }

    cairo_show_page(cr);
    if(save_png)
        cairo_surface_write_to_png(cr_s, filename);
    cairo_destroy (cr);
    cairo_surface_destroy (cr_s);
}

void save_cairo(GSimpleAction *, GVariant *, gpointer) {
    GtkWidget* d = gtk_file_chooser_dialog_new(
        "Save file as svg, pdf or png", GTK_WINDOW(the_window), GTK_FILE_CHOOSER_ACTION_SAVE,
        "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
    if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
        const gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));
        write_image(filename);
    }
    gtk_widget_destroy(d);
}

static gint delete_event(GtkWidget*, GdkEventAny*, gpointer) {
    quit(nullptr, nullptr, nullptr);
    return FALSE;
}


static void toggle_action(GSimpleAction *action, GVariant *, gpointer) {
    GVariant *state = g_action_get_state(G_ACTION(action));
    g_action_change_state(G_ACTION(action), g_variant_new_boolean(!g_variant_get_boolean(state)));
    g_variant_unref(state);
}

static void set_show_timings(GSimpleAction *action, GVariant *variant, gpointer) {
    the_toy->show_timings = g_variant_get_boolean(variant);
    g_simple_action_set_state(action, variant);
}

static gboolean draw_callback(GtkWidget *widget, cairo_t *cr)
{
    int width = gdk_window_get_width(gtk_widget_get_window(widget));
    int height = gdk_window_get_height(gtk_widget_get_window(widget));

    std::ostringstream notify;

    static bool resized = false;
    if(!resized) {
	Geom::Rect alloc_size(Geom::Interval(0, width),
			      Geom::Interval(0, height));
	if(the_toy != NULL)
	    the_toy->resize_canvas(alloc_size);
	resized = true;
    }
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_set_source_rgba(cr,1,1,1,1);
    cairo_fill(cr);
    if (the_toy != NULL) {
        std::ostringstream * timer_stream = new std::ostringstream;
        
        if (the_toy->spool_file) {
            the_toy->save(the_toy->spool_file);
        }

        the_toy->draw(cr, &notify, width, height, false, timer_stream);
        delete timer_stream;
    }

    return TRUE;
}

static gint mouse_motion_event(GtkWidget* widget, GdkEventMotion* e, gpointer data) {
    (void)(data);
    (void)(widget);

    if(the_toy != NULL) the_toy->mouse_moved(e);

    return FALSE;
}

static gint mouse_event(GtkWidget* widget, GdkEventButton* e, gpointer data) {
    (void)(data);
    (void)(widget);

    if(the_toy != NULL) the_toy->mouse_pressed(e);

    return FALSE;
}

static gint scroll_event(GtkWidget* widget, GdkEventScroll* e, gpointer data) {
    (void)(data);
    (void)(widget);
    if(the_toy != NULL) the_toy->scroll(e);

    return FALSE;
}

static gint mouse_release_event(GtkWidget* widget, GdkEventButton* e, gpointer data) {
    (void)(data);
    (void)(widget);

    if(the_toy != NULL) the_toy->mouse_released(e);

    return FALSE;
}

static gint key_press_event(GtkWidget *widget, GdkEventKey *e, gpointer data) {
    (void)(data);
    (void)(widget);

    if(the_toy != NULL) the_toy->key_hit(e);

    return FALSE;
}

static gint size_allocate_event(GtkWidget* widget, GtkAllocation *allocation, gpointer data) {
    (void)(data);
    (void)(widget);

    Geom::Rect alloc_size(Geom::Interval(allocation->x, allocation->x+ allocation->width),
			  Geom::Interval(allocation->y, allocation->y+allocation->height));
    if(the_toy != NULL) the_toy->resize_canvas(alloc_size);

    return FALSE;
}


const char *the_builder_xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <menu id="menu">
    <submenu>
      <attribute name="label">File</attribute>
      <section>
        <item>
          <attribute name="label">Open Handles...</attribute>
          <attribute name="action">app.open-handles</attribute>
        </item>
        <item>
          <attribute name="label">Save Handles...</attribute>
          <attribute name="action">app.save-handles</attribute>
        </item>
      </section>
      <section>
        <item>
          <attribute name="label">Save as SVG of PDF...</attribute>
          <attribute name="action">app.save-image</attribute>
        </item>
      </section>
      <section>
        <item>
          <attribute name="label">Show Timings</attribute>
          <attribute name="action">app.show-timings</attribute>
        </item>
        <item>
          <attribute name="label">Quit</attribute>
          <attribute name="action">app.quit</attribute>
        </item>
      </section>
    </submenu>
    <submenu>
      <attribute name="label">Help</attribute>
      <item>
        <attribute name="label">About...</attribute>
        <attribute name="action">app.about</attribute>
      </item>
    </submenu>
  </menu>
</interface>
)xml";

static GActionEntry the_actions[] =
{
    {"open-handles", open_handles, nullptr, nullptr, nullptr},
    {"save-handles", save_handles, nullptr, nullptr, nullptr},
    {"save-image", save_cairo, nullptr, nullptr, nullptr},
    {"show-timings", toggle_action, nullptr, "false", set_show_timings},
    {"quit", quit, nullptr, nullptr, nullptr},
    {"about", show_about_dialog, nullptr, nullptr, nullptr},
};

static GOptionEntry const the_options[] = {
    {"handles", 'h', G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME, &arg_handles_filename,
     "Load handle positions from given file", "FILE"},
    {"spool", 'm', G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME, &arg_spool_filename,
     "Record all interaction to the given file", "FILE"},
    {"screenshot", 's', G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME, &arg_screenshot_filename,
     "Take screenshot and exit", nullptr},
    {G_OPTION_REMAINING, 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME_ARRAY, &arg_extra_files,
     "Additional data files", "FILES..."},
    {nullptr, 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, nullptr, nullptr, nullptr},
};

static void activate(GApplication *app, gpointer);
static void startup(GApplication *app, gpointer);

void init(int argc, char **argv, Toy* t, int width, int height) {
    the_toy = t;
    the_requested_width = width;
    the_requested_height = height;

    std::string app_name = "org.inkscape.lib2geom.toy.";
    char const *dir_pos = strrchr(argv[0], G_DIR_SEPARATOR);
    std::string argv_name = dir_pos ? dir_pos + 1 : argv[0];

    // Erase extension for Windows
    size_t dot_pos = argv_name.rfind('.');
    if (dot_pos != std::string::npos) {
        argv_name.erase(dot_pos);
    }
    the_toy->name = argv_name;
    app_name += argv_name;

    GtkApplication* app = gtk_application_new(app_name.c_str(), G_APPLICATION_FLAGS_NONE);
    g_application_add_main_option_entries(G_APPLICATION(app), the_options);
    g_action_map_add_action_entries(G_ACTION_MAP(app), the_actions, G_N_ELEMENTS(the_actions), nullptr);
    g_signal_connect(G_OBJECT(app), "startup", G_CALLBACK(startup), nullptr);
    g_signal_connect(G_OBJECT(app), "activate", G_CALLBACK(activate), nullptr);

    g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
}

static void startup(GApplication *app, gpointer) {
    GtkBuilder *builder = gtk_builder_new_from_string(the_builder_xml, -1);
    GMenuModel *menu = G_MENU_MODEL(gtk_builder_get_object(builder, "menu"));
    gtk_application_set_menubar(GTK_APPLICATION(app), menu);
    g_object_unref(builder);
}

static void activate(GApplication *app, gpointer) {
    if (arg_spool_filename) {
        the_toy->spool_file = fopen(arg_spool_filename, "w");
    }

    int const emulated_argc = arg_extra_files ? g_strv_length(arg_extra_files) + 1 : 1;
    gchar const **emulated_argv = new gchar const*[emulated_argc];
    emulated_argv[0] = the_toy->name.c_str();
    for (int i = 1; i < emulated_argc; ++i) {
        emulated_argv[i] = arg_extra_files[i-1];
    }
    the_toy->first_time(emulated_argc, const_cast<char**>(emulated_argv));
    delete[] emulated_argv;

    if (arg_handles_filename) {
        FILE *handles_file = fopen(arg_handles_filename, "r");
        the_toy->load(handles_file);
        fclose(handles_file);
    }

    if (arg_screenshot_filename) {
        write_image(arg_screenshot_filename);
        g_application_quit(app);
        return;
    }

    the_window = GTK_APPLICATION_WINDOW(gtk_application_window_new(GTK_APPLICATION(g_application_get_default())));
    gtk_window_set_title(GTK_WINDOW(the_window), the_toy->name.c_str());
    g_signal_connect(G_OBJECT(the_window), "delete_event", G_CALLBACK(delete_event), NULL);

    the_canvas = gtk_drawing_area_new();
    gtk_widget_add_events(the_canvas, (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_KEY_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK));
    g_signal_connect(G_OBJECT(the_canvas), "draw", G_CALLBACK(draw_callback), 0);
    g_signal_connect(G_OBJECT(the_canvas), "scroll-event", G_CALLBACK(scroll_event), 0);
    g_signal_connect(G_OBJECT(the_canvas), "button-press-event", G_CALLBACK(mouse_event), 0);
    g_signal_connect(G_OBJECT(the_canvas), "button-release-event", G_CALLBACK(mouse_release_event), 0);
    g_signal_connect(G_OBJECT(the_canvas), "motion-notify-event", G_CALLBACK(mouse_motion_event), 0);
    g_signal_connect(G_OBJECT(the_canvas), "key-press-event", G_CALLBACK(key_press_event), 0);
    g_signal_connect(G_OBJECT(the_canvas), "size-allocate", G_CALLBACK(size_allocate_event), 0);

    gtk_container_add(GTK_CONTAINER(the_window), the_canvas);
    gtk_window_set_default_size(GTK_WINDOW(the_window), the_requested_width, the_requested_height);
    gtk_widget_show_all(GTK_WIDGET(the_window));

    // Make sure the canvas can receive key press events.
    gtk_widget_set_can_focus(the_canvas, TRUE);
    gtk_widget_grab_focus(the_canvas);
    assert(gtk_widget_is_focus(the_canvas));
}


void Toggle::draw(cairo_t *cr, bool /*annotes*/) {
    cairo_pattern_t* source = cairo_get_source(cr);
    double rc, gc, bc, aa;
    cairo_pattern_get_rgba(source, &rc, &gc, &bc, &aa);
    cairo_set_source_rgba(cr,0,0,0,1);
    cairo_rectangle(cr, bounds.left(), bounds.top(),
		    bounds.width(), bounds.height());
    if(on) {
	cairo_fill(cr);
	cairo_set_source_rgba(cr,1,1,1,1);
    } //else cairo_stroke(cr);
    cairo_stroke(cr);
    draw_text(cr, bounds.corner(0) + Geom::Point(5,2), text);
    cairo_set_source_rgba(cr, rc, gc, bc, aa);
}

void Toggle::toggle() {
    on = !on;
}
void Toggle::set(bool state) {
    on = state;
}


void Toggle::handle_click(GdkEventButton* e) {
    if(bounds.contains(Geom::Point(e->x, e->y)) && e->button == 1) toggle();
}

void* Toggle::hit(Geom::Point mouse)
{
    if (bounds.contains(mouse))
    {
        toggle();
        return this;
    }
    return 0;
}

void toggle_events(std::vector<Toggle> &ts, GdkEventButton* e) {
    for(unsigned i = 0; i < ts.size(); i++) ts[i].handle_click(e);
}

void draw_toggles(cairo_t *cr, std::vector<Toggle> &ts) {
    for(unsigned i = 0; i < ts.size(); i++) ts[i].draw(cr);
}



Slider::value_type Slider::value() const
{
    Slider::value_type v = m_handle.pos[m_dir] - m_pos[m_dir];
    v =  ((m_max - m_min) / m_length) * v;
    //std::cerr << "v : " << v << std::endl;
    if (m_step != 0)
    {
        int k = std::floor(v / m_step);
        v = k * m_step;
    }
    v = v + m_min;
    //std::cerr << "v : " << v << std::endl;
    return v;
}

void Slider::value(Slider::value_type _value)
{
    if ( _value < m_min ) _value = m_min;
    if ( _value > m_max ) _value = m_max;
    if (m_step != 0)
    {
        _value = _value - m_min;
        int k = std::floor(_value / m_step);
        _value = k * m_step + m_min;
    }
    m_handle.pos[m_dir]
           = (m_length / (m_max - m_min)) * (_value - m_min) + m_pos[m_dir];
}

void Slider::max_value(Slider::value_type _value)
{
    Slider::value_type v = value();
    m_max = _value;
    value(v);
}

void Slider::min_value(Slider::value_type _value)
{
    Slider::value_type v = value();
    m_min = _value;
    value(v);
}

// dir = X horizontal slider dir = Y vertical slider
void Slider::geometry( Geom::Point _pos,
                       Slider::value_type _length,
                       Geom::Dim2 _dir )
{
    Slider::value_type v = value();
    m_pos = _pos;
    m_length = _length;
    m_dir = _dir;
    Geom::Dim2 fix_dir = static_cast<Geom::Dim2>( (m_dir + 1) % 2 );
    m_handle.pos[fix_dir] = m_pos[fix_dir];
    value(v);
}

void Slider::draw(cairo_t* cr, bool annotate)
{
    cairo_pattern_t* source = cairo_get_source(cr);
    double rc, gc, bc, aa;
    cairo_pattern_get_rgba(source, &rc, &gc, &bc, &aa);
    double lw = cairo_get_line_width(cr);
    std::ostringstream os;
    os << m_label << ": " << (*m_formatter)(value());
    cairo_set_source_rgba(cr, 0.1, 0.1, 0.7, 1.0);
    cairo_set_line_width(cr, 0.7);
    m_handle.draw(cr, annotate);
    cairo_stroke(cr);
    cairo_set_source_rgba(cr, 0.1, 0.1, 0.1, 1.0);
    cairo_set_line_width(cr, 0.4);
    m_handle.draw(cr, annotate);
    cairo_move_to(cr, m_pos[Geom::X], m_pos[Geom::Y]);
    Geom::Point offset;
    if ( m_dir == Geom::X )
    {
        cairo_rel_line_to(cr, m_length, 0);
        offset = Geom::Point(0,5);
    }
    else
    {
        cairo_rel_line_to(cr, 0, m_length);
        offset = Geom::Point(5,0);
    }
    cairo_stroke(cr);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    draw_text(cr, m_pos + offset, os.str().c_str());
    cairo_set_source_rgba(cr, rc, gc, bc, aa);
    cairo_set_line_width(cr, lw);
}

void Slider::move_to(void* hit, Geom::Point om, Geom::Point m)
{
    // fix_dir == ! m_dir
    Geom::Dim2 fix_dir = static_cast<Geom::Dim2>( (m_dir + 1) % 2 );
    m[fix_dir] = m_pos[fix_dir];
    double diff = m[m_dir] - m_pos[m_dir];
//        if (m_step != 0)
//        {
//            double step =  (m_step * m_length) / (m_max - m_min) ;
//            int k = std::floor(diff / step);
//            double v = k * step;
//            m[m_dir] = v + m_pos[m_dir];
//        }
    if ( diff < 0 ) m[m_dir] = m_pos[m_dir];
    if ( diff > m_length ) m[m_dir] = m_pos[m_dir] + m_length;
    m_handle.move_to(hit, om, m);
}



void PointHandle::draw(cairo_t *cr, bool /*annotes*/) {
    draw_circ(cr, pos);
}

void* PointHandle::hit(Geom::Point mouse) {
    if(Geom::distance(mouse, pos) < 5)
	return this;
    return 0;
}

void PointHandle::move_to(void* /*hit*/, Geom::Point /*om*/, Geom::Point m) {
    pos = m;
}

void PointHandle::load(FILE* f) {
    pos = read_point(f);
}

void PointHandle::save(FILE* f) {
    fprintf(f, "%lf %lf\n", pos[0], pos[1]);
}

void PointSetHandle::draw(cairo_t *cr, bool annotes) {
    for(unsigned i = 0; i < pts.size(); i++) {
	draw_circ(cr, pts[i]);
        if(annotes) draw_number(cr, pts[i], i, name);
    }
}

void* PointSetHandle::hit(Geom::Point mouse) {
    for(unsigned i = 0; i < pts.size(); i++) {
	if(Geom::distance(mouse, pts[i]) < 5)
	    return (void*)(&pts[i]);
    }
    return 0;
}

void PointSetHandle::move_to(void* hit, Geom::Point /*om*/, Geom::Point m) {
    if(hit) {
	*(Geom::Point*)hit = m;
    }
}

void PointSetHandle::load(FILE* f) {
    int n = 0;
    assert(1 == fscanf(f, "%d\n", &n));
    pts.clear();
    for(int i = 0; i < n; i++) {
	pts.push_back(read_point(f));
    }
}

void PointSetHandle::save(FILE* f) {
    fprintf(f, "%d\n", (int)pts.size());
    for(unsigned i = 0; i < pts.size(); i++) {
	fprintf(f, "%lf %lf\n", pts[i][0], pts[i][1]);
    }
}

#include <2geom/bezier-to-sbasis.h>

Geom::D2<Geom::SBasis> PointSetHandle::asBezier() {
    return handles_to_sbasis(pts.begin(), size()-1);
}

void RectHandle::draw(cairo_t *cr, bool /*annotes*/) {
    cairo_rectangle(cr, pos);
    cairo_stroke(cr);
    if(show_center_handle) {
        draw_circ(cr, pos.midpoint());
    }
    draw_text(cr, pos.corner(0), name);
}

void* RectHandle::hit(Geom::Point mouse) {
    if(show_center_handle) {
	if(Geom::distance(mouse, pos.midpoint()) < 5)
            return (void*)(intptr_t)1;
    }
    for(int i = 0; i < 4; i++) {
	if(Geom::distance(mouse, pos.corner(i)) < 5)
            return (void*)(intptr_t)(2+i);
    }
    for(int i = 0; i < 4; i++) {
        Geom::LineSegment ls(pos.corner(i), pos.corner(i+1));
	if(Geom::distance(ls.pointAt(ls.nearestTime(mouse)),mouse) < 5)
            return (void*)(intptr_t)(6+i);
    }
    return 0;
    
}

void RectHandle::move_to(void* hit, Geom::Point om, Geom::Point m) {
    using Geom::X;
    using Geom::Y;

    unsigned h = (unsigned)(uintptr_t)(hit);
    if(h == 1)
        pos += (m-om);
    else if(h >= 2 and h <= 5) {// corners
        int xi = (h-2)& 1;
        int yi = (h-2)&2;
        if(yi)
            xi = 1-xi; // clockwise
        if (xi) {
            pos[X].setMax(m[0]);
        } else {
            pos[X].setMin(m[0]);
        }
        if (yi/2) {
            pos[Y].setMax(m[1]);
        } else {
            pos[Y].setMax(m[1]);
        }
    } else if(h >= 6 and h <= 9) {// edges
        int side, d;
        switch(h-6) {
            case 0: d = 1; side = 0; break;
            case 1: d = 0; side = 1; break;
            case 2: d = 1; side = 1; break;
            case 3: d = 0; side = 0; break;
        }
        if (side) {
            pos[d].setMax(m[d]);
        } else {
            pos[d].setMin(m[d]);
        }
    }
}

void RectHandle::load(FILE* f) {
    assert(0 == fscanf(f, "r\n"));
    for(int i = 0; i < 2; i++) {
	pos[i] = read_interval(f);
    }
 
}

void RectHandle::save(FILE* f) {
    fprintf(f, "r\n");
    for(unsigned i = 0; i < 2; i++) {
	fprintf(f, "%lf %lf\n", pos[i].min(), pos[i].max());
    }
}



/*
	Local Variables:
	mode:c++
	c-file-style:"stroustrup"
	c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
	indent-tabs-mode:nil
	fill-column:99
	End:
      */
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
