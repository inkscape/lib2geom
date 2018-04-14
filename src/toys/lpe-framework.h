/**
 * \file lpe-framework.h
 * \brief A framework for writing an Inkscape Live Path Effect toy.
 *
 * Instead of using the standard toy framework, one can use this LPE 
 * framework when creating an LPE for Inkscape. When new 2geom functions 
 * are required for the LPE, adding this functionality directly in Inkscape 
 * greatly increases compile times. Using this framework, one only has to
 * rebuild 2geom, which speeds up development considerably. (Think about
 * how much of Inkscape will have to be rebuild if you change/add something
 * in point.h ...)
 * An example of how to use this framework is lpe.test.cpp.
 *//*
 * Copyright 2009  Johan Engelen <goejendaagh@zonnet.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 */

#ifndef _2GEOM_LPE_TOY_FRAMEWORK_H_
#define _2GEOM_LPE_TOY_FRAMEWORK_H_

/**
 *  This should greatly simplify creating toy code for a Live Path Effect (LPE) for Inkscape
 */


#include <toys/toy-framework-2.h>
#include <2geom/pathvector.h>

class LPEToy: public Toy {
public:
    LPEToy();
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) override;
    virtual Geom::PathVector
            doEffect_path (Geom::PathVector const & path_in);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
