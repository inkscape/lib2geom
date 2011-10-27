/** 
 * adaptive quad tree for display graph, search operations
 * (njh)
 */

#include <2geom/quadtree.h>
#include <2geom/path.h>
#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/svg-path-parser.h>

#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/sbasis-2d.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/transforms.h>
#include <2geom/sbasis-math.h>
#include <2geom/pathvector.h>


using std::vector;
using namespace Geom;

void draw_quad_tree(cairo_t* cr, Geom::Quad *q, double x, double y, double d) {
    if(q) {
        cairo_rectangle(cr, x, y, d, d);
        cairo_stroke(cr);
        double dd = d/2;
        draw_quad_tree(cr, q->children[0], x, y, dd);
        draw_quad_tree(cr, q->children[1], x+dd, y, dd);
        draw_quad_tree(cr, q->children[2], x, y+dd, dd);
        draw_quad_tree(cr, q->children[3], x+dd, y+dd, dd);
    }
}

// returns true if the subtree is empty, and deletes any empty subtrees.
bool clean_quad_tree(Geom::Quad *q) { 
    if(q) {
        bool all_clean = q->data.empty();
        for(unsigned i = 0; i < 4; i++)
            if(clean_quad_tree(q->children[i])) {
                delete q->children[i];
                q->children[i] = 0;
            } else if(q->children[i])
                all_clean = false;
        if(all_clean) {
            return true;
        }
    }
    return false;
}


// We could be a lot cleverer here:
// for crop we actually only want axis aligned stuff, so inner is just one of the dims.
// We could possibly add curve specific forms
// perform all clipping at once
PathVector
half_plane(Coord d, Point n, PathVector path_a) {
    PathVector res;
    for(PathVector::const_iterator it = path_a.begin(); it != path_a.end(); ++it) {
	for(unsigned i = 0; i <= it->size(); i++) {
	    D2<SBasis> curpw = (*it)[i].toSBasis();
	    SBasis inner = n[0]*curpw[0] + n[1]*curpw[1] - d;
	    vector<double> lr = roots(inner);
	    sort(lr.begin(), lr.end());
	    lr.insert(lr.begin(), 0);
	    lr.insert(lr.end(), 1);
	    for(unsigned j = 0; j < lr.size()-1; j++) {
		/*Point s = curpw(lr[j]);
		Point e = curpw(lr[j+1]);*/
		Point m = curpw((lr[j] + lr[j+1])/2);
		double dd = dot(n, m) - d;
		if(0 > dd) {
		    res.push_back((*it).portion(i+lr[j], i+lr[j+1]));
		}
	    }
	}
    }
    return res;
}

PathVector
crop(Rect const &r, PathVector in) {
    return half_plane(-r[0].min(), Point(-1,0), 
		      half_plane(r[0].max(), Point(1,0),
				 half_plane(-r[1].min(), Point(0,-1), 
					    half_plane(r[1].max(), Point(0,1), in))));
}

void split_to_quad_tree(Geom::QuadTree & /*qt*/, 
			PathVector /*pv*/) {
    // not implemented
}

class QuadToy: public Toy {
    Geom::PathVector paths_a;
    Geom::QuadTree qt;
    Point centre;
    PointHandle offset_handle;
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        
	PathVector out = paths_a*Translate(offset_handle.pos - centre);
	Geom::Rect r(Geom::Interval(250, 500), 
		     Geom::Interval(250, 500));
        cairo_set_source_rgba (cr, 0., 1, 0, 0.2);
        cairo_set_line_width (cr, 3);
	cairo_path(cr, out);
	cairo_stroke(cr);
	out = crop(qt.root->bounds(3, qt.bx0, qt.by0, qt.bx1 - qt.bx0), out);
        cairo_set_source_rgba (cr, 0., 0, 0, 1);
        cairo_set_line_width (cr, 1);
	cairo_path(cr, out);
	cairo_stroke(cr);
	
        cairo_set_source_rgba (cr, 0.5, 0.125, 0, 1);
        draw_quad_tree(cr, qt.root, qt.bx0, qt.by0, qt.bx1 - qt.bx0);
    
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    virtual int should_draw_bounds () { return 0; }
    virtual bool should_draw_numbers () { return false; }

    virtual void first_time(int argc, char** argv) {
        const char *path_a_name="star.svgd";
        if(argc > 1)
            path_a_name = argv[1];
        paths_a = read_svgd(path_a_name);
        assert(paths_a.size() > 0);
	double area;
	assert(0 == centroid(paths_a[0].toPwSb(), centre, area));
	offset_handle = centre;
    }
    public:
    QuadToy() {
      for(int i =0; i < 100; i++) {
	Geom::Rect r(Geom::Interval(uniform()*500, uniform()*500), 
		     Geom::Interval(uniform()*500, uniform()*500));
	qt.insert(r, 0);
      }
      handles.push_back(&offset_handle);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new QuadToy);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
