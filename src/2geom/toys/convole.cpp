#include <2geom/d2.h>
#include <2geom/sbasis-of.h>
#include <2geom/sbasis.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-geometric.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <vector>
using std::vector;
using namespace Geom;

SBasis toSBasis(SBasisOf<double> const &f){
    SBasis result;
    for (unsigned i=0; i<f.size(); i++){
        result.push_back(Linear(f[i][0],f[i][1]));
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



SBasis integral(SBasis const &c) {
    SBasis a;
    a.resize(c.size() + 1, Linear(0,0));
    a[0] = Linear(0,0);

    for(unsigned k = 1; k < c.size() + 1; k++) {
        double ahat = -Tri(c[k-1])/(2*k);
        a[k] = Hat(ahat);
    }
    double aTri = 0;
    for(int k = c.size()-1; k >= 0; k--) {
        aTri = (Hat(c[k]).d + (k+1)*aTri/2)/(2*k+1);
        a[k][0] -= aTri/2;
        a[k][1] += aTri/2;
    }
    a.normalize();
    return a;
}
template<typename T>
SBasisOf<T> integraaal(SBasisOf<T> const &c){
    SBasisOf<T> a;
    a.resize(c.size() + 1, LinearOf<T>(T(0.),T(0.)));

    for(unsigned k = 1; k < c.size() + 1; k++) {
        T ahat = (c[k-1][0]-c[k-1][1])/(2*k);
        a[k] = LinearOf<T>(ahat);
    }

    T aTri = T(0.);
    for(int k = c.size()-1; k >= 0; k--) {
        aTri = (HatOf<T>(c[k]).d + (k+1)*aTri/2)/(2*k+1);
        a[k][0] -= aTri/2;
        a[k][1] += aTri/2;
    }
    //a.normalize();
    return a;
}

SBasisOf<SBasisOf<double> > integral(SBasisOf<SBasisOf<double> > const &f, unsigned var){
    //variable of f = 1, variable of f's coefficients = 0. 
    if (var == 1) return integraaal(f);
    SBasisOf<SBasisOf<double> > result;
    for(unsigned i = 0; i< f.size(); i++) {
        //result.push_back(LinearOf<SBasisOf<double> >( integral(f[i][0]),integral(f[i][1])));
        result.push_back(LinearOf<SBasisOf<double> >( integraaal(f[i][0]),integraaal(f[i][1])));
    }
    return result;
}

template <typename T>
SBasisOf<T> multi_compose(SBasisOf<double> const &f, SBasisOf<T> const &g ){

    //assert( f.input_dim() <= g.size() );
    
    SBasisOf<T> u1 = g;
    SBasisOf<T> u0 = -g + LinearOf<SBasisOf<double> >(SBasisOf<double>(LinearOf<double>(1,1)));
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

Piecewise<SBasis> convole(SBasisOf<double> const &f, Interval dom_f, 
                          SBasisOf<double> const &g, Interval dom_g,
                          bool f_cst_ends = false){

    if ( dom_f.extent() < dom_g.extent() ) return convole(g, dom_g, f, dom_f);

    Piecewise<SBasis> result;

    SBasisOf<SBasisOf<double> > u,v;
    u.push_back(LinearOf<SBasisOf<double> >(SBasisOf<double>(LinearOf<double>(0,1))));
    v.push_back(LinearOf<SBasisOf<double> >(SBasisOf<double>(LinearOf<double>(0,0)),
                                            SBasisOf<double>(LinearOf<double>(1,1))));
    SBasisOf<SBasisOf<double> > v_u = (v - u)*(dom_f.extent()/dom_g.extent());
    v_u += SBasisOf<SBasisOf<double> >(SBasisOf<double>(-dom_g.min()/dom_g.extent()));
    SBasisOf<SBasisOf<double> > gg = multi_compose(g,v_u);
    SBasisOf<SBasisOf<double> > ff = SBasisOf<SBasisOf<double> >(f);
    SBasisOf<SBasisOf<double> > hh = integral(ff*gg,0);
    
    result.cuts.push_back(dom_f.min()+dom_g.min());
    //Note: we know dom_f.extent() >= dom_g.extent()!!
    double rho = dom_f.extent()/dom_g.extent();
    double t0 = dom_g.min()/dom_f.extent();
    double t1 = dom_g.max()/dom_f.extent();
    double t2 = t0+1;
    double t3 = t1+1;
    SBasisOf<double> a,b,t;
    SBasis seg;
    a = SBasisOf<double>(LinearOf<double>(0,0)); 
    b = SBasisOf<double>(LinearOf<double>(0,t1-t0)); 
    t = SBasisOf<double>(LinearOf<double>(t0,t1)); 
    seg = toSBasis(compose(hh,b,t)-compose(hh,a,t));
    result.push(seg,dom_f.min() + dom_g.max());
    if (dom_f.extent() > dom_g.extent()){
        a = SBasisOf<double>(LinearOf<double>(0,t2-t1)); 
        b = SBasisOf<double>(LinearOf<double>(t1-t0,1)); 
        t = SBasisOf<double>(LinearOf<double>(t1,t2)); 
        seg = toSBasis(compose(hh,b,t)-compose(hh,a,t));
        result.push(seg,dom_f.max() + dom_g.min());
    }
    a = SBasisOf<double>(LinearOf<double>(t2-t1,1.)); 
    b = SBasisOf<double>(LinearOf<double>(1.,1.)); 
    t = SBasisOf<double>(LinearOf<double>(t2,t3)); 
    seg = toSBasis(compose(hh,b,t)-compose(hh,a,t));
    result.push(seg,dom_f.max() + dom_g.max());
    result*=dom_f.extent();
    
    //--constant ends correction--------------
    if (f_cst_ends){
        SBasis ig = toSBasis(integraaal(g))*dom_g.extent();
        ig -= ig.at0();
        Piecewise<SBasis> cor;
        cor.cuts.push_back(dom_f.min()+dom_g.min());
        cor.push(reverse(ig)*f.at0(),dom_f.min()+dom_g.max());
        cor.push(Linear(0),dom_f.max()+dom_g.min());
        cor.push(ig*f.at1(),dom_f.max()+dom_g.max());
        result+=cor;
    }
    //----------------------------------------
    return result;
}

static void dot_plot(cairo_t *cr, Piecewise<D2<SBasis> > const &M, double space=10){
    //double dt=(M[0].cuts.back()-M[0].cuts.front())/space;
    Piecewise<D2<SBasis> > Mperp = rot90(derivative(M)) * 2;
    for( double t = M.cuts.front(); t < M.cuts.back(); t += space) {
        Point pos = M(t), perp = Mperp(t);
        draw_line_seg(cr, pos + perp, pos - perp);
    }
    cairo_pw_d2(cr, M);
    cairo_stroke(cr);
}

static void plot_graph(cairo_t *cr, Piecewise<SBasis> const &f,
                       double x_scale=300,
                       double y_scale=100){
    //double dt=(M[0].cuts.back()-M[0].cuts.front())/space;
    D2<Piecewise<SBasis> > g;
    g[X] = Piecewise<SBasis>( SBasis(Linear(100+f.cuts.front()*x_scale, 
                                            100+f.cuts.back()*x_scale)));
    g[X].setDomain(f.domain());
    g[Y] = -f*y_scale+400;
    cairo_d2_pw(cr, g);
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
plot3d(cairo_t *cr, SBasisOf<SBasisOf<double> > const &f, Frame frame, int NbRays=5){
        for (int i=0; i<=NbRays; i++){
            SBasisOf<double> var = LinearOf<double>(0,1);
            SBasisOf<double> cst = LinearOf<double>(i*1./NbRays,i*1./NbRays);
            SBasis f_on_seg = toSBasis(compose(f,var,cst));
            plot3d(cr,Linear(0,1),Linear(i*1./NbRays),f_on_seg,frame);
            f_on_seg = toSBasis(compose(f,cst,var));
            plot3d(cr,Linear(i*1./NbRays),Linear(0,1),f_on_seg,frame);
        }
}


#define SIZE 4

class ConvolutionToy: public Toy {

    PointHandle adjuster, adjuster2, adjuster3;

public:
    PointSetHandle b1_handle;
    PointSetHandle b2_handle;
    void draw(cairo_t *cr,
	      std::ostringstream *notify,
	      int width, int height, bool save) {
    
        D2<SBasis> B1 = b1_handle.asBezier();
        D2<SBasis> B2 = b2_handle.asBezier();
        D2<Piecewise<SBasis> >B;
        B[X].concat(Piecewise<SBasis>(B1[X]));
        B[X].concat(Piecewise<SBasis>(B2[X]));
        B[Y].concat(Piecewise<SBasis>(B1[Y]));
        B[Y].concat(Piecewise<SBasis>(B2[Y]));

//-----------------------------------------------------
#if 0
        Frame frame;
        frame.O = Point(50,400);
        frame.x = Point(300,0);
        frame.y = Point(120,-75);
        frame.z = Point(0,-300); 
        SBasisOf<SBasisOf<double> > u,v,cst;
        cst.push_back(LinearOf<SBasisOf<double> >(SBasisOf<double>(LinearOf<double>(1,1))));
        u.push_back(LinearOf<SBasisOf<double> >(SBasisOf<double>(LinearOf<double>(0,1))));
        v.push_back(LinearOf<SBasisOf<double> >(SBasisOf<double>(LinearOf<double>(0,0)),
                                                SBasisOf<double>(LinearOf<double>(1,1))));
        plot3d(cr, integral(v,1) ,frame);
        plot3d(cr, v ,frame);
        cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
        cairo_stroke(cr);
        plot3d(cr, Linear(0,1), Linear(0), Linear(0), frame);
        plot3d(cr, Linear(0), Linear(0,1), Linear(0), frame);
        plot3d(cr, Linear(0), Linear(0), Linear(0,1), frame);
        plot3d(cr, Linear(0,1), Linear(1), Linear(0), frame);
        plot3d(cr, Linear(1), Linear(0,1), Linear(0), frame);
        cairo_set_source_rgba (cr, 0., 0.0, 0.9, 1);
        cairo_stroke(cr);
#endif
//-----------------------------------------------------

        SBasisOf<double> smoother;
        smoother.push_back(LinearOf<double>(0.));
        smoother.push_back(LinearOf<double>(0.));
        smoother.push_back(LinearOf<double>(30.));//could be less degree hungry!

        adjuster.pos[X] = 400;
        if(adjuster.pos[Y]>400) adjuster.pos[Y] = 400;
        if(adjuster.pos[Y]<100) adjuster.pos[Y] = 100;
        double scale=(400.-adjuster.pos[Y])/300+.01;
        D2<Piecewise<SBasis> > smoothB1,smoothB2;
        smoothB1[X] = convole(toSBasisOfDouble(B1[X]),Interval(0,4),smoother/scale,Interval(-scale/2,scale/2));
        smoothB1[Y] = convole(toSBasisOfDouble(B1[Y]),Interval(0,4),smoother/scale,Interval(-scale/2,scale/2));
        smoothB1[X].push(Linear(0.), 8+scale/2);
        smoothB1[Y].push(Linear(0.), 8+scale/2);
        smoothB2[X] = Piecewise<SBasis>(Linear(0));
        smoothB2[X].setDomain(Interval(-scale/2,4-scale/2));
        smoothB2[Y] = smoothB2[X];
        smoothB2[X].concat(convole(toSBasisOfDouble(B2[X]),Interval(4,8),smoother/scale,Interval(-scale/2,scale/2)));
        smoothB2[Y].concat(convole(toSBasisOfDouble(B2[Y]),Interval(4,8),smoother/scale,Interval(-scale/2,scale/2)));

        Piecewise<D2<SBasis> > smoothB;
        smoothB = sectionize(smoothB1)+sectionize(smoothB2);
        //cairo_md_sb(cr, B1);
        //cairo_md_sb(cr, B2);
        cairo_pw_d2(cr, smoothB);
        
        cairo_move_to(cr,100,400);
        cairo_line_to(cr,500,400);
        cairo_set_line_width (cr, .5);
        cairo_set_source_rgba (cr, 0., 0., 0., 1);
        cairo_stroke(cr);

        Piecewise<SBasis>bx = Piecewise<SBasis>(B1[X]);
        bx.setDomain(Interval(0,4));
        Piecewise<SBasis>smth = Piecewise<SBasis>(toSBasis(smoother));
        smth.setDomain(Interval(-scale/2,scale/2));
        cairo_md_sb(cr, B1);
        plot_graph(cr, bx, 100, 1);
        plot_graph(cr, smth/scale, 100, 100);
        plot_graph(cr, smoothB1[X],100,1);
        
        //cairo_pw_d2(cr, Piecewise<D2<SBasis> >(B1) );
        //cairo_pw_d2(cr, sectionize(smoothB1));        

        cairo_set_line_width (cr, .5);
        cairo_set_source_rgba (cr, 0.7, 0.2, 0., 1);
        cairo_stroke(cr);


        Toy::draw(cr, notify, width, height, save);
    }        
  
public:
    ConvolutionToy(){
        for(int i = 0; i < SIZE; i++) {
            b1_handle.push_back(150+uniform()*300,150+uniform()*300);
            b2_handle.push_back(150+uniform()*300,150+uniform()*300);
        }
        b1_handle.pts[0] = Geom::Point(150,150);
        b1_handle.pts[1] = Geom::Point(150,150);
        b1_handle.pts[2] = Geom::Point(150,450);
        b1_handle.pts[3] = Geom::Point(450,150);
        handles.push_back(&b1_handle);
        handles.push_back(&b2_handle);

        adjuster.pos = Geom::Point(400,100+300*uniform());
        handles.push_back(&adjuster);

    }
};

int main(int argc, char **argv) {
    init(argc, argv, new ConvolutionToy);
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
//vim:filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99:
