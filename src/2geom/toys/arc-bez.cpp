#include <2geom/d2.h>

#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-geometric.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <vector>
using std::vector;
using namespace Geom;


class ArcBez: public Toy {
    PointSetHandle bez_handle;
public:
    ArcBez() {
        for(int i = 0; i < 6; i++)
            bez_handle.push_back(uniform()*400, uniform()*400);
        handles.push_back(&bez_handle);
    }

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        
        D2<SBasis> B = bez_handle.asBezier();
        cairo_md_sb(cr, B);
        cairo_stroke(cr);
        
        cairo_set_source_rgba (cr, 0.25, 0.5, 0, 0.8);

        double units = 1e6;
        std::string units_string("us");
        double timer_precision = 0.1;
        clock_t end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        // Base loop to remove overhead
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        long iterations = 0;
        while(end_t > clock()) {
            iterations++;
        }
        double overhead = timer_precision*units/iterations;
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            Piecewise<SBasis> als = arcLengthSb(B);
            iterations++;
        }
        *notify << "arcLengthSb based " 
                << ", time = " << timer_precision*units/iterations-overhead 
                << units_string << std::endl;
        Piecewise<SBasis> als = arcLengthSb(B);
            
        cairo_d2_pw(cr, D2<Piecewise<SBasis> >(Piecewise<SBasis>(SBasis(Linear(0, width))) , Piecewise<SBasis>(Linear(height-5)) - Piecewise<SBasis>(als)) );

        double abs_error = 0;
        double integrating_arc_length = 0;
        end_t = clock()+clock_t(timer_precision*CLOCKS_PER_SEC);
        iterations = 0;
        while(end_t > clock()) {
            length_integrating(B, integrating_arc_length, abs_error, 1e-10);
            iterations++;
        }
    
        *notify << "gsl integrating " 
                << ", time = " << timer_precision*units/iterations-overhead 
                << units_string << std::endl;
        length_integrating(B, integrating_arc_length, abs_error, 1e-10);
        *notify << "arc length = " << integrating_arc_length << "; abs error = " << abs_error << std::endl;
        double als_arc_length = als.segs.back().at1();
        *notify << "arc length = " << als_arc_length << "; error = " << als_arc_length - integrating_arc_length << std::endl;
        Toy::draw(cr, notify, width, height, save);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new ArcBez());

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
