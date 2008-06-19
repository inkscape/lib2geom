
#ifndef _2GEOM_TOY_FRAMEWORK_H_
#define _2GEOM_TOY_FRAMEWORK_H_



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
    double notify_offset;

    Toy() { mouse_down = false; selected = -1; notify_offset = 0;}

    virtual ~Toy() {}

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int w, int h, bool save);

    virtual void mouse_moved(GdkEventMotion* e);
    virtual void mouse_pressed(GdkEventButton* e);
    virtual void mouse_released(GdkEventButton* e);

    virtual void key_hit(GdkEventKey *e) {}

    //Cheapo way of informing the framework what the toy would like drawn for it.
    virtual bool should_draw_numbers() { return true; }
    virtual int should_draw_bounds() { return 0; }
    
    virtual void first_time(int argc, char** argv) {}
    
    virtual void resize_canvas(Geom::Rect const & s) {}
};

//Framework Accesors
void redraw();
void take_screenshot(const char* file);
void init(int argc, char **argv, Toy *t, int width=600, int height=600);

struct Toggle {
  Geom::Rect bounds;
  const char* text;
  bool on;
  Toggle(const char* txt, bool v) : bounds(Geom::Point(0,0), Geom::Point(0,0)), text(txt), on(v) {}
  Toggle(Geom::Rect bnds, char* txt, bool v) : bounds(bnds), text(txt), on(v) {}
  void draw(cairo_t *cr);
  void toggle();
  void set(bool state);
  void handle_click(GdkEventButton* e);
};

void toggle_events(std::vector<Toggle> &ts, GdkEventButton* e);
void draw_toggles(cairo_t *cr, std::vector<Toggle> &ts);

#endif // _2GEOM_TOY_FRAMEWORK_H_
