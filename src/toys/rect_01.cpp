/*
 * SimpleRect examples
 *
 * Copyright 2009  Evangelos Katsikaros <vkatsikaros at yahoo dot gr>
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

/*
Very Simple example of a toy that creates a rectangle on screen

I am still very inexperienced with lib2geom
so don't use these examples as a reference :)
*/



#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>


#include <2geom/transforms.h>

using std::vector;
using namespace Geom;




class SimpleRect: public Toy {
    PointSetHandle psh;
    void draw( cairo_t *cr, std::ostringstream *notify,
                   int width, int height, bool save, std::ostringstream *timer_stream) override
	{
	  {
	    cairo_save(cr);
	    Path p1;
	    p1.appendNew<LineSegment>(Point(0, 0));
	    p1.appendNew<LineSegment>(Point(100, 0));
	    p1.appendNew<LineSegment>(Point(100, 100));
	    p1.appendNew<LineSegment>(Point(0, 100));
	    p1.appendNew<LineSegment>(Point(0, 0));
	    p1.close();

	    Path p2 = p1 * Rotate::from_degrees(45); //
	
	    cairo_set_source_rgb(cr, 0,0,0);
	    cairo_path(cr, p1);
	    cairo_path(cr, p2);
	    cairo_stroke(cr);
	    cairo_restore(cr);
	  }
        PointHandle p1, p2;
        p1.pos = Point(300, 50);
        p2.pos = Point(450, 450);

        Rect r1(p1.pos, p2.pos);

        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
        cairo_set_line_width(cr, 0.3);
        cairo_rectangle(cr, r1);
        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save, timer_stream);
    }

    public:
    SimpleRect(){
    }
};

int main(int argc, char **argv) {   
    init(argc, argv, new SimpleRect());

    return 0;
}

