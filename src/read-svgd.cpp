#include "read-svgd.h"
#include "sbasis-to-bezier.h"

using std::FILE;
using std::fgetc;
using std::feof;

void curve_to_svgd(FILE* f, Geom::Path2::Curve const* c) {
    if(Geom::Path2::LineSegment const *line_segment = dynamic_cast<Geom::Path2::LineSegment const  *>(c)) {
        fprintf(f, "L %g,%g ", (*line_segment)[1][0], (*line_segment)[1][1]);
    }
    else if(Geom::Path2::QuadraticBezier const *quadratic_bezier = dynamic_cast<Geom::Path2::QuadraticBezier const  *>(c)) {
        fprintf(f, "Q %g,%g %g,%g ", 
                (*quadratic_bezier)[1][0], (*quadratic_bezier)[1][0], 
                (*quadratic_bezier)[2][0], (*quadratic_bezier)[2][1]);
    }
    else if(Geom::Path2::CubicBezier const *cubic_bezier = dynamic_cast<Geom::Path2::CubicBezier const  *>(c)) {
        fprintf(f, "C %g,%g %g,%g %g,%g ", 
                (*cubic_bezier)[1][0], (*cubic_bezier)[1][1], 
                (*cubic_bezier)[2][0], (*cubic_bezier)[2][1], 
                (*cubic_bezier)[3][0], (*cubic_bezier)[3][1]);
    }
//    else if(Geom::Path2::SVGEllipticalArc const *svg_elliptical_arc = dynamic_cast<Geom::Path2::SVGEllipticalArc *>(c)) {
//        //get at the innards and spit them out as svgd
//    }
    else { 
        //this case handles sbasis as well as all other curve types
        Geom::Path2::Path sbasis_path;
        path_from_sbasis(sbasis_path, c->sbasis(), 0.1);

        //recurse to convert the new path resulting from the sbasis to svgd
        for(Geom::Path2::Path::iterator iter = sbasis_path.begin(); iter != sbasis_path.end(); ++iter) {
            curve_to_svgd(f, &(*iter));
        }
    }
}

void write_svgd(FILE* f, Geom::Path2::Path const &p) {
    if(f == NULL)
        f = stderr;
    fprintf(f, "M %g,%g ", p.initialPoint()[0], p.initialPoint()[1]);
    
    for(Geom::Path2::Path::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        curve_to_svgd(f, &(*iter));
    }
    if(p.closed())
        fprintf(f, "Z ");
}

void write_svgd(FILE* f, std::vector<Geom::Path2::Path> const &p) {
    std::vector<Geom::Path2::Path>::const_iterator it(p.begin());
    for(; it != p.end(); it++) {
        write_svgd(f, *it);
    }
}

Geom::Point point(double d1, double d2) {
    Geom::Point p;
    p[0] = d1;
    p[1] = d2;
    return p;
}

Geom::PathSet
read_svgd(FILE* f) {
    assert(f);

    Geom::PathSetBuilder builder;

    char mode = 0;

    double nums[7];
    int cur = 0;
    while(!feof(f)) {
        char ch = fgetc(f);

        if((ch >= 'A' and ch <= 'Z') or (ch >= 'a' and ch <= 'z')) {
            mode = ch;
            cur = 0;
        } else if (ch == ' ' or ch == '\t' or ch == '\n' or ch == '\r' or ch == ',')
            continue;
        else if ((ch >= '0' and ch <= '9') or ch == '-' or ch == '.' or ch == '+') {
            ungetc(ch, f);
            //TODO: use something else, perhaps.  Unless the svg path number spec matches scan.
            int nflts = fscanf(f, "%lf", &nums[cur]);
            cur++;
        }
        
        switch(mode) {
        case 'm':
            if(cur >= 2) {
                builder.start_subpath_rel(Geom::Point(nums[0], nums[1]));
                cur = 0;
            }
            break;
        case 'M':
            if(cur >= 2) {
                builder.start_subpath(Geom::Point(nums[0], nums[1]));
                cur = 0;
            }
            break;
        case 'l':
            if(cur >= 2) {
                builder.push_line_rel(Geom::Point(nums[0], nums[1]));
                cur = 0;
            }
            break;
        case 'L':
            if(cur >= 2) {
                builder.push_line(Geom::Point(nums[0], nums[1]));
                cur = 0;
            }
            break;
        case 'h':
            if(cur >= 1) {
                builder.push_horizontal_rel(nums[0]);
                cur = 0;
            }
            break;
        case 'H':
            if(cur >= 1) {
                builder.push_horizontal(nums[0]);
                cur = 0;
            }
            break;
        case 'v':
            if(cur >= 1) {
                builder.push_vertical_rel(nums[0]);
                cur = 0;
            }
            break;
        case 'V':
            if(cur >= 1) {
                builder.push_vertical(nums[0]);
                cur = 0;
            }
            break;
        case 'c':
            if(cur >= 6) {
                builder.push_cubic_rel(Geom::Point(nums[0], nums[1]), Geom::Point(nums[2], nums[3]), Geom::Point(nums[4], nums[5]));
                cur = 0;
            }
            break;
        case 'C':
            if(cur >= 6) {
                builder.push_cubic(Geom::Point(nums[0], nums[1]), Geom::Point(nums[2], nums[3]), Geom::Point(nums[4], nums[5]));
                cur = 0;
            }
            break;
        case 'q':
            if(cur >= 4) {
                builder.push_quad_rel(Geom::Point(nums[0], nums[1]), Geom::Point(nums[2], nums[3]));
                cur = 0;
            }
            break;
        case 'Q':
            if(cur >= 4) {
                builder.push_quad(Geom::Point(nums[0], nums[1]), Geom::Point(nums[2], nums[3]));
                cur = 0;
            }
            break;
        case 'a':
            if(cur >= 7) {
                //TODO
                cur = 0;
            }
            break;
        case 'A':
            if(cur >= 7) {
                builder.push_ellipse(Geom::Point(nums[0], nums[1]), nums[2], nums[3] > 0, nums[4] > 0, Geom::Point(nums[5], nums[6]));
                cur = 0;
            }
            break;
        case 'z':
        case 'Z':
            builder.close_subpath();
            break;
        }
    }
    return builder.peek();
}


/* Possibly useful function I wrote and then didn't use:
const char* whitespace = " \t\r\n"
const char* seperator = ", \t\r\n"
void eat_chars(FILE* f, const char* chars) {
 while(!feof(f)) {
  int i = 0;
  int ch = fgetc(f);
  while(1) {
   if(chars[i] == 0) {
    ungetc(ch, f);
    return;
   }
   if(chars[i] == ch) break;
   i++;
  }
 }
}
*/

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
