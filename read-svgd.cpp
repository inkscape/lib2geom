#include "read-svgd.h"
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
    printf("size %d %d \n",  p.cmd.size(),  p.handles.size());
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
    Geom::Path path;
    assert(f);
    
    while(!feof(f)) {
        eat_space(f);
        int cmd = fgetc(f);
        eat_space(f);
        switch(cmd) {
        case 'M':
            path.subpaths.push_back(Geom::SubPath());
            path.subpaths.back().cmd.push_back(Geom::moveto);
            path.subpaths.back().handles.push_back(read_point(f));
            break;
        case 'L':
            path.subpaths.back().cmd.push_back(Geom::lineto);
            path.subpaths.back().handles.push_back(read_point(f));
            break;
        case 'C':
            path.subpaths.back().cmd.push_back(Geom::cubicto);
            path.subpaths.back().handles.push_back(read_point(f));
            path.subpaths.back().handles.push_back(read_point(f));
            path.subpaths.back().handles.push_back(read_point(f));
            break;
        case 'Q':
            path.subpaths.back().cmd.push_back(Geom::quadto);
            path.subpaths.back().handles.push_back(read_point(f));
            path.subpaths.back().handles.push_back(read_point(f));
            break;
        case 'z':
            path.subpaths.back().closed = true;
            break;
        default:
            ungetc(cmd, f);
            break;
        }
    }
    return path;
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
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

