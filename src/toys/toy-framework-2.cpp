#include <cstring>
#include <cstdint>
#include <toys/toy-framework-2.h>

#include <cairo-features.h>
#if CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
#endif
#if CAIRO_HAS_SVG_SURFACE
#include <cairo-svg.h>
#endif

#include <getopt.h>

GtkWindow* window;
static GtkWidget *canvas;
Toy* current_toy;

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
    pango_layout_get_pixel_extents(layout, nullptr, &logical_extent);
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
void redraw() { gtk_widget_queue_draw(GTK_WIDGET(window)); }

#include <typeinfo>

Toy::Toy() {
    mouse_down = false;
    selected = nullptr;
    notify_offset = 0;
}


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
    for(auto & handle : handles) {
        cairo_set_source_rgb (cr, handle->rgb[0], handle->rgb[1], handle->rgb[2]);
	handle->draw(cr, should_draw_numbers());
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
    selected = nullptr;
    hit_data = nullptr;
    canvas_click_button = e->button;
    if(e->button == 1) {
        for(auto & handle : handles) {
    	    void * hit = handle->hit(mouse);
    	    if(hit) {
    		selected = handle;
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
    if(selected == nullptr) {
        Geom::Point mouse(e->x, e->y);
        canvas_click(mouse, canvas_click_button);
        canvas_click_button = 0;
    }
    selected = nullptr;
    hit_data = nullptr;
    if(e->button == 1)
	mouse_down = false;
    redraw();
}

void Toy::load(FILE* f) {
    char data[1024];
    if (fscanf(f, "%1024s", data)) {
        name = data;
    }
    for(auto & handle : handles) {
        handle->load(f);
    }
}

void Toy::save(FILE* f) {
	fprintf(f, "%s\n", name.c_str());
    for(auto & handle : handles)
	handle->save(f);
}

//Gui Event Callbacks

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

void open() {
    if(current_toy != nullptr) {
	GtkWidget* d = gtk_file_chooser_dialog_new("Open handle configuration", window, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
        if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
            const char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));
            FILE* f = fopen(filename, "r");
	    current_toy->load(f);
            fclose(f);
        }
        gtk_widget_destroy(d);
    }
}

void save() {
    if(current_toy != nullptr) {
        GtkWidget* d = gtk_file_chooser_dialog_new("Save handle configuration", window, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
        if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
            const char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));
            FILE* f = fopen(filename, "w");
	    current_toy->save(f);
            fclose(f);
        }
        gtk_widget_destroy(d);
    }
}

void save_cairo_backend(const char* filename) {
    cairo_surface_t* cr_s;
    unsigned l = strlen(filename);
    int width = 600;
    int height = 600;
    gdk_drawable_get_size(canvas->window, &width, &height);
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
    if(current_toy != nullptr) {
        std::ostringstream * notify = new std::ostringstream;
        std::ostringstream * timer_stream = new std::ostringstream;
        current_toy->draw(cr, notify, width, height, true, timer_stream);
        delete notify;
        delete timer_stream;
    }

    cairo_show_page(cr);
    if(save_png)
        cairo_surface_write_to_png(cr_s, filename);
    cairo_destroy (cr);
    cairo_surface_destroy (cr_s);
}

void save_cairo() {
    GtkWidget* d = gtk_file_chooser_dialog_new("Save file as svg, pdf or png", window, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
        const char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));
        save_cairo_backend(filename);
    }
    gtk_widget_destroy(d);
}

void take_screenshot(const char* filename) {
    int width = 256;
    int height = 256;
    gdk_drawable_get_size(canvas->window, &width, &height);

    cairo_surface_t* cr_s = cairo_image_surface_create ( CAIRO_FORMAT_ARGB32, width, height );
    cairo_t* cr = cairo_create(cr_s);

    if(current_toy != nullptr) {
        std::ostringstream * notify = new std::ostringstream;
        std::ostringstream * timer_stream = new std::ostringstream;
	current_toy->draw(cr, notify, width, height, true, timer_stream);
        delete notify;
        delete timer_stream;
    }

    cairo_show_page(cr);
    cairo_surface_write_to_png(cr_s, filename);
    cairo_destroy (cr);
    cairo_surface_destroy (cr_s);
}

void save_image() {
    GtkWidget* d = gtk_file_chooser_dialog_new("Save file as png", window, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
        take_screenshot(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d)));
    }
    gtk_widget_destroy(d);
}

static gint delete_event(GtkWidget* window, GdkEventAny* e, gpointer data) {
    (void)( window);
    (void)( e);
    (void)( data);

    gtk_main_quit();
    return FALSE;
}

void toggle_show_timings() {
    if(current_toy)
        current_toy->show_timings = !current_toy->show_timings;
}

static gboolean expose_event(GtkWidget *widget, GdkEventExpose */*event*/, gpointer data)
{
    (void)(data);
    cairo_t *cr = gdk_cairo_create(widget->window);

    int width = 256;
    int height = 256;
    gdk_drawable_get_size(widget->window, &width, &height);

    std::ostringstream notify;

    static bool resized = false;
    if(!resized) {
	Geom::Rect alloc_size(Geom::Interval(0, width),
			      Geom::Interval(0, height));
	if(current_toy != nullptr)
	    current_toy->resize_canvas(alloc_size);
	resized = true;
    }
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_set_source_rgba(cr,1,1,1,1);
    cairo_fill(cr);
    if(current_toy != nullptr) {
        std::ostringstream * timer_stream = new std::ostringstream;
        
        if(current_toy->spool_file) {
            current_toy->save(current_toy->spool_file);
        } else if(current_toy->to_load_file and !feof(current_toy->to_load_file)) {
            current_toy->load(current_toy->to_load_file);
            redraw();
        }

        current_toy->draw(cr, &notify, width, height, false, timer_stream);
        delete timer_stream;
    }
    cairo_destroy(cr);

    return TRUE;
}

static gint mouse_motion_event(GtkWidget* widget, GdkEventMotion* e, gpointer data) {
    (void)(data);
    (void)(widget);

    if(current_toy != nullptr) current_toy->mouse_moved(e);

    return FALSE;
}

static gint mouse_event(GtkWidget* widget, GdkEventButton* e, gpointer data) {
    (void)(data);
    (void)(widget);

    if(current_toy != nullptr) current_toy->mouse_pressed(e);

    return FALSE;
}

static gint scroll_event(GtkWidget* widget, GdkEventScroll* e, gpointer data) {
    (void)(data);
    (void)(widget);
    if(current_toy != nullptr) current_toy->scroll(e);

    return FALSE;
}

static gint mouse_release_event(GtkWidget* widget, GdkEventButton* e, gpointer data) {
    (void)(data);
    (void)(widget);

    if(current_toy != nullptr) current_toy->mouse_released(e);

    return FALSE;
}

static gint key_press_event(GtkWidget *widget, GdkEventKey *e, gpointer data) {
    (void)(data);
    (void)(widget);

    if(current_toy != nullptr) current_toy->key_hit(e);

    return FALSE;
}

static gint size_allocate_event(GtkWidget* widget, GtkAllocation *allocation, gpointer data) {
    (void)(data);
    (void)(widget);

    Geom::Rect alloc_size(Geom::Interval(allocation->x, allocation->x+ allocation->width),
			  Geom::Interval(allocation->y, allocation->y+allocation->height));
    if(current_toy != nullptr) current_toy->resize_canvas(alloc_size);

    return FALSE;
}

GtkItemFactoryEntry menu_items[] = {
    { (gchar*)"/_File",             nullptr,           nullptr,           0,  (gchar*)"<Branch>", nullptr                    },
    { (gchar*)"/File/_Open Handles",(gchar*)"<CTRL>O",      open,           0,  (gchar*)"<StockItem>", GTK_STOCK_OPEN },
    { (gchar*)"/File/_Save Handles",(gchar*)"<CTRL>S",      save,           0,  (gchar*)"<StockItem>", GTK_STOCK_SAVE_AS },
    { (gchar*)"/File/sep",          nullptr,           nullptr,           0,  (gchar*)"<Separator>", nullptr                 },
    { (gchar*)"/File/Save SVG or PDF", nullptr,           save_cairo,     0,  (gchar*)"<StockItem>", GTK_STOCK_SAVE },
    { (gchar*)"/File/sep",          nullptr,           nullptr,           0,  (gchar*)"<Separator>" , nullptr                },
    { (gchar*)"/File/_Show Timings",  nullptr,         toggle_show_timings,  0,  (gchar*)"<CheckItem>", nullptr },
    { (gchar*)"/File/_Quit",        (gchar*)"<CTRL>Q",      gtk_main_quit,  0,  (gchar*)"<StockItem>", GTK_STOCK_QUIT },
    { (gchar*)"/_Help",             nullptr,           nullptr,           0,  (gchar*)"<LastBranch>", nullptr                },
    { (gchar*)"/Help/About",        nullptr,           make_about,     0,  (gchar*)"<StockItem>", GTK_STOCK_ABOUT}
};
gint nmenu_items = 9;

void init(int argc, char **argv, Toy* t, int width, int height) {
    current_toy = t;
    gtk_init (&argc, &argv);

    gdk_rgb_init();

    int c;
    //int digit_optind = 0;

    int screenshot_only_type = 0;
    char *screenshot_output_name = nullptr;
    t->name = typeid(*t).name();

    while (1)
    {
        //int this_option_optind = optind ? optind : 1;
        int option_index = 0;

        c = getopt_long (argc, argv, "hs:m:d:012",
                         nullptr, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
            case 'h':
                t->to_load_file = fopen(argv[2], "r");
                break;

            case 's':
                screenshot_only_type = 1;
                screenshot_output_name = strdup(optarg);
                break;

            case 'm':
                current_toy->spool_file = fopen(strdup(optarg), "w");
                break;

            case 'c':
                printf ("option c with value `%s'\n", optarg);
                break;

            case 'd':
                printf ("option d with value `%s'\n", optarg);
                break;

            case '?':
                break;

            default:
                printf ("?? getopt returned character code 0%o ??\n", c);
        }
    }

    for(int i = 1; i <= argc-optind; i++)
        argv[i] = argv[i-1+optind];
    argc -= optind-1;

    t->first_time(argc, argv);

    if(t->to_load_file)
        t->load(t->to_load_file);

    if(screenshot_only_type > 0) {
        if(screenshot_output_name)
            save_cairo_backend(screenshot_output_name);
        return;
    }

    window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));

//Find last slash - remainder is title
    char* title = nullptr;
    for(char* ch = argv[0]; *ch != '\0'; ch++)
        if(*ch == '/') title = ch+1;

    gtk_window_set_title(GTK_WINDOW(window), title);

//Creates the menu from the menu data above
    GtkAccelGroup* accel_group = gtk_accel_group_new();
    GtkItemFactory* item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);
    gtk_item_factory_create_items(item_factory, nmenu_items, menu_items, nullptr);
    gtk_window_add_accel_group(window, accel_group);
    GtkWidget* menu = gtk_item_factory_get_widget(item_factory, "<main>");

//gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, TRUE);

    gtk_signal_connect(GTK_OBJECT(window), "delete_event", GTK_SIGNAL_FUNC(delete_event), NULL);

    gtk_widget_push_visual(gdk_rgb_get_visual());
    gtk_widget_push_colormap(gdk_rgb_get_cmap());
    canvas = gtk_drawing_area_new();

    gtk_widget_add_events(canvas, (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_KEY_PRESS_MASK | GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK));

    gtk_signal_connect(GTK_OBJECT (canvas), "expose_event", GTK_SIGNAL_FUNC(expose_event), 0);
    gtk_signal_connect(GTK_OBJECT(canvas), "scroll_event", GTK_SIGNAL_FUNC(scroll_event), 0);
    gtk_signal_connect(GTK_OBJECT(canvas), "button_press_event", GTK_SIGNAL_FUNC(mouse_event), 0);
    gtk_signal_connect(GTK_OBJECT (canvas), "button_release_event", GTK_SIGNAL_FUNC(mouse_release_event), 0);
    gtk_signal_connect(GTK_OBJECT (canvas), "motion_notify_event", GTK_SIGNAL_FUNC(mouse_motion_event), 0);
    gtk_signal_connect(GTK_OBJECT(canvas), "key_press_event", GTK_SIGNAL_FUNC(key_press_event), 0);
    gtk_signal_connect(GTK_OBJECT(canvas), "size-allocate", GTK_SIGNAL_FUNC(size_allocate_event), 0);

    gtk_widget_pop_colormap();
    gtk_widget_pop_visual();

    GtkWidget* box = gtk_vbox_new (FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    gtk_box_pack_start (GTK_BOX (box), menu, FALSE, FALSE, 0);

    GtkWidget* pain = gtk_vpaned_new();
    gtk_box_pack_start(GTK_BOX(box), pain, TRUE, TRUE, 0);
    gtk_paned_add1(GTK_PANED(pain), canvas);

    gtk_window_set_default_size(GTK_WINDOW(window), width, height);

    gtk_widget_show_all(GTK_WIDGET(window));

    // Make sure the canvas can receive key press events.
    GTK_WIDGET_SET_FLAGS(canvas, GTK_CAN_FOCUS);
    assert(GTK_WIDGET_CAN_FOCUS(canvas));
    gtk_widget_grab_focus(canvas);
    assert(gtk_widget_is_focus(canvas));

    gtk_main();
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
    return nullptr;
}

void toggle_events(std::vector<Toggle> &ts, GdkEventButton* e) {
    for(auto & t : ts) t.handle_click(e);
}

void draw_toggles(cairo_t *cr, std::vector<Toggle> &ts) {
    for(auto & t : ts) t.draw(cr);
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
    return nullptr;
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
    for(auto & pt : pts) {
	if(Geom::distance(mouse, pt) < 5)
	    return (void*)(&pt);
    }
    return nullptr;
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
    for(auto & pt : pts) {
	fprintf(f, "%lf %lf\n", pt[0], pt[1]);
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
    return nullptr;
    
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
