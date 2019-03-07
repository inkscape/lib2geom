
#ifndef _2GEOM_TOY_FRAMEWORK2_H_
#define _2GEOM_TOY_FRAMEWORK2_H_



#include <iostream>
#include <sstream>
#include <gtk/gtk.h>
#include <cairo.h>
#include <vector>
#include <string>

#include <assert.h>
#include <2geom/exception.h>
#include <2geom/point.h>
#include <2geom/geom.h>
#include <2geom/sbasis.h>
#include <2geom/d2.h>
#include <sched.h>
#include <toys/path-cairo.h>

using std::vector;

//Utility functions
double uniform();

void draw_text(cairo_t *cr, Geom::Point pos, const char* txt, bool bottom = false, const char* fontdesc = "Sans");
void draw_text(cairo_t *cr, Geom::Point pos, const std::string& txt, bool bottom = false, const std::string& fontdesc = "Sans");
void draw_number(cairo_t *cr, Geom::Point pos, int num, std::string name=std::string(), bool bottom = true);
void draw_number(cairo_t *cr, Geom::Point pos, unsigned num, std::string name=std::string(), bool bottom = true);
void draw_number(cairo_t *cr, Geom::Point pos, double num, std::string name=std::string(), bool bottom = true);

struct colour{
    double r,g,b,a;
    colour(double r, double g, double b, double a) : r(r), g(g), b(b), a(a) {}
    static colour from_hsv( float H,          // hue shift (radians)
                            float S,          // saturation shift (scalar)
                            float V,          // value multiplier (scalar)
                            float A
			    );
    static colour from_hsl( float H,          // hue shift (radians)
                            float S,          // saturation shift (scalar)
                            float L,          // value multiplier (scalar)
                            float A
			    );
};
void cairo_set_source_rgba(cairo_t* cr, colour c);

class Handle{
public:
    std::string name;
    float rgb[3];
    Handle() {rgb[0] = rgb[1] = rgb[2] = 0;}
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
    Toggle() : bounds(Geom::Point(0,0), Geom::Point(0,0)), text(""), on(false) {}
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


template< typename T>
class VectorHandle : public Handle
{
  public:
    VectorHandle()
        : m_handles()
    {
    }
    virtual void draw(cairo_t *cr, bool annotes=false)
    {
        for (iterator it = m_handles.begin(); it != m_handles.end(); ++it)
            it->draw(cr, annotes);
    }

    virtual void* hit(Geom::Point pos)
    {
        void* result = NULL;
        for (iterator it = m_handles.begin(); it != m_handles.end(); ++it)
        {
            result = it->hit(pos);
            if (result != NULL)  break;
        }
        return result;
    }

    virtual void move_to(void* hit, Geom::Point om, Geom::Point m)
    {
        if (hit != NULL)
        {
            static_cast<T*>(hit)->move_to(hit, om, m);
        }
    }

    virtual void load(FILE* f)
    {
        for (iterator it = m_handles.begin(); it != m_handles.end(); ++it)
            it->load(f);
    }

    virtual void save(FILE* f)
    {
        for (iterator it = m_handles.begin(); it != m_handles.end(); ++it)
            it->save(f);
    }

    void clear()
    {
        m_handles.clear();
    }

    void reserve(size_t sz)
    {
        m_handles.reserve(sz);
    }

    size_t size() const
    {
        return m_handles.size();
    }

    void push_back (const T& _handle)
    {
        m_handles.push_back(_handle);
    }

    const T& operator[] (size_t i) const
    {
        return m_handles.at(i);
    }

    T& operator[] (size_t i)
    {
        return m_handles.at(i);
    }

  private:
      typedef typename std::vector<T>::iterator iterator;
      std::vector<T> m_handles;
};  // end class VectorHandle


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

class RectHandle : public Handle{
public:
    RectHandle() {}
    RectHandle(Geom::Rect pos, bool show_center_handle) : pos(pos), show_center_handle(show_center_handle) {}
    Geom::Rect pos;
    bool show_center_handle;
    virtual void draw(cairo_t *cr, bool annotes = false);

    virtual void* hit(Geom::Point mouse);
    virtual void move_to(void* hit, Geom::Point om, Geom::Point m);
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

    Slider()
        : m_handle(), m_pos(Geom::Point(0,0)), m_length(1),
         m_min(0), m_max(1), m_step(0), m_dir(Geom::X),
         m_label(""), m_formatter(&default_formatter)
    {
        value(0);
    }

    // pass step = 0 for having a continuos value variation
    Slider( value_type _min, value_type _max, value_type _step,
            value_type _value, const std::string& _label = "" )
        : m_handle(),m_pos(Geom::Point(0,0)), m_length(1),
          m_min(_min), m_max(_max), m_step(_step), m_dir(Geom::X),
          m_label(_label), m_formatter(&default_formatter)
    {
        value(_value);
    }

    void set( value_type _min, value_type _max, value_type _step,
              value_type _value, const std::string& _label = "" )
    {
          m_min = _min;
          m_max = _max;
          m_step = _step;
          m_label = _label;
          value(_value);
    }

    value_type value() const;

    void value(value_type _value);

    value_type max_value() const
    {
        return m_max;
    }

    void max_value(value_type _value);

    value_type min_value() const
    {
        return m_min;
    }

    void min_value(value_type _value);

    // dir = X horizontal slider dir = Y vertical slider
    void geometry(Geom::Point _pos, value_type _length, Geom::Dim2 _dir = Geom::X);

    void draw(cairo_t* cr, bool annotate = false);

    void formatter( formatter_t _formatter )
    {
        m_formatter = _formatter;
    }

    void* hit(Geom::Point pos)
    {
        if (m_handle.hit(pos) != NULL)
            return this;
        return NULL;
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
    std::string m_label;
    formatter_t m_formatter;
};





class Toy {
public:
    vector<Handle*> handles;
    bool mouse_down = false;
    Geom::Point old_mouse_point;
    Handle* selected = nullptr;
    void* hit_data = nullptr;
    int canvas_click_button = 0;
    double notify_offset = 0.0;
    std::string name;
    bool show_timings = false;
    FILE* spool_file = nullptr; // if non-NULL we record all interactions to this file

    Toy() {}

    virtual ~Toy() {}

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int w, int h, bool save, std::ostringstream *timing_stream);

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





const long long NS_PER_SECOND = 1000000000LL;
const long long NS_PER_NS = 1;


class Timer{
public:
  Timer() {}
  // note that CPU time is tracked per-thread, so the timer is only useful
  // in the thread it was start()ed from.

  class Time{
  public:
    double value;
  Time(long long /*s*/, long long l) : value(l) {}
  Time(double v) : value(v) {}
    Time operator/(double iters) const {
      return Time(value / iters);
    }
  };

  void start() {
    nsec(start_time);
  }
  void lap(long long &ns) {
    nsec(ns);
    ns -= start_time;
  }
  Time lap() {
    long long ns;
    nsec(ns);
    return Time(start_time, ns - start_time);
  }
  void nsec(long long &ns) {
#if ! (defined(_WIN32) || defined(__APPLE__))
    clock_gettime(clock, &ts);
    ns = ts.tv_sec * NS_PER_SECOND + ts.tv_nsec / NS_PER_NS;
#else
    ns = 0;
#endif
  }
  /** Ask the OS nicely for a big time slice */
  void ask_for_timeslice() {
#ifndef _WIN32
    sched_yield();
#endif
  }
private:
  long long start_time;
#if ! (defined(_WIN32) || defined(__APPLE__))
  struct timespec ts;
#  ifdef _POSIX_THREAD_CPUTIME
  static const clockid_t clock = CLOCK_THREAD_CPUTIME_ID;
#  else
#    ifdef CLOCK_MONOTONIC
  static const clockid_t clock = CLOCK_MONOTONIC;
#    else
  static const clockid_t clock = CLOCK_REALTIME;
#    endif
#  endif
#endif
};

inline std::ostream& operator<<(std::ostream& o, Timer::Time const &t) {
    double tm = t.value;
    unsigned prefix = 0;
    char prefixes[] = "num kMGT";
    while(prefix < sizeof(prefixes) and tm > 1000) {
        tm /= 1000.0;
        prefix += 1;
    }
    o << tm << prefixes[prefix]  << "s";
  return o;
}



#endif // _2GEOM_TOY_FRAMEWORK2_H_
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
