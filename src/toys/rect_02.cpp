/*
 * SimpleRect examples
 *
 * Copyright 2009 Evangelos Katsikaros <vkatsikaros at yahoo dot gr>
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
Simple example of a toy that creates a rectangle on screen.
We also add two handles (that we don't use in our program)

I am still very inexperienced with lib2geom
so don't use these examples as a reference :)
*/

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

using std::vector;
using namespace Geom;


class SimpleRect: public Toy {
    PointSetHandle psh;
    void draw( cairo_t *cr, std::ostringstream *notify,
                   int width, int height, bool save, std::ostringstream *timer_stream) override
	{

        PointHandle p1, p2;
        p1.pos = Point(400, 50);
        p2.pos = Point(450, 450);

        Rect r1(p1.pos, p2.pos);

        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
        cairo_set_line_width(cr, 0.3);
        cairo_rectangle(cr, r1);

        cairo_stroke(cr);

        Toy::draw(cr, notify, width, height, save, timer_stream);
    }
    public:
    SimpleRect (unsigned no_of_handles) {
        handles.push_back(&psh);
        for(unsigned i = 0; i < no_of_handles; i++)
            psh.push_back( 200 + ( i * 10 ), 300 + ( i * 10 ) );
    }
};

int main(int argc, char **argv) {   
    unsigned no_of_handles=2;
    if(argc > 1)
        sscanf(argv[1], "%d", &no_of_handles);
    init(argc, argv, new SimpleRect(no_of_handles));

    return 0;
}

