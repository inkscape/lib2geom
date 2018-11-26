#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/path-intersection.h>
#include <2geom/basic-intersection.h>
#include <2geom/pathvector.h>
#include <2geom/exception.h>

#include <vector>
#include <algorithm>
#include "sweeper.cpp"

/*
Topology Class:
This class mainly consists in 3 vectors: vertices, edges, and areas.
-edges: have start/end, left/right pointing to vertices or areas.
-vertices: have a "boundary"= the sequence of edges sorted in CCW order.
-areas: have one outer "boundary" + a vector of inner boundaries, which are
 sequence of edges.

To build this data, the strategy is to let a line sweep the plane (from left
to right, say) and consider the topology of what is on the left of the sweep line.
Topology changes are called events, and we call an external "sweeper" to generate
them for us.

So we start with an empty data, and respond to events to always describe the
topology of what is on the left of the sweep line. [more precisely, we start with
one region that has empty boundary, and since the external sweeper knows how many
edges we'll have at the end, so we create them from scratch, leaving their ends
as "unknown"]

Note: see the sweeper for more info about events; they are essentially generated
when the sweep line crosses a vertex (which is in fact a box), but are in fact split
into smaller events, one for each edge around the vertex...


The code is using a lot of vectors: unsing pointers instead of vectors could speed
things up (?), but vector indices are easier to debug than memory addresses.:P
*/

using namespace Geom;
using namespace std;

class Topology {
public:

    // -!- convention:
    // In a boundary, reversed edges point away from the vertex or CW around the area.
    struct OrientedEdge{
        unsigned edge; //edge index.
        bool reversed; //true if the intrinsic edge orientation points away (from vertex) or backward (along area boundary)
        OrientedEdge(unsigned edge_idx, bool o){
            edge = edge_idx;
            reversed = o;
        }
        OrientedEdge(){
            edge = NULL_IDX;
            reversed = false;
        }
        bool operator == ( OrientedEdge const &other) const {
            return (edge == other.edge && edge!=NULL_IDX && reversed == other.reversed);
        }
    };

    class Boundary : public std::vector<OrientedEdge>{
    public:
        bool of_area;//true if this is the boundary of an area. Fix this with templates?
        Boundary(bool area_type): of_area(area_type){}
    };

    class Vertex{
    public:
        Boundary boundary; // list of edges in CCW order around the vertex
        Geom::Rect bounds;
        Vertex():boundary(false){}
    };

    class Area {//an area is a connected comp of the complement of the graph.  .
    public:
        Boundary boundary; // outermost boundary component, CCW oriented (i.e. area is on the left of the boundary).
        std::vector<Boundary> inner_boundaries;//same conventions, area on the left, so this gives the CW orientation for inner components.
        std::vector<int> windings;//one winding number for each input path.
        Area(unsigned size): boundary(true), windings(size, 0){}
    };

    class Edge {
    public:
        unsigned left, right;// the indices of the areas on the left and on the right this edge.
        unsigned start, end; // the indices of vertices at start and at end of this edge.
        Geom::Interval portion;
        unsigned path;
        unsigned curve;
        Edge(){
            left = NULL_IDX;
            right =NULL_IDX;
            start = NULL_IDX;
            end = NULL_IDX;
            portion = Interval();
            path = NULL_IDX;
            curve = NULL_IDX;
        }
    };

    vector<Area> areas;
    vector<Edge> edges;
    vector<Vertex> vertices;

    PathVector input_paths;//we don't need our own copy...
    cairo_t* cr;

    //debug only!!
    int steps_max;
    //----------


    //----------------------------------------------------
    //-- utils...
    //----------------------------------------------------

    void printIdx(unsigned idx){ (idx == NULL_IDX)? std::printf("?") : std::printf("%u", idx); }
    void printVertex(unsigned i){
        std::printf("vertex %u: ", i);
        printBoundary(vertices[i].boundary);
        std::printf("\n");
        }
    void printEdge(unsigned i){
        std::printf("edge %u: ", i);
        printIdx(edges[i].start);
        std::printf(" -> ");
        printIdx(edges[i].end);
        std::printf(" ^");
        printIdx(edges[i].left);
        std::printf(" _");
        printIdx(edges[i].right);
        std::printf("\n");
        }
    void printArea(unsigned i){
        std::printf("area %u: ", i);
        printBoundary(areas[i].boundary);
        for (unsigned j=0; j<areas[i].inner_boundaries.size(); j++){
            std::printf(", ");
            printBoundary(areas[i].inner_boundaries[j]);
        }
        std::printf("\n");
    }

    void printOrientedEdge(OrientedEdge const &f){
        ( f.reversed ) ? std::printf("-") : std::printf("+");
        printIdx(f.edge);
        std::printf(" ");
    }
    void printBoundary(Boundary const &bndry){
        (bndry.of_area) ? std::printf("[") : std::printf("<");
        for (unsigned i=0; i<bndry.size(); i++){
            printOrientedEdge(bndry[i]);
        }
        (bndry.of_area) ? std::printf("]") : std::printf(">");
    }

    void print(){
        std::cout<<"\nCrossing Data:\n";
        for (unsigned i=0; i<vertices.size(); i++){
            printVertex(i);
        }
        std::cout<<"\n";
        for (unsigned i=0; i<edges.size(); i++){
            printEdge(i);
        }
        std::cout<<"\n";
        for (unsigned i=0; i<areas.size(); i++){
            printArea(i);
        }
    }

    D2<SBasis> edgeAsSBasis(unsigned e){
    	//beurk! optimize me.
    	D2<SBasis> c = input_paths[edges[e].path][edges[e].curve].toSBasis();
    	return portion(c, edges[e].portion);
    }

    Path edgeToPath(Topology::OrientedEdge o_edge){
    	Topology::Edge e = edges[o_edge.edge];
    	D2<SBasis> p = input_paths[e.path][e.curve].toSBasis();
    	Interval dom = e.portion;
    	p = portion(p, dom);
    	if ( o_edge.reversed ){
    		p = compose( p, Linear(1.,0.) );
		}
    	Path ret;
        ret.setStitching(true);
    	Point center;
    	unsigned c_idx = source(o_edge, true);
    	if ( c_idx == NULL_IDX ){
    		ret.append(p);
		}else{
			center = vertices[c_idx].bounds.midpoint();
			ret 	= Path(center);
			ret.append(p);
		}
    	c_idx = target(o_edge, true);
    	if ( c_idx == NULL_IDX ){
    		return ret;
		}else{
			center = vertices[c_idx].bounds.midpoint();
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
            unsigned first_v = source(o_edge, true);
            if ( first_v != NULL_IDX ){
                pt = vertices[first_v].bounds.midpoint();
                bndary = Path(pt);
            }

            for (unsigned i = 0; i < b.size(); i++){
                bndary.append( edgeToPath(b[i]));
            }
            bndary.close();
            return bndary;
        }



    //----------------------------------------------------
    //-- Boundary Navigation/Modification
    //----------------------------------------------------

    //TODO: this should be an OrientedEdge method, be requires access to the edges.
    unsigned source(OrientedEdge const &f, bool as_area_bndry){
        unsigned prev;
        if (f.reversed )
            prev = (as_area_bndry)? edges[f.edge].end : edges[f.edge].right;
        else
            prev = (as_area_bndry)? edges[f.edge].start : edges[f.edge].left;
        return prev;
    }
    unsigned target(OrientedEdge const &f, bool as_area_bndry){
        unsigned prev;
        if (f.reversed )
            prev = (as_area_bndry)? edges[f.edge].start : edges[f.edge].left;
        else
            prev = (as_area_bndry)? edges[f.edge].end : edges[f.edge].right;
        return prev;
    }

    //TODO: this should be a Boundary method, but access to the full data is required...
    bool prolongate( Boundary &bndry, OrientedEdge const &f){
        if ( bndry.empty() ){
            bndry.push_back(f);
            return true;
        }
        unsigned src = source(f, bndry.of_area);
        if ( src == target( bndry.back(),  bndry.of_area ) && src != NULL_IDX ){
            bndry.push_back(f);
            return true;
        }
        unsigned tgt = target( f, bndry.of_area );
        if ( tgt == source( bndry.front(), bndry.of_area ) && tgt != NULL_IDX ){
            bndry.insert( bndry.begin(), f);
            return true;
        }
        return false;
    }

    bool prolongate(Boundary &a, Boundary &b){
        if (a.size()==0 || b.size()==0 || (a.of_area != b.of_area) ) return false;
        unsigned src;
        src = source(a.front(), a.of_area);

//        unsigned af = a.front().edge, as=source(a.front(), a.of_area), ab=a.back().edge, at=target(a.back(), a.of_area);
//        unsigned bf = b.front().edge, bs=source(b.front(), b.of_area), bb=b.back().edge, bt=target(b.back(), b.of_area);
//        std::printf("a=%u(%u)...(%u)%u\n", as, af,ab,at);
//        std::printf("b=%u(%u)...(%u)%u\n", bs, bf,bb,bt);

//        std::printf("%u == %u?\n", src, target( b.back(), b.of_area ));
        if ( src == target( b.back(), b.of_area ) && src != NULL_IDX ){
            a.insert( a.begin(), b.begin(), b.end() );
//            std::printf("boundaries fused!!\n");
            return true;
        }
        src = source(b.front(), b.of_area);
        if ( src == target( a.back(), a.of_area ) && src != NULL_IDX ){
            a.insert( a.end(), b.begin(), b.end() );
            return true;
        }
        return false;
    }

    //TODO: this should be a Boundary or Area method, but requires access to the full data...
    //TODO: systematically check for connected boundaries before returning?
    void addAreaBoundaryPiece(unsigned a, OrientedEdge const &f){
        if ( areas[a].boundary.size()>0 && prolongate( areas[a].boundary, f ) ) return;
        for ( unsigned i=0; i<areas[a].inner_boundaries.size(); i++){
//            printBoundary(areas[a].inner_boundaries[i]);
//            printf(" matches ");
//            printOrientedEdge(f);
//            printf("?");
            if ( areas[a].inner_boundaries[i].size()>0 && prolongate( areas[a].inner_boundaries[i], f) ) return;
//            printf("no. (%u vs %u)", target(areas[a].inner_boundaries[i].back(), true), source(f, true));
        }
        Boundary new_comp(true);
        new_comp.push_back(f);
        areas[a].inner_boundaries.push_back(new_comp);
    }


    bool fuseConnectedBoundaries(unsigned a){
//        std::printf(" fuseConnectedBoundaries %u\n",a);

        bool ret = false;
        if ( areas[a].boundary.size()>0 ){
            for ( unsigned i=0; i<areas[a].inner_boundaries.size(); i++){
                if ( prolongate( areas[a].boundary, areas[a].inner_boundaries[i] ) ){
                    areas[a].inner_boundaries.erase(areas[a].inner_boundaries.begin()+i);
                    i--;
                    ret = true;
                }
            }
        }
        for ( unsigned i=0; i<areas[a].inner_boundaries.size(); i++){
            for ( unsigned j=i+1; j<areas[a].inner_boundaries.size(); j++){
                if ( prolongate( areas[a].inner_boundaries[i], areas[a].inner_boundaries[j] ) ){
                    areas[a].inner_boundaries.erase(areas[a].inner_boundaries.begin()+j);
                    j--;
                    ret = true;
                }
            }
        }
        return ret;
    }

    //-------------------------------
    //-- Some basic area manipulation.
    //-------------------------------

    void renameArea(unsigned oldi, unsigned newi){
        for (unsigned e=0; e<edges.size(); e++){
            if ( edges[e].left  == oldi ) edges[e].left  = newi;
            if ( edges[e].right == oldi ) edges[e].right = newi;
        }
    }
    void deleteArea(unsigned a0){//ptrs would definitely be helpful here...
        assert(a0<areas.size());
        for (unsigned a=a0+1; a<areas.size(); a++){
            renameArea(a,a-1);
        }
        areas.erase(areas.begin()+a0);
    }

    //fuse open(=not finished!) areas. The boundaries are supposed to match. true on success.
    void fuseAreas(unsigned a, unsigned b){
//        std::printf("fuse Areas %u and %u\n", a, b);
        if (a==b) return;
        if (a>b) swap(a,b);//this is important to keep track of the outermost component!!

        areas[a].inner_boundaries.push_back(areas[b].boundary);
        for (unsigned i=0; i<areas[b].inner_boundaries.size(); i++){
            areas[a].inner_boundaries.push_back(areas[b].inner_boundaries[i]);
        }
        renameArea(b,a);
        deleteArea(b);
        assert( fuseConnectedBoundaries(a) );
        return;
    }

    PathVector areaToPath(unsigned a){
        PathVector bndary;
        if ( areas[a].boundary.size()!=0 ){//this is not the unbounded component...
            bndary.push_back( boundaryToPath(areas[a].boundary ) );
        }
        for (unsigned j = 0; j < areas[a].inner_boundaries.size(); j++){
            bndary.push_back( boundaryToPath(areas[a].inner_boundaries[j]) );
        }
        return bndary;
    }

    //DEBUG ONLY: we add a rect round the unbounded comp, and glue the bndries
    //for easy drawing in the toys...
    Path glued_areaToPath(unsigned a){
        Path bndary;
        if ( areas[a].boundary.size()==0 ){//this is the unbounded component...
            OptRect bbox = bounds_fast( input_paths );
            if (!bbox ){return Path();}//???
            bbox->expandBy(50);
            bndary = Path(bbox->corner(0));
            bndary.appendNew<LineSegment>(bbox->corner(1));
            bndary.appendNew<LineSegment>(bbox->corner(2));
            bndary.appendNew<LineSegment>(bbox->corner(3));
            bndary.appendNew<LineSegment>(bbox->corner(0));
        }else{
            bndary =  boundaryToPath(areas[a].boundary);
        }
        for (unsigned j = 0; j < areas[a].inner_boundaries.size(); j++){
            bndary.append( boundaryToPath(areas[a].inner_boundaries[j]));
            bndary.appendNew<LineSegment>( bndary.initialPoint() );
        }
        bndary.close();
        return bndary;
    }

    void drawAreas( cairo_t *cr, bool fill=true ){
        //don't draw the first one...
        for (unsigned a=0; a<areas.size(); a++){
            drawArea(cr, a, fill);
        }
    }
    void drawArea( cairo_t *cr, unsigned a, bool fill=true ){
        if (a>=areas.size()) return;
        Path bndary = glued_areaToPath(a);
        cairo_path(cr, bndary);
        if (fill){
            cairo_fill(cr);
        }else{
            cairo_stroke(cr);
        }
    }
    void highlightRay( cairo_t *cr, unsigned b, unsigned r ){
        if (b>=vertices.size()) return;
        if (r>=vertices[b].boundary.size()) return;
        Rect box = vertices[b].bounds;
        //box.expandBy(2);
        cairo_rectangle(cr, box);
        cairo_set_source_rgba (cr, 1., 0., 0, 1.0);
        cairo_set_line_width (cr, 1);
        cairo_fill(cr);
        unsigned eidx = vertices[b].boundary[r].edge;
        Topology::Edge e = edges[eidx];
        D2<SBasis> p = input_paths[e.path][e.curve].toSBasis();
        Interval dom = e.portion;
        if (vertices[b].boundary[r].reversed){
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

    void drawEdge( cairo_t *cr, unsigned eidx ){
        if (eidx>=edges.size()) return;
        Topology::Edge e = edges[eidx];
        D2<SBasis> p = input_paths[e.path][e.curve].toSBasis();
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
    void drawEdges( cairo_t *cr){
        for (unsigned e=0; e<edges.size(); e++){
            drawEdge(cr, e);
        }
    }
    void drawKnownEdges( cairo_t *cr){
        for (unsigned v=0; v<vertices.size(); v++){
            for (unsigned e=0; e<vertices[v].boundary.size(); e++){
                drawEdge(cr, vertices[v].boundary[e].edge);
            }
        }
    }


    void drawBox( cairo_t *cr, unsigned b ){
        if (b>=vertices.size()) return;
        Rect box = vertices[b].bounds;
        //box.expandBy(5);
        cairo_rectangle(cr, box);
        cairo_set_source_rgba (cr, 1., 0., 0, .5);
        cairo_set_line_width (cr, 1);
        cairo_stroke(cr);
        cairo_rectangle(cr, box);
        cairo_set_source_rgba (cr, 1., 0., 0, .2);
        cairo_fill(cr);
    }

    void drawBoxes( cairo_t *cr){
        for (unsigned b=0; b<vertices.size(); b++){
            drawBox(cr, b);
        }
    }










    //----------------------------------------------------
    //-- Fill data using a sweeper...
    //----------------------------------------------------

    Topology(){}
    ~Topology(){}
    Topology(PathVector const &paths, cairo_t* cairo, double tol=EPSILON, int stepsmax=-1){
//        std::printf("\n---------------------\n---------------------\n---------------------\n");
//        std::printf("Topology creation\n");
        cr = cairo;

        //debug only:
        steps_max = stepsmax;
        //-------------

        input_paths = paths;

        vertices.clear();
        edges.clear();
        areas.clear();
        Area empty( input_paths.size() );
        areas.push_back(empty);

        Sweeper sweeper( paths, X, tol );

        edges = std::vector<Edge>( sweeper.tiles_data.size(), Edge() );
        for (unsigned i=0; i<edges.size(); i++){
            edges[i].path = sweeper.tiles_data[i].path;
            edges[i].curve = sweeper.tiles_data[i].curve;
            edges[i].portion = Interval(sweeper.tiles_data[i].f, sweeper.tiles_data[i].t);
        }

        //std::printf("entering event loop:\n");
        int step=0;
        for(Sweeper::Event event = sweeper.getNextEvent(); ; event = sweeper.getNextEvent() ){
//            std::printf("   new event received: ");
            //print();
            //debug only!!!
            if ( steps_max >= 0 && step > steps_max ){
                break;
            }else{
                step++;
            }
            //---------

            if (event.empty()){
                //std::printf("   empty event received\n");
                break;
            }

            //std::printf("   non empty event received:");
            //sweeper.printEvent(event);

            //is this a new event or the continuation of an old one?
            unsigned v;
            Rect r = sweeper.context.pending_vertex;
            if (vertices.empty() || !r.intersects( vertices.back().bounds ) ){
                v = vertices.size();
                vertices.push_back(Vertex());
                vertices[v].bounds = r;
//                std::printf("   new intersection created (%u).\n",v);
            }else{
                v = vertices.size()-1;
//                std::printf("   continue last intersection (%u).\n",v);
            }

            //--Closing an edge:-------------
            if( !event.opening ){
                unsigned e = event.tile, a, b;
//                std::printf("   closing edge %u\n", e);
                bool reversed = sweeper.tiles_data[e].reversed;//Warning: true means v==e.start
                if (reversed){
                    edges[e].start = v;
                    a = edges[e].right;
                    b = edges[e].left;
                }else{
                    edges[e].end = v;
                    a = edges[e].left;
                    b = edges[e].right;
                }
                OrientedEdge vert_edge(e, reversed);
                if (vertices[v].boundary.size()>0){//Make sure areas are compatible (only relevant if the last event was an opening).
                    fuseAreas ( a, target( vertices[v].boundary.back(), false ) );
                }
                assert( prolongate( vertices[v].boundary, vert_edge) );
                fuseConnectedBoundaries(a);//there is no doing both: tests are performed twice but for 2 areas.
                fuseConnectedBoundaries(b);//
            }else{
            //--Opening an edge:-------------
                unsigned e = event.tile;
//                std::printf("   opening edge %u\n", e);
                bool reversed = !sweeper.tiles_data[e].reversed;//Warning: true means v==start.

                //--Find first and last area around this vertex:-------------
                unsigned cur_a;
                if ( vertices[v].boundary.size() > 0 ){
                    cur_a = target( vertices[v].boundary.back(), false );
                }else{//this vertex is empty
                    if ( event.insert_at < sweeper.context.size() ){
                        unsigned upper_tile = sweeper.context[event.insert_at].first;
                        cur_a = (sweeper.tiles_data[upper_tile].reversed) ? edges[upper_tile].left : edges[upper_tile].right;
                    }else{
                        cur_a = 0;
                    }
                }

                unsigned new_a = areas.size();

                Area new_area(paths.size());
                new_area.boundary.push_back( OrientedEdge(e, !reversed ) );
                new_area.windings = areas[cur_a].windings;//FIXME: escape boundary cases!!!
                if ( input_paths[edges[e].path].closed() ){
                    new_area.windings[edges[e].path] += (reversed) ? +1 : -1;
                }
                areas.push_back(new_area);

                //update edge
                if (reversed){
                    edges[e].start = v;
                    edges[e].left = new_a;
                    edges[e].right = cur_a;
                }else{
                    edges[e].end = v;
                    edges[e].left = cur_a;
                    edges[e].right = new_a;
                }
                //update vertex
                OrientedEdge f(e, reversed);
                assert( prolongate( vertices[v].boundary, f) );
                addAreaBoundaryPiece(cur_a, OrientedEdge(e, reversed) );
            }
            if (!event.to_be_continued && vertices[v].boundary.size()>0){
                unsigned first_a = source( vertices[v].boundary.front(), false );
                unsigned last_a = target( vertices[v].boundary.back(), false );
                fuseAreas(first_a, last_a);
            }

//            this->print();
//            std::printf("----------------\n");
            //std::printf("\n");
        }
    }



    //----------------------------------------------------
    //-- done.
    //----------------------------------------------------
};


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
