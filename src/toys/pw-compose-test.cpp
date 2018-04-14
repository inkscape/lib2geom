#include <2geom/piecewise.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

using namespace Geom;

class PwToy: public Toy {
public:
    vector<PointSetHandle*> pw_handles;
    PointSetHandle slids;
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override {
        cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
        cairo_set_line_width (cr, 1);
       
        D2<Piecewise<SBasis> > pws;
        for(unsigned i = 0; i < pw_handles.size(); i++) {
            D2<SBasis> foo = pw_handles[i]->asBezier();
            cairo_d2_sb(cr, foo);
            for(unsigned d = 0; d < 2; d++) {
                pws[d].cuts.push_back(150*i);
                pws[d].segs.push_back(foo[d]);
            }
        }
        for(unsigned d = 0; d < 2; d++)
            pws[d].cuts.push_back(150*pw_handles.size());
        
        slids.pts[0][1]=450;
        slids.pts[1][1]=450;
        slids.pts[2][1]=450;
        slids.pts[3][1]=450;
	
        cairo_set_source_rgba (cr, 0.2, 0.2, 0.2, 1);
        D2<SBasis> foo = slids.asBezier();
        SBasis g = foo[0] - Linear(150);
        cairo_d2_sb(cr, foo);
	    for(unsigned i=0;i<20;i++){
            double t=i/20.;
            draw_handle(cr, foo(t));
        }
        cairo_stroke(cr);
        foo[1]=foo[0];
        foo[0]=Linear(150,450);
        cairo_d2_sb(cr, foo);

        cairo_d2_pw_sb(cr, pws);
        
        cairo_stroke(cr);
        cairo_set_source_rgba (cr, 0.9, 0., 0., 1);
        D2<Piecewise<SBasis> > res = compose(pws, Piecewise<SBasis>(g));
        cairo_d2_pw_sb(cr, res);
        for(unsigned i=0;i<20;i++){
            double t=(res[0].cuts.back()-res[0].cuts.front())*i/20.;
            draw_handle(cr, Point(res[0](t),res[1](t)));
        }
        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    bool should_draw_numbers() override { return false; }
        
    public:
    PwToy () {
        unsigned segs = 5;
        unsigned handles_per_seg = 4;
        double x = 150;
        for(unsigned a = 0; a < segs; a++) {
            PointSetHandle* psh = new PointSetHandle;
            
            for(unsigned i = 0; i < handles_per_seg; i++, x+= 25)
                psh->push_back(Point(x, uniform() * 150));
            pw_handles.push_back(psh);
            handles.push_back(psh);
        }
        for(unsigned i = 0; i < 4; i++)
            slids.push_back(Point(150 + segs*50*i,100));
        handles.push_back(&slids);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new PwToy());
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
