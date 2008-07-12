/*
 * A simple toy to reproduce a crashing case
 *
 * Copyright 2007 Aaron Spike <aaron@ekips.org>
 * Copyright 2008 Johan Engelen (goejendaagh@zonnet.nl>
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


#include <iostream>
#include <2geom/svg-path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/pathvector.h>
#include <2geom/transforms.h>

int main(int /*argc*/, char** /*argv*/) {

/*
Crasher = 
  <g
     id="edit_undo_history"
     transform="matrix(1.00075,0,0,0.995637,-4.124129e-2,0.179073)">
       transform="translate(-254.4684,4.926264)">
    <g
      <path
         style="opacity:1.0000000;color:#000000;fill:url(#linearGradient5612);fill-opacity:1.0;fill-rule:evenodd;stroke:url(#linearGradient5620);stroke-width:0.71197993;stroke-linecap:round;stroke-linejoin:round;marker:none;marker-start:none;marker-mid:none;marker-end:none;stroke-miterlimit:0.0000000;stroke-dasharray:none;stroke-dashoffset:0.0000000;stroke-opacity:1.0000000;visibility:visible;display:inline;overflow:visible"
         id="path14377"
         d="M 284.00000 23.000000 A 3.0000000 3.0000000 0 1 1  278.00000,23.000000 A 3.0000000 3.0000000 0 1 1  284.00000 23.000000 z"
         transform="matrix(1.331759,0,0,1.327869,-53.22381,-6.040984)" />
*/

    Geom::Matrix tempMat(1.3327578,0,0,1.3220755,-307.96422,-0.9307835);
    char const * d = "M 284.00000 23.000000 A 3.0000000 3.0000000 0 1 1  278.00000,23.000000 A 3.0000000 3.0000000 0 1 1  284.00000 23.000000 z";
    Geom::PathVector yo = Geom::parse_svg_path(d);
    yo *= tempMat;
    std::cout << "success!" << std::endl;

    return 0;
};

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
