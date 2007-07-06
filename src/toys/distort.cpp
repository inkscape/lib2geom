#include "d2.h"
#include "piecewise.h"
#include "sbasis.h"
#include "sbasis-2d.h"
#include "svg-path-parser.h"

#include "path-cairo.h"
#include "toy-framework.cpp"

#include <vector>
using namespace Geom;

class DistortToy: public Toy {
    std::vector<Geom::Path> p;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        D2<SBasis2d> sb2;
        for(unsigned dim = 0; dim < 2; dim++) {
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
            for(unsigned vi = 0; vi < sb2[0].vs; vi++)
                for(unsigned ui = 0; ui < sb2[0].us; ui++)
                for(unsigned iv = 0; iv < 2; iv++)
                        for(unsigned iu = 0; iu < 2; iu++)
                        handles.push_back(Geom::Point((2*(iu+ui)/(2.*ui+1)+1)*width/4.,
                                                        (2*(iv+vi)/(2.*vi+1)+1)*width/4.));
        }
        
        for(unsigned dim = 0; dim < 2; dim++) {
            Geom::Point dir(0,0);
            dir[dim] = 1;
            for(unsigned vi = 0; vi < sb2[dim].vs; vi++) {
                for(unsigned ui = 0; ui < sb2[dim].us; ui++) {
                    for(unsigned iv = 0; iv < 2; iv++) {
                        for(unsigned iu = 0; iu < 2; iu++) {
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

        for(unsigned dim = 0; dim < 2; dim++)
            for(unsigned i = 0; i < sb2[dim].size(); i++)
                for(unsigned j = 0; j < 4; j++)
                    sb2[dim][i][j] *= 3;

         //**** FRESH STARTS HERE
        //Theoretically we could concatenate a md_Piecewise<SBasis> and do it in one go
        for(unsigned i = 0; i < p.size(); i++) {
            D2<Piecewise<SBasis> > foo = p[i].toMdSb();
            foo *= 1./20;
            //foo[1] = foo[1] + (Piecewise<SBasis>)portion(sin(Linear(0,3.14), 4), 0, foo[0].cuts.back());
            D2<Piecewise<SBasis> > out;          
            //out[0] = compose(sb2[0], foo);
            //out[1] = compose(sb2[1], foo);
            //cairo_d2_pw(cr, out);
        }
        //**** AND ENDS HERE

        Toy::draw(cr, notify, width, height, save);
    }

    public:
    DistortToy () {
        FILE* f = fopen("banana.svgd", "r");
        char string[1000];
        fgets(string, 1000, f);
        p = parse_svg_path(string);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new DistortToy());
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
