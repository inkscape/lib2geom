#include <2geom/sbasis-of.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <time.h>
#include <vector>
using namespace Geom;
using namespace std;

SBasis toSBasis(SBasisOf<double> const &f){
    SBasis result(f.size(), Linear());
    for (unsigned i=0; i<f.size(); i++){
        result[i] = Linear(f[i][0],f[i][1]);
    }
    return result;
}
SBasisOf<double> toSBasisOfDouble(SBasis const &f){
    SBasisOf<double> result;
    for (unsigned i=0; i<f.size(); i++){
        result.push_back(LinearOf<double>(f[i][0],f[i][1]));
    }
    return result;
}


#if 0
template<unsigned dim>
class LinearDim;
template<unsigned dim>
class SBasisDim;

template<unsigned dim>
class LinearDim : public LinearOf<SBasisDim<dim-1> >{
public:
    LinearDim() : LinearOf<SBasisDim<dim-1> >() {};
    LinearDim(SBasisDim <dim-1> const &a, SBasisDim<dim-1> const &b ) : LinearOf<SBasisDim<dim-1> >(a,b) {};
    LinearDim(LinearDim <dim-1> const &a, LinearDim<dim-1> const &b ) : 
              LinearOf<SBasisDim<dim-1> >(SBasisDim<dim-1>(a),SBasisDim<dim-1>(b)) {};
};

template<>
class LinearDim<1> : public LinearOf<double>{
public:
    LinearDim() : LinearOf<double>() {};
    LinearDim(double const &a, double const &b ) : LinearOf<double>(a,b) {};
};


template<unsigned dim>
class SBasisDim : public SBasisOf<SBasisDim<dim-1> >{
public:
    SBasisDim() : SBasisOf<SBasisDim<dim-1> >() {};
    SBasisDim(LinearDim<dim> const &lin) : 
        SBasisOf<SBasisDim<dim-1> >(LinearOf<SBasisDim<dim-1> >(lin[0],lin[1])) {};
};

template<>
class SBasisDim<1> : public SBasisOf<double>{
/*
public:
    SBasisDim<1>() : SBasisOf<double>() {};
    SBasisDim(SBasisOf<double> other) : SBasisOf<double>(other) {};
    SBasisDim(LinearOf<double> other) : SBasisOf<double>(other) {};
*/
};

SBasis toSBasis(SBasisDim<1> f){
    SBasis result(f.size(), Linear());
    for (unsigned i=0; i<f.size(); i++){
        result[i] = Linear(f[i][0],f[i][1]);
    }
    return result;
}

template<unsigned dim_f,unsigned dim_g>
SBasisDim<dim_g> compose(SBasisDim<dim_f> const &f, std::vector<SBasisDim<dim_g> > const &g ){

    assert( dim_f <= g.size() );
    
    SBasisDim<dim_g> u0 = g[dim_f-1];
    SBasisDim<dim_g> u1 = -u0 + 1;
    SBasisDim<dim_g> s = multiply(u0,u1);
    SBasisDim<dim_g> r;

    for(int i = f.size()-1; i >= 0; i--) {
        if ( dim_f>1 ){
            r = s*r + compose(f[i][0],g)*u0 + compose(f[i][1],g)*u1;
        }else{
            r = s*r + f[i][0]*u0 + f[i][1]*u1;
        }
    }
    return r;
}

#endif

template <typename T>
SBasisOf<T> multi_compose(SBasisOf<double> const &f, SBasisOf<T> const &g ){

    //assert( f.input_dim() <= g.size() );
    
    SBasisOf<T> u0 = g;
    SBasisOf<T> u1 = -u0 + LinearOf<SBasisOf<double> >(SBasisOf<double>(LinearOf<double>(1,1)));
    SBasisOf<T> s = multiply(u0,u1);
    SBasisOf<T> r;

    for(int i = f.size()-1; i >= 0; i--) {
        r = s*r + f[i][0]*u0 + f[i][1]*u1;
    }
    return r;
}
SBasisOf<double> compose(SBasisOf<SBasisOf<double> > const &f, 
                         SBasisOf<double> const &x, 
                         SBasisOf<double> const &y){
    SBasisOf<double> y0 = -y + LinearOf<double>(1,1);
    SBasisOf<double> s = multiply(y0,y);
    SBasisOf<double> r;

    for(int i = f.size()-1; i >= 0; i--) {
        r = s*r + compose(f[i][0],x)*y0 + compose(f[i][1],x)*y;
    }
    return r;
}

SBasisOf<double> compose(SBasisOf<SBasisOf<double> > const &f, 
                         D2<SBasisOf<double> > const &X){
    return compose(f, X[0], X[1]);
}

SBasisOf<double> compose(SBasisOf<SBasisOf<double> > const &f, 
                         D2<SBasis> const &X){
    return compose(f, toSBasisOfDouble(X[0]), toSBasisOfDouble(X[1]));
}

/*
static
SBasis eval_v(SBasisOf<SBasis> const &f, double v){
    SBasis result(f.size(), Linear());
    for (unsigned i=0; i<f.size(); i++){
        result[i] = Linear(f[i][0].valueAt(v),f[i][1].valueAt(v));
    }
    return result;
}
static
SBasis eval_v(SBasisOf<SBasisOf<double> > const &f, double v){
    SBasis result(f.size(), Linear());
    for (unsigned i=0; i<f.size(); i++){
        result[i] = Linear(f[i][0].valueAt(v),f[i][1].valueAt(v));
    }
    return result;
}*/
static
SBasisOf<double> eval_dim(SBasisOf<SBasisOf<double> > const &f, double t, unsigned dim){
    if (dim == 1) return f.valueAt(t);
    SBasisOf<double> result;
    for (unsigned i=0; i<f.size(); i++){
        result.push_back(LinearOf<double>(f[i][0].valueAt(t),f[i][1].valueAt(t)));
    }
    return result;
}

/*
static
SBasis eval_v(SBasisDim<2> const &f, double v){
    SBasis result;
    for (unsigned i=0; i<f.size(); i++){
        result.push_back(Linear(f[i][0].valueAt(v),f[i][1].valueAt(v)));
    }
    return result;
}
*/

struct Frame
{
    Geom::Point O;
    Geom::Point x;
    Geom::Point y;
    Geom::Point z;
    // find the point on the x,y plane that projects to P
    Point unproject(Point P) {
        return P * from_basis(x, y, O).inverse();
    }
};

void
plot3d(cairo_t *cr, SBasis const &x, SBasis const &y, SBasis const &z, Frame frame){
    D2<SBasis> curve;
    for (unsigned dim=0; dim<2; dim++){
        curve[dim] = x*frame.x[dim] + y*frame.y[dim] + z*frame.z[dim];
        curve[dim] += frame.O[dim];
    }
    cairo_d2_sb(cr, curve);
}
void
plot3d(cairo_t *cr, SBasis const &x, SBasis const &y, SBasisOf<double> const &z, Frame frame){
    D2<SBasis> curve;
    for (unsigned dim=0; dim<2; dim++){
        curve[dim] = x*frame.x[dim] + y*frame.y[dim] + toSBasis(z)*frame.z[dim];
        curve[dim] += frame.O[dim];
    }
    cairo_d2_sb(cr, curve);
}

void
plot3d(cairo_t *cr, 
       Piecewise<SBasis> const &x, 
       Piecewise<SBasis> const &y, 
       Piecewise<SBasis> const &z, Frame frame){
    
    Piecewise<SBasis> xx = partition(x,y.cuts);
    Piecewise<SBasis> xxx = partition(xx,z.cuts);
    Piecewise<SBasis> yyy = partition(y,xxx.cuts);
    Piecewise<SBasis> zzz = partition(z,xxx.cuts);
    
    for (unsigned i=0; i<xxx.size(); i++){
        plot3d(cr, xxx[i], yyy[i], zzz[i], frame);
    }
}

void
plot3d(cairo_t *cr, SBasisOf<SBasisOf<double> > const &f, Frame frame){
    int iMax = 5;
    for (int i=0; i<iMax; i++){
        plot3d(cr, Linear(0,1), Linear(i/(iMax-1.)), eval_dim(f, i/(iMax-1.), 0), frame);
        plot3d(cr, Linear(i/(iMax-1.)), Linear(0,1), eval_dim(f, i/(iMax-1.), 1), frame);
    }
}

SBasisOf<SBasisOf<double> > integral(SBasisOf<SBasisOf<double> > const &f, unsigned var){
    //variable of f = 1, variable of f's coefficients = 0. 
    if (var == 1) return integral(f);
    SBasisOf<SBasisOf<double> > result;
    for(unsigned i = 0; i< f.size(); i++) {
        result.push_back(LinearOf<SBasisOf<double> >( integral(f[i][0]),integral(f[i][1])));
    }
    return result;
}

Piecewise<SBasis> convole(SBasisOf<double> const &f, Interval dom_f, 
                          SBasisOf<double> const &g, Interval dom_g){

    if ( dom_f.extent() < dom_g.extent() ) return convole(g, dom_g, f, dom_f);

    SBasisOf<SBasisOf<double> > u,v;
    u.push_back(LinearOf<SBasisOf<double> >(LinearOf<double>(0,1),
                                            LinearOf<double>(0,1)));
    v.push_back(LinearOf<SBasisOf<double> >(LinearOf<double>(0,0),
                                            LinearOf<double>(1,1)));
    SBasisOf<SBasisOf<double> > v_u = v - u*(dom_f.extent()/dom_g.extent());
    v_u += SBasisOf<SBasisOf<double> >(SBasisOf<double>(dom_f.min()/dom_g.extent()));
    SBasisOf<SBasisOf<double> > gg = multi_compose(g,(v - u*(dom_f.extent()/dom_g.extent())));
    SBasisOf<SBasisOf<double> > ff = SBasisOf<SBasisOf<double> >(f);
    SBasisOf<SBasisOf<double> > hh = integral(ff*gg,0);
    
    Piecewise<SBasis> result;
    result.cuts.push_back(dom_f.min()+dom_g.min());
    //Note: we know dom_f.extent() >= dom_g.extent()!!
    double rho = dom_f.extent()/dom_g.extent();
    SBasisOf<double> a,b,t;
    SBasis seg;
    a = SBasisOf<double>(LinearOf<double>(0.,0.)); 
    b = SBasisOf<double>(LinearOf<double>(0.,1/rho)); 
    t = SBasisOf<double>(LinearOf<double>(dom_f.min()/dom_g.extent(),dom_f.min()/dom_g.extent()+1)); 
    seg = toSBasis(compose(hh,b,t)-compose(hh,a,t));
    result.push(seg,dom_f.min() + dom_g.max());
    if (dom_f.extent() > dom_g.extent()){
        a = SBasisOf<double>(LinearOf<double>(0.,1-1/rho)); 
        b = SBasisOf<double>(LinearOf<double>(1/rho,1.)); 
        t = SBasisOf<double>(LinearOf<double>(dom_f.min()/dom_g.extent()+1, dom_f.max()/dom_g.extent() )); 
        seg = toSBasis(compose(hh,b,t)-compose(hh,a,t));
        result.push(seg,dom_f.max() + dom_g.min());
    }
    a = SBasisOf<double>(LinearOf<double>(1.-1/rho,1.)); 
    b = SBasisOf<double>(LinearOf<double>(1.,1.)); 
    t = SBasisOf<double>(LinearOf<double>(dom_f.max()/dom_g.extent(), dom_f.max()/dom_g.extent()+1 )); 
    seg = toSBasis(compose(hh,b,t)-compose(hh,a,t));
    result.push(seg,dom_f.max() + dom_g.max());
    return result;
}

template <typename T>
SBasisOf<T> subderivative(SBasisOf<T> const& f) {
    SBasisOf<T> res;
    for(unsigned i = 0; i < f.size(); i++) {
        res.push_back(LinearOf<T>(derivative(f[i][0]), derivative(f[i][1])));
    }
    return res;
}

OptInterval bounds_fast(SBasisOf<double> const &f) {
    return bounds_fast(toSBasis(f));
}

/**
 * Finds a path which traces the 0 contour of f, traversing from A to B as a single cubic d2<sbasis>.
 * The algorithm is based on matching direction and curvature at each end point.
 */
//TODO: handle the case when B is "behind" A for the natural orientation of the level set.
//TODO: more generally, there might be up to 4 solutions. Choose the best one!
D2<SBasis>
sbofsb_cubic_solve(SBasisOf<SBasisOf<double> > const &f, Geom::Point const &A, Geom::Point const &B){
    D2<SBasis>result;//(Linear(A[X],B[X]),Linear(A[Y],B[Y]));
    //g_warning("check 0 = %f = %f!", f.apply(A[X],A[Y]), f.apply(B[X],B[Y]));

    SBasisOf<SBasisOf<double> > f_u  = derivative(f);
    SBasisOf<SBasisOf<double> > f_v  = subderivative(f);
    SBasisOf<SBasisOf<double> > f_uu = derivative(f_u);
    SBasisOf<SBasisOf<double> > f_uv = derivative(f_v);
    SBasisOf<SBasisOf<double> > f_vv = subderivative(f_v);

    Geom::Point dfA(f_u.valueAt(A[X]).valueAt(A[Y]),f_v.valueAt(A[X]).valueAt(A[Y]));
    Geom::Point dfB(f_u.valueAt(B[X]).valueAt(B[Y]),f_v.valueAt(B[X]).valueAt(B[Y]));

    Geom::Point V0 = rot90(dfA);
    Geom::Point V1 = rot90(dfB);
    
    double D2fVV0 = f_uu.valueAt(A[X]).valueAt(A[Y])*V0[X]*V0[X]+
                  2*f_uv.valueAt(A[X]).valueAt(A[Y])*V0[X]*V0[Y]+
                    f_vv.valueAt(A[X]).valueAt(A[Y])*V0[Y]*V0[Y];
    double D2fVV1 = f_uu.valueAt(B[X]).valueAt(B[Y])*V1[X]*V1[X]+
                  2*f_uv.valueAt(B[X]).valueAt(B[Y])*V1[X]*V1[Y]+
                    f_vv.valueAt(B[X]).valueAt(B[Y])*V1[Y]*V1[Y];

    std::vector<D2<SBasis> > candidates = cubics_fitting_curvature(A,B,V0,V1,D2fVV0,D2fVV1);
    if (candidates.size()==0) {
        return D2<SBasis>(Linear(A[X],B[X]),Linear(A[Y],B[Y]));
    }
    //TODO: I'm sure std algorithm could do that for me...
    double error = -1;
    unsigned best = 0;
    for (unsigned i=0; i<candidates.size(); i++){
        Interval bounds = *bounds_fast(compose(f,candidates[i]));
        double new_error = (fabs(bounds.max())>fabs(bounds.min()) ? fabs(bounds.max()) : fabs(bounds.min()) );
        if ( new_error < error || error < 0 ){
            error = new_error;
            best = i;
        }
    }
    return candidates[best];
}

class SBasis0fSBasisToy: public Toy {
    unsigned size;
    PointSetHandle hand;
    PointSetHandle cut_hand;
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        
        double slider_top = width/4.;
        double slider_bot = width*3./4.;
        double slider_margin = width/8.;
        if(hand.pts.empty()) {
            hand.pts.push_back(Geom::Point(width*3./16., 3*width/4.));
            hand.pts.push_back(hand.pts[0] + Geom::Point(width/2., 0));
            hand.pts.push_back(hand.pts[0] + Geom::Point(width/8., -width/12.));
            hand.pts.push_back(hand.pts[0] + Geom::Point(0,-width/4.));
            hand.pts.push_back(Geom::Point(slider_margin,slider_bot));
            hand.pts.push_back(Geom::Point(width-slider_margin,slider_top));
        }
        
        hand.pts[4][X] = slider_margin;
        if (hand.pts[4][Y]<slider_top) hand.pts[4][Y] = slider_top; 
        if (hand.pts[4][Y]>slider_bot) hand.pts[4][Y] = slider_bot; 
        hand.pts[5][X] = width-slider_margin;
        if (hand.pts[5][Y]<slider_top) hand.pts[5][Y] = slider_top; 
        if (hand.pts[5][Y]>slider_bot) hand.pts[5][Y] = slider_bot; 

        //double tA = (slider_bot-hand.pts[4][Y])/(slider_bot-slider_top);
        //double tB = (slider_bot-hand.pts[5][Y])/(slider_bot-slider_top);
        
        cairo_move_to(cr,Geom::Point(slider_margin,slider_bot));
        cairo_line_to(cr,Geom::Point(slider_margin,slider_top));
        cairo_move_to(cr,Geom::Point(width-slider_margin,slider_bot));
        cairo_line_to(cr,Geom::Point(width-slider_margin,slider_top));
        cairo_set_line_width(cr,.5);
        cairo_set_source_rgba (cr, 0., 0.3, 0., 1.);
        cairo_stroke(cr);
        
        Frame frame;
        frame.O = hand.pts[0];//
        frame.x = hand.pts[1]-hand.pts[0];//
        frame.y = hand.pts[2]-hand.pts[0];//
        frame.z = hand.pts[3]-hand.pts[0];// 

        plot3d(cr,Linear(0,1),Linear(0,0),Linear(0,0),frame);
        plot3d(cr,Linear(0,1),Linear(1,1),Linear(0,0),frame);
        plot3d(cr,Linear(0,0),Linear(0,1),Linear(0,0),frame);
        plot3d(cr,Linear(1,1),Linear(0,1),Linear(0,0),frame);
        cairo_set_line_width(cr,.2);
        cairo_set_source_rgba (cr, 0., 0., 0., 1.);
        cairo_stroke(cr);
        


        SBasisOf<SBasisOf<double> > f,u,v;
        u.push_back(LinearOf<SBasisOf<double> >(LinearOf<double>(-1,-1),LinearOf<double>(1,1)));
        v.push_back(LinearOf<SBasisOf<double> >(LinearOf<double>(-1,1),LinearOf<double>(-1,1)));
#if 1
        f = u*u + v*v - LinearOf<SBasisOf<double> >(LinearOf<double>(1,1),LinearOf<double>(1,1));
        //*notify << "input dim = " << f.input_dim() <<"\n";
        plot3d(cr,f,frame);
        cairo_set_line_width(cr,1);        
        cairo_set_source_rgba (cr, .5, 0.5, 0.5, 1.);
        cairo_stroke(cr);
        
        LineSegment ls(frame.unproject(cut_hand.pts[0]),
                       frame.unproject(cut_hand.pts[1]));
        SBasis cutting = toSBasis(compose(f, ls.toSBasis()));
        //cairo_sb(cr, cutting);
        //cairo_stroke(cr);
        plot3d(cr, ls.toSBasis()[0], ls.toSBasis()[1], SBasis(0.0), frame);
        vector<double> rts = roots(cutting);
        if(rts.size() >= 2) {
            Geom::Point A = ls.pointAt(rts[0]);
            Geom::Point B = ls.pointAt(rts[1]);

            //Geom::Point A(1,0.5);
            //Geom::Point B(0.5,1);
            D2<SBasis> zeroset = sbofsb_cubic_solve(f,A,B);
            plot3d(cr, zeroset[X], zeroset[Y], SBasis(Linear(0.)),frame);
            cairo_set_line_width(cr,1);        
            cairo_set_source_rgba (cr, 0.9, 0., 0., 1.);
            cairo_stroke(cr);
        }
#else

        SBasisOf<SBasisOf<double> > g = u - v ;
        g += LinearOf<SBasisOf<double> >(SBasisOf<double>(LinearOf<double>(.5,.5)));
        SBasisOf<double> h;
        h.push_back(LinearOf<double>(0,0));
        h.push_back(LinearOf<double>(0,0));
        h.push_back(LinearOf<double>(1,1));

        f = multi_compose(h,g);
        plot3d(cr,f,frame);
        cairo_set_line_width(cr,1);        
        cairo_set_source_rgba (cr, .75, 0.25, 0.25, 1.);
        cairo_stroke(cr);
/*
        SBasisDim<1> g = SBasisOf<double>(LinearOf<double>(0,1));
        g.push_back(LinearOf<double>(-1,-1));
        std::vector<SBasisDim<2> > vars;
        vars.push_back(ff);
        plot3d(cr,compose(g,vars),frame);
        cairo_set_source_rgba (cr, .5, 0.9, 0.5, 1.);
        cairo_stroke(cr);
*/
#endif
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }        
    
public:
    SBasis0fSBasisToy(){
        handles.push_back(&hand);
        handles.push_back(&cut_hand); 
        cut_hand.push_back(100,100);
        cut_hand.push_back(500,500);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new SBasis0fSBasisToy);
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
// vim: filetype = cpp:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :
