#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <vector>

#include <gtk/gtk.h>
#include "interactive-bits.h"

using std::vector;

bool screen_lines = true;

std::vector<Geom::Point> handles;
Geom::Point *selected_handle;
Geom::Point old_mouse_point;

static GtkWidget *canvas;

class Toy {
public:
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int w, int h, bool save) {}

    virtual void mouse_pressed(GdkEventButton* e) {}
    virtual void mouse_released(GdkEventButton* e) {}
    virtual void mouse_moved(GdkEventMotion* e) {}

    virtual void key_pressed(GdkEventKey *e) {}
};

Toy* current_toy;

void init(int argc, char **argv, char *title, Toy *t);
void make_about();
double uniform();
void redraw();
