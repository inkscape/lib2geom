#include "read-svgd.h"
#include "path-builder.h"

using std::FILE;
using std::fgetc;
using std::feof;

char const SubPathOpNames[] = {'L', 'Q', 'C', 'A'};
void write_svgd(FILE* f, Geom::SubPath const &p) {
    //printf("size %d %d \n",  p.cmd.size(),  p.handles.size());
    if(f == NULL)
        f = stderr;
    fprintf(f, "M %g,%g ", p.initial_point()[0], p.initial_point()[1]);
    
    for(Geom::SubPath::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        fprintf(f, "%c ", SubPathOpNames[iter.cmd()]);
        for(std::vector<Geom::Point>::const_iterator h(iter.begin()), e(iter.end());
            h != e; ++h) {
            Geom::Point pt(*h);
            fprintf(f, "%g,%g ", pt[0], pt[1]);
        }
    }
    if(p.is_closed())
        fprintf(f, "Z ");
}

void write_svgd(FILE* f, Geom::Path const &p) {
    for(Geom::Path::const_iterator it = p.begin(); it != p.end(); it++) {
        write_svgd(f, *it);
    }
}


Geom::Point point(double d1, double d2) {
    Geom::Point p;
    p[0] = d1;
    p[1] = d2;
    return p;
}

Geom::Path read_svgd(FILE* f) {
    assert(f);

    Geom::PathBuilder builder;

    char mode = 0;

    double nums[7];
    int cur = 0;
    while(!feof(f)) {
        char ch = fgetc(f);

        if((ch >= 'A' and ch <= 'Z') or (ch >= 'a' and ch <= 'z'))
            mode = ch;
        else if (ch == ' ' or ch == '\t' or ch == '\n' or ch == '\r' or ch == ',')
            continue;
        else if ((ch >= '0' and ch <= '9') or ch == '-' or ch == '.' or ch == '+') {
            ungetc(ch, f);
            int nflts = fscanf(f, "%lf", &nums[cur]);
            cur++;
        }
        
        switch(mode) {
        case 'M':
            if(cur == 2) {
                builder.start_subpath(point(nums[0], nums[1]));
                cur = 0;
            }
            break;
        case 'L':
            if(cur == 2) {
                builder.push_line(point(nums[0], nums[1]));
                cur = 0;
            }
            break;
        case 'C':
            if(cur == 6) {
                builder.push_cubic(point(nums[0], nums[1]), point(nums[2], nums[3]), point(nums[4], nums[5]));
                cur = 0;
            }
            break;
        case 'Q':
            if(cur == 4) {
                builder.push_quad(point(nums[0], nums[1]), point(nums[2], nums[3]));
                cur = 0;
            }
            break;
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
