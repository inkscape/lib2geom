#include <2geom/convex-cover.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/d2.h>
#include <2geom/geom.h>

#include <aa.h>

using std::vector;
using namespace Geom;

// draw ax + by + c = 0
void draw_line_in_rect(cairo_t*cr, Rect &r, Point n, double c) {
    vector<Geom::Point> result;
    Point resultp;
    if(intersects == line_intersection(Point(1, 0), r.left(),
				       n, c,
				       resultp) && r[1].contains(resultp[1]))
	result.push_back(resultp);
    if(intersects == line_intersection(Point(1, 0), r.right(),
				       n, c,
				       resultp) && r[1].contains(resultp[1]))
	result.push_back(resultp);
    if(intersects == line_intersection(Point(0, 1), r.top(),
				       n, c,
				       resultp) && r[0].contains(resultp[0]))
	result.push_back(resultp);
    if(intersects == line_intersection(Point(0, 1), r.bottom(),
				       n, c,
				       resultp) && r[0].contains(resultp[0]))
	result.push_back(resultp);
    if(result.size() == 2) {
	cairo_move_to(cr, result[0]);
	cairo_line_to(cr, result[1]);
	cairo_stroke(cr);
    }


}

boost::optional<Rect> tighten(Rect &r, Point n, Interval lu) {
    vector<Geom::Point> result;
    Point resultp;
    for(int i = 0; i < 4; i++) {
        Point cnr = r.corner(i);
        double z = dot(cnr, n);
        if((z > lu[0]) and (z < lu[1]))
            result.push_back(cnr);
    }
    for(int i = 0; i < 2; i++) {
        double c = lu[i];
        if(intersects == line_intersection(Point(1, 0), r.left(),
                                           n, c,
                                           resultp) && r[1].contains(resultp[1]))
            result.push_back(resultp);
        if(intersects == line_intersection(Point(1, 0), r.right(),
                                           n, c,
                                           resultp) && r[1].contains(resultp[1]))
            result.push_back(resultp);
        if(intersects == line_intersection(Point(0, 1), r.top(),
                                           n, c,
                                           resultp) && r[0].contains(resultp[0]))
            result.push_back(resultp);
        if(intersects == line_intersection(Point(0, 1), r.bottom(),
                                           n, c,
                                           resultp) && r[0].contains(resultp[0]))
            result.push_back(resultp);
    }
    if(result.size() < 2)
        return boost::optional<Rect>();
    Rect nr(result[0], result[1]);
    for(size_t i = 2; i < result.size(); i++) {
        nr.expandTo(result[i]);
    }
    return intersect(nr, r);
}

AAF eval(AAF x, AAF y) {
  x = x-500;
  y = y-300;
  x = x/20;
  y = y/20;
  //return x*y;
  //return 4*x+3*y-1;
  //return x*x + y*y - 200*200;
  //return pow((x*x + y*y), 2) - (x*x-y*y);
  //return x*x-y;
  return (x*x*x-y*x)*sin(x) + (x-y*y)*cos(y)-0.5;
}

class ConvexTest: public Toy {
    public:
    ConvexTest () {
    }
    int iters;
    int splits[4];
  
    void recursive_implicit(Rect r, cairo_t*cr, double w) {
        iters++;
	AAF x(interval(r.left(), r.right()));
	AAF y(interval(r.top(), r.bottom()));
        assert(x.rad() > 0);
        assert(y.rad() > 0);
	AAF f = eval(x, y);
        double a = f.index_coeff(x.get_index(0))/x.index_coeff(x.get_index(0));
        double b = f.index_coeff(y.get_index(0))/y.index_coeff(y.get_index(0));
        AAF d = a*x + b*y - f;
        interval ivl = d.convert();
        Point n(a,b);
        boost::optional<Rect> out = tighten(r, n, Interval(ivl.left(), ivl.right()));
        if(ivl.width() < 1*L2(n)) {
            draw_line_in_rect(cr, r, n, ivl.mid());
            return;
        }
	if((r.width() > w) or (r.height()>w)) {
	    if(f.straddles_zero()) {
                // Three possibilities:
                // 1) the trim operation buys us enough that we should just iterate
                Point c = r.midpoint();
                Rect oldr = r;
                if(out)
                    r = *out;
#if 1
                if(out && (r.area() < oldr.area()*0.75)) {
                    splits[0] ++;
                    recursive_implicit(r, cr, w);
                // 2) one dimension is significantly smaller
                } else if(r[0].extent() < r[1].extent()*0.5) {
                    splits[1]++;
                    recursive_implicit(Rect(Interval(r.left(), r.right()),
                                            Interval(r.top(), c[1])), cr,w);
                    recursive_implicit(Rect(Interval(r.left(), r.right()),
                                            Interval(c[1], r.bottom())), cr,w);
                } else if((r[1].extent() < r[0].extent()*0.5)) {
                    splits[2]++;
                    recursive_implicit(Rect(Interval(r.left(), c[0]),
                                            Interval(r.top(), r.bottom())), cr,w);
                    recursive_implicit(Rect(Interval(c[0], r.right()),
                                            Interval(r.top(), r.bottom())), cr,w);
                // 3) to ensure progress we must do a four way split
                } else 
#endif
                {
                    splits[3]++;
                    recursive_implicit(Rect(Interval(r.left(), c[0]),
                                            Interval(r.top(), c[1])), cr,w);
                    recursive_implicit(Rect(Interval(c[0], r.right()),
                                            Interval(r.top(), c[1])), cr,w);
                    recursive_implicit(Rect(Interval(r.left(), c[0]),
                                            Interval(c[1], r.bottom())), cr,w);
                    recursive_implicit(Rect(Interval(c[0], r.right()),
                                            Interval(c[1], r.bottom())), cr,w);
                }
	    }
	} else {
            cairo_rectangle(cr, r);
            cairo_stroke(cr);
        }
    }
    
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        iters = 0;
        for(int i = 0; i < 4; i++)
            splits[i] = 0;
	recursive_implicit(Rect(Interval(0,width), Interval(0, height)), cr, 3);
        for(int i = 0; i < 4; i++)
            *notify << splits[i] << " + ";
        *notify << " = " << iters;

        Toy::draw(cr, notify, width, height, save);
    }

};

int main(int argc, char **argv) {
    init(argc, argv, new ConvexTest());

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
