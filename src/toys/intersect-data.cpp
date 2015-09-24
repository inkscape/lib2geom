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

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>
#include <2geom/bezier-to-sbasis.h>
#include <2geom/ord.h>

#include "topology.cpp"


static double exp_rescale(double x){ return pow(10, x);}
std::string exp_formatter(double x){ return default_formatter(exp_rescale(x));}


class IntersectDataTester: public Toy {
    unsigned nb_paths;
    unsigned nb_curves_per_path;
    unsigned degree;
    double tol;

    PathVector cmd_line_paths;

    std::vector<PointSetHandle> paths_handles;
    std::vector<Slider> sliders;
    int nb_steps;

    Topology topo;

    //TODO conversions to path should be owned by the relevant classes.
    Path edgeToPath(Topology::OrientedEdge o_edge){
        Topology::Edge e = topo.edges[o_edge.edge];
        D2<SBasis> p = topo.input_paths[e.path][e.curve].toSBasis();
        Interval dom = e.portion;
        p = portion(p, dom);
        if ( o_edge.reversed ){
            p = compose( p, Linear(1.,0.) );
        }
        Path ret;
        ret.setStitching(true);
        Point center;
        unsigned c_idx = topo.source(o_edge, true);
        if ( c_idx == NULL_IDX ){
            ret.append(p);
        }else{
            center = topo.vertices[c_idx].bounds.midpoint();
            ret = Path(center);
            ret.append(p);
        }
        c_idx = topo.target(o_edge, true);
        if ( c_idx == NULL_IDX ){
            return ret;
        }else{
            center = topo.vertices[c_idx].bounds.midpoint();
            if ( center != p.at1() ) ret.appendNew<LineSegment>(center);
            return ret;
        }
    }

    Path boundaryToPath(Topology::Boundary b){
        Point pt;
        Path bndary;
        bndary.setStitching(true);

        if (b.size()==0){ return Path(); }

        Topology::OrientedEdge o_edge = b.front();
        unsigned first_v = topo.source(o_edge, true);
        if ( first_v != NULL_IDX ){
            pt = topo.vertices[first_v].bounds.midpoint();
            bndary = Path(pt);
        }

        for (unsigned i = 0; i < b.size(); i++){
            bndary.append( edgeToPath(b[i]));
        }
        bndary.close();
        return bndary;
    }

    //TODO:this should return a path vector, but we glue the components for easy drawing in the toy.
    Path areaToPath(unsigned a){
        Path bndary;
        bndary.setStitching(true);
        if ( topo.areas[a].boundary.size()==0 ){//this is the unbounded component...
            OptRect bbox = bounds_fast( topo.input_paths );
            if (!bbox ){return Path();}//???
            bbox->expandBy(50);
            bndary = Path(bbox->corner(0));
            bndary.appendNew<LineSegment>(bbox->corner(1));
            bndary.appendNew<LineSegment>(bbox->corner(2));
            bndary.appendNew<LineSegment>(bbox->corner(3));
            bndary.appendNew<LineSegment>(bbox->corner(0));
        }else{
            bndary =  boundaryToPath(topo.areas[a].boundary);
        }
        for (unsigned j = 0; j < topo.areas[a].inner_boundaries.size(); j++){
            bndary.append( boundaryToPath(topo.areas[a].inner_boundaries[j]));
            bndary.appendNew<LineSegment>( bndary.initialPoint() );
        }
        bndary.close();
        return bndary;
    }
    void drawAreas( cairo_t *cr, Topology const &topo, bool fill=true ){
        //don't draw the first one...
        for (unsigned a=0; a<topo.areas.size(); a++){
            drawArea(cr, topo, a, fill);
        }
    }
    void drawArea( cairo_t *cr, Topology const &topo, unsigned a, bool fill=true ){
        if (a>=topo.areas.size()) return;
        Path bndary = areaToPath(a);
        cairo_path(cr, bndary);
        double r,g,b;

        int winding = 0;
        for (unsigned k=0; k<topo.areas[a].windings.size(); k++){
            winding += topo.areas[a].windings[k];
        }

        //convertHSVtoRGB(0, 1., .5 + winding/10, r,g,b);
        //convertHSVtoRGB(360*a/topo.areas.size(), 1., .5, r,g,b);
        convertHSVtoRGB(180+30*winding, 1., .5, r,g,b);
        cairo_set_source_rgba (cr, r, g, b, 1);
        //cairo_set_source_rgba (cr, 1., 0., 1., .3);

        if (fill){
            cairo_fill(cr);
        }else{
            cairo_set_line_width (cr, 5);
            cairo_stroke(cr);
        }
    }

    void highlightRay( cairo_t *cr, Topology &topo, unsigned b, unsigned r ){
        if (b>=topo.vertices.size()) return;
        if (r>=topo.vertices[b].boundary.size()) return;
        Rect box = topo.vertices[b].bounds;
        //box.expandBy(2);
        cairo_rectangle(cr, box);
        cairo_set_source_rgba (cr, 1., 0., 0, 1.0);
        cairo_set_line_width (cr, 1);
        cairo_fill(cr);
        unsigned eidx = topo.vertices[b].boundary[r].edge;
        Topology::Edge e = topo.edges[eidx];
        D2<SBasis> p = topo.input_paths[e.path][e.curve].toSBasis();
        Interval dom = e.portion;
        if (topo.vertices[b].boundary[r].reversed){
            //dom[0] += e.portion.extent()*2./3;
            cairo_set_source_rgba (cr, 0., 1., 0., 1.0);
        }else{
            //dom[1] -= e.portion.extent()*2./3;
            cairo_set_source_rgba (cr, 0., 0., 1., 1.0);
        }
        p = portion(p, dom);
        cairo_d2_sb(cr, p);
        cairo_set_source_rgba (cr, 1., 0., 0, 1.0);
        cairo_set_line_width (cr, 5);
        cairo_stroke(cr);
    }

    void drawEdge( cairo_t *cr, Topology const &topo, unsigned eidx ){
        if (eidx>=topo.edges.size()) return;
        Topology::Edge e = topo.edges[eidx];
        D2<SBasis> p = topo.input_paths[e.path][e.curve].toSBasis();
        Interval dom = e.portion;
        p = portion(p, dom);
        cairo_d2_sb(cr, p);
        if (e.start == NULL_IDX || e.end == NULL_IDX )
            cairo_set_source_rgba (cr, 0., 1., 0, 1.0);
        else
            cairo_set_source_rgba (cr, 0., 0., 0, 1.0);
        cairo_set_line_width (cr, 1);
        cairo_stroke(cr);
    }
    void drawEdges( cairo_t *cr, Topology const &topo ){
        for (unsigned e=0; e<topo.edges.size(); e++){
            drawEdge(cr, topo, e);
        }
    }
    void drawKnownEdges( cairo_t *cr, Topology const &topo ){
        for (unsigned v=0; v<topo.vertices.size(); v++){
            for (unsigned e=0; e<topo.vertices[v].boundary.size(); e++){
                drawEdge(cr, topo, topo.vertices[v].boundary[e].edge);
            }
        }
    }


    void drawBox( cairo_t *cr, Topology const &topo, unsigned b ){
        if (b>=topo.vertices.size()) return;
        Rect box = topo.vertices[b].bounds;
        //box.expandBy(5);
        cairo_rectangle(cr, box);
        cairo_set_source_rgba (cr, 1., 0., 0, .5);
        cairo_set_line_width (cr, 1);
        cairo_stroke(cr);
        cairo_rectangle(cr, box);
        cairo_set_source_rgba (cr, 1., 0., 0, .2);
        cairo_fill(cr);

//         //std::cout<<"\nintersection boundary:\n";
//         for (unsigned i = 0; i < topo.vertices[b].boundary.size(); i++){
//             unsigned eidx = topo.vertices[b].boundary[i].edge;
//             Topology::Edge e = topo.edges[eidx];
//             D2<SBasis> p = topo.input_paths[e.path][e.curve].toSBasis();
//             Interval dom = e.portion;
//             if (topo.vertices[b].boundary[i].reversed){
//                 dom[0] += e.portion.extent()*2./3;
//                 cairo_set_source_rgba (cr, 0., 1., .5, 1);
//             }else{
//                 dom[1] -= e.portion.extent()*2./3;
//                 cairo_set_source_rgba (cr, 0., .5, 1., 1);
//             }
//             p = portion(p, dom);
//             cairo_d2_sb(cr, p);
//             cairo_set_line_width (cr, 2);
//             cairo_stroke(cr);
//         }
    }
    void drawBoxes( cairo_t *cr, Topology const &topo ){
        for (unsigned b=0; b<topo.vertices.size(); b++){
            drawBox(cr, topo, b);
        }
    }

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        *notify<<"line command args: svgd file or (nb paths, nb curves/path, degree of curves).\n";
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);

        PathVector paths;
        if (!cmd_line_paths.empty()){
            paths = cmd_line_paths;
            for (unsigned i = 0; i < paths.size(); i++){
                paths[i] *= Translate( paths_handles[i].pts[0] - paths[i].initialPoint() );
            }
        }else{
            for (unsigned i = 0; i < nb_paths; i++){
                paths_handles[i].pts.back()=paths_handles[i].pts.front();
                paths.push_back(Path(paths_handles[i].pts[0]));
                for (unsigned j = 0; j+degree < paths_handles[i].size(); j+=degree){
                    D2<SBasis> c = handles_to_sbasis(paths_handles[i].pts.begin()+j, degree);
                    if ( j + degree == paths_handles[i].size()-1 ){
                        c[X].at(0)[1] = paths_handles[i].pts.front()[X];
                        c[Y].at(0)[1] = paths_handles[i].pts.front()[Y];
                    }
                    paths[i].append(c);
                }
                paths[i].close();
            }
        }
        *notify<<"Use '<' and '>' keys to move backward/forward in the sweep: (currently doing "<<nb_steps<<" steps)\n";
        *notify<<"nb_steps: "<<nb_steps<<"\n";


#if 0
        cairo_path(cr, paths);
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        cairo_stroke(cr);
#endif

        tol = exp_rescale( sliders[3].value() );
        topo = Topology(paths, cr, tol, nb_steps );

#if 1
        unsigned v = (unsigned)(sliders[0].value()*(double(topo.vertices.size())));
        unsigned r = (unsigned)(sliders[1].value()*(double(topo.vertices[v].boundary.size())));
        unsigned a = (unsigned)(sliders[2].value()*(double(topo.areas.size())));
        if( v == topo.vertices.size() ) v--;
        if( r == topo.vertices[v].boundary.size()) r--;
        if( a == topo.areas.size()) a--;
        drawAreas(cr, topo);
        drawKnownEdges(cr, topo);
        //drawArea(cr, topo, a, false);
        //highlightRay(cr, topo, v, r );
        //*notify<<"highlighted edge: "<< topo.vertices[v].boundary[r].edge<<"\n";

        //drawBox(cr,topo, unsigned(sliders[0].value()));
        drawBoxes(cr,topo);
#endif
        Toy::draw(cr, notify, width, height, save, timer_stream);
    }


    void initSliders(){
        sliders.push_back(Slider(0.0, 1, 0, 0.0, "intersection chooser"));
        sliders.push_back(Slider(0.0, 1, 0, 0.0, "ray chooser"));
        sliders.push_back(Slider(0.0, 1, 0, 0.0, "area chooser"));
        sliders.push_back(Slider(-5.0, 2, 0, 0.0, "tolerance chooser"));

        handles.push_back(&(sliders[0]));
        handles.push_back(&(sliders[1]));
        handles.push_back(&(sliders[2]));
        handles.push_back(&(sliders[3]));

        sliders[0].geometry(Point(50, 20), 250);
        sliders[1].geometry(Point(50, 50), 250);
        sliders[2].geometry(Point(50, 80), 250);
        sliders[3].geometry(Point(50, 110), 250);
        sliders[3].formatter( &exp_formatter );

    }

    public:
    IntersectDataTester(PathVector input_paths){
        cmd_line_paths = input_paths;
        //nb_paths=0; nb_curves_per_path = 0; degree = 0;//meaningless
        paths_handles = std::vector<PointSetHandle>( cmd_line_paths.size(), PointSetHandle() );
        for(unsigned i = 0; i < cmd_line_paths.size(); i++){
            //TODO: use path iterators to deal with closed/open paths!!!
            //cmd_line_paths[i].close();
            if ( cmd_line_paths[i].closed() ){
                cmd_line_paths[i].appendNew<LineSegment>(cmd_line_paths[i].initialPoint() );
            }
            Point p = cmd_line_paths[i].initialPoint();
            paths_handles.push_back(PointSetHandle());
            paths_handles[i].push_back(p);
            handles.push_back( &paths_handles[i] );
        }        
        initSliders();
    }

    IntersectDataTester(unsigned paths, unsigned curves_in_path, unsigned degree) :
        nb_paths(paths), nb_curves_per_path(curves_in_path), degree(degree) {

        paths_handles = std::vector<PointSetHandle>( nb_paths, PointSetHandle() );
        for(unsigned i = 0; i < nb_paths; i++){
            for(unsigned j = 0; j < (nb_curves_per_path*degree)+1; j++){
                paths_handles[i].push_back(uniform()*400, 100+ uniform()*300);
            }
            handles.push_back(&paths_handles[i]);
        }
        initSliders();
    }

    IntersectDataTester(){
        nb_paths=3; nb_curves_per_path = 5; degree = 1;

        paths_handles.push_back(PointSetHandle());
        paths_handles[0].push_back(100,100);
        paths_handles[0].push_back(100,200);
        paths_handles[0].push_back(300,200);
        paths_handles[0].push_back(300,100);
        paths_handles[0].push_back(100,100);

        paths_handles.push_back(PointSetHandle());
        paths_handles[1].push_back(120,190);
        paths_handles[1].push_back(200,210);
        paths_handles[1].push_back(280,190);
        paths_handles[1].push_back(200,300);
        paths_handles[1].push_back(120,190);

        paths_handles.push_back(PointSetHandle());
        paths_handles[2].push_back(180,150);
        paths_handles[2].push_back(200,140);
        paths_handles[2].push_back(220,150);
        paths_handles[2].push_back(300,160);
        paths_handles[2].push_back(180,150);

        handles.push_back(&paths_handles[0]);
        handles.push_back(&paths_handles[1]);
        handles.push_back(&paths_handles[2]);

        initSliders();
    }


    void first_time(int /*argc*/, char** /*argv*/) {
        nb_steps = -1;
    }

    void key_hit(GdkEventKey *e)
    {
        char choice = std::toupper(e->keyval);
        switch ( choice )
        {
            case '>':
                nb_steps++;
                break;
            case '<':
                if ( nb_steps > -1 ) nb_steps--;
                break;
        }
        redraw();
    }

};

int main(int argc, char **argv) {
    if(argc == 2){
        const char *path_name = argv[1];
        PathVector cmd_line_paths = read_svgd(path_name); //* Scale(3);
        OptRect bounds = bounds_exact(cmd_line_paths);
        if (bounds) {
            cmd_line_paths *= Translate(Point(10,10) - bounds->min());
        }
        init(argc, argv, new IntersectDataTester(cmd_line_paths));
    }else{
        unsigned nb_paths=3, nb_curves_per_path = 5, degree = 1;
        if(argc > 3)
            sscanf(argv[3], "%d", &degree);
        if(argc > 2)
            sscanf(argv[2], "%d", &nb_curves_per_path);
        if(argc > 1){
            sscanf(argv[1], "%d", &nb_paths);
            init(argc, argv, new IntersectDataTester( nb_paths, nb_curves_per_path, degree ) );
        }else{
            init(argc, argv, new IntersectDataTester());
        }
    }
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
