/*
 * SVGPathParser - parse SVG path specifications
 *
 * Copyright 2007 MenTaLguY <mental@rydia.net>
 * Copyright 2007 Aaron Spike <aaron@ekips.org>
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
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <vector>

#include "point.h"

#include "svg_path_parser.h"

using Geom::Point;

%%{
    machine svg_path;
    write data noerror;
}%%

void SVGPathParser::parse(char const *str) throw(SVGPathParser::ParseError) {
    char const *p = str;
    char const *pe = str + strlen(str);
    char const *start = NULL;
    int cs;
    bool absolute = false;
    std::vector<double> params;

    _reset();

    %%{
        action start_number {
            start = p;
        }

        action push_number {
            char const *end=p;
            std::string buf(start, end);
            params.push_back(strtod(start, (char **)&end));
            start = NULL;
        }

        action push_true {
            params.push_back(1.0);
        }

        action push_false {
            params.push_back(0.0);
        }

        action mode_abs {
            absolute = true;
        }
    
        action mode_rel {
            absolute = false;
        }
    
        action moveto {
            Point point(params[0], params[1]);
            params.clear();
            if (!absolute) {
                point = point + _current;
            }
            _moveTo(point);
        }    

        action lineto {
            Point point(params[0], params[1]);
            params.clear();
            if (!absolute) {
                point = point + _current;
            }
            _lineTo(point);
        }

        action horizontal_lineto {
            double x=params[0];
            params.clear();
            if (!absolute) {
                x = x + _current[0];
            }
            _lineTo(Point(x, _current[1]));
        }

        action vertical_lineto {
            double y=params[0];
            params.clear();
            if (!absolute) {
                y = y + _current[1];
            }
            _lineTo(Point(_current[0], y));
        }

        action curveto {
            Point points[3];
            for ( int i = 0 ; i < 3 ; i++ ) {
                points[i] = Point(params[i*2], params[i*2+1]);
                if (!absolute) {
                    points[i] = points[i] + _current;
                }
            }
            params.clear();
            _curveTo(points[0], points[1], points[2]);
        }

        action smooth_curveto {
            Point points[3];
            for ( int i = 1 ; i < 3 ; i++ ) {
                points[i] = Point(params[(i-1)*2], params[(i-1)*2+1]);
                if (!absolute) {
                    points[i] = points[i] + _current;
                }
            }
            points[0] = _cubic_tangent;
            params.clear();
            _curveTo(points[0], points[1], points[2]);
        }

        action quadratic_bezier_curveto {
            Point points[2];
            for ( int i = 0 ; i < 2 ; i++ ) {
                points[i] = Point(params[i*2], params[i*2+1]);
                if (!absolute) {
                    points[i] = points[i] + _current;
                }
            }
            params.clear();
            _quadTo(points[0], points[1]);
        }

        action smooth_quadratic_bezier_curveto {
            Point points[2];
            points[1] = Point(params[0], params[1]);
            if (!absolute) {
                points[1] = points[1] + _current;
            }
            points[0] = _quad_tangent;
            params.clear();
            _quadTo(points[0], points[1]);
        }

        action elliptical_arc {
            double rx;
            double ry;
            double angle;
            bool large_arc;
            bool sweep;
            Point point;

            rx = params[0];
            ry = params[1];
            angle = params[2];
            large_arc = params[3];
            sweep = params[4];
            point = Point(params[5], params[6]);

            if (!absolute) {
                point = point + _current;
            }

            _arcTo(rx, ry, angle, large_arc, sweep, point);
        }
        
        action closepath {
            _closePath();
        }

        wsp = (' ' | 9 | 10 | 13);
        sign = ('+' | '-');
        digit_sequence = digit+;
        exponent = ('e' | 'E') sign? digit_sequence;
        fractional_constant =
            digit_sequence? '.' digit_sequence
            | digit_sequence '.';
        floating_point_constant =
            fractional_constant exponent?
            | digit_sequence exponent;
        integer_constant = digit_sequence;
        comma = ',';
        comma_wsp = (wsp+ comma? wsp*) | (comma wsp*);

        flag = ('0' %push_false | '1' %push_true);
        
        number =
            ( sign? integer_constant
            | sign? floating_point_constant )
            >start_number %push_number;

        nonnegative_number =
            ( integer_constant
            | floating_point_constant)
            >start_number %push_number;

        coordinate = number;
        coordinate_pair = coordinate $1 %0 comma_wsp? coordinate;
        
        elliptical_arc_argument =
            (nonnegative_number $1 %0 comma_wsp?
             nonnegative_number $1 %0 comma_wsp?
             number comma_wsp
             flag comma_wsp flag comma_wsp
             coordinate_pair)
            %elliptical_arc;
        elliptical_arc_argument_sequence =
            elliptical_arc_argument $1 %0
            (comma_wsp? elliptical_arc_argument $1 %0)*;
        elliptical_arc =
            ('A' %mode_abs| 'a' %mode_rel) wsp*
            elliptical_arc_argument_sequence;
        
        smooth_quadratic_bezier_curveto_argument =
            coordinate_pair %smooth_quadratic_bezier_curveto;
        smooth_quadratic_bezier_curveto_argument_sequence =
            smooth_quadratic_bezier_curveto_argument $1 %0
            (comma_wsp?
             smooth_quadratic_bezier_curveto_argument $1 %0)*;
        smooth_quadratic_bezier_curveto =
            ('T' %mode_abs| 't' %mode_rel) wsp*
             smooth_quadratic_bezier_curveto_argument_sequence;

        quadratic_bezier_curveto_argument =
            (coordinate_pair $1 %0 comma_wsp? coordinate_pair)
            %quadratic_bezier_curveto;
        quadratic_bezier_curveto_argument_sequence =
            quadratic_bezier_curveto_argument $1 %0
            (comma_wsp? quadratic_bezier_curveto_argument $1 %0)*;
        quadratic_bezier_curveto =
            ('Q' %mode_abs| 'q' %mode_rel) wsp* 
            quadratic_bezier_curveto_argument_sequence;

        smooth_curveto_argument =
            (coordinate_pair $1 %0 comma_wsp? coordinate_pair)
            %smooth_curveto;
        smooth_curveto_argument_sequence =
            smooth_curveto_argument $1 %0
            (comma_wsp? smooth_curveto_argument $1 %0)*;
        smooth_curveto =
            ('S' %mode_abs| 's' %mode_rel)
            wsp* smooth_curveto_argument_sequence;

        curveto_argument =
            (coordinate_pair $1 %0 comma_wsp?
             coordinate_pair $1 %0 comma_wsp?
             coordinate_pair) 
            %curveto;
        curveto_argument_sequence =
            curveto_argument $1 %0
            (comma_wsp? curveto_argument $1 %0)*;
        curveto =
            ('C' %mode_abs| 'c' %mode_rel)
            wsp* curveto_argument_sequence;

        vertical_lineto_argument = coordinate %vertical_lineto;
        vertical_lineto_argument_sequence =
            vertical_lineto_argument $1 %0
            (comma_wsp? vertical_lineto_argument $1 %0)*;
        vertical_lineto =
            ('V' %mode_abs| 'v' %mode_rel)
            wsp* vertical_lineto_argument_sequence;

        horizontal_lineto_argument = coordinate %horizontal_lineto;
        horizontal_lineto_argument_sequence =
            horizontal_lineto_argument $1 %0
            (comma_wsp? horizontal_lineto_argument $1 %0)*;
        horizontal_lineto =
            ('H' %mode_abs| 'h' %mode_rel)
            wsp* horizontal_lineto_argument_sequence;

        lineto_argument = coordinate_pair %lineto;
        lineto_argument_sequence =
            lineto_argument $1 %0
            (comma_wsp? lineto_argument $1 %0)*;
        lineto =
            ('L' %mode_abs| 'l' %mode_rel) wsp*
            lineto_argument_sequence;

        closepath = ('Z' | 'z') %closepath;

        moveto_argument = coordinate_pair %moveto;
        moveto_argument_sequence =
            moveto_argument $1 %0
            (comma_wsp? lineto_argument $1 %0)*;
        moveto =
            ('M' %mode_abs | 'm' %mode_rel)
            wsp* moveto_argument_sequence;

        drawto_command =
            closepath | lineto |
            horizontal_lineto | vertical_lineto |
            curveto | smooth_curveto |
            quadratic_bezier_curveto |
            smooth_quadratic_bezier_curveto |
            elliptical_arc;

        drawto_commands = drawto_command (wsp* drawto_command)*;
        moveto_drawto_command_group = moveto wsp* drawto_commands?;
        moveto_drawto_command_groups =
            moveto_drawto_command_group
            (wsp* moveto_drawto_command_group)*;

        svg_path = wsp* moveto_drawto_command_groups? wsp*;

        main := svg_path;

        # Inintialize and execute.
        write init;
        write exec;
    }%%

    if ( cs < svg_path_first_final ) {
        throw ParseError();
    }
}


#define BUFSIZE 1024

class Foo : public SVGPathParser {
protected:
    void moveTo(Point p) {
        emit('M', p);
        emit_nl();
    }
    void lineTo(Point p) {
        emit('L', p);
        emit_nl();
    }
    void curveTo(Point c0, Point c1, Point p) {
        emit('C', c0);
        emit(' ', c1);
        emit(' ', p);
        emit_nl();
    }
    void quadTo(Point c, Point p) {
        emit('Q', c);
        emit(' ', p);
        emit_nl();
    }
    void arcTo(double rx, double ry, double angle, bool large_arc, bool sweep,
               Point p)
    {
        emit('A', rx);
        emit(',', ry);
        emit(' ', angle);
        emit(' ', large_arc);
        emit(' ', sweep);
        emit(' ', p);
        emit_nl();
    }
    void closePath() {
        emit('z');
        emit_nl();
    }
private:
    void emit(char op, Point p) {
        emit(op);
        emit(' ', p[0]);
        emit(',', p[1]);
    }
    void emit(char op, double n) {
        emit(op);
        std::cout << n;
    }
    void emit(char op, bool b) {
        emit(op);
        std::cout << (int)b;
    }
    void emit(char op) {
        std::cout.put(op);
    }
    void emit_nl() {
        std::cout << std::endl;
    }
};

int main()
{
    Foo foo;
    char buf[BUFSIZE];
    while ( fgets( buf, sizeof(buf), stdin ) != 0 ) {
        foo.parse( buf );
    }
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
