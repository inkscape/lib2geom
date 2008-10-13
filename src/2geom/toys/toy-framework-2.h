
#ifndef _2GEOM_TOY_FRAMEWORK2_H_
#define _2GEOM_TOY_FRAMEWORK2_H_



#include <iostream>
#include <sstream>
#include <gtk/gtk.h>
#include <cairo.h>
#include <vector>
#include <string>

#include <assert.h>
#include <2geom/toys/interactive-bits.h>
#include <2geom/point.h>
#include <2geom/geom.h>
#include <2geom/sbasis.h>
#include <2geom/d2.h>

using std::vector;

//Utility functions
double uniform();

void draw_text(cairo_t *cr, Geom::Point pos, const char* txt, bool bottom = false, const char* fontdesc = "Sans");
void draw_number(cairo_t *cr, Geom::Point pos, int num, std::string name=std::string());

class Handle{
public:
    std::string name;
    Handle() {}
    virtual ~Handle() {}
    virtual void draw(cairo_t *cr, bool annotes=false) = 0;
  
    virtual void* hit(Geom::Point pos) = 0;
    virtual void move_to(void* hit, Geom::Point om, Geom::Point m) = 0;
    virtual void load(FILE* f)=0;
    virtual void save(FILE* f)=0;
};

class Toggle : public Handle{
public:
    Geom::Rect bounds;
    const char* text;
    bool on;
    Toggle(const char* txt, bool v) : bounds(Geom::Point(0,0), Geom::Point(0,0)), text(txt), on(v) {}
    Toggle(Geom::Rect bnds, const char* txt, bool v) : bounds(bnds), text(txt), on(v) {}
    void draw(cairo_t *cr, bool annotes = false);
    void toggle();
    void set(bool state);
    void handle_click(GdkEventButton* e);
    void* hit(Geom::Point pos);
    void move_to(void* /*hit*/, Geom::Point /*om*/, Geom::Point /*m*/) { /* not implemented */ }
    void load(FILE* /*f*/) { /* not implemented */ }
    void save(FILE* /*f*/) { /* not implemented */ }
};





class PointHandle : public Handle{
public:
    PointHandle(double x, double y) : pos(x,y) {}
    PointHandle(Geom::Point pt) : pos(pt) {}
    PointHandle() {}
    Geom::Point pos;
    virtual void draw(cairo_t *cr, bool annotes = false);
  
    virtual void* hit(Geom::Point mouse);
    virtual void move_to(void* hit, Geom::Point om, Geom::Point m);
    virtual void load(FILE* f);
    virtual void save(FILE* f);
};

class PointSetHandle : public Handle{
public:
    PointSetHandle() {}
    std::vector<Geom::Point> pts;
    virtual void draw(cairo_t *cr, bool annotes = false);
  
    virtual void* hit(Geom::Point mouse);
    virtual void move_to(void* hit, Geom::Point om, Geom::Point m);
    void push_back(double x, double y) {pts.push_back(Geom::Point(x,y));}
    void push_back(Geom::Point pt) {pts.push_back(pt);}
    unsigned size() {return pts.size();}
    Geom::D2<Geom::SBasis> asBezier();
    virtual void load(FILE* f);
    virtual void save(FILE* f);
};


// used by Slider 
inline std::string default_formatter(double x)
{
    std::ostringstream os;
    os << x;
    return os.str();
}

class Slider : public Handle
{
  public:
      
    typedef std::string (*formatter_t) (double );
    typedef double value_type;
    
    // pass step = 0 for having a continuos value variation
    Slider( value_type _min, value_type _max, value_type _step, 
            value_type _value, const char * _label = "" )
        : m_handle(),m_pos(Geom::Point(0,0)), m_length(1), 
          m_min(_min), m_max(_max), m_step(_step), m_dir(Geom::X), 
          m_label(_label), m_formatter(&default_formatter)
    {
        value(_value);
    }
    
    value_type value() const;
    
    void value(value_type _value);
    
    // dir = X horizontal slider dir = Y vertical slider
    void geometry(Geom::Point _pos, value_type _length, Geom::Dim2 _dir = Geom::X);
    
    void draw(cairo_t* cr, bool annotate = false);
    
    void formatter( formatter_t _formatter )
    {
        m_formatter = _formatter;
    }
    
    void* hit(Geom::Point pos)
    {
        return m_handle.hit(pos);
    }
    
    void move_to(void* hit, Geom::Point om, Geom::Point m);
    
    void load(FILE* f) 
    {
        m_handle.load(f);
    }
    
    void save(FILE* f) 
    { 
        m_handle.save(f);
    }

  private:
    PointHandle m_handle;
    Geom::Point m_pos;
    value_type m_length;
    value_type m_min, m_max, m_step;
    int m_dir;
    const char* m_label;
    formatter_t m_formatter;
};




class Toy {
public:
    vector<Handle*> handles;
    bool mouse_down;
    Geom::Point old_mouse_point;
    Handle* selected;
    void* hit_data;
    int canvas_click_button;
    double notify_offset;
    std::string name;

    Toy();

    virtual ~Toy() {}

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int w, int h, bool save);

    virtual void mouse_moved(GdkEventMotion* e);
    virtual void mouse_pressed(GdkEventButton* e);
    virtual void mouse_released(GdkEventButton* e);
    virtual void canvas_click(Geom::Point at, int button);
    virtual void scroll(GdkEventScroll* e);

    virtual void key_hit(GdkEventKey */*e*/) {}

    //Cheapo way of informing the framework what the toy would like drawn for it.
    virtual bool should_draw_numbers() { return true; }
    virtual int should_draw_bounds() { return 0; }
    
    virtual void first_time(int /*argc*/, char** /*argv*/) {}
    
    virtual void resize_canvas(Geom::Rect const & /*s*/) {}
    virtual void load(FILE* f);
    virtual void save(FILE* f);
};

//Framework Accesors
void redraw();
void take_screenshot(const char* file);
void init(int argc, char **argv, Toy *t, int width=600, int height=600);

void toggle_events(std::vector<Toggle> &ts, GdkEventButton* e);
void draw_toggles(cairo_t *cr, std::vector<Toggle> &ts);
Geom::Point read_point(FILE* f);

#endif // _2GEOM_TOY_FRAMEWORK2_H_
