#include "path-cairo.h"

#include <iterator>
#include <iostream>
#include <vector>

#include "toy-framework.cpp"

#include "path2.h"
#include "read-svgd.h"
#include "d2.h"
#include "pw-sb.h"
#include "s-basis-2d.h"

using namespace Geom;

class DistortToy: public Toy {
    std::vector<Geom::Path2::Path> p;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        D2<SBasis2d> sb2;
        for(int dim = 0; dim < 2; dim++) {
            sb2[dim].us = 2;
            sb2[dim].vs = 2;
            const int depth = sb2[dim].us*sb2[dim].vs;
            const int surface_handles = 4*depth;
            sb2[dim].resize(depth, Linear2d(0));
        }
        const int depth = sb2[0].us*sb2[0].vs;
        const int surface_handles = 4*depth;
        Geom::Point dir(1,-2);
        if(handles.empty()) {
            for(int vi = 0; vi < sb2[0].vs; vi++)
                for(int ui = 0; ui < sb2[0].us; ui++)
                for(int iv = 0; iv < 2; iv++)
                        for(int iu = 0; iu < 2; iu++)
                        handles.push_back(Geom::Point((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
                                                        (2*(iv+vi)/(2.*vi+1)+1)*width/4.));
        }
        
        for(int dim = 0; dim < 2; dim++) {
            Geom::Point dir(0,0);
            dir[dim] = 1;
            for(int vi = 0; vi < sb2[dim].vs; vi++) {
                for(int ui = 0; ui < sb2[dim].us; ui++) {
                    for(int iv = 0; iv < 2; iv++) {
                        for(int iu = 0; iu < 2; iu++) {
                            unsigned corner = iu + 2*iv;
                            unsigned i = ui + vi*sb2[dim].us;
                            Geom::Point base((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
                                                (2*(iv+vi)/(2.*vi+1)+1)*width/4.);
                            if(vi == 0 && ui == 0) {
                                base = Geom::Point(width/4., width/4.);
                            }
                            double dl = dot((handles[corner+4*i] - base), dir)/dot(dir,dir);
                            sb2[dim][i][corner] = dl/(width/2)*pow(4.0,ui+vi);
                        }
                    }
                }
            }
        }
        
        cairo_2dsb2d(cr, sb2, dir*0.1, width);
        cairo_set_source_rgba (cr, 0., 0., 0, 0.5);
        cairo_stroke(cr);

        for(int dim = 0; dim < 2; dim++)
            for(int i = 0; i < sb2[dim].size(); i++)
                for(int j = 0; j < 4; j++)
                    sb2[dim][i][j] *= 3;

         //**** FRESH STARTS HERE
        //Theoretically we could concatenate a md_pw_sb and do it in one go
        for(int i = 0; i < p.size(); i++) {
            D2<pw_sb> foo = p[i].toMdSb();
            foo *= 1./20;
            //foo[1] = foo[1] + (pw_sb)portion(sin(Linear(0,3.14), 4), 0, foo[0].cuts.back());
            D2<pw_sb> out;          
            //out[0] = compose(sb2[0], foo);
            //out[1] = compose(sb2[1], foo);
            //cairo_md_pw(cr, out);
        }
        //**** AND ENDS HERE

        Toy::draw(cr, notify, width, height, save);
    }

    public:
    DistortToy () {
        FILE* f = fopen("banana.svgd", "r");
        p = read_svgd(f);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "Distort Toy", new DistortToy());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
