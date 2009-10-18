#include <2geom/toys/lpe-framework.h>

using std::vector;
using namespace Geom;
using namespace std;

// Author: Johan Engelen, 2009
//-----------------------------------------------

class LPETest: public LPEToy {
public:
    LPETest() {
        concatenate_before_pwd2 = false;
    }

    Geom::Piecewise<Geom::D2<Geom::SBasis> >
    doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in)
    {
        using namespace Geom;

        Piecewise<D2<SBasis> > pwd2_out = pwd2_in;

        Point vector(50,100);
        // generate extrusion bottom: (just a copy of original path, displaced a bit)
        pwd2_out.concat( pwd2_in + vector );

        // generate connecting lines (the 'sides' of the extrusion)
        Path path(Point(0.,0.));
        path.appendNew<Geom::LineSegment>( vector );
        Piecewise<D2<SBasis> > connector = path.toPwSb();
        // connecting lines should be put at cusps
        Piecewise<D2<SBasis> > deriv = derivative(pwd2_in);
        std::vector<double> cusps; // = roots(deriv);
        for (unsigned i = 0; i < cusps.size() ; ++i) {
            pwd2_out.concat( connector + pwd2_in.valueAt(cusps[i]) );
        }
        // connecting lines should be put where the tangent of the path equals the extrude_vector in direction
        std::vector<double> rts = roots(dot(deriv, rot90(vector)));
        for (unsigned i = 0; i < rts.size() ; ++i) {
            pwd2_out.concat( connector + pwd2_in.valueAt(rts[i]) );
        }

        return pwd2_out;
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new LPETest);
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
//vim:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :


