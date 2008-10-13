#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/sbasis-math.h>
#include <2geom/sbasis-2d.h>
#include <2geom/bezier-to-sbasis.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <gsl/gsl_poly.h>

using std::vector;
using namespace Geom;

//see a sb2d as an sb of u with coef in sbasis of v.
void
u_coef(SBasis2d f, unsigned deg, SBasis &a, SBasis &b) {
    a = SBasis();
    b = SBasis();
    for (unsigned v=0; v<f.vs; v++){
        a.push_back(Linear(f.index(deg,v)[0], f.index(deg,v)[2]));
        b.push_back(Linear(f.index(deg,v)[1], f.index(deg,v)[3]));
    }
}
void
v_coef(SBasis2d f, unsigned deg, SBasis &a, SBasis &b) {
    a = SBasis();
    b = SBasis();
    for (unsigned u=0; u<f.us; u++){
        a.push_back(Linear(f.index(deg,u)[0], f.index(deg,u)[1]));
        b.push_back(Linear(f.index(deg,u)[2], f.index(deg,u)[3]));
    }
}

SBasis2d partial_derivative(SBasis2d const &f, int dim) {
    SBasis2d result;
    for(unsigned i = 0; i < f.size(); i++) {
        result.push_back(Linear2d(0,0,0,0));
    }
    result.us = f.us;
    result.vs = f.vs;

    for(unsigned i = 0; i < f.us; i++) {
        for(unsigned j = 0; j < f.vs; j++) {
            Linear2d lin = f.index(i,j);
            Linear2d dlin(lin[1+dim]-lin[0], lin[1+2*dim]-lin[dim], lin[3-dim]-lin[2*(1-dim)], lin[3]-lin[2-dim]);
            result[i+j*result.us] += dlin;
            unsigned di = dim?j:i;
            if (di>=1){
                float motpi = dim?-1:1;
                Linear2d ds_lin_low( lin[0], -motpi*lin[1], motpi*lin[2], -lin[3] );
                result[(i+dim-1)+(j-dim)*result.us] += di*ds_lin_low;
                
                Linear2d ds_lin_hi( lin[1+dim]-lin[0], lin[1+2*dim]-lin[dim], lin[3]-lin[2-dim], lin[3-dim]-lin[2-dim] );
                result[i+j*result.us] += di*ds_lin_hi;                
            }
        }
    }
    return result;
}

D2<SBasis>
sb2dsolve(SBasis2d const &f, Geom::Point const &A, Geom::Point const &B, unsigned degmax=2){
    D2<SBasis>result(Linear(A[X],B[X]),Linear(A[Y],B[Y]));
    g_warning("check f(A)= %f = f(B) = %f =0!", f.apply(A[X],A[Y]), f.apply(B[X],B[Y]));

    SBasis2d dfdu = partial_derivative(f, 0);
    SBasis2d dfdv = partial_derivative(f, 1);
    Geom::Point dfA(dfdu.apply(A[X],A[Y]),dfdv.apply(A[X],A[Y]));
    Geom::Point dfB(dfdu.apply(B[X],B[Y]),dfdv.apply(B[X],B[Y]));
    Geom::Point nA = dfA/(dfA[X]*dfA[X]+dfA[Y]*dfA[Y]);
    Geom::Point nB = dfB/(dfB[X]*dfB[X]+dfB[Y]*dfB[Y]);

    double fact_k=1;
    double sign = 1.;
    for(unsigned k=1; k<degmax; k++){
        // these two lines make the solutions worse!
        //fact_k *= k;
        //sign = -sign;
        SBasis f_on_curve = compose(f,result);
        Linear reste = f_on_curve[k];
        double ax = -reste[0]/fact_k*nA[X];
        double ay = -reste[0]/fact_k*nA[Y];
        double bx = -sign*reste[1]/fact_k*nB[X];
        double by = -sign*reste[1]/fact_k*nB[Y];

        result[X].push_back(Linear(ax,bx));
        result[Y].push_back(Linear(ay,by));
        //sign *= 3;
    }    
    return result;
}

//TODO: handle the case when B is "behind" A for the natural orientation of the level set.
//TODO: more generally, there might be up to 4 solutions. Choose the best one!
D2<SBasis>
sb2d_cubic_solve(SBasis2d const &f, Geom::Point const &A, Geom::Point const &B){
    D2<SBasis>result;//(Linear(A[X],B[X]),Linear(A[Y],B[Y]));
    g_warning("check 0 = %f = %f!", f.apply(A[X],A[Y]), f.apply(B[X],B[Y]));

    SBasis2d f_u  = partial_derivative(f  , 0);
    SBasis2d f_v  = partial_derivative(f  , 1);
    SBasis2d f_uu = partial_derivative(f_u, 0);
    SBasis2d f_uv = partial_derivative(f_v, 0);
    SBasis2d f_vv = partial_derivative(f_v, 1);

    Geom::Point dfA(f_u.apply(A[X],A[Y]),f_v.apply(A[X],A[Y]));
    Geom::Point dfB(f_u.apply(B[X],B[Y]),f_v.apply(B[X],B[Y]));

    Geom::Point V0 = rot90(dfA);
    Geom::Point V1 = rot90(dfB);
    
    double D2fVV0 = f_uu.apply(A[X],A[Y])*V0[X]*V0[X]+
                  2*f_uv.apply(A[X],A[Y])*V0[X]*V0[Y]+
                    f_vv.apply(A[X],A[Y])*V0[Y]*V0[Y];
    double D2fVV1 = f_uu.apply(B[X],B[Y])*V1[X]*V1[X]+
                  2*f_uv.apply(B[X],B[Y])*V1[X]*V1[Y]+
                    f_vv.apply(B[X],B[Y])*V1[Y]*V1[Y];

    std::vector<D2<SBasis> > candidates = cubics_fitting_curvature(A,B,V0,V1,D2fVV0,D2fVV1);
    if (candidates.size()==0) {
        return D2<SBasis>(Linear(A[X],B[X]),Linear(A[Y],B[Y]));
    }
    //TODO: I'm sure std algorithm could do that for me...
    double error = -1;
    unsigned best = 0;
    for (unsigned i=0; i<candidates.size(); i++){
        Interval bounds = bounds_fast(compose(f,candidates[i]));
        double new_error = (fabs(bounds.max())>fabs(bounds.min()) ? fabs(bounds.max()) : fabs(bounds.min()) );
        if ( new_error < error || error < 0 ){
            error = new_error;
            best = i;
        }
    }
    return candidates[best];
}




//TODO: implement sb2d algebra!!
SBasis2d y_x2(){
    SBasis2d result(Linear2d(0,-1,1,0));    
    result.push_back(Linear2d(1,1,1,1));
    result.us = 2;
    result.vs = 1;
    return result;
}

SBasis2d x2_plus_y2_1(){
/*TODO: implement sb2d algebra!!
    SBasis2d one(Linear2d(1,1,1,1));
    SBasis2d u(Linear2d(0,1,0,1));
    SBasis2d v(Linear2d(0,0,1,1));
    return(u*u+v*v-one);
*/
    SBasis2d result(Linear2d(-1,0,0,1));//x+y-1    
    result.push_back(Linear2d(-1,-1,-1,-1));
    result.push_back(Linear2d(-1,-1,-1,-1));
    result.push_back(Linear2d(0,0,0,0));
    result.us = 2;
    result.vs = 2;
    return result;
}

struct Frame
{
    Geom::Point O;
    Geom::Point x;
    Geom::Point y;
    Geom::Point z;
};

void
plot3d(cairo_t *cr, SBasis const &x, SBasis const &y, SBasis const &z, Frame frame){
    D2<SBasis> curve;
    for (unsigned dim=0; dim<2; dim++){
        curve[dim] = x*frame.x[dim] + y*frame.y[dim] + z*frame.z[dim];
        curve[dim] += frame.O[dim];
    }
    cairo_md_sb(cr, curve);
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
plot3d(cairo_t *cr, SBasis2d const &f, Frame frame, int NbRays=5){
        for (int i=0; i<=NbRays; i++){
            D2<SBasis> seg(Linear(0,1),Linear(i*1./NbRays,i*1./NbRays));
            SBasis f_on_seg = compose(f,seg);
            plot3d(cr,seg[X],seg[Y],f_on_seg,frame);
        }
        for (int i=0; i<NbRays; i++){
            D2<SBasis> seg(Linear(i*1./NbRays,i*1./NbRays),Linear(0,1));
            SBasis f_on_seg = compose(f,seg);
            plot3d(cr,seg[X],seg[Y],f_on_seg,frame);
        }
}

void
plot3d_top(cairo_t *cr, SBasis2d const &f, Frame frame, int NbRays=5){
        for (int i=0; i<=NbRays; i++){
            for(int j=0; j<2; j++){
                D2<SBasis> seg;
                if (j==0){
                    seg = D2<SBasis>(Linear(0,1),Linear(i*1./NbRays,i*1./NbRays));
                }else{
                    seg = D2<SBasis>(Linear(i*1./NbRays,i*1./NbRays),Linear(0,1));
                }
                SBasis f_on_seg = compose(f,seg);
                std::vector<double> rts = roots(f_on_seg);
                if (rts.size()==0||rts.back()<1) rts.push_back(1.);
                double t1,t0 = 0;
                for (unsigned i=(rts.front()<=0?1:0); i<rts.size(); i++){
                    t1 = rts[i];
                    if (f_on_seg((t0+t1)/2)>0) 
                        plot3d(cr,seg[X](Linear(t0,t1)),seg[Y](Linear(t0,t1)),f_on_seg(Linear(t0,t1)),frame);
                    t0=t1;
                }
            //plot3d(cr,seg[X],seg[Y],f_on_seg,frame);
            }
        }
}

class Sb2dSolverToy: public Toy {
public:
    PointSetHandle hand;
    Sb2dSolverToy() {
        handles.push_back(&hand);
    }
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {

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

        double tA = (slider_bot-hand.pts[4][Y])/(slider_bot-slider_top);
        double tB = (slider_bot-hand.pts[5][Y])/(slider_bot-slider_top);

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

#if 0
        SBasis2d f = y_x2();
        D2<SBasis> true_solution(Linear(0,1),Linear(0,1));
        true_solution[Y].push_back(Linear(-1,-1));
        SBasis zero = SBasis(Linear(0.));
        Geom::Point A = true_solution(tA);
        Geom::Point B = true_solution(tB);

#else
        SBasis2d f = x2_plus_y2_1();
        D2<Piecewise<SBasis> > true_solution;
        true_solution[X] = cos(SBasis(Linear(0,3.14/2)));
        true_solution[Y] = sin(SBasis(Linear(0,3.14/2)));
        Piecewise<SBasis> zero = Piecewise<SBasis>(SBasis(Linear(0.)));
        Geom::Point A = true_solution(tA);
        Geom::Point B = true_solution(tB);
#endif

        plot3d(cr,Linear(0,1),Linear(0,0),Linear(0,0),frame);
        plot3d(cr,Linear(0,1),Linear(1,1),Linear(0,0),frame);
        plot3d(cr,Linear(0,0),Linear(0,1),Linear(0,0),frame);
        plot3d(cr,Linear(1,1),Linear(0,1),Linear(0,0),frame);
        cairo_set_line_width(cr,.2);
        cairo_set_source_rgba (cr, 0., 0., 0., 1.);
        cairo_stroke(cr);

        plot3d_top(cr,f,frame);
        cairo_set_line_width(cr,1);        
        cairo_set_source_rgba (cr, .5, 0.5, 0.5, 1.);
        cairo_stroke(cr);
        plot3d(cr,f,frame);
        cairo_set_line_width(cr,.2);        
        cairo_set_source_rgba (cr, .5, 0.5, 0.5, 1.);
        cairo_stroke(cr);

        plot3d(cr, true_solution[X], true_solution[Y], zero, frame);
        cairo_set_line_width(cr,.5);
        cairo_set_source_rgba (cr, 0., 0., 0., 1.);
        cairo_stroke(cr);
        double error;
        for(int degree = 1; degree < 4; degree++) {
            //D2<SBasis> zeroset = sb2dsolve(f,A,B,degree);
            D2<SBasis> zeroset = sb2d_cubic_solve(f,A,B);
            plot3d(cr, zeroset[X], zeroset[Y], SBasis(Linear(0.)),frame);
            cairo_set_line_width(cr,1);        
            cairo_set_source_rgba (cr, 0.9, 0., 0., 1.);
            cairo_stroke(cr);
            
            SBasis comp = compose(f,zeroset);
            plot3d(cr, zeroset[X], zeroset[Y], comp, frame);
            cairo_set_source_rgba (cr, 0.7, 0., 0.7, 1.);
            cairo_stroke(cr);
            //Fix Me: bounds_exact does not work here?!?!
            Interval bounds = bounds_fast(comp);
            error = (bounds.max()>-bounds.min() ? bounds.max() : -bounds.min() );
        }
        *notify << "Gray: f-graph and true solution,\n";
        *notify << "Red: solver solution,\n";
        *notify << "Purple: value of f over solver solution.\n";
        *notify << "  error: "<< error <<".\n";
                
        Toy::draw(cr, notify, width, height, save);
    }
};

int main(int argc, char **argv) {
        init(argc, argv, new Sb2dSolverToy());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
