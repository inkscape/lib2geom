#include <string.h>
#include <stdint.h>
#include <2geom/toys/lpe-framework.h>

#include <2geom/sbasis-to-bezier.h>
#include <2geom/matrix.h>
#include <2geom/pathvector.h>

#define  LPE_CONVERSION_TOLERANCE 0.01

void LPEToy::draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > pwd2(curve_handle.asBezier());
    std::vector<Geom::Path> A = Geom::path_from_piecewise( pwd2, LPE_CONVERSION_TOLERANCE);
    cairo_set_line_width (cr, 2);
    cairo_set_source_rgba (cr, 1., 0.0, 0., 1);
    cairo_path(cr, A);
    cairo_stroke(cr);

    // perform the effect:
    std::vector<Geom::Path> B = doEffect_path(A);

    cairo_set_line_width (cr, 1);
    cairo_set_source_rgba (cr, 0., 0.0, 0., 1);
    cairo_path(cr, B);
    cairo_stroke(cr);

    Toy::draw(cr, notify, width, height, save,timer_stream);
}

LPEToy::LPEToy(){
    if(handles.empty()) {
        handles.push_back(&curve_handle);
        for(unsigned i = 0; i < 4; i++) {
            curve_handle.push_back(150+uniform()*300,150+uniform()*300);
        }
    }
}

/*
 *  Here be the doEffect function chain:  (this is copied code from Inkscape)
 */
std::vector<Geom::Path>
LPEToy::doEffect_path (std::vector<Geom::Path> const & path_in)
{
    std::vector<Geom::Path> path_out;

    if ( !concatenate_before_pwd2 ) {
        // default behavior
        for (unsigned int i=0; i < path_in.size(); i++) {
            Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_in = path_in[i].toPwSb();
            Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_out = doEffect_pwd2(pwd2_in);
            std::vector<Geom::Path> path = Geom::path_from_piecewise( pwd2_out, LPE_CONVERSION_TOLERANCE);
            // add the output path vector to the already accumulated vector:
            for (unsigned int j=0; j < path.size(); j++) {
                path_out.push_back(path[j]);
            }
        }
    } else {
      // concatenate the path into possibly discontinuous pwd2
        Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_in;
        for (unsigned int i=0; i < path_in.size(); i++) {
            pwd2_in.concat( path_in[i].toPwSb() );
        }
        Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2_out = doEffect_pwd2(pwd2_in);
        path_out = Geom::path_from_piecewise( pwd2_out, LPE_CONVERSION_TOLERANCE);
    }

    return path_out;
}

Geom::Piecewise<Geom::D2<Geom::SBasis> >
LPEToy::doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
{
    // default effect does nothing
    return pwd2_in;
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
