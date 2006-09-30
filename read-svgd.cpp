#include "read-svgd.h"
#include "path-builder.h"

using std::FILE;
using std::fgetc;
using std::feof;

char const PathOpNames[] = {'L', 'Q', 'C', 'A'};

char path_op_name(Geom::CurveType* ct) {
    if(dynamic_cast<Geom::LineTo *>(ct))
        return 'L';
    else if(dynamic_cast<Geom::QuadTo *>(ct))
        return 'Q';
    else if(dynamic_cast<Geom::CubicTo *>(ct))
        return 'C';
    return 0;
}

void write_svgd(FILE* f, Geom::Path const &p) {
    //printf("size %d %d \n",  p.cmd.size(),  p.handles.size());
    if(f == NULL)
        f = stderr;
    fprintf(f, "M %g,%g ", p.initial_point()[0], p.initial_point()[1]);
    
    for(Geom::Path::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        fprintf(f, "%c ",  path_op_name(iter.cmd()));
        for(std::vector<Geom::Point>::const_iterator h(iter.begin()), e(iter.end());
            h != e; ++h) {
            Geom::Point pt(*h);
            fprintf(f, "%g,%g ", pt[0], pt[1]);
        }
    }
    if(p.is_closed())
        fprintf(f, "Z ");
}

void write_svgd(FILE* f, Geom::Arrangement const &p) {
    for(Geom::Arrangement::const_iterator it = p.begin(); it != p.end(); it++) {
        write_svgd(f, *it);
    }
}

std::ostream &operator<< (std::ostream &out_file, const Geom::Path & p) {
    out_file << "M " << p.initial_point()[0] << "," << p.initial_point()[1] << " ";
    
    for(Geom::Path::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        out_file << path_op_name(iter.cmd()) << " ";
        for(std::vector<Geom::Point>::const_iterator h(iter.begin()), e(iter.end());
            h != e; ++h) {
            Geom::Point pt(*h);
            out_file << pt[0] << "," << pt[1] << " ";
        }
    }
    if(p.is_closed())
        out_file << "Z ";
            return out_file;
}

std::ostream &operator<< (std::ostream &out_file, const Geom::Arrangement & p) {
    for(Geom::Arrangement::const_iterator it = p.begin(); it != p.end(); it++) {
        out_file << *it;
    }
                                                                                   return out_file;
}

Geom::Point point(double d1, double d2) {
    Geom::Point p;
    p[0] = d1;
    p[1] = d2;
    return p;
}

Geom::Arrangement read_svgd(FILE* f) {
    assert(f);

    Geom::ArrangementBuilder builder;

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
                builder.start_subpath_rel(point(nums[0], nums[1]));
                cur = 0;
            }
            break;
        case 'M':
            if(cur >= 2) {
                builder.start_subpath(point(nums[0], nums[1]));
                cur = 0;
            }
            break;
        case 'l':
            if(cur >= 2) {
                builder.push_line_rel(point(nums[0], nums[1]));
                cur = 0;
            }
            break;
        case 'L':
            if(cur >= 2) {
                builder.push_line(point(nums[0], nums[1]));
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
                builder.push_cubic_rel(point(nums[0], nums[1]), point(nums[2], nums[3]), point(nums[4], nums[5]));
                cur = 0;
            }
            break;
        case 'C':
            if(cur >= 6) {
                builder.push_cubic(point(nums[0], nums[1]), point(nums[2], nums[3]), point(nums[4], nums[5]));
                cur = 0;
            }
            break;
        case 'q':
            if(cur >= 4) {
                builder.push_quad_rel(point(nums[0], nums[1]), point(nums[2], nums[3]));
                cur = 0;
            }
            break;
        case 'Q':
            if(cur >= 4) {
                builder.push_quad(point(nums[0], nums[1]), point(nums[2], nums[3]));
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
                builder.push_ellipse(point(nums[0], nums[1]), nums[2], nums[3] > 0, nums[4] > 0, point(nums[5], nums[6]));
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
   if(chars[i] == ch)
   break;
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
