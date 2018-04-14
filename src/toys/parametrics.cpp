#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/sbasis-math.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>
#include <2geom/affine.h>

#include <glib.h>
#include <vector>
#include <iostream>
using std::vector;
using namespace Geom;

int mode;

static void dot_plot(cairo_t *cr, Piecewise<D2<SBasis> > const &M, double max, double space=10){
    Piecewise<D2<SBasis> > Mperp = rot90(derivative(M)) * 3;
    for( double t = M.cuts.front(); t < max; t += space) {
        Point pos = M(t), perp = Mperp(t);
        draw_line_seg(cr, pos + perp, pos - perp);
    }
    cairo_stroke(cr);
}

static void draw_axis(cairo_t *cr, Piecewise<D2<SBasis> > const &pw, unsigned d, Affine m) {
    double mult;
    if(abs(mode)==1) mult = 20;
    if(abs(mode)==2) mult = 1;
    if(abs(mode)==3) mult = 100;
    if(abs(mode)==4) mult = 20;
    if(abs(mode)==5) mult = 20;
    if(abs(mode)==6) mult = 100;
    for(unsigned i = 0; i < pw.size(); i++) {
        cairo_d2_sb(cr, D2<SBasis>(SBasis(pw.cuts[i]-pw.cuts[0], pw.cuts[i+1]-pw.cuts[0])*mult, SBasis(pw[i][d]))*m);
    }
}
/*
void dump_latex(PathVector ps) {
    for(unsigned d = 0; d < 2; d++) {
        std::cout << "$$\n" << (d?"y":"x") << "(t) = \\left\\{\n\\begin{array}{ll}\n";
        int seg = 0;
	for(unsigned i = 0; i < ps.size(); i++)
        for(unsigned j = 0; j < ps[i].size(); j++) {
            Bezier<3> &b = dynamic_cast<Bezier<3>& >(const_cast<Curve&>(ps[i][j]));
	    std::cout << b[0][d] << "(" << seg+1 << "-t)^3 + "
		 << 3*b[1][d] << "t(" << seg+1 << "-t)^2 + "
		 << 3*b[2][d] << "t^2(" << seg+1 << "-t) + "
		 << b[3][d] << "t^3,& " << seg << "\\leq t < " << seg+1 << "\\\\\n";
            seg++;
        }
        std::cout << "\\end{array}\n$$\n";
    }
}
*/
class Parametrics: public Toy {
    Piecewise<D2<SBasis> > cat, alcat, box, arc, monk, traj;
#ifdef USE_TIME
    GTimer* time;
    bool st;
#endif
    double waitt;
    double t;
    int count;
    void draw(cairo_t *cr,
	      std::ostringstream *notify,
	      int width, int height, bool save, std::ostringstream *timer_stream) override {
      //double t = handles[0][0] / 20.;

#ifdef USE_TIME
      gulong* foo = 0;
      t = g_timer_elapsed(time, foo) * 100;
#else
      double inc;
      if(mode==1) inc = .1;
      if(mode==2) inc = 5;
      if(mode==3) inc = .01;
      if(mode==4) inc = .04;
      if(mode==5) inc = .1;
      if(mode==6) inc = .01;
      if(mode<0) inc = .01*M_PI;
      if(!save && !waitt) {
          t += inc;
      }
      if(waitt) waitt += 1;
      if(waitt>20) waitt = 0;
#endif
      Piecewise<D2<SBasis> > obj;
      if(abs(mode)==1) obj = cat;
      if(abs(mode)==2) obj = alcat;
      if(abs(mode)==3) obj = arc;
      if(abs(mode)==4) obj = box;
      if(abs(mode)==5) obj = monk;
      if(abs(mode)==6) obj = traj;
      if(t==obj.cuts.back()) t += inc/2;
      cairo_set_source_rgb(cr, 1,1,1);
      if(save) {
          cairo_rectangle(cr, 0, 0, width, height);
          cairo_fill(cr);
      }
      Piecewise<D2<SBasis> > port, rport;
      if(mode>0) {
          port = portion(obj, 0, t); 
          rport = mode>0? portion(obj, t, obj.cuts[obj.size()]) : obj;
          cairo_set_source_rgba (cr, 0., 0., 0., 1);
	  Point curpt = rport[0].at0();
	  if(t<obj.cuts.back()) {
              draw_line_seg(cr, curpt, Point(curpt[0], 350));
              draw_line_seg(cr, curpt, Point(350, curpt[1]));
              cairo_stroke(cr);
          }

          char tlab[64];
          sprintf(tlab, "t=%.02f", t);
          draw_text(cr, curpt, tlab , true);

          cairo_set_line_width (cr, 2);
          cairo_set_source_rgba (cr, 0., 0.5, 0., 1);
          cairo_pw_d2_sb(cr, port);
          cairo_stroke(cr);
      }
      if(mode>=0 && t>=obj.cuts.back()+inc) t = 0;
      cairo_set_source_rgba (cr, 0.9, 0., 0., 1);
      if(mode<0) {
          draw_axis(cr, obj, 0, from_basis(Point(cos(t),sin(t)),Point(sin(t),-cos(t)),Point(0, 350)));
          if(cos(t) <= 0) {
              mode = -mode;
              t = 0;
              waitt = 1;
          }
      } else
          draw_axis(cr, rport, 0, from_basis(Point(0,1),Point(1,0),Point(0, 350)));
      cairo_stroke(cr);

      cairo_set_source_rgba (cr, 0., 0., 0.9, 1);
      if(mode<0)
           draw_axis(cr, obj, 1, from_basis(Point(1,0),Point(0,1),Point(350*t/M_PI*2, 0)));
      else
           draw_axis(cr, rport, 1, from_basis(Point(1,0),Point(0,1),Point(350, 0)));
      cairo_stroke(cr);

      if(mode==2 && t>0) {
      cairo_set_line_width (cr, 1);
      cairo_set_source_rgba (cr, 0., 0., 0.9, 1);
      dot_plot(cr, port, t);
      cairo_stroke(cr);
      }

      if(!save) {
      char file[100];
      sprintf(file, "output/%04d.png", count);
      //take_screenshot(file);
      count++;
      }
     // *notify << "pieces = " << alcat.size() << ";\n";

      Toy::draw(cr, notify, width, height, save,timer_stream);
      redraw();
    }

#ifdef USE_TIME
    virtual void mouse_moved(GdkEventMotion* e) {
        if(st) {
            g_timer_start(time);
            st = false;
        }
        Toy::mouse_moved(e);
    }
#endif

  public:
    Parametrics(){
      mode = 2;
      PathVector cp = read_svgd("cat.svgd");
      //dump_latex(cp);
      cat = paths_to_pw(cp);
      cat *= .3;
      cat += Point(50, 50);
      alcat = arc_length_parametrization(cat);

      monk = paths_to_pw(read_svgd("monkey.svgd"));
      //monk *= .3;
      monk += Point(50,50);
      
      arc = sectionize(D2<Piecewise<SBasis> >(cos(Linear(0,M_PI))*120, sin(Linear(0,M_PI))*-120));
      arc += Point(200, 200);
      
      box = Piecewise<D2<SBasis> >();
      box.push_cut(0);
      box.push(D2<SBasis>(SBasis(100.,300.), SBasis(100.)), 1);
      box.push(D2<SBasis>(SBasis(300.), SBasis(100.,300.)), 2);
      box.push(D2<SBasis>(SBasis(300.,100.), SBasis(300.)), 3);
      box.push(D2<SBasis>(SBasis(100.), SBasis(300.,100.)), 4);
      //handles.push_back(Point(100, 100));
      traj = Piecewise<D2<SBasis> >();
      SBasis quad = Linear(0,1)*Linear(0,1)*256-Linear(0,256)+200;
      traj.push_cut(0);
      traj.push(D2<SBasis>(SBasis(100.,300.),SBasis(quad)), 1);
#ifdef USE_TIME
      time = g_timer_new();
      g_timer_reset(time);
      st = true;
#endif
      waitt = 0;
      count = 0;
      t = 0;
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new Parametrics, 720, 480);
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
//vim:filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99:
