
#include "path-cairo.h"
#include "toy-framework.cpp"

using namespace Geom;

struct Event {
    double x;
    unsigned ix;
    bool closing;
    Event(double pos, unsigned i, bool c) : x(pos), ix(i), closing(c) {}
    bool operator<(Event const &other) const {
        if(x < other.x) return true;
        if(x > other.x) return false;
        return closing < other.closing;
    }
};

std::vector<std::vector<unsigned> > sweep1(std::vector<Rect> rs) {
    std::vector<Event> events; events.reserve(rs.size()*2);
    std::vector<std::vector<unsigned> > pairs(rs.size());
    
    for(unsigned i = 0; i < rs.size(); i++) {
        events.push_back(Event(rs[i].left(), i, false));
        events.push_back(Event(rs[i].right(), i, true));
    }
    std::sort(events.begin(), events.end());

    std::vector<unsigned> open;
    for(unsigned i = 0; i < events.size(); i++) {
        unsigned ix = events[i].ix;
        if(events[i].closing) {
            std::vector<unsigned>::iterator iter = std::find(open.begin(), open.end(), ix);
            if(iter != open.end()) open.erase(iter);
        } else {
            for(unsigned j = 0; j < open.size(); j++)
                pairs[open[j]].push_back(ix);
            pairs[ix].insert(pairs[ix].end(), open.begin(), open.end());
            open.push_back(ix);
        }
    }
    return pairs;
}

class Sweep: public Toy {
    unsigned count_a, count_b;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        std::vector<Rect> rects_a, rects_b;

        for(unsigned i = 0; i < count_a; i++)
            rects_a.push_back(Rect(handles[i*2], handles[i*2+1]));

        for(unsigned i = 0; i < count_b; i++)
            rects_b.push_back(Rect(handles[i*2 + count_a*2], handles[i*2+1 + count_a*2]));
        
        /*std::vector<std::vector<unsigned> > res = sweep_bounds(rects_a, rects_b);
        cairo_set_line_width(cr,0.5);
        for(unsigned i = 0; i < res.size(); i++) {
            for(unsigned j = 0; j < res[i].size(); j++) {
                draw_line_seg(cr, rects_a[i].midpoint(), rects_b[j].midpoint());
                cairo_stroke(cr);
            }
        }*/
        
        std::vector<std::vector<unsigned> > res = sweep1(rects_a);
        cairo_set_line_width(cr,0.5);
        for(unsigned i = 0; i < res.size(); i++) {
            for(unsigned j = 0; j < res[i].size(); j++) {
                draw_line_seg(cr, rects_a[i].midpoint(), rects_a[j].midpoint());
                cairo_stroke(cr);
            }
        }
        
        cairo_set_line_width(cr,3);
        cairo_set_source_rgba(cr,1,0,0,1);
        for(unsigned i = 0; i < count_a; i++)
            cairo_rectangle(cr, rects_a[i].left(), rects_a[i].top(), rects_a[i].width(), rects_a[i].height());
        cairo_stroke(cr);
        
        /*
        cairo_set_source_rgba(cr,0,0,1,1);
        for(unsigned i = 0; i < count_b; i++)
            cairo_rectangle(cr, rects_b[i].left(), rects_b[i].top(), rects_b[i].width(), rects_b[i].height());
        cairo_stroke(cr);
        */
        
        Toy::draw(cr, notify, width, height, save);
    }

    public:
    Sweep () {
        count_a = count_b = 20;
        for(unsigned i = 0; i < (count_a + count_b); i++) {
            Point dim(uniform() * 90 + 10, uniform() * 90 + 10),
                  pos(uniform() * 500 + 50, uniform() * 500 + 50);
            handles.push_back(pos - dim/2);
            handles.push_back(pos + dim/2);
        }
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new Sweep());
    return 0;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
