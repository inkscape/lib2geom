#include <iostream>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/path-intersection.h>
#include <2geom/basic-intersection.h>
#include <2geom/pathvector.h>
#include <2geom/exception.h>

#include <cstdlib>
#include <map>
#include <vector>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>
#include <2geom/ord.h>
using namespace Geom;
using namespace std;


struct ExitPoint{
public:
    unsigned side; //0:y=min; 1:x=max; 2:y=max; 3:x=min.
    double place; //x or y value on the exit line.
    unsigned ray_idx;
    double time; //exit time.
    ExitPoint(){}
    ExitPoint(unsigned s, double p, unsigned r, double t){
        side =s;
        place = p;
        ray_idx = r;
        time = t;
    }
};

bool compareExitPoints(ExitPoint a, ExitPoint b) {
    if ( a.side < b.side ) return true;
    if ( a.side <= 1) return ( a.place < b.place );
    return ( a.place > b.place );
}



class IntersectionData {
public:
    // A flag describes an edge + an adjacent area/intersection 
    // boundaries of intersections/areas are sequences of flags.
    // -!- convention: elt comes right after edge w/r to the positive boundary orientation.
    struct Flag{
        unsigned edge; //edge index.
        int elt;       //element (area or intersection) index.
        bool reverse;  //is the edge pointing outward/backward?
        Flag(unsigned edge_idx, int elt_idx, bool o){
            edge = edge_idx;
            elt = elt_idx;
            reverse = o;
        }
        Flag(){
            edge = 0;
            elt = 0;
            reverse = false;
        }
        bool operator == ( Flag const &other) const {
            return (edge == other.edge && elt == other.elt && reverse == other.reverse);
        }
    };

    class Boundary : public std::vector<Flag>{
    public:
        std::vector<unsigned> findEdge(unsigned e){
            std::vector<unsigned> ret;
            for (unsigned k=0; k < size(); k++){
                if ( (*this)[k].edge == e ){
                    ret.push_back(k);
                }
            }
            return ret;
        }
        bool findOrientedEdge(unsigned e, bool reversed, unsigned &idx){
            //std::cout<<"FindOrientedEdge: begin...";
            for (unsigned k=0; k < size(); k++){
                if ( (*this)[k].edge == e && (*this)[k].reverse == reversed){
                    idx = k;
                    //std::cout<<"end (successfull)\n";
                    return true;
                }
            }
            //std::cout<<"end (failed)\n";
            return false;
        }
        std::vector<unsigned> findElt(unsigned a){
            std::vector<unsigned> ret;
            for (unsigned k=0; k < size(); k++){
                if ( (*this)[k].elt == a){
                    ret.push_back(k);
                }
            }
            return ret;
        }
    };

    class IntersectionBox{
    public:
        Boundary boundary; // pairs of (edge,area) indices
        Geom::Rect bounds;
        std::vector<unsigned> findEdge(unsigned e){
            return boundary.findEdge(e);
        }
        bool findOrientedEdge(unsigned e, bool inward, unsigned &idx){
            return boundary.findOrientedEdge( e, inward, idx );
        }
        std::vector<unsigned> findArea(unsigned a){
            return boundary.findElt(a);
        }
    };
    
    class Area {//an area is a connected comp of a nesting level subgraph  .
    public:
        Boundary boundary; // pairs of (edge,intersection) indices
        int winding;

        std::vector<unsigned> findEdge(unsigned e){
            return boundary.findEdge( e );
        }
        bool findOrientedEdge(unsigned e, bool backward, unsigned &idx){
            return boundary.findOrientedEdge( e, backward, idx );
        }
        std::vector<unsigned> findIntersection(unsigned i){
            return boundary.findElt(i);
        }
    };
    
    class Edge {
    public:
        int left, right; // the area indices of the left and right areas of this curve
        int start, end; // the intersection indices of start and end.(redundant, but keeps duality between pt and areas...)
        Geom::Interval portion;
        unsigned path;
        unsigned curve;
    };
    
    vector<Area> areas;
    vector<Edge> edges;
    vector<IntersectionBox> intersections;

    vector<vector<unsigned> > area_tree;
    
    PathVector input_paths;
    cairo_t* cr;

    IntersectionData(){}
    ~IntersectionData(){}
    void setInputPaths(PathVector paths){input_paths = paths;}
    void print(){
        std::cout<<"\nCrossing Data:\n";
        for (unsigned i=0; i<intersections.size(); i++){
            std::cout<<"  intersection "<<i<<":\n";
            for (unsigned j=0; j<intersections[i].boundary.size(); j++){
                std::cout<<"    edge:"<<intersections[i].boundary[j].edge;
                std::cout<<", area:"<<intersections[i].boundary[j].elt;
                std::cout<<", reverse:"<<intersections[i].boundary[j].reverse<<"\n";
            }
        }
        std::cout<<"--------\n";
        for (unsigned i=0; i<edges.size(); i++){
            std::cout<<"  edge "<<i<<":";
            std::cout<<" start: "<<edges[i].start;
            std::cout<<" end: "<<edges[i].end;
            std::cout<<" left: "<<edges[i].left;
            std::cout<<" right: "<<edges[i].right;
            std::cout<<", portion: ["<<edges[i].portion.min();
            std::cout<<", "<<edges[i].portion.max()<<"]\n";
        }
        std::cout<<"--------\n";
        for (unsigned i=0; i<areas.size(); i++){
            std::cout<<"  area "<<i<<":\n";
            for (unsigned j=0; j<areas[i].boundary.size(); j++){
                std::cout<<"    edge:"<<areas[i].boundary[j].edge;
                std::cout<<", pt:"<<areas[i].boundary[j].elt;
                std::cout<<", reverse:"<<areas[i].boundary[j].reverse<<"\n";
            }
        }
    }

    void renameArea(unsigned oldi, unsigned newi){
        for (unsigned e=0; e<edges.size(); e++){
            if ( edges[e].left  == oldi ) edges[e].left  = newi;
            if ( edges[e].right == oldi ) edges[e].right = newi;
        }
        for (unsigned i=0; i<intersections.size(); i++){
            for (unsigned f=0; f<intersections[i].boundary.size(); f++){
                if ( intersections[i].boundary[f].elt == oldi ) intersections[i].boundary[f].elt = newi;
            }
        }
    }
    void renameIntersection(unsigned oldi, unsigned newi){
        for (unsigned e=0; e<edges.size(); e++){
            if ( edges[e].start == oldi ) edges[e].start = newi;
            if ( edges[e].end   == oldi ) edges[e].end   = newi;
        }
        for (unsigned a=0; a<areas.size(); a++){
            for (unsigned f=0; f<areas[a].boundary.size(); f++){
                if ( areas[a].boundary[f].elt == oldi ) areas[a].boundary[f].elt = newi;
            }
        }
    }
    void renameEdge(unsigned oldi, unsigned newi){
        for (unsigned i=0; i<intersections.size(); i++){
            for (unsigned j=0; j < intersections[i].boundary.size(); j++){
                if ( intersections[i].boundary[j].edge  == oldi) intersections[i].boundary[j].edge = newi;
            }
        }
        for (unsigned a=0; a<areas.size(); a++){
            for (unsigned j=0; j < areas[a].boundary.size(); j++){
                if ( areas[a].boundary[j].edge  == oldi) areas[a].boundary[j].edge = newi;
            }
        }
    }


    D2<SBasis> edgeAsSBasis(unsigned e){
        //beurk!
        D2<SBasis> c = input_paths[edges[e].path][edges[e].curve].toSBasis();
        return portion(c, edges[e].portion); 
    }
    OptRect edgeBounds(unsigned e){
        //beurk!
        return bounds_exact( edgeAsSBasis(e) );
    }

    unsigned findEdge(unsigned path, unsigned curve, double t){
        for (unsigned i=0; i<edges.size(); i++){
            if ( edges[i].path  == path &&
                 edges[i].curve == curve &&
                 edges[i].portion.contains(t) )
                return i;
        }
        return edges.size();
    }
    unsigned edgeStart(unsigned e){
        for (unsigned i=0; i<intersections.size(); i++){
            for (unsigned j=0; j<intersections[i].boundary.size(); j++){
                if ( intersections[i].boundary[j].edge  == e && !intersections[i].boundary[j].reverse )
                    return i;
            }
        }
        THROW_EXCEPTION("Invalid intersection data");
        return intersections.size();
    }
    unsigned edgeEnd(unsigned e){
        for (unsigned i=0; i<intersections.size(); i++){
            for (unsigned j=0; j<intersections[i].boundary.size(); j++){
                if ( intersections[i].boundary[j].edge  == e && intersections[i].boundary[j].reverse )
                    return i;
            }
        }
        THROW_EXCEPTION("Invalid intersection data");
        return intersections.size();
    }

    void splitEdge(unsigned i0, double t, unsigned box_id){
        if (!( edges[i0].portion.contains(t) ) ) 
            THROW_EXCEPTION("Invalid intersection data.");

        if (edges[i0].portion.min()==t ){
            assert (edges[i0].start == box_id);
            return;
        }
        if (edges[i0].portion.max()==t ){
            assert (edges[i0].end == box_id);
            return;
        }
        
        //create the new edge
        unsigned i1 = edges.size();
        edges.push_back(Edge());
        edges[i1] = edges[i0];

        //set edge portions
        edges[i1].portion = Interval(t,edges[i0].portion.max());
        edges[i0].portion = Interval(edges[i0].portion.min(),t);

        //replace old edge name at the end
        unsigned b;
        if (!intersections[edges[i0].end].findOrientedEdge(i0,true,b) ){
            THROW_EXCEPTION("Invalid intersection data.");
        }
        intersections[edges[i0].end].boundary[b].edge = i1;

        //set new start/end names
        edges[i0].end = box_id;
        edges[i1].start = box_id;

        //append the two edges at the splitting point
        intersections[box_id].boundary.push_back(Flag(i1, -2, false ));//end piece goes out.
        intersections[box_id].boundary.push_back(Flag(i0, -3, true  ));//start piece comes in.
    }

    void updateAfterGrowth(unsigned &b){
        assert( b < intersections.size() );
        for (int i=0; i<intersections.size(); i++){
            if (i!=b && intersections[i].bounds.intersects(intersections[b].bounds) ){
                b = fuseBoxes(i,b);
                updateAfterGrowth(b);
                return;
            }
        }
    }

    //warning: non sorted fuse!
    unsigned fuseBoxes(unsigned b1, unsigned b2){
        if ( b1 == b2 ) return b1;
        for (unsigned e=0; e<edges.size(); e++){
            assert(edgeStart(e) == edges[e].start);
            assert(edgeEnd(e)   == edges[e].end);
        }


        unsigned i = ( b1 < b2 ? b1 : b2 );
        unsigned j = ( b1 < b2 ? b2 : b1 );
        intersections[i].bounds.unionWith(intersections[j].bounds);
        renameIntersection(j,i);
        for (int k=j+1; k<intersections.size(); k++){
            renameIntersection(k,k-1);
        }
        intersections[i].boundary.insert( intersections[i].boundary.end(),
                                          intersections[j].boundary.begin(),
                                          intersections[j].boundary.end() );
        
        intersections.erase( intersections.begin() + j );
        return i;
    }

    //create a new box, and perform the eventual subsequent merge with overlapping neighbors.
    unsigned insertNewBox(Rect bounds){
        for (unsigned i=0; i<intersections.size(); i++){
            if (intersections[i].bounds.contains(bounds) ){
                return i;
            }else if (intersections[i].bounds.intersects(bounds) ){
                intersections[i].bounds.unionWith(bounds);
                updateAfterGrowth(i);
                return i;
            }
        }
        
        IntersectionBox newbox;
        newbox.bounds = bounds;
        intersections.push_back(newbox);
        return ( intersections.size()-1 );
    }

    void buildIntersections(PathVector paths){
        std::cout<<"\n\nCompute intersections:\n";
        input_paths = paths;
        double precision = 5;//FIXME: use a meaningfull precision!!

        for (unsigned i=0; i<paths.size(); i++){
            for (unsigned ii=0; ii<paths[i].size(); ii++){
                Edge e;
                e.left = -1;
                e.right = -1;
                e.path = i;
                e.curve = ii;
                e.portion = Interval(0,1);
                edges.push_back(e);
                Point p;
                Rect bounds;
                unsigned newbox_id, e_idx = edges.size()-1;

                //add an intersection at start.
                p = paths[i][ii].initialPoint();
                bounds = Rect(p,p);
                bounds.expandBy(precision);//FIXME: precision should be 0 here (debug purpose only).
                newbox_id = insertNewBox(bounds);
                intersections[newbox_id].boundary.push_back(Flag(e_idx,-2, false));
                edges[e_idx].start = newbox_id;

                //add an intersection at end.
                p = paths[i][ii].finalPoint();
                bounds = Rect(p,p);
                bounds.expandBy(precision);//FIXME: precision should be 0 here (debug purpose only).
                newbox_id = insertNewBox(bounds);
                intersections[newbox_id].boundary.push_back(Flag(e_idx,-3, true));
                edges[e_idx].end = newbox_id;
            }
        }
        for (unsigned i=0; i<edges.size(); i++){
            assert(edgeStart(i) == edges[i].start);
            assert(edgeEnd(i)   == edges[i].end);
        }
            //TODO: Do sweeping here.
            //this is just the quick implemented brutal stupid thing...
        for (unsigned i=0; i<paths.size(); i++){
            for (unsigned ii=0; ii<paths[i].size(); ii++){
                for (unsigned j=i; j<paths.size(); j++){
                    for (unsigned jj=( i==j ? ii+1 : 0 ); jj<paths[j].size(); jj++){
                        std::cout<<"i="<<i<<", ii="<<ii<<", j="<<j<<", jj="<<jj<<"\n";
                        std::vector<std::pair<double,double> > times;
                        //FIXME: look for self intersection when i=j and ii=jj.
                        find_intersections( times, paths[i][ii].toSBasis(), paths[j][jj].toSBasis() );
                        for (unsigned k=0; k<times.size(); k++){
                            unsigned ei = findEdge(i,ii,times[k].first);
                            unsigned ej = findEdge(j,jj,times[k].second);

                            for (unsigned toto=0; toto<edges.size(); toto++){
                                assert(edgeStart(toto) == edges[toto].start);
                                assert(edgeEnd(toto)   == edges[toto].end);
                            }

                            Point p = paths[edges[ei].path][edges[ei].curve].pointAt(times[k].first);
                            Rect bounds = Rect(p,p);
                            bounds.expandBy(precision);//FIXME: use a meaningfull precision here!!
                            unsigned newbox_id = insertNewBox(bounds);

                            splitEdge(ei, times[k].first, newbox_id);
                            splitEdge(ej, times[k].second, newbox_id);

                        }
                    }
                }
            }
        }
        //TODO: add intersections for edges meeting boxes. 
        //TODO: remove edges contained in a box! (how?) 
    }

    //returns a rect around a separating it from b. Nota: 3 sides are infinite!
    //TODO: place the cut where there is most space...
    OptRect separate(Rect const &a, Rect const &b){        
        Rect ret ( Interval( -infinity(), infinity() ) , Interval(-infinity(), infinity() ) );
        if (a.max()[X]<b.min()[X]){
            ret[X][1] = ( a.max()[X] + b.min()[X] )/ 2;
            return  OptRect(ret);
        }
        if (b.max()[X]<a.min()[X]){
            ret[X][0] = ( b.max()[X] + a.min()[X] )/ 2;
            return  OptRect(ret);
        }
        if (a.max()[Y]<b.min()[Y]){
            ret[Y][1] = ( a.max()[Y] + b.min()[Y] )/ 2;
            return  OptRect(ret);
        }
        if (b.max()[Y]<a.min()[Y]){
            ret[Y][1] = ( b.max()[Y] + a.min()[Y] )/ 2;
            return OptRect(ret);
        }
        return OptRect();
    }

    void sortIntersectionBoundaries(){
        for (unsigned i=0; i<intersections.size(); i++){
            sortIntersectionBoundary(i);
        }
    }

    void sortIntersectionBoundary(unsigned b){

        //First find 4 lines s.t. all the edges have to cross at least one.
        //
        Rect box = intersections[b].bounds;
        OptRect sep ( Interval( -infinity(), infinity() ) , Interval(-infinity(), infinity() ) );
        //separate this intersection from the others
        for (unsigned i=0; i<intersections.size(); i++){
            if (i!=b){
                OptRect sepi = separate(box, intersections[i].bounds);
                if ( sep && sepi ){
                    sep = intersect(*sep, *sepi);
                }else{
                    THROW_EXCEPTION("Invalid in intersection data.");
                }
            }
        }
        //now look for edges looping from the box to itself
        for (unsigned i=0; i<intersections[b].boundary.size(); i++){
            unsigned ei = intersections[b].boundary[i].edge;
            if (edges[ei].start == edges[ei].end) {
                Rect sepi ( Interval( -infinity(), infinity() ) , Interval( -infinity(), infinity() ) );
                OptRect bbox = edgeBounds(ei);
                if (bbox){
                    if (bbox->min()[X]<box.min()[X]) {
                        sepi[X][0] = ( bbox->min()[X] + box.min()[X] )/2;
                    }
                    if (bbox->max()[X]<box.max()[X]) {
                        sepi[X][1] = ( bbox->max()[X] + box.max()[X] )/2;
                    }
                    if (bbox->min()[Y]<box.min()[Y]) {
                        sepi[Y][0] = ( bbox->min()[Y] + box.min()[Y] )/2;
                    }
                    if (bbox->max()[Y]<box.max()[Y]) {
                        sepi[Y][1] = ( bbox->max()[Y] + box.max()[Y] )/2;
                    }
                }
                if ( sep ){
                    sep = intersect( *sep, sepi);
                }else{
                    THROW_EXCEPTION("Invalid in intersection data.");
                }
            }
        }
        if (!sep) THROW_EXCEPTION("Invalid intersection data.");


        //Now find first exit point for each edge, and sort accordingly.
        //
        std::vector<ExitPoint> exits (intersections[b].boundary.size(), ExitPoint(4,0,0,0));
        //for (unsigned i=0; i < intersections[b].boundary.size(); i++){//setting idx correctly here is important for edges contained in the box...
            //exits[i] = ExitPoint( 4, 0, i, 0);
        //}
        for (unsigned side=0; side<4; side++){//scan X or Y direction, on level min or max...
            double level = sep->corner( side )[1-side%2];
            if (level != infinity() && level != -infinity() ){
                for (unsigned i=0; i < intersections[b].boundary.size(); i++){
                    unsigned e = intersections[b].boundary[i].edge;
                    D2<SBasis> c = edgeAsSBasis( e );
                    
                    std::vector<double> times = roots(c[1-side%2]-level);
                    if ( times.size() > 0 ) {
                        double t;
                        if (intersections[b].boundary[i].reverse){
                            t = times.back();
                            if ( exits[i].side > 3 || exits[i].time < t )
                                exits[i] = ExitPoint( side, c[side%2](t), i, t);
                        }else{
                            t = times.front();
                            if ( exits[i].side > 3 || exits[i].time > t )
                                exits[i] = ExitPoint( side, c[side%2](t), i, t);
                        }
                    }
                }
            }
        }

        //Rk: at this point, side == 4 means the edge is contained in the intersection box (?)...;
        std::cout<<"exits before sorting:\n   ";
        for (unsigned i=0; i < exits.size(); i++){
            std::cout<<exits[i].ray_idx<<", ";
        }
        std::cout<<"\n";

        std::sort( exits.begin(), exits.end(), compareExitPoints );

        std::cout<<"exits after sorting:\n   ";
        for (unsigned i=0; i < exits.size(); i++){
            std::cout<<exits[i].ray_idx<<", ";
        }
        std::cout<<"\n";

        Boundary sorted_boundary = intersections[b].boundary;
        for (unsigned i=0; i < intersections[b].boundary.size(); i++){
            sorted_boundary[i] = intersections[b].boundary[exits[i].ray_idx];
        }
        //TODO: remove the edges as well!! caution: each deletion invalidates all the names...
        //while( exits.back().side == 4 ){
        //    exits.pop_back();
        //}
        intersections[b].boundary = sorted_boundary;
    }
        

    void diffuseAreaName(unsigned a, unsigned from_edge, bool reverse){
        if ( a>=areas.size() ) THROW_EXCEPTION("invalid intersction data: unable to name areas.");
        std::cout<<"e="<<(reverse?"+":"-")<<from_edge<<", ";
        unsigned b,idx;
        if ( reverse ){
            if (edges[from_edge].right == a) return;
            edges[from_edge].right = a;
            b = edges[from_edge].start;
        }else{
            if (edges[from_edge].left == a) return;
            edges[from_edge].left = a;
            b = edges[from_edge].end;
        }
        std::cout<<" ("<<from_edge<<", "<<b<<")";

        if ( !intersections[b].findOrientedEdge( from_edge, !reverse, idx ) )
            THROW_EXCEPTION("invalid intersction data: unable to name areas.");
        idx = ( idx + intersections[b].boundary.size() - 1 )%intersections[b].boundary.size();
        intersections[b].boundary[idx].elt = a;
        areas[a].boundary.push_back( Flag( from_edge, b, reverse) );
        diffuseAreaName(a, intersections[b].boundary[idx].edge,  intersections[b].boundary[idx].reverse );
    }

    //FIXME: at the end of this, nested areas are considered disjoint...
    void nameAreas(){

        for (unsigned b=0; b<intersections.size(); b++){
            for (unsigned i=0; i<intersections[b].boundary.size(); i++){
                if (intersections[b].boundary[i].elt < 0 ) {
                    unsigned a = areas.size();
                    areas.push_back(Area());
                    unsigned e = intersections[b].boundary[i].edge;
                    std::cout<<"looping on area "<<a<<" boundary:\n    ";
                    diffuseAreaName(a,e,intersections[b].boundary[i].reverse);
                }
            }
        }
    }
};





class IntersectDataTester: public Toy {
    int NB_PATHS;
    int NB_PTS_ON_PATH;

    std::vector<PointSetHandle> paths_handles;
    std::vector<Slider> sliders;

    void drawArea( cairo_t *cr, IntersectionData const &topo, unsigned a ){
        if (a>=topo.areas.size()) return;
        for (int i = 0; i < topo.areas[a].boundary.size(); i++){
            int eidx = topo.areas[a].boundary[i].edge;
            int iidx = topo.areas[a].boundary[i].elt;
            IntersectionData::Edge e = topo.edges[eidx];
            D2<SBasis> p = topo.input_paths[e.path][e.curve].toSBasis();
            Interval dom = e.portion;
            if (topo.areas[a].boundary[i].reverse){
                cairo_set_source_rgba (cr, 1., 1., 0., 1);
            }else{
                cairo_set_source_rgba (cr, 1., 0., 1., 1);
            }
            p = portion(p, dom);
            cairo_d2_sb(cr, p);
            cairo_set_line_width (cr, 3);
            cairo_stroke(cr);
        }
    }

    void highlightRay( cairo_t *cr, IntersectionData const &topo, unsigned b, unsigned r ){
        if (b>=topo.intersections.size()) return;
        if (r>=topo.intersections[b].boundary.size()) return;
        Rect box = topo.intersections[b].bounds;
        box.expandBy(2);
        cairo_rectangle(cr, box);
        cairo_set_source_rgba (cr, 1., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        cairo_stroke(cr);
        unsigned eidx = topo.intersections[b].boundary[r].edge;
        IntersectionData::Edge e = topo.edges[eidx];
        D2<SBasis> p = topo.input_paths[e.path][e.curve].toSBasis();
        Interval dom = e.portion;
        if (topo.intersections[b].boundary[r].reverse){
            dom[0] += e.portion.extent()*2./3;
            cairo_set_source_rgba (cr, 0., 1., 0., 1);
        }else{
            dom[1] -= e.portion.extent()*2./3;
            cairo_set_source_rgba (cr, 0., 0., 1., 1);
        }
        p = portion(p, dom);
        cairo_d2_sb(cr, p);
        cairo_set_source_rgba (cr, 1., 0., 0, 1);
        cairo_set_line_width (cr, 5);
        cairo_stroke(cr);
    }

    void drawBox( cairo_t *cr, IntersectionData const &topo, unsigned b ){
        if (b>=topo.intersections.size()) return;
        Rect box = topo.intersections[b].bounds;
        //box.expandBy(5);
        cairo_rectangle(cr, box);
        cairo_set_source_rgba (cr, 1., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        cairo_stroke(cr);

        //std::cout<<"\nintersection boundary:\n";
        for (int i = 0; i < topo.intersections[b].boundary.size(); i++){
            int eidx = topo.intersections[b].boundary[i].edge;
            int aidx = topo.intersections[b].boundary[i].elt;
            //std::cout<<"       ("<<eidx<<", "<<aidx<<"), dom = "<<topo.edges[eidx].portion<<"\n";
            IntersectionData::Edge e = topo.edges[eidx];
            D2<SBasis> p = topo.input_paths[e.path][e.curve].toSBasis();
            Interval dom = e.portion;
            if (topo.intersections[b].boundary[i].reverse){
                dom[0] += e.portion.extent()*2./3;
                cairo_set_source_rgba (cr, 0., 1., 0., 1);
            }else{
                dom[1] -= e.portion.extent()*2./3;
                cairo_set_source_rgba (cr, 0., 0., 1., 1);
            }
            p = portion(p, dom);
            cairo_d2_sb(cr, p);
            cairo_set_line_width (cr, 3);
            cairo_stroke(cr);
        }
    }
    void drawBoxes( cairo_t *cr, IntersectionData const &topo ){
        for (unsigned b=0; b<topo.intersections.size(); b++){
            drawBox(cr, topo, b);
        }
    }

    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        
        std::vector<Path> paths(NB_PATHS, Path());
        for (int i = 0; i < NB_PATHS; i++){
            paths[i] = Path(paths_handles[i].pts[0]);
            for (int j = 1; j < paths_handles[i].size(); j++){
                paths[i].appendNew<LineSegment>(paths_handles[i].pts[j]);
            }
        }
        
        cairo_path(cr, paths);
        cairo_set_source_rgba (cr, 0., 0., 0, 1);
        cairo_set_line_width (cr, 1);
        cairo_stroke(cr);

        IntersectionData topo;
        topo.buildIntersections(paths);
        topo.print();
        topo.sortIntersectionBoundaries();
        highlightRay(cr, topo, sliders[0].value(), sliders[1].value() );
        //topo.nameAreas();
        //topo.print();

        //drawBox(cr,topo, unsigned(sliders[0].value()));
        drawBoxes(cr,topo);

        Toy::draw(cr, notify, width, height, save, timer_stream);
    }

    public:
    IntersectDataTester(int paths, int pts_on_path, int degree) :
        NB_PATHS(paths), NB_PTS_ON_PATH(pts_on_path) {
        for (int i = 0; i < NB_PATHS; i++){
            paths_handles.push_back(PointSetHandle());
        }
        sliders.push_back(Slider(0.0, 10.0, 1, 0.0, "intersection chooser"));
        sliders.push_back(Slider(0.0, 10.0, 1, 0.0, "ray chooser"));
        handles.push_back(&(sliders[0]));
        handles.push_back(&(sliders[1]));
        for(int i = 0; i < NB_PATHS; i++){
            for(int j = 0; j < NB_PTS_ON_PATH; j++){
                paths_handles[i].push_back(uniform()*200, 100+ uniform()*200);
            }
            handles.push_back(&paths_handles[i]);
        }
        sliders[0].geometry(Point(50, 20), 180);
        sliders[1].geometry(Point(50, 40), 180);
    }

    void first_time(int argc, char** argv) {

    }
};

int main(int argc, char **argv) {
    unsigned paths=10;
    unsigned pts_on_path=3;
    unsigned degree=1;
    if(argc > 3)
        sscanf(argv[3], "%d", &degree);
    if(argc > 2)
        sscanf(argv[2], "%d", &pts_on_path);
    if(argc > 1)
        sscanf(argv[1], "%d", &paths);
    init(argc, argv, new IntersectDataTester(paths, pts_on_path, degree));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
