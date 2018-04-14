#include <2geom/d2.h>

#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-math.h>
#include <2geom/sbasis-geometric.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#include <vector>
using std::vector;
using namespace Geom;

Piecewise<SBasis> 
arcLengthSb2(Piecewise<D2<SBasis> > const &M, double /*tol*/){
    Piecewise<D2<SBasis> > dM = derivative(M);
    Piecewise<SBasis> length = integral(dot(dM, unitVector(dM)));
    length-=length.segs.front().at0();
    return length;
}



class ArcBez: public Toy {
    PointSetHandle bez_handle;
public:
    ArcBez() {
        for(int i = 0; i < 6; i++)
            bez_handle.push_back(uniform()*400, uniform()*400);
        handles.push_back(&bez_handle);
    }

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timing_stream) override {
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        
        D2<SBasis> B = bez_handle.asBezier();
        cairo_d2_sb(cr, B);
        cairo_stroke(cr);
        
        cairo_set_source_rgba (cr, 0.25, 0.5, 0, 0.8);
        
        double tol = 0.01;
        bool time_operations = true;
        if(time_operations) {
            std::string units_string("us");
            Timer tm;
            tm.ask_for_timeslice();
            tm.start();
            Piecewise<SBasis> als = arcLengthSb(B, tol);
            Timer::Time als_time = tm.lap();
            *timing_stream << "arcLengthSb based " 
                           << ", time = " << als_time 
                           << units_string << std::endl;

            tm.start();
            Piecewise<SBasis> als2 = arcLengthSb2(Piecewise<D2<SBasis> >(B), 0.01);
            Timer::Time als2_time = tm.lap();

            *timing_stream << "arcLengthSb2 based " 
                           << ", time = " << als2_time 
                           << units_string << std::endl;
            double abs_error = 0;
            double integrating_arc_length = 0;
            tm.start();
            length_integrating(B, integrating_arc_length, abs_error, 1e-10);
            Timer::Time li_time = tm.lap();
    
            *timing_stream << "gsl integrating " 
                           << ", time = " << li_time 
                           << units_string << std::endl;
        }
        Piecewise<SBasis> als = arcLengthSb(B, tol);
        Piecewise<SBasis> als2 = arcLengthSb2(Piecewise<D2<SBasis> >(B), 0.01);
            
        cairo_d2_pw_sb(cr, D2<Piecewise<SBasis> >(Piecewise<SBasis>(SBasis(Linear(0, width))) , Piecewise<SBasis>(Linear(height-5)) - Piecewise<SBasis>(als)) );

        double abs_error = 0;
        double integrating_arc_length = 0;
        length_integrating(B, integrating_arc_length, abs_error, 1e-10);
        *notify << "arc length = " << integrating_arc_length << "; abs error = " << abs_error << std::endl;
        double als_arc_length = als.segs.back().at1();
        *notify << "arc length = " << als_arc_length << "; error = " << als_arc_length - integrating_arc_length << std::endl;
        double als_arc_length2 = als2.segs.back().at1();
        *notify << "arc length2 = " << als_arc_length2 << "; error = " << als_arc_length2 - integrating_arc_length << std::endl;

        {
            double err = fabs(als_arc_length - integrating_arc_length);
            double scale = 10./err;
            Piecewise<D2<SBasis> > dM = derivative(Piecewise<D2<SBasis> >(B));
            Piecewise<SBasis> ddM = dot(dM,dM);
            Piecewise<SBasis> dMlength = sqrt(ddM,tol,3);
            double plot_width = (width - 200);
            
            Point org(100,height - 200);
            cairo_move_to(cr, org);
            for(double t = 0; t < 1; t += 0.01) {
                cairo_line_to(cr, org + Point(t*plot_width, scale*(sqrt(ddM.valueAt(t)) - dMlength.valueAt(t))));
            }
            cairo_move_to(cr, org);
            cairo_line_to(cr, org+Point(plot_width, 0));
            cairo_stroke(cr);
            
            draw_number(cr, org, scale);
            
        }


        Toy::draw(cr, notify, width, height, save,timing_stream);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
