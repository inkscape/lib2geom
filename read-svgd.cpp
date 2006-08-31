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

char const SubPathOpNames[] = {'M', 'L', 'Q', 'C', 'A', 'z'};
void write_svgd(FILE* f, Geom::SubPath const &p) {
    //printf("size %d %d \n",  p.cmd.size(),  p.handles.size());
    for(Geom::SubPath::const_iterator iter(p.begin()), end(p.end()); iter != end; ++iter) {
        //printf("%c ", SubPathOpNames[iter.cmd()]);
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
        int cmd = fgetc(f);
        eat_space(f);
        switch(cmd) {
        case 'M':
            builder.start_subpath(read_point(f));
            break;
        case 'L':
            builder.push_line(read_point(f));
            break;
        case 'C':
            builder.push_cubic(read_point(f), read_point(f), read_point(f));
            break;
        case 'Q':
            builder.push_quad(read_point(f), read_point(f));
            break;
        case 'z':
            builder.close_subpath();
            break;
        default:
            ungetc(cmd, f);
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

