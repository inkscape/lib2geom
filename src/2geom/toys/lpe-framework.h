#ifndef _2GEOM_LPE_TOY_FRAMEWORK_H_
#define _2GEOM_LPE_TOY_FRAMEWORK_H_

/**
 *  This should greatly simplify creating toy code for a Live Path Effect (LPE) for Inkscape
 */


#include <2geom/toys/toy-framework-2.h>
#include <2geom/pathvector.h>

class LPEToy: public Toy {
public:
    LPEToy();
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream);
    virtual std::vector<Geom::Path> 
            doEffect_path (std::vector<Geom::Path> const & path_in);
    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> >
            doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    /** this boolean defaults to false, it concatenates the input path to one pwd2,
     * instead of normally 'splitting' the path into continuous pwd2 paths. */
    bool concatenate_before_pwd2;
    PointSetHandle curve_handle;
};

#endif // _2GEOM_LPE_TOY_FRAMEWORK_H_
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
