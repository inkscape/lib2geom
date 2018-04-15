#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/svg-path-parser.h>
#include <2geom/sbasis-math.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

//Random walkers toy, written by mgsloan, initially for a school video proj.

using namespace Geom;

static void dot_plot(cairo_t *cr, Piecewise<D2<SBasis> > const &M, double min, double max, double space=10){
    for( double t = min; t < max; t += space) {
        Point pos = M(t), perp = M.valueAndDerivatives(t, 2)[1].cw() * 3;
        draw_line_seg(cr, pos + perp, pos - perp);
    }
    cairo_stroke(cr);
}

D2<SBasis> random_d2() {
    D2<SBasis> ret(SBasis(6, Linear()),
                   SBasis(6, Linear()));
    ret[0][0] = Linear(uniform()*720, uniform()*720);
    ret[1][0] = Linear(uniform()*480, uniform()*480);
    
    int mul = 1;
    for(int i = 1; i < 6; i++) {
        ret[0][i] = Linear(uniform()*2000*mul - 1000, uniform()*2000*mul - 1000);
        ret[1][i] = Linear(uniform()*2000*mul - 1000, uniform()*2000*mul - 1000);
        mul*=2;
    }
    return ret;
}

class Worm {
    Piecewise<D2<SBasis> > path;
    int spawn_time, last_time;
    double red, green, blue, length;
  public:
    void tele(int t) {
        Piecewise<D2<SBasis> > new_path(portion(path, 0, t - last_time));
        new_path.push(random_d2(), path.domain().max()+1);
        path = arc_length_parametrization(new_path);
    }
    void add_section(const D2<SBasis> x) {
        Piecewise<D2<SBasis> > new_path(path);
        D2<SBasis> seg(x);
        seg[0][0][0] = path.segs.back()[0][0][1];
        seg[1][0][0] = path.segs.back()[1][0][1];
        new_path.push(seg, path.domain().max()+1);
        path = arc_length_parametrization(new_path);
    }
    Worm (int t, double r, double g, double b, double l) : spawn_time(t), last_time(t), red(r), green(g), blue(b), length(l) {
        path = Piecewise<D2<SBasis> >(random_d2());
        add_section(random_d2());
    }
    void draw(cairo_t *cr, int t) {
        if(t - last_time > path.domain().max()) add_section(random_d2());
        if(t - last_time - length > path.cuts[1]) {
            Piecewise<D2<SBasis> > new_path;
            new_path.push_cut(0);
            for(unsigned i = 1; i < path.size(); i++) {
                new_path.push(path[i], path.cuts[i+1] - path.cuts[1]);
            }
            last_time = t - length;
            path = new_path;
        }
        cairo_set_source_rgb(cr, red, green, blue);
        Piecewise<D2<SBasis> > port = portion(path, std::max(t - last_time - length, 0.), t - last_time);
        cairo_pw_d2_sb(cr, port);
        cairo_stroke(cr);
        
        double d = 4;
        cairo_set_dash(cr, &d, 1, 0);
        for(unsigned i = 1; i < path.size(); i++) {
            if(path[i].at0() != path[i-1].at1()) {
                draw_line_seg(cr, path[i].at0(), path[i-1].at1());
            }
        }
        cairo_stroke(cr);
        cairo_set_dash(cr, &d, 0, 0);
        
        cairo_set_source_rgb(cr, 0., 0., 1.);
        dot_plot(cr, path, std::max(t - last_time - length, 0.), t - last_time);
    }
    void reverse_direction(int t) {
        path = portion(path, 0, t - last_time);
        D2<SBasis> seg = random_d2(), last = path[path.size()-1];
        for(unsigned c = 0; c < 2; c++)
            for(unsigned d = 1; d < seg[c].size() && d < last[c].size(); d++)
                seg[c][d][0] = -last[c][d][1];
        add_section(seg);
    }
};

class Intro: public Toy {
    int t;
    vector<Worm> worms;
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        t++;
        if(t < 40 && t % 2 == 0) {
            worms.emplace_back(t, uniform(), uniform(), uniform(), uniform() * 200 + 50);
        }

        for(auto & worm : worms) {
            worm.draw(cr, t);
            if(uniform() > .999) worm.tele(t);
        }
        
        Toy::draw(cr, notify, width, height, save,timer_stream);
        redraw();
    }

    int should_draw_bounds() override { return 0; }
    
    public:
    Intro () {
        t = 0;
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new Intro(), 720, 480);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
