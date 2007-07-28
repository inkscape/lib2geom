#include "d2.h"
#include "sbasis.h"
#include "sbasis-geometric.h"
#include "svg-path-parser.h"
#include "sbasis-math.h"

#include "path-cairo.h"
#include "toy-framework.cpp"

using namespace Geom;

static void dot_plot(cairo_t *cr, Piecewise<D2<SBasis> > const &M, double max, double space=10){
    Piecewise<D2<SBasis> > Mperp = rot90(derivative(M)) * 3;
    for( double t = M.cuts.front(); t < max; t += space) {
        Point pos = M(t), perp = Mperp(t);
        draw_line_seg(cr, pos + perp, pos - perp);
    }
    cairo_stroke(cr);
}

D2<SBasis> random_d2() {
    D2<SBasis> ret;
    ret[0].push_back(Linear(uniform()*720, uniform()*720));
    ret[1].push_back(Linear(uniform()*480, uniform()*480));
    
    for(int i = 0; i < 5; i++) {
        ret[0].push_back(Linear(uniform()*2000 - 1000, uniform()*2000 - 1000));
        ret[1].push_back(Linear(uniform()*2000 - 1000, uniform()*2000 - 1000));
    }
    return ret;
}

class Intro: public Toy {
    double t;
    int count;
    vector<Piecewise<D2<SBasis> > > walkers, stayers;
    Piecewise<D2<SBasis> > title, norms;
    Piecewise<SBasis> patt;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        if(!save) {
            count++;
            t = count*5;
            char file[10];
            sprintf(file, "output/%04d.png", count);
            //take_screenshot(file);
        }
        for(int i = 0; i <= std::pow(2.7,t/1000.); i++) {
            cairo_set_source_rgb(cr, 0., 1., 0.);
            Piecewise<D2<SBasis> > port = portion(walkers[i], std::max(0.,t-200), t);
            cairo_pw_d2(cr, port);
            cairo_stroke(cr);
            cairo_set_source_rgb(cr, 0., 0., 1.);
            dot_plot(cr, port, t);
        }
        Piecewise<D2<SBasis> > output;
        double temp = 0;
        for (double offs=walkers[0].segT(t)*20; offs<title.cuts.back(); offs+=20){
            output.concat(compose(title,Linear(offs,offs+20))+patt*compose(norms,Linear(offs,offs+20)));
        }
        cairo_set_source_rgb(cr,1.,0.1,0.1);
        cairo_pw_d2(cr, output);
        cairo_stroke(cr);
        /*
        for(int i = 0; i <= stayers.size()*(t/1000.); i++) {
            cairo_set_source_rgb(cr, 0., 1., 0.);
            Piecewise<D2<SBasis> > port = portion(stayers[i], std::max(0.,t-200), t);
            cairo_pw_d2(cr, port);
            cairo_stroke(cr);
            cairo_set_source_rgb(cr, 0., 0., 1.);
            dot_plot(cr, port, t);
        }*/

        Toy::draw(cr, notify, width, height, save);
        redraw();
    }

    virtual bool should_draw_bounds() { return false; }
    
    public:
    Intro () {
        count = 0; t = 0;
        for(int n = 0; n < 20; n++) {
            Piecewise<D2<SBasis> > walker;
            walker.push_cut(0);
            D2<SBasis> prev = D2<SBasis>(Linear(200), Linear(200));
            for(unsigned i = 1; i < 10; i++) {
                D2<SBasis> seg = random_d2();
                seg[0][0][0] = prev[0][0][1];
                seg[1][0][0] = prev[1][0][1];
                walker.push(seg, i);
                prev = seg;
            }
            walker = arc_length_parametrization(walker);
            walkers.push_back(walker);
            /*Piecewise<D2<SBasis> > stayer;
            stayer.push_cut(0);
            double homex = uniform()*720, homey = uniform()*480;
            for(unsigned i = 1; i < 10; i++) {
                D2<SBasis> seg = random_d2();
                seg[0][0][0] = seg[0][0][1] = homex;
                seg[1][0][0] = seg[1][0][1] = homey;
                stayer.push(seg, i);
            }
            stayer = arc_length_parametrization(stayer);
            stayers.push_back(stayer);*/
        }
        title = arc_length_parametrization(paths_to_pw(read_svgd("ptitle.svgd"))*1.5-Point(175,0));
        norms = rot90(derivative(title));
        patt = sin(Linear(0,M_PI*2));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
