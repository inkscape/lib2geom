#include <iostream>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/path-intersection.h>
#include <2geom/basic-intersection.h>
#include <2geom/pathvector.h>
#include <2geom/exception.h>

#include <cstdlib>
#include <cstdio>
#include <set>
#include <vector>
#include <algorithm>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/ord.h>

#include "topology.cpp"

static double exp_rescale(double x){ return pow(10, x);}
std::string exp_formatter(double x){ return default_formatter(exp_rescale(x));}


bool even_odd (int winding){ return (winding%2); }
bool non_zero (int winding){ return  winding   ; }

bool inter_op (bool inA, bool inB){ return  inA &&  inB; }
bool union_op (bool inA, bool inB){ return  inA ||  inB; }
bool minus_op (bool inA, bool inB){ return  inA && !inB; }
bool sunim_op (bool inA, bool inB){ return  inB && !inA; }
bool delta_op (bool inA, bool inB){ return  inA ^   inB; }


class BoolOpsTester: public Toy {
    unsigned nb_paths;
    unsigned nb_curves_per_path;
    unsigned degree;
    double tol;

    Slider slider;
    PathVector cmd_line_pathsA, cmd_line_pathsB;

    PointHandle handleA, handleB;
    int nb_steps;
    bool (*boolop) (bool, bool);
    bool (*interior_rule) (int);

    Topology topo;

    int windingA(unsigned area){
    	int result=0;
    	for (unsigned i=0; i<cmd_line_pathsA.size(); i++){
    		result += topo.areas[area].windings[i];
    	}
    	return result;
    }
    int windingB(unsigned area){
    	int result=0;
    	for (unsigned i=cmd_line_pathsA.size(); i<cmd_line_pathsA.size()+cmd_line_pathsB.size(); i++){
    		result += topo.areas[area].windings[i];
    	}
    	return result;
    }


    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        *notify<<"1: A && B\n2: A||B\n3: A-B\n4: B-A\n5: A^B\n6:even/odd fill rule\n7:non zero fill rule.";


    	cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);

        Point offsetA = handleA.pos - cmd_line_pathsA[0].initialPoint();
        cmd_line_pathsA *= Translate(offsetA);
        Point offsetB = handleB.pos - cmd_line_pathsB[0].initialPoint();
        cmd_line_pathsB *= Translate(offsetB);

        PathVector paths = cmd_line_pathsA;
        paths.insert(paths.end(), cmd_line_pathsB.begin(), cmd_line_pathsB.end());
#if 1
        cairo_path(cr, paths);
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, .5);
        cairo_stroke(cr);
#endif

        tol = exp_rescale( slider.value() );
        topo = Topology(paths, cr, tol );
        cairo_set_source_rgba (cr, 0.7, 0., 0., 1.);
        topo.drawBoxes(cr);

        cairo_set_source_rgba (cr, 0.7, 0.7, 1., 1.);
        for (unsigned area=0; area<topo.areas.size(); area++ ){
        	int wA = windingA(area);
        	int wB = windingB(area);
        	if ( boolop(interior_rule(wA),interior_rule(wB)) ){
        		topo.drawArea(cr, area,true);
        	}
        }
        Toy::draw(cr, notify, width, height, save, timer_stream);
    }

    void key_hit(GdkEventKey *e)
    {
        char choice = std::toupper(e->keyval);
        switch ( choice )
        {
            case '1':
                boolop = inter_op;
                break;
            case '2':
                boolop = union_op;
                break;
            case '3':
                boolop = minus_op;
                break;
            case '4':
                boolop = sunim_op;
                break;
            case '5':
                boolop = delta_op;
                break;
            case '6':
                interior_rule = even_odd;
                break;
            case '7':
                interior_rule = non_zero;
                break;
        }
        redraw();
    }

    public:

    BoolOpsTester(PathVector input_pathsA, PathVector input_pathsB){

    	cmd_line_pathsA = input_pathsA;
    	cmd_line_pathsB = input_pathsB;

    	handleA = PointHandle( 10+ uniform()*300, 10+ uniform()*300);
    	handleB = PointHandle( 10+ uniform()*300, 10+ uniform()*300);
        handles.push_back( &handleA );
        handles.push_back( &handleB );

        //TODO: use path iterators to deal with closed/open paths!!!
        //cmd_line_paths[i].close();
        for(unsigned i = 0; i < cmd_line_pathsA.size(); i++){
        	cmd_line_pathsA[i].close();
        	if ( cmd_line_pathsA[i].closed() ){
                cmd_line_pathsA[i].appendNew<LineSegment>(cmd_line_pathsA[i].initialPoint() );
            }
        }        
        boolop = inter_op;
        interior_rule = even_odd;
        for(unsigned i = 0; i < cmd_line_pathsB.size(); i++){
        	cmd_line_pathsB[i].close();
            if ( cmd_line_pathsB[i].closed() ){
                cmd_line_pathsB[i].appendNew<LineSegment>(cmd_line_pathsB[i].initialPoint() );
            }
        }

        slider = Slider(-5.0, 2, 0, 0.0, "tolerance");
        handles.push_back(&slider);
        slider.geometry(Point(50, 20), 250);
        slider.formatter( &exp_formatter );

    }

};

int main(int argc, char **argv) {
    const char *path_nameA = "star.svgd";
    const char *path_nameB = "banana.svgd";
    if(argc == 3){
        path_nameA = argv[1];
        path_nameB = argv[2];
    }
    PathVector cmd_line_pathsA = read_svgd(path_nameA);
    PathVector cmd_line_pathsB = read_svgd(path_nameB);

    init(argc, argv, new BoolOpsTester(cmd_line_pathsA, cmd_line_pathsB));
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
