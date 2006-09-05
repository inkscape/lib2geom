#include "read-svgd.h"
#include "path-builder.h"

using std::FILE;
using std::fgetc;
using std::feof;

static void eat_space(FILE* f) {
    int c;
    if(!isspace(c = fgetc(f)))
        ungetc(c, f);
}

void eat(FILE* f, char cc) {
    char c;
    if((c = fgetc(f)) != cc) ungetc(c, f);
}

Geom::Point read_point(FILE* f) {
    Geom::Point p;
    int i = 0;
    while(i < 2) {
        int nflts = fscanf(f, " %lf ", &p[i]);
        assert(nflts);
        i++;
        eat(f, ',');
    }
    
    return p;
}

char const SubPathOpNames[] = {'L', 'Q', 'C', 'A'};
void write_svgd(FILE* f, Geom::SubPath const &p) {
    //printf("size %d %d \n",  p.cmd.size(),  p.handles.size());
    for(Geom::SubPath::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        printf("%c ", SubPathOpNames[iter.cmd()]);
        for(std::vector<Geom::Point>::const_iterator h(iter.begin()), e(iter.end());
            h != e; ++h) {
            Geom::Point pt(*h);
            printf("%g,%g ", pt[0], pt[1]);
        }
    }
}

Geom::Path read_svgd(FILE* f) {
    assert(f);

    Geom::PathBuilder builder;
    
    while(!feof(f)) {
        eat_space(f);
        char cmd = fgetc(f);
        eat_space(f);
        switch(cmd) {
        case 'M':
            builder.start_subpath(read_point(f));
            break;
        case 'L':
            builder.push_line(read_point(f));
            break;
        case 'C': {
            Geom::Point p0 = read_point(f);
            Geom::Point p1 = read_point(f);
            Geom::Point p2 = read_point(f);
            builder.push_cubic(p0, p1, p2);
        }
            break;
        case 'Q': {
            Geom::Point p0 = read_point(f);
            Geom::Point p1 = read_point(f);
            builder.push_quad(p0, p1);
        }
            break;
        case 'A':
            builder.push_ellipse(read_point(f), read_point(f), read_point(f), read_point(f));
            break;
        case 'Z':
            builder.close_subpath();
            break;
        default:
            //ungetc(cmd, f);
            //This makes an infinite loop if its an invalid svgd
            break;
        }
    }
    return builder.peek();
}


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

