#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/sbasis-2d.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/transforms.h>
#include <2geom/sbasis-math.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>

#include <gsl/gsl_matrix.h>

#include <vector>
using std::vector;
using namespace Geom;
using namespace std;

unsigned total_pieces_sub;
unsigned total_pieces_inc;

void cairo_pw(cairo_t *cr, Piecewise<SBasis> p) {
    for(unsigned i = 0; i < p.size(); i++) {
        D2<SBasis> B;
        B[0] = Linear(p.cuts[i], p.cuts[i+1]);
        B[1] = p[i];
        cairo_md_sb(cr, B);
    }
}

void draw_line(cairo_t* cr, Geom::Point n, double d) {
    cairo_move_to(cr, d*n + rot90(n)*1000);
    cairo_line_to(cr, d*n - rot90(n)*1000);
    cairo_move_to(cr, d*n);
    cairo_line_to(cr, (d+10)*n);
}

class InnerProductClip: public Toy {
    Path path_a;
    Piecewise<D2<SBasis> >  path_a_pw;
    std::vector<Toggle> togs;
    PointHandle start_handle, end_handle;
    
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
	cairo_set_source_rgba (cr, 0., 0.125, 0, 1);

	D2<Piecewise<SBasis> > B = make_cuts_independent(path_a_pw);
	
	Point n;
	double d;
        {
        double x = width - 60, y = height - 60;
        Point p(x, y), d(25,25), xo(25,0), yo(0,25);
        togs[0].bounds = Rect(p,     p + d);
        togs[1].bounds = Rect(p + xo, p + xo + d);
        draw_toggles(cr, togs);
        }
        if(togs[0].on)
            d = L2(end_handle.pos - start_handle.pos);
        else {
            n = unit_vector(rot90(end_handle.pos - start_handle.pos));
            d = dot(n, start_handle.pos);
            draw_line(cr, n, d);
        }
        //printf("%g\n", d);
	
        vector<double> all_roots;
        for(unsigned i = 0; i <= path_a.size(); i++) {
            //deriv = p[i].derivative();
            D2<SBasis> curpw = path_a[i].toSBasis();
            SBasis inner;
            if(togs[0].on) {
                D2<SBasis> test = curpw - start_handle.pos;
                inner = test[0]*test[0] + test[1]*test[1] - d*d;
            } else {
                inner = n[0]*curpw[0] + n[1]*curpw[1] - d;
            }
            vector<double> lr = roots(inner);
            all_roots.insert(all_roots.end(), lr.begin(), lr.end());
            for(unsigned i = 0; i < lr.size(); i++)
                draw_handle(cr, curpw(lr[i]));
            sort(lr.begin(), lr.end());
            lr.insert(lr.begin(), 0);
            lr.insert(lr.end(), 1);
            Path out;
            for(unsigned j = 0; j < lr.size()-1; j++) {
                Point s = curpw(lr[j]);
                Point m = curpw((lr[j] + lr[j+1])/2);
                if(togs[0].on)
                    m -= start_handle.pos;
                Point e = curpw(lr[j+1]);
                double dd;
                if(togs[0].on) 
                    dd = dot(m, m) - d*d;
                else
                    dd = dot(n, m) - d;
                if(togs[1].on)
                    dd = -dd;
                //printf("%d [%g, %g] %g (%g, %g) (%g, %g)\n", 
                //       i, lr[j], lr[j+1], dd, s[0], s[1], e[0], e[1]);
                if(0 > dd) {
                    //Curve * cv = path_a[i].portion(lr[j], lr[j+1]);
                    cairo_md_sb(cr, portion(curpw, lr[j], lr[j+1]));
                    cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
                    cairo_stroke(cr);
                    /*cairo_curve(cr, path_a[i]);
                    cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
                    cairo_stroke(cr);*/
                }
                
            }
	}
	
	//cairo_pw_d2(cr, path_a_pw);
	cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
	cairo_stroke(cr);
        
        Toy::draw(cr, notify, width, height, save);
    }
    void key_hit(GdkEventKey *e) {
        if(e->keyval == 's') togs[1].toggle(); else
        if(e->keyval == 'c') togs[0].toggle();
        redraw();
    }
    void mouse_pressed(GdkEventButton* e) {
        toggle_events(togs, e);
        Toy::mouse_pressed(e);
    }
    void first_time(int argc, char** argv) {
        const char *path_a_name="star.svgd";
        if(argc > 1)
            path_a_name = argv[1];
        std::vector<Path> paths_a = read_svgd(path_a_name);
        assert(paths_a.size() > 0);
        path_a = paths_a[0];
        
	path_a.close(true);
        path_a_pw = path_a.toPwSb();

        // Finite images of the three vanishing points and the origin
        handles.push_back(&start_handle);
        handles.push_back(&end_handle);
        togs.push_back(Toggle("C", true));
        togs.push_back(Toggle("S", true));
    }
    int should_draw_bounds() {return 1;}
public:
    InnerProductClip() : start_handle(150,300),
           end_handle(380,40)  {}
};

int main(int argc, char **argv) {
    init(argc, argv, new InnerProductClip);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
