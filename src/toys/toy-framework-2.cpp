#include "toy-framework-2.h"

#include "cairo-features.h"
#if CAIRO_HAS_PDF_SURFACE
#include "cairo-pdf.h"
#endif
#if CAIRO_HAS_SVG_SURFACE
#include "cairo-svg.h"
#endif

GtkWindow* window;
static GtkWidget *canvas;
Toy* current_toy;

//Utility functions

double uniform() {
    return double(rand()) / RAND_MAX;
}

void draw_text(cairo_t *cr, Geom::Point loc, const char* txt, bool bottom) {
    PangoLayout* layout = pango_cairo_create_layout (cr);
    pango_layout_set_text(layout, txt, -1);
    /*PangoFontDescription *font_desc = pango_font_description_new();
      pango_font_description_set_family(font_desc, "Sans");
      pango_layout_set_font_description(layout, font_desc);*/
    PangoRectangle logical_extent;
    pango_layout_get_pixel_extents(layout, NULL, &logical_extent);
    cairo_move_to(cr, loc - Geom::Point(0, bottom ? logical_extent.height : 0));
    pango_cairo_show_layout(cr, layout);
}

void draw_number(cairo_t *cr, Geom::Point pos, int num, std::string name=std::string()) {
    std::ostringstream number;
    if (name.size())
	number << name;
    number << num;
    draw_text(cr, pos, number.str().c_str(), true);
}

//Framework Accessors
void redraw() { gtk_widget_queue_draw(GTK_WIDGET(window)); }

#include <typeinfo>

Toy::Toy() : hit_data(0) {
    mouse_down = false; 
    selected = NULL; 
    notify_offset = 0;
}


void Toy::draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save)
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

    cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
    cairo_set_line_width (cr, 1);
    for(unsigned i = 0; i < handles.size(); i++) {
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

void Toy::mouse_released(GdkEventButton* e) {
    selected = NULL;
    hit_data = NULL;
    if(e->button == 1)
	mouse_down = false;
    redraw();
}

void Toy::load(FILE* f) {
    char data[1024];
    fscanf(f, "%s", data);
    if( strlen(data))
	name = data;
    for(unsigned i = 0; i < handles.size(); i++)
	handles[i]->load(f);
}

void Toy::save(FILE* f) {
	fprintf(f, "%s\n", name.c_str());
    for(unsigned i = 0; i < handles.size(); i++)
	handles[i]->save(f);
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

void open() {
    if(current_toy != NULL) {
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
    if(current_toy != NULL) {
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

void save_cairo() {
    GtkWidget* d = gtk_file_chooser_dialog_new("Save file as svg or pdf", window, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    if(gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_ACCEPT) {
        const char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(d));
        cairo_surface_t* cr_s;
        unsigned l = strlen(filename);
#if CAIRO_HAS_PDF_SURFACE
        if (l >= 4 && strcmp(filename + l - 4, ".pdf") == 0)
            cr_s = cairo_pdf_surface_create(filename, 600., 600.);
#endif
#if CAIRO_HAS_SVG_SURFACE
#if CAIRO_HAS_PDF_SURFACE        
        else
#endif
            cr_s = cairo_svg_surface_create(filename, 600., 600.);
#endif
        cairo_t* cr = cairo_create(cr_s);
        
        if(current_toy != NULL)
            current_toy->draw(cr, new std::ostringstream, 600, 600, true);

        cairo_show_page(cr);
        cairo_destroy (cr);
        cairo_surface_destroy (cr_s);
    }
    gtk_widget_destroy(d);
}

void take_screenshot(const char* filename) {
    int width = 256;
    int height = 256;
    gdk_drawable_get_size(canvas->window, &width, &height);

    cairo_surface_t* cr_s = cairo_image_surface_create ( CAIRO_FORMAT_ARGB32, width, height );
    cairo_t* cr = cairo_create(cr_s);
        
    if(current_toy != NULL)
	current_toy->draw(cr, new std::ostringstream, width, height, true);

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

static gboolean expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
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
	if(current_toy != NULL)
	    current_toy->resize_canvas(alloc_size);
	resized = true;
    }
    if(current_toy != NULL) current_toy->draw(cr, &notify, width, height, false);
    cairo_destroy(cr);

    return TRUE;
}

static gint mouse_motion_event(GtkWidget* widget, GdkEventMotion* e, gpointer data) {
    (void)(data);
    (void)(widget);

    if(current_toy != NULL) current_toy->mouse_moved(e);

    return FALSE;
}

static gint mouse_event(GtkWidget* widget, GdkEventButton* e, gpointer data) {
    (void)(data);
    (void)(widget);

    if(current_toy != NULL) current_toy->mouse_pressed(e);

    return FALSE;
}

static gint mouse_release_event(GtkWidget* widget, GdkEventButton* e, gpointer data) {
    (void)(data);
    (void)(widget);

    if(current_toy != NULL) current_toy->mouse_released(e);

    return FALSE;
}

static gint key_release_event(GtkWidget *widget, GdkEventKey *e, gpointer data) {
    (void)(data);
    (void)(widget);

    if(current_toy != NULL) current_toy->key_hit(e);

    return FALSE;
}

static gint size_allocate_event(GtkWidget* widget, GtkAllocation *allocation, gpointer data) {
    (void)(data);
    (void)(widget);

    Geom::Rect alloc_size(Geom::Interval(allocation->x, allocation->x+ allocation->width),
			  Geom::Interval(allocation->y, allocation->y+allocation->height));
    if(current_toy != NULL) current_toy->resize_canvas(alloc_size);

    return FALSE;
}

GtkItemFactoryEntry menu_items[] = {
    { (gchar*)"/_File",             NULL,           NULL,           0,  (gchar*)"<Branch>"                    },
    { (gchar*)"/File/_Open Handles",(gchar*)"<CTRL>O",      open,           0,  (gchar*)"<StockItem>", GTK_STOCK_OPEN },
    { (gchar*)"/File/_Save Handles",(gchar*)"<CTRL>S",      save,           0,  (gchar*)"<StockItem>", GTK_STOCK_SAVE_AS },
    { (gchar*)"/File/sep",          NULL,           NULL,           0,  (gchar*)"<Separator>"                 },
    { (gchar*)"/File/Save SVG/PDF", NULL,           save_cairo,     0,  (gchar*)"<StockItem>", GTK_STOCK_SAVE },
    { (gchar*)"/File/Save PNG",     NULL,           save_image,     0,  (gchar*)"<StockItem>", GTK_STOCK_SELECT_COLOR }, 
    { (gchar*)"/File/sep",          NULL,           NULL,           0,  (gchar*)"<Separator>"                 },
    { (gchar*)"/File/_Quit",        (gchar*)"<CTRL>Q",      gtk_main_quit,  0,  (gchar*)"<StockItem>", GTK_STOCK_QUIT },
    { (gchar*)"/_Help",             NULL,           NULL,           0,  (gchar*)"<LastBranch>"                },
    { (gchar*)"/Help/About",        NULL,           make_about,     0,  (gchar*)"<StockItem>", GTK_STOCK_ABOUT}
};
gint nmenu_items = 10;

void init(int argc, char **argv, Toy* t, int width, int height) {
    current_toy = t;
    gtk_init (&argc, &argv);
    
    gdk_rgb_init();

    t->first_time(argc, argv);
    
    window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    
    //Find last slash - remainder is title
    char* title = 0;
    for(char* ch = argv[0]; *ch != '\0'; ch++)
        if(*ch == '/') title = ch+1;

    gtk_window_set_title(GTK_WINDOW(window), title);

    //Creates the menu from the menu data above
    GtkAccelGroup* accel_group = gtk_accel_group_new();
    GtkItemFactory* item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);
    gtk_item_factory_create_items(item_factory, nmenu_items, menu_items, NULL);
    gtk_window_add_accel_group(window, accel_group);
    GtkWidget* menu = gtk_item_factory_get_widget(item_factory, "<main>");

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

    t->name = typeid(*t).name();
    gtk_main();
}


void Toggle::draw(cairo_t *cr) {
    cairo_set_source_rgba(cr,0,0,0,1);
    cairo_rectangle(cr, bounds.left(), bounds.top(),
		    bounds.width(), bounds.height());
    if(on) {
	cairo_fill(cr);
	cairo_set_source_rgba(cr,1,1,1,1);
    } else cairo_stroke(cr);
    draw_text(cr, bounds.corner(0) + Geom::Point(5,2), text);
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

void toggle_events(std::vector<Toggle> &ts, GdkEventButton* e) {
    for(unsigned i = 0; i < ts.size(); i++) ts[i].handle_click(e);
}

void draw_toggles(cairo_t *cr, std::vector<Toggle> &ts) {
    for(unsigned i = 0; i < ts.size(); i++) ts[i].draw(cr);
}


void PointHandle::draw(cairo_t *cr, bool annotes) {
    draw_circ(cr, pos);
}

void* PointHandle::hit(Geom::Point mouse) {
    if(Geom::distance(mouse, pos) < 5)
	return this;
    return 0;
}

void PointHandle::move_to(void* hit, Geom::Point om, Geom::Point m) {
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

void PointSetHandle::move_to(void* hit, Geom::Point om, Geom::Point m) {
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
    fprintf(f, "%d\n", pts.size());
    for(unsigned i = 0; i < pts.size(); i++) {
	fprintf(f, "%lf %lf\n", pts[i][0], pts[i][1]);
    }
}

#include "bezier-to-sbasis.h"

Geom::D2<Geom::SBasis> PointSetHandle::asBezier() {
    return handles_to_sbasis(pts.begin(), size()-1);
}

#if 1
#include "d2.h"
#include "sbasis.h"
#include "sbasis-2d.h"
#include "bezier-to-sbasis.h"
#include "choose.h"
#include "convex-cover.h"

#include "path.h"

#include "path-cairo.h"
#include "toy-framework.h"
#include "sbasis-math.h"

#include <vector>
using std::vector;
using namespace Geom;

extern unsigned total_steps, total_subs;

double& handle_to_sb(unsigned i, unsigned n, SBasis &sb) {
    assert(i < n);
    assert(n <= sb.size()*2);
    unsigned k = i;
    if(k >= n/2) {
        k = n - k - 1;
        return sb[k][1];
    } else
        return sb[k][0];
}

double handle_to_sb_t(unsigned i, unsigned n) {
    double k = i;
    if(i >= n/2)
        k = n - k - 1;
    double t = k/(2*k+1);
    if(i >= n/2)
        return 1 - t;
    else
        return t;
}

class Sb1d: public Toy {
    PointSetHandle psh;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
        
        if(!save) {
            for(unsigned i = 0; i < psh.pts.size(); i++) {
                psh.pts[i][0] = width*handle_to_sb_t(i, psh.pts.size())/2 + width/4;
            }
        }
        
        D2<SBasis> B;
        B[0] = Linear(width/4, 3*width/4);
        B[1].resize(psh.pts.size()/2);
        for(unsigned i = 0; i < B[1].size(); i++) {
            B[1][i] = Linear(0);
        }
        for(unsigned i = 0; i < psh.pts.size(); i++) {
            handle_to_sb(i, psh.pts.size(), B[1]) = 3*width/4 - psh.pts[i][1];
        }
        for(unsigned i = 1; i < B[1].size(); i++) {
            B[1][i] = B[1][i]*choose<double>(2*i+1, i);
        }
        
        Geom::Path pb;
        B[1] = SBasis(Linear(3*width/4)) - B[1];
        pb.append(B);
        pb.close(false);
        cairo_path(cr, pb);
    
        cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
        cairo_stroke(cr);
	
	B[1] /= 100;
	D2<Piecewise<SBasis> > arcurv(cos(B[1]),sin(B[1]));
	Piecewise< D2<SBasis> > pwc = sectionize(arcurv*10);
	pwc = integral(pwc)*30;
	pwc -= pwc.valueAt(0);
	pwc += Point(width/2, height/2);
	
	cairo_pw_d2(cr, pwc);
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_stroke(cr);

    
        Toy::draw(cr, notify, width, height, save);
    }
public:
Sb1d () : psh(PointSetHandle()) {
    psh.push_back(0,450);
	handles.push_back(&psh );
	for(unsigned i = 0; i < 4; i++)
	    psh.push_back(uniform()*400, uniform()*400);
	psh.push_back(0,450);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new Sb1d());
    return 0;
}

#endif/*
	Local Variables:
	mode:c++
	c-file-style:"stroustrup"
	c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
	indent-tabs-mode:nil
	fill-column:99
	End:
      */
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
