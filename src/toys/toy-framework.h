#include <iostream>
#include <sstream>
#include <gtk/gtk.h>
#include <cairo.h>
#include <vector>

#include "assert.h"
#include "interactive-bits.h"
#include "point.h"
#include "geom.h"
#include "d2.h"

using std::vector;

//Utility functions
double uniform();
void draw_text(cairo_t *cr, Geom::Point pos, const char* txt, bool bottom = false);
void draw_number(cairo_t *cr, Geom::Point pos, int num);

class Toy {
public:
    vector<Geom::Point> handles;
    bool mouse_down;
    Geom::Point old_mouse_point;
    int selected;

    Toy() { mouse_down = false; selected = -1;}

    virtual ~Toy() {}

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int w, int h, bool save);

    virtual void mouse_moved(GdkEventMotion* e);
    virtual void mouse_pressed(GdkEventButton* e);
    virtual void mouse_released(GdkEventButton* e);

    virtual void key_hit(GdkEventKey *e) {}

    //Cheapo way of informing the framework what the toy would like drawn for it.
    virtual bool should_draw_numbers() { return true; }
    virtual int should_draw_bounds() { return 1; }
    
    virtual void first_time(int argc, char** argv) {}
    
    virtual void resize_canvas(Geom::Rect const & s) {}
};

//Framework Accesors
void redraw();
void take_screenshot(const char* file);
void init(int argc, char **argv, Toy *t, int width=600, int height=600);
