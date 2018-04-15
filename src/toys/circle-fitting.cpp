/*
 * Circle and Elliptical Arc Fitting Example
 *
 * Authors:
 *      Marco Cecchetti <mrcekets at gmail.com>
 *
 * Copyright 2008  authors
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
 */

#include <memory>
#include <2geom/circle.h>
#include <2geom/elliptical-arc.h>
#include <2geom/numeric/fitting-tool.h>
#include <2geom/numeric/fitting-model.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>


using namespace Geom;


class CircleFitting : public Toy
{
  private:
    void draw( cairo_t *cr, std::ostringstream *notify,
               int width, int height, bool save, std::ostringstream *timer_stream) override
    {
        if (first_time)
        {
            first_time = false;
            Point toggle_sp( 300, height - 50);
            toggles[0].bounds = Rect( toggle_sp, toggle_sp + Point(120,25) );
            sliders[0].geometry(Point(50, height - 50), 100);
        }

        size_t n = (size_t)(sliders[0].value()) + 3;
        if (n < psh.pts.size())
        {
            psh.pts.resize(n);
        }
        else if (n > psh.pts.size())
        {
            psh.push_back(400*uniform()+50, 300*uniform()+50);
        }

        try
        {
            c.fit(psh.pts);
        }
        catch(RangeError exc)
        {
            std::cerr << exc.what() << std::endl;
            Toy::draw(cr, notify, width, height, save,timer_stream);
            return;
        }

        if (toggles[0].on)
        {
            try
            {
                std::unique_ptr<EllipticalArc> eap( c.arc(psh.pts[0], psh.pts[1], psh.pts[2]) );
                ea = *eap;
            }
            catch(RangeError exc)
            {
                std::cerr << exc.what() << std::endl;
                Toy::draw(cr, notify, width, height, save,timer_stream);
                return;
            }
        }

        std::cerr << "center = " << c.center() << "  ray = " << c.radius() << std::endl;

        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
        cairo_set_line_width (cr, 0.3);
        if (!toggles[0].on)
        {
            cairo_arc(cr, c.center(X), c.center(Y), c.radius(), 0, 2*M_PI);
        }
        else
        {
            draw_text(cr, psh.pts[0], "initial");
            draw_text(cr, psh.pts[1], "inner");
            draw_text(cr, psh.pts[2], "final");

            D2<SBasis> easb = ea.toSBasis();
            cairo_d2_sb(cr, easb);
        }
        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

  public:
    CircleFitting()
    {
        first_time = true;

        psh.pts.resize(3);
        psh.pts[0] = Point(450, 250);
        psh.pts[1] = Point(250, 100);
        psh.pts[2] = Point(250, 400);


        toggles.emplace_back(" arc / circle ", false);
        sliders.emplace_back(0, 5, 1, 0, "more handles");

        handles.push_back(&psh);
        handles.push_back(&(toggles[0]));
        handles.push_back(&(sliders[0]));
    }

  private:
    Circle c;
    EllipticalArc ea;
    bool first_time;
    PointSetHandle psh;
    std::vector<Toggle> toggles;
    std::vector<Slider> sliders;
};



int main(int argc, char **argv)
{
    init( argc, argv, new CircleFitting(), 600, 600 );
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
