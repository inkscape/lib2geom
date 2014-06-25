#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <2geom/svg-path-parser.h>
#include <2geom/pathvector.h>
#include <2geom/cairo-path-sink.h>

#include <cstdlib>

#include <CGAL/basic.h>
#include <CGAL/Cartesian.h>
#include <CGAL/CORE_algebraic_number_traits.h>
#include <CGAL/Arr_Bezier_curve_traits_2.h>
#include <CGAL/Gps_traits_2.h>
#include <CGAL/Boolean_set_operations_2.h>

typedef CGAL::CORE_algebraic_number_traits Nt_traits;
typedef Nt_traits::Rational Rational;
typedef Nt_traits::Algebraic Algebraic;
typedef CGAL::Cartesian<Rational> Rat_kernel;
typedef CGAL::Cartesian<Algebraic> Alg_kernel;
typedef CGAL::Arr_Bezier_curve_traits_2<Rat_kernel, Alg_kernel, Nt_traits> Traits_2;
typedef Rat_kernel::Point_2 Rat_point_2;
typedef Traits_2::Curve_2 Bezier_curve_2;
typedef Traits_2::X_monotone_curve_2 X_monotone_curve_2;
typedef CGAL::Gps_traits_2<Traits_2> Gps_traits_2;
typedef Gps_traits_2::General_polygon_2 Polygon_2;
typedef Gps_traits_2::General_polygon_with_holes_2 Polygon_with_holes_2;
//typedef std::list<Polygon_with_holes_2> Polygon_set;


using namespace Geom;

// the codes adhere to obvious bitwise operations
// e.g. BOOLOP_A & BOOLOP_B = BOOLOP_INTERSECTION
enum BooleanOperation {
    BOOLOP_ZERO = 0,
    BOOLOP_SUBTRACT_AB = 1,
    BOOLOP_SUBTRACT_BA = 2,
    BOOLOP_XOR = 3,
    BOOLOP_INTERSECTION = 4,
    BOOLOP_A = 5,
    BOOLOP_B = 6,
    BOOLOP_UNION = 7
};

typedef CGAL::CORE_algebraic_number_traits Nt_traits;
typedef Nt_traits::Rational Rational;
typedef Nt_traits::Algebraic Algebraic;
typedef CGAL::Cartesian<Rational> Rat_kernel;
typedef CGAL::Cartesian<Algebraic> Alg_kernel;
typedef CGAL::Arr_Bezier_curve_traits_2<Rat_kernel, Alg_kernel, Nt_traits>
Traits_2;
typedef Rat_kernel::Point_2 Rat_point_2;
typedef Traits_2::Curve_2 Bezier_curve_2;
typedef Traits_2::X_monotone_curve_2 X_monotone_curve_2;
typedef CGAL::Gps_traits_2<Traits_2> Gps_traits_2;
typedef Gps_traits_2::General_polygon_2 Polygon_2;
typedef Gps_traits_2::General_polygon_with_holes_2 Polygon_with_holes_2;

class CGALPathSink : public PathSink {
    Traits_2 _traits;
    Traits_2::Make_x_monotone_2 _mk_x_monotone;

    Point _subpath_start;
    Point _current_point;
    std::vector<X_monotone_curve_2> _xmcurves;
    std::vector<Polygon_2> _polys;

    void _processBcurve(Bezier_curve_2 const &bc) {
        std::vector<CGAL::Object> ov;
        X_monotone_curve_2 xmc;
        _mk_x_monotone(bc, std::back_inserter(ov));
        for (unsigned i = 0; i < ov.size(); ++i) {
            if (CGAL::assign(xmc, ov[i])) {
                _xmcurves.push_back(xmc);
            }
        }
    }
    void _flushCurves() {
        if (_xmcurves.empty()) return;

        if (_subpath_start != _current_point) {
            lineTo(_subpath_start);
        }

        Polygon_2 poly(_xmcurves.begin(), _xmcurves.end());
        CGAL::Orientation orient = poly.orientation();
        if (( _polys.empty() && (orient == CGAL::CLOCKWISE)) ||
            (!_polys.empty() && (orient == CGAL::COUNTERCLOCKWISE)))
        {
            poly.reverse_orientation();
        }
        _polys.push_back(poly);
        _xmcurves.clear();
    }

public:
    CGALPathSink()
        : _mk_x_monotone(_traits.make_x_monotone_2_object())
    {}

    void moveTo(Point const &p) {
        // new path
        _flushCurves();
        _subpath_start = p;
        _current_point = p;
    }

    void hlineTo(Coord v) {
        Point p(v, _current_point[Y]);
        lineTo(p);
    }

    virtual void vlineTo(Coord v) {
        Point p(_current_point[X], v);
        lineTo(p);
    }

    virtual void lineTo(Point const &p) {
        std::vector<Rat_point_2> pts;
        pts.reserve(2);
        pts.push_back(Rat_point_2(_current_point[X], _current_point[Y]));
        pts.push_back(Rat_point_2(p[X], p[Y]));
        _processBcurve(Bezier_curve_2(pts.begin(), pts.end()));
        _current_point = p;
    }

    virtual void curveTo(Point const &p1, Point const &p2, Point const &p3) {
        std::vector<Rat_point_2> pts;
        pts.reserve(4);
        pts.push_back(Rat_point_2(_current_point[X], _current_point[Y]));
        pts.push_back(Rat_point_2(p1[X], p1[Y]));
        pts.push_back(Rat_point_2(p2[X], p2[Y]));
        pts.push_back(Rat_point_2(p3[X], p3[Y]));
        _processBcurve(Bezier_curve_2(pts.begin(), pts.end()));
        _current_point = p3;
    }

    virtual void quadTo(Point const &p1, Point const &p2) {
        std::vector<Rat_point_2> pts;
        pts.reserve(3);
        pts.push_back(Rat_point_2(_current_point[X], _current_point[Y]));
        pts.push_back(Rat_point_2(p1[X], p1[Y]));
        pts.push_back(Rat_point_2(p2[X], p2[Y]));
        _processBcurve(Bezier_curve_2(pts.begin(), pts.end()));
        _current_point = p2;
    }

    virtual void arcTo(double rx, double ry, double angle,
                       bool large_arc, bool sweep, Point const &p)
    {
        // XXX
        lineTo(p);
    }

    void closePath() {
        _flushCurves();
        _current_point = _subpath_start;
    }
    void flush() {
        _flushCurves();
    }

    Polygon_with_holes_2 getResult() {
        _flushCurves();
        Polygon_with_holes_2 res(_polys.front(), ++_polys.begin(), _polys.end());
        return res;
    }
};

void sink_CGAL(PathSink &sink, Polygon_2 const &poly)
{
    typedef Polygon_2::Curve_const_iterator CIter;
    bool in_path = false;

    for (CIter ci = poly.curves_begin(); ci != poly.curves_end(); ++ci) {
        Bezier_curve_2 bc = ci->supporting_curve();
        std::pair<double, double> range = ci->parameter_range();

        std::vector<Point> pts;
        for (unsigned i = 0; i < bc.number_of_control_points(); ++i) {
            double x = CGAL::to_double(bc.control_point(i).x());
            double y = CGAL::to_double(bc.control_point(i).y());
            Point p(x, y);
            if (!p.isFinite()) {
                std::cout << "infinite point produced by CGAL" << std::endl;
            }
            pts.push_back(p);
        }
        std::auto_ptr<BezierCurve> geombc(BezierCurve::create(pts));
        std::auto_ptr<BezierCurve> part(dynamic_cast<BezierCurve*>(geombc->portion(range.first, range.second)));

        if (!in_path) {
            sink.moveTo(part->initialPoint());
            in_path = true;
        }

        std::vector<Point> part_pts;
        for (unsigned i = 1; i <= part->order(); ++i) {
            Point p = (*part)[i];
            if (p.isFinite()) {
                part_pts.push_back(p);
            } else {
                part_pts.push_back(Point(0,0));
            }
        }

        switch (part_pts.size() + 1) {
        case 2:
            sink.lineTo(part_pts[0]);
            break;
        case 3:
            sink.quadTo(part_pts[0], part_pts[1]);
            break;
        case 4:
            sink.curveTo(part_pts[0], part_pts[1], part_pts[2]);
            break;
        default:
            sink.lineTo(part_pts.back());
            break;
        }
    }
    sink.closePath();
}

void sink_CGAL(PathSink &sink, Polygon_with_holes_2 const &hpoly)
{
    typedef Polygon_with_holes_2::Hole_const_iterator PIter;

    sink_CGAL(sink, hpoly.outer_boundary());
    for (PIter pi = hpoly.holes_begin(); pi != hpoly.holes_end(); ++pi) {
        sink_CGAL(sink, *pi);
    }
}

PathVector boolean_operation(PathVector const &a, PathVector const &b, unsigned type)
{
    Polygon_with_holes_2 p1, p2;
    {
        CGALPathSink sink;
        sink.pathvector(a);
        sink.flush();
        p1 = sink.getResult();
    }
    {
        CGALPathSink sink;
        sink.pathvector(b);
        sink.flush();
        p2 = sink.getResult();
    }

    std::vector<Polygon_with_holes_2> result;
    CGAL::intersection(p1, p2, std::back_inserter(result));

    PathBuilder pb;
    typedef std::vector<Polygon_with_holes_2>::iterator PolyIter;
    for (PolyIter i = result.begin(); i != result.end(); ++i) {
        sink_CGAL(pb, *i);
    }
    return pb.peek();
}

class BoolOpsCGAL : public Toy {
    std::vector<Toggle> toggles;
    PathVector as, bs;
    PointHandle p;
    Point b_offset;

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        Geom::Translate t(p.pos - b_offset);
        PathVector bst = bs * t;
        
        cairo_set_line_width(cr, 1);
               
        unsigned ttl = 0, v = 1;
        for(unsigned i = 0; i < 4; i++, v*=2)
            if(toggles[i].on) ttl += v; 
        
        
        PathVector s = boolean_operation(as, bst, 0);

        CairoPathSink sink(cr);

        sink.pathvector(as);
        cairo_set_source_rgba(cr, 1, 0, 0, 0.5);
        cairo_fill(cr);

        sink.pathvector(bst);
        cairo_set_source_rgba(cr, 0, 1, 0, 0.5);
        cairo_fill(cr);

        sink.pathvector(s);
        cairo_set_source_rgba(cr, 0, 0, 0, 1);
        //cairo_set_line_width(cr, 2);
        cairo_fill(cr);

        if(toggles[4].on) {
            cairo_save(cr);
            cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
            cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
            cairo_set_source_rgba(cr, 1, 0, 0, 0.25);
            cairo_set_line_width(cr, 8);
            //show_stitches(cr, s);
            cairo_restore(cr);
        }

        double x = width - 60, y = height - 60;

        //Draw the info

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 1);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER);
        
        draw_text(cr, Point(x + 20, y - 34), "A");
        draw_text(cr, Point(x + 5, y - 18),  "T");
        draw_text(cr, Point(x + 32, y - 18), "F");
        
        draw_text(cr, Point(x - 25, y + 17), "B");
        draw_text(cr, Point(x - 15, y + 2),  "T");
        draw_text(cr, Point(x - 15, y + 28), "F");

        draw_text(cr, Point(width - 425, height - 70), "KEY:");
        draw_text(cr, Point(width - 425, height - 50), "T/F = Containment/Non-containment,");
        draw_text(cr, Point(width - 425, height - 30), "Q/W/A/S = The keys on the keyboard");
        
        Point p(x, y), d(25,25), xo(25,0), yo(0,25);
        toggles[2].bounds = Rect(p,     p + d);
        toggles[0].bounds = Rect(p + xo, p + xo + d);
        toggles[1].bounds = Rect(p + yo, p + yo + d);
        toggles[3].bounds = Rect(p + d, p + d + d);
        toggles[4].bounds = Rect(Point(10, y)+d, Point(10+120, y+d[1])+d);

        draw_toggles(cr, toggles);

        //*notify << "Operation: " << (mode ? (mode == 1 ? "union" : (mode == 2 ? "subtract" : (mode == 3 ? "intersect" : "exclude"))) : "none");
        //*notify << "\nKeys:\n u = Union   s = Subtract   i = intersect   e = exclude   0 = none   a = invert A   b = invert B \n";
        
        //*notify << "A " << (as.isFill() ? "" : "not") << " filled, B " << (bs.isFill() ? "" : "not") << " filled..\n";
        cairo_set_line_width(cr, 1);

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
    void key_hit(GdkEventKey *e) {
        if(e->keyval == 'w') toggles[0].toggle(); else
        if(e->keyval == 'a') toggles[1].toggle(); else
        if(e->keyval == 'q') toggles[2].toggle(); else
        if(e->keyval == 's') toggles[3].toggle();
        redraw();
    }
    void mouse_pressed(GdkEventButton* e) {
        toggle_events(toggles, e);
        Toy::mouse_pressed(e);
    }
    public:
    BoolOpsCGAL () {}

    void first_time(int argc, char** argv) {
        const char *path_a_name="banana.svgd";
        const char *path_b_name="star.svgd";
        if(argc > 1)
            path_a_name = argv[1];
        if(argc > 2)
            path_b_name = argv[2];

        as = read_svgd(path_a_name);
        bs = read_svgd(path_b_name);

        p = PointHandle(Point(300,300));
        handles.push_back(&p);

        b_offset = initialPoint(bs);
        
        toggles.push_back(Toggle("W", true));
        toggles.push_back(Toggle("A", true));
        toggles.push_back(Toggle("Q", true));
        toggles.push_back(Toggle("S", false));
        toggles.push_back(Toggle("Show Stitches", false));
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new BoolOpsCGAL());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
