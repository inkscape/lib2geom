#include <iostream>
#include <2geom/path.h>
#include <2geom/path-intersection.h>
#include <2geom/basic-intersection.h>
#include <2geom/pathvector.h>
#include <2geom/exception.h>

#include <cstdlib>
#include <cstdio>
#include <set>
#include <vector>
#include <algorithm>

#include <limits.h>
#define NULL_IDX UINT_MAX

using namespace Geom;
using namespace std;


/*
The sweeper class takes a PathVector as input and generates "events" to let clients construct the relevant graph.

The basic strategy is the following:
The path is split into "tiles": a tile consists in 2 boxes related by a (monotonic) curve.

The tiles are created at the very beginning, using a sweep, but *no care* is taken to topology
information at this step! All the boxes of all the tiles are then enlarged so that they are
either equal or disjoint.
[TODO: we should look for curves traversing boxes, split them and repeat the process...]

The sweeper maintains a virtual sweepline, that is the limit of the "known area". The tiles can have 2 states:
open if they have one end in the known area, and one in the unknown, closed otherwise.
[TODO: open/close should belong to tiles pointers, not tiles...]

The sorted list of open tiles intersecting the sweep line is called the "context".
*!*WARNING*!*: because the sweep line is not straight, closed tiles can still be in the context!!
they can only be removed once the end of the last box is reached.

The events are changes in the context when the sweep line crosses boxes.
They are obtained by sorting the tiles according to one or the other of theire end boxes depending
on the open/close state.

A "big" event happens when the sweep line reaches a new 'box'. After such a "big" event, the sweep
line goes round the new box along it's 3 other sides.
N.B.: in an ideal world, all tiles ending at one box would be on one side, all the tiles starting
there on the other. Unfortunately, because we have boxes as vertices, things are not that nice:
open/closed tiles can appear in any order around a vertex, even in the monotonic case(!). Morover,
our fat vertices have a non zero "duration", during which many things can happen: this is why we
have to keep closed edges in the context until both ends of theire boxes are reached...


To keep things uniform, such "big" events are split into elementary ones: opening/closing of a single
edge. One such event is generated for each tile around the current 'box', in CCW order (geometrically,
the sweepline is deformed in a neighborhood of the box to go round it for a certain amount, enter the
box and come back inside the box; the piece inside the box is a "virtual edge" that is not added for
good but that we keep track of). The event knows if it's the last one in such a sequence, so that the
client knows when to do the additional work required to "close" the vertex construction. Hmmm. It's
hard to explain the moves without a drawing here...(see sweep.svg in the doc dir). There are

*Closings: insert a new the relevant tile in the context with a "exit" flag.

*Openings: insert a new the relevant tile in the context with a "entry" flag.

At the end of a box, the relevant exit/entries are purged from the context.


N.B. I doubt we can do boolops without building the full graph, i.e. having different clients to obtain
different outputs. So splitting sweeper/grpah builder is maybe not so relevant w/r to functionality
(only code organization).
*/


class Sweeper{
public:


    //---------------------------
    // utils...
    //---------------------------

    //near predicate utilized in process_splits
    template<typename T>
    struct NearPredicate {
        double tol;
        NearPredicate(double eps):tol(eps){}
        NearPredicate(){tol = EPSILON;}//???
        bool operator()(T x, T y) { return are_near(x, y, tol); } };

    // ensures that f and t are elements of a vector, sorts and uniqueifies
    // also asserts that no values fall outside of f and t
    // if f is greater than t, the sort is in reverse
    void process_splits(std::vector<double> &splits, double f, double t, double tol=EPSILON) {
        //splits.push_back(f);
        //splits.push_back(t);
        std::sort(splits.begin(), splits.end());
        std::vector<double>::iterator end = std::unique(splits.begin(), splits.end(), NearPredicate<double>(tol));
        splits.resize(end - splits.begin());

        //remove any splits which fall outside t / f
        while(!splits.empty() && splits.front() < f+tol) splits.erase(splits.begin());
        splits.insert(splits.begin(), f);
        //splits[0] = f;
        while(!splits.empty() && splits.back()  > t-tol) splits.erase(splits.end() - 1);
        splits.push_back(t);
        //splits.back() = t;
    }

    Rect fatPoint(Point const &p, double radius){
        return Rect( p+Point(-radius,-radius), p+Point( radius, radius) ) ;
    }



    //---------------------------
    // Tiles.
    //---------------------------

    //A tile is a "light edge": just two boxes, joint by a curve.
    //it is open iff intersected by the sweepline.
    class Tile{
    public:
        unsigned path;
        unsigned curve;
        double f;
        double t;
        Rect fbox, tbox;
        bool reversed;//with respect to sweep direction. Flip f/t instead?
        bool open;//means sweepline currently cuts it (i.e. one end in the known area, the other in the unknown).
        int state;//-1: both ends in unknown area, 0:one end in each, 1: both in known area.
        //Warning: we can not delete a tile immediately when it's past(=closed again), only when the end of it's tbox is!.
        Rect bbox(){Rect b = fbox; b.unionWith(tbox); return b;}
        Point min(){return ( bbox().min() ); }
        Point max(){return ( bbox().max() ); }
//        Rect cur_box() const {return ((open)^(reversed) ) ? tbox : fbox; }
        Rect cur_box() const { return ((state>=0)^(reversed) ) ? tbox : fbox; }
        Rect first_box() const {return ( reversed ) ? tbox : fbox; }
        Rect last_box() const {return ( reversed ) ? fbox : tbox; }
    };

    D2<SBasis> tileToSB(Tile const &tile){
        //TODO: don't convert each time!!!!!!
        assert( tile.path < paths.size() );
        assert( tile.curve < paths[tile.path].size() );
        D2<SBasis> c = paths[tile.path][tile.curve].toSBasis();
        c = portion( c, Interval( tile.f, tile.t ) );
        return c;
    }

    //SweepOrder for Rects or Tiles.
    class SweepOrder{
    public:
        Dim2 dim;
        SweepOrder(Dim2 d) : dim(d) {}
        bool operator()(const Rect &a, const Rect &b) const {
            return Point::LexOrderRt(dim)(a.min(), b.min());
        }
        bool operator()(const Tile &a, const Tile &b) const {
            return Point::LexOrderRt(dim)(a.cur_box().min(), b.cur_box().min());
        }
    };

    class PtrSweepOrder{
    public:
        Dim2 dim;
        std::vector<Tile>::iterator const begin;
        PtrSweepOrder(std::vector<Tile>::iterator const beg, Dim2 d) : dim(d), begin(beg){}
        bool operator()(const unsigned a, const unsigned b) const {
            return Point::LexOrderRt(dim)((begin+a)->cur_box().min(), (begin+b)->cur_box().min());
        }
    };


    //---------------------------
    // Vertices.
    //---------------------------

    //A ray is nothing but an edge ending or starting at a given vertex, + some info about when/where it exited a "separating" box;
    struct Ray{
    public:
        unsigned tile;
        bool centrifuge;//true if the intrinsic orientation of curve points away from the vertex.
        //exit info:
        unsigned exit_side;//0:y=min; 1:x=max; 2:y=max; 3:x=min.
        double exit_place; //x or y value on the exit line.
        double exit_time;  //exit time on curve.
        Ray(){tile = NULL_IDX; exit_side = 4;}
        Ray(unsigned tile_idx,  unsigned s, double p, double t){
            tile = tile_idx;
            exit_side =s;
            exit_place = p;
            exit_time = t;
        }
        Ray(unsigned tile_idx, bool outward){
            tile = tile_idx;
            exit_side = 4;
            centrifuge = outward;
            exit_time = (centrifuge) ? 2 : -1 ;
        }
        void setExitInfo( unsigned side, double place, double time){
            exit_side = side;
            exit_place = place;
            exit_time = time;
        }
    };

    class FatVertex : public Rect{
    public:
        std::vector<Ray> rays;
        FatVertex(const Rect &r, unsigned const tile, bool centrifuge) : Rect(r){
            rays.push_back( Ray(tile, centrifuge) );
        }
        FatVertex(Rect r) : Rect(r){}
        FatVertex() : Rect(){}
        void erase(unsigned from, unsigned to){
            unsigned size = to-from;
            from = from % rays.size();
            to = from + size;

            if (to >= rays.size() ){
                to  = to % rays.size();
                rays.erase( rays.begin()+from, rays.end() );
                rays.erase( rays.begin(), rays.begin()+to );
            }else{
                rays.erase( rays.begin()+from, rays.begin()+to );
            }

        }
    };

    //---------------------------
    // Context related stuff.
    //---------------------------

    class Event{
    public:
        bool opening;//true means an edge is added, otherwise an edge is removed from context.
        unsigned tile;//which tile to open/close.
        unsigned insert_at;//where to insert the next tile in the context.
        //unsigned erase_at;//idx of the tile to close in the context.  = context.find(tile).
        bool to_be_continued;
        bool empty(){
            return  tile==NULL_IDX;
        }
        Event(){
            opening = false;
            insert_at = 0;
            //erase_at = 0;
            tile = NULL_IDX;
            to_be_continued = false;
        }
    };

    void printEvent(Event const &e){
        std::printf("Event: ");
        std::printf("%s, ", e.opening?"opening":"closing");
        std::printf("insert_at:%u, ", e.insert_at);
        //std::printf("erase_at:%u, ", e.erase_at);
        std::printf("tile:%u.\n", e.tile);
    }

    class Context : public std::vector<std::pair<unsigned,bool> >{//first = tile, second = true if it's a birth (+).
    public:
        Point last_pos;
        FatVertex pending_vertex;
        Event pending_event;

        unsigned find(unsigned const tile, bool positive_only=false){
            for (unsigned i=0; i<size(); i++){
                if ( (*this)[i].first == tile ){
                    if ( (*this)[i].second || !positive_only ) return i;
                }
            }
            return (*this).size();
        }
    };
    void printContext(){
        std::printf("context:[");
        for (unsigned i=0; i<context.size(); i++){
            unsigned tile = context[i].first;
            assert( tile<tiles_data.size() );
            std::printf(" %s%u%s", (tiles_data[ tile ].reversed)?"-":"+", tile, (context[i].second)?"o":"c");
//            assert( context[i].second || !tiles_data[ tile ].open);
            assert( context[i].second || tiles_data[ tile ].state==1);
        }
        std::printf("]\n");
    }



    //----
    //This is the heart of it all!! Take particular care to non linear sweep line...
    //----
    //Given a point on the sweep line, (supposed to be the min() of a vertex not yet connected to the already known part),
    //find the first edge "after" it in the context. Pretty tricky atm :-(
    //TODO: implement this as a lower_bound (?).

    unsigned contextRanking(Point const &pt){

//        std::printf("contextRanking:------------------------------------\n");

        unsigned rank = context.size();
        std::vector<unsigned> unmatched_closed_tiles = std::vector<unsigned>();

//        std::printf("Scan context.\n");

        for (unsigned i=0; i<context.size(); i++){

            unsigned tile_idx = context[i].first;
            assert(  tile_idx < tiles_data.size() );
//            std::printf("testing %u (e=%u),", i, tile_idx);

            Tile tile = tiles_data[tile_idx];
            assert( tile.state >= 0 );

            //if the tile is open (i.e. not both ends in the known area) and point is below/above the tile's bbox:
            if ( tile.state == 0 ){
//                std::printf("opened tile, ");
                if (pt[1-dim] < tile.min()[1-dim] ) {
//                    printContext();
//                    std::printf("below bbox %u!\n", i);
                    rank = i;
                    break;
                }
                if (pt[1-dim] > tile.max()[1-dim] ){
//                    std::printf("above bbox %u!\n", i);
                    continue;
                }
                
                //TODO: don't convert each time!!!!!!
                D2<SBasis> c = tileToSB( tile );
            
                std::vector<double> times = roots(c[dim]-pt[dim]);
                if (times.size()==0){
                    assert( tile.first_box()[dim].contains(pt[dim]) );
                    if ( pt[1-dim] < tile.first_box()[1-dim].min() ){
//                        std::printf("open+hit box %u!\n", i);
                        rank = i;
                        break;
                    }else{
                        continue;
                    }
                }
                if ( pt[1-dim] < c[1-dim](times.front()) ){
//                    std::printf("open+hit curve %u!\n", i);
                    rank = i;
                    break;
                }            
            }

//            std::printf("closed tile, ");


            //At this point, the tile is closed (i.e. both ends are in the known area)
            //Such tiles do 'nested parens' like travels in the unknown area.
            //We are interested in the second occurence only (to give a chance to open tiles to exist inbetween).
            if ( unmatched_closed_tiles.size()==0 || tile_idx != unmatched_closed_tiles.back() ){
                unmatched_closed_tiles.push_back( tile_idx );
//                std::printf("open paren %u\n",tile_idx);
                continue;
            }
            unmatched_closed_tiles.pop_back();

//            std::printf("close paren, ");
            
            if ( !tile.bbox().contains( pt ) ){
                continue;
            }
            
//            std::printf("in bbox, ");

            //At least one of fbox[dim], tbox[dim] has to contain the pt[dim]: assert it?

            //Find intersection with the hline(vline if dim=Y) through the point
            double hit_place;
            //TODO: don't convert each time!!!!!!
           D2<SBasis> c = tileToSB( tile );
           std::vector<double> times = roots(c[1-dim]-pt[1-dim]);
           if ( times.size()>0 ){
//               std::printf("hit curve,");
               hit_place = c[dim](times.front());
           }else{
//               std::printf("hit box, ");
               //if there was no intersection, the line went through the first_box
               assert( tile.first_box()[1-dim].contains(pt[1-dim]) );
               continue;
           }
            
            if ( pt[dim] > hit_place ){
//                std::printf("wrong side, ");
                continue;
            }
//            std::printf("good side, ");
            rank = i;
            break;
        }

//        std::printf("rank %u.\n", rank);
//        printContext();
        assert( rank<=tiles_data.size() );
        return rank;
    }

    //TODO: optimize this.
    //it's done the slow way for debugging purpose...
    void purgeDeadTiles(){
        //std::printf("purge ");
        //printContext();
        for (unsigned i=0; i<context.size(); i++){
            assert( context[i].first<tiles_data.size() );
            Tile tile = tiles_data[context[i].first];
            if (tile.state==1 && Point::LexOrderRt(dim)( tile.fbox.max(), context.last_pos ) && Point::LexOrderRt(dim)( tile.tbox.max(), context.last_pos ) ){
//            if (!tile.open && Point::LexOrderRt(dim)( tile.fbox.max(), context.last_pos ) && Point::LexOrderRt(dim)( tile.tbox.max(), context.last_pos ) ){
                unsigned j;
                for (j=i+1; j<context.size() && context[j].first != context[i].first; j++){}
                assert ( j < context.size() );
                if ( context[j].first == context[i].first){
                    assert ( context[j].second == !context[i].second );
                    context.erase(context.begin()+j);
                    context.erase(context.begin()+i);
//                    printContext();
                    i--;
                }
            }
        }
        return;
    }

    void applyEvent(Event event){
//        std::printf("Apply event : ");
        if(event.empty()){
//            std::printf("empty event!\n");
            return;
        }

//        printEvent(event);
//        std::printf("    old ");
//        printContext();

        assert ( event.insert_at <= context.end()-context.begin() );

        if (!event.opening){
//             unsigned idx = event.erase_at;
//             assert( idx == context.find(event.tile) );
//             assert( context[idx].first == event.tile);
            tiles_data[event.tile].open = false;
            tiles_data[event.tile].state = 1;
            //context.erase(context.begin()+idx);
            unsigned idx = event.insert_at;
            context.insert(context.begin()+idx, std::pair<unsigned, bool>(event.tile, false) );
        }else{
            unsigned idx = event.insert_at;
            tiles_data[event.tile].open = true;
            tiles_data[event.tile].state = 0;
            context.insert(context.begin()+idx, std::pair<unsigned, bool>(event.tile, true) );
            sortTiles();
        }
        context.last_pos = context.pending_vertex.min();
        context.last_pos[1-dim] = context.pending_vertex.max()[1-dim];

//        std::printf("    new ");
//        printContext();
//        std::printf("\n");
        //context.pending_event = Event();is this a good idea?
    }



    //---------------------------
    // Sweeper.
    //---------------------------

    PathVector paths;
    std::vector<Tile> tiles_data;
    std::vector<unsigned> tiles;
    std::vector<Rect> vtxboxes;
    Context context;
    double tol;
    Dim2 dim;


    //-------------------------------
    //-- Tiles preparation.
    //-------------------------------

    //split input paths into monotonic pieces...
    void createMonotonicTiles(){
        for ( unsigned i=0; i<paths.size(); i++){
            for ( unsigned j=0; j<paths[i].size(); j++){
                //find the points of 0 derivative
                Curve* deriv = paths[i][j].derivative();
                std::vector<double> splits = deriv->roots(0, X);
                std::vector<double> splitsY = deriv->roots(0, Y);
                splits.insert(splits.begin(), splitsY.begin(), splitsY.end() );
                delete deriv;
                process_splits(splits,0,1);

                double t=0;
                for(unsigned k = 1; k < splits.size(); k++){
                    Tile tile;
                    tile.path = i;
                    tile.curve = j;
                    tile.f = t;
                    tile.t = splits[k];
                    //TODO: use meaningful tolerance here!!
                    Point fp = paths[i][j].pointAt(tile.f);
                    Point tp = paths[i][j].pointAt(tile.t);
#if 1
                    tile.fbox = fatPoint(fp, 0 );
                    tile.tbox = fatPoint(tp, 0 );
#else
                    tile.fbox = fatPoint(fp, tol );
                    tile.tbox = fatPoint(tp, tol );
#endif
                    tile.open = false;
                    tile.state = -1;
                    tile.reversed = Point::LexOrderRt(dim)(tp, fp);

                    tiles_data.push_back(tile);
//                     //TODO: maybe too early??
//                     //*Yes*!!! this tile will be removed, but it's boxes play a role at enlarging time!!
//                     if ( !tile.fbox.intersects(tile.tbox) ){
//                         tiles_data.push_back(tile);
//                     }
                    t = splits[k];
                }
            }
        }
        std::sort(tiles_data.begin(), tiles_data.end(), SweepOrder(dim) );
    }

    void splitTile(unsigned i, double t, double tolerance=0, bool sort = true){
        assert( i<tiles_data.size() );
        Tile newtile = tiles_data[i];
        assert( newtile.f < t && t < newtile.t );
        newtile.f = t;
        newtile.fbox = fatPoint(paths[newtile.path][newtile.curve].pointAt(t), tolerance );
        tiles_data[i].tbox = newtile.fbox;
        tiles_data[i].t = t;
        tiles_data.insert(tiles_data.begin()+i+1, newtile);
        if (sort)
            std::sort(tiles_data.begin()+i+1, tiles_data.end(), SweepOrder(dim) );
    }

    void splitTile(unsigned i, std::vector<double> const &times, double tolerance=0, bool sort = true){
        if ( times.size()<3 ) return;
        assert( i<tiles_data.size() );
        std::vector<Tile> pieces ( times.size()-2, tiles_data[i] );
        Rect prevbox = tiles_data[i].fbox;
        for (unsigned k=0; k < times.size()-2; k++){
                 pieces[k].f = times[k];
                 pieces[k].t = times[k+1];
                 pieces[k].fbox = prevbox;
                 //TODO: use relevant precision here.
                 prevbox = fatPoint(paths[tiles_data[i].path][tiles_data[i].curve].pointAt(times[k+1]), tolerance );
                 pieces[k].tbox = prevbox;
        }
        tiles_data.insert(tiles_data.begin()+i, pieces.begin(), pieces.end() );
        unsigned newi = i + times.size()-2;
        assert( newi<tiles_data.size() );
        assert( newi>=1 );
        tiles_data[newi].f    = tiles_data[newi-1].t;
        tiles_data[newi].fbox = tiles_data[newi-1].tbox;

        if (sort)
            std::sort(tiles_data.begin()+i, tiles_data.end(), SweepOrder(dim) );
    }

    //TODO: maybe not optimal. For a fully optimized sweep, it would be nice to have
    //an efficient way to way find *only the first* intersection (in sweep direction)...
    void splitIntersectingTiles(){
        //make sure it is sorted, but should be ok. (remove sorting at the end of monotonic tiles creation?
        std::sort(tiles_data.begin(), tiles_data.end(), SweepOrder(dim) );

//        std::printf("\nFind intersections: tiles_data.size():%u\n", tiles_data.size() );

        for (unsigned i=0; i+1<tiles_data.size(); i++){
//            std::printf("\ni=%u (%u([%f,%f]))\n", i, tiles_data[i].curve, tiles_data[i].f, tiles_data[i].t );
            std::vector<double > times_i;
            for (unsigned j=i+1; j<tiles_data.size(); j++){
//                std::printf("  j=%u (%u)\n", j,tiles_data[j].curve );
                if ( Point::LexOrderRt(dim)(tiles_data[i].max(), tiles_data[j].min()) ) break;
//                g_warning("FIXME: Sweeper tolerance meaning?? make mono_intersect tolerance aware.");
                Crossings crossings = mono_intersect(paths[tiles_data[i].path][tiles_data[i].curve], Interval(tiles_data[i].f, tiles_data[i].t),
                                                     paths[tiles_data[j].path][tiles_data[j].curve], Interval(tiles_data[j].f, tiles_data[j].t) );
                std::vector<double > times_j (crossings.size(), 0 );
                for (unsigned k=0; k < crossings.size(); k++){
                    Crossing c = crossings[k];
                    times_i.push_back( c.ta );
                    times_j[k] = c.tb;
                }
                process_splits(times_j, tiles_data[j].f, tiles_data[j].t);

//                std::printf("  >|%u/%u|=%u. times_j:%u", i, j, crossings.size(),  times_j.size() );

                splitTile(j, times_j, tol, false);
                j+=times_j.size()-2;
//                std::printf("  new j:%u\n",j );
            }

            process_splits(times_i, tiles_data[i].f, tiles_data[i].t);
            assert(times_i.size()>=2);
            splitTile(i, times_i, tol, false);
            i+=times_i.size()-2;
//             std::printf("new i:%u",i );
            std::sort(tiles_data.begin()+i+1, tiles_data.end(), SweepOrder(dim) );
        }
        //this last sorting should be useless!!
        std::sort(tiles_data.begin(), tiles_data.end(), SweepOrder(dim) );
    }

    void sortTiles(){
        std::sort(tiles.begin(), tiles.end(), PtrSweepOrder(tiles_data.begin(), dim) );
    }


    //-------------------------------
    //-- Vertices boxes cookup.
    //-------------------------------

    void fuseInsert(const Rect &b,  std::vector<Rect> &boxes, Dim2 dim){
        //TODO: this can be optimized...
        for (unsigned i=0; i<boxes.size(); i++){
            if ( Point::LexOrderRt(dim)( b.max(), boxes[i].min() ) ) break;
            if ( b.intersects( boxes[i] ) ){
                Rect bigb = b;
                bigb.unionWith( boxes[i] );
                boxes.erase( boxes.begin()+i );
                fuseInsert( bigb, boxes, dim);
                return;
            }
        }
        std::vector<Rect>::iterator pos = std::lower_bound(boxes.begin(), boxes.end(), b, SweepOrder(dim) );
        boxes.insert( pos, b );
    }

    //debug only!!
    bool isContained(Rect b,   std::vector<Rect> const &boxes ){
        for (unsigned i=0; i<boxes.size(); i++){
            if ( boxes[i].contains(b) ) return true;
        }
        return false;
    }

    //Collect vertex boxes. Fuse overlapping ones.
    //NB: enlarging a vertex may create intersection with already scanned ones...
    std::vector<Rect> collectBoxes(){
        std::vector<Rect> ret;
        for (unsigned i=0; i<tiles_data.size(); i++){
            fuseInsert(tiles_data[i].fbox, ret, dim);
            fuseInsert(tiles_data[i].tbox, ret, dim);
        }
        return ret;
    }

    //enlarge tiles ends to match the vertices bounding boxes.
    //remove edges fully contained in one vertex bbox.
    void enlargeTilesEnds(const std::vector<Rect> &boxes ){
        for (unsigned i=0; i<tiles_data.size(); i++){
            std::vector<Rect>::const_iterator f_it;
            f_it = std::lower_bound(boxes.begin(), boxes.end(), tiles_data[i].fbox, SweepOrder(dim) );
            if ( f_it==boxes.end() ) f_it--;
            while (!(*f_it).contains(tiles_data[i].fbox) && f_it != boxes.begin()){
                f_it--;
            }
            assert( (*f_it).contains(tiles_data[i].fbox) );
            tiles_data[i].fbox = *f_it;

            std::vector<Rect>::const_iterator t_it;
            t_it = std::lower_bound(boxes.begin(), boxes.end(), tiles_data[i].tbox, SweepOrder(dim) );
            if ( t_it==boxes.end() ) t_it--;
            while (!(*t_it).contains(tiles_data[i].tbox) && t_it != boxes.begin()){
                t_it--;
            }
            assert( (*t_it).contains(tiles_data[i].tbox) );
            tiles_data[i].tbox = *t_it;

            //NB: enlarging the ends may swapp their sweep order!!!
            tiles_data[i].reversed = Point::LexOrderRt(dim)( tiles_data[i].tbox.min(), tiles_data[i].fbox.min());

            if ( f_it==t_it ){
                tiles_data.erase(tiles_data.begin()+i);
                i-=1;
            }
        }
    }

    //Make sure tiles stop at vertices. Split them if needed.
    //Returns true if at least one tile was split.
    bool splitTilesThroughFatPoints(std::vector<Rect> &boxes ){
        
        std::sort(tiles.begin(), tiles.end(), PtrSweepOrder(tiles_data.begin(), dim) );
        std::sort(boxes.begin(), boxes.end(), SweepOrder(dim) );

        bool result = false;
        for (unsigned i=0; i<tiles_data.size(); i++){
            for (unsigned k=0; k < boxes.size(); k++){
                if ( Point::LexOrderRt(dim)( tiles_data[i].max(), boxes[k].min()) ) break;
                if ( Point::LexOrderRt(dim)( boxes[k].max(), tiles_data[i].min()) ) continue;
                if ( !boxes[k].intersects( tiles_data[i].bbox() ) ) continue;
                if ( tiles_data[i].fbox.intersects( boxes[k] ) ) continue;
                if ( tiles_data[i].tbox.intersects( boxes[k] ) ) continue;

                //at this point box[k] intersects the curve bbox away from the fbox and tbox.

                D2<SBasis> c = tileToSB( tiles_data[i] );
                for (unsigned corner=0; corner<4; corner++){
                    unsigned D = corner % 2;
                    double val = boxes[k].corner(corner)[D];
                    std::vector<double> times = roots( c[D] - val );
                    if ( times.size()>0 ){
                        double t = lerp( times.front(), tiles_data[i].f, tiles_data[i].t );
                        double hit_place = c[1-D](times.front());
                        if ( boxes[k][1-D].contains(hit_place) ){
                            result = true;
                            //taking a point on the boundary is dangerous!! 
                            //Either use >0 tolerance here, or find 2 intersection points and split inbetween.
                            splitTile( i, t, tol, false);
                            break;
                        }
                    }
                }
            }
        }
        return result;
    }





    //TODO: rewrite all this!...
    //-------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------
    //-------------------------------
    //-- ccw Sorting of rays around a vertex.
    //-------------------------------
    //-------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------
    //-------------------------------------------------------------------------------------------
    //returns an (infinite) rect around "a" separating it from "b". Nota: 3 sides are infinite!
    //TODO: place the cut where there is most space...
    OptRect separate(Rect const &a, Rect const &b){
        Rect ret ( Interval( -infinity(), infinity() ) , Interval(-infinity(), infinity() ) );
        double gap = 0;
        unsigned dir = 4;
        if (b[X][0] - a[X][1] > gap){
            gap = b[X][0] - a[X][1];
            dir = 0;
        }
        if (a[X][0] - b[X][1] > gap){
            gap = a[X][0] - b[X][1];
            dir = 1;
        }
        if (b[Y][0] - a[Y][1] > gap){
            gap = b[Y][0] - a[Y][1];
            dir = 2;
        }
        if (a[Y][0] - b[Y][1] > gap){
            gap = a[Y][0] - b[Y][1];
            dir = 3;
        }
        switch (dir) {
            case 0: ret[X][1] = ( a.max()[X] + b.min()[X] )/ 2; break;
            case 1: ret[X][0] = ( b.max()[X] + a.min()[X] )/ 2; break;
            case 2: ret[Y][1] = ( a.max()[Y] + b.min()[Y] )/ 2; break;
            case 3: ret[Y][0] = ( b.max()[Y] + a.min()[Y] )/ 2; break;
            case 4: return OptRect();
        }
        return OptRect(ret);
    }

    //Find 4 lines (returned as a Rect sides) that cut all the rays (=edges). *!* some side might be infinite.
    OptRect isolateVertex(Rect const &box){
        OptRect sep ( Interval( -infinity(), infinity() ) , Interval(-infinity(), infinity() ) );
        //separate this vertex from the others. Find a better way.
        for (unsigned i=0; i<vtxboxes.size(); i++){
            if ( Point::LexOrderRt(dim)( sep->max(), vtxboxes[i].min() ) ){
                break;
            }
            if ( vtxboxes[i]!=box ){//&& !vtxboxes[i].intersects(box) ){
                OptRect sepi = separate(box, vtxboxes[i]);
                if ( sep && sepi ){
                    sep = intersect(*sep, *sepi);
                }else{
                    std::cout<<"box="<<box<<"\n";
                    std::cout<<"vtxboxes[i]="<<vtxboxes[i]<<"\n";
                    assert(sepi);
                }
            }
        }
        if (!sep) THROW_EXCEPTION("Invalid intersection data.");
        return sep;
    }

    //TODO: argh... rewrite to have "dim"=min first place.
    struct ExitPoint{
    public:
        unsigned side; //0:y=min; 1:x=max; 2:y=max; 3:x=min.
        double place; //x or y value on the exit line.
        unsigned ray_idx;
        double time; //exit time on curve.
        ExitPoint(){}
        ExitPoint(unsigned s, double p, unsigned r, double t){
            side =s;
            place = p;
            ray_idx = r;
            time = t;
        }
    };


    class ExitOrder{
    public:
        bool operator()(Ray a, Ray b) const {
            if ( a.exit_side < b.exit_side ) return true;
            if ( a.exit_side > b.exit_side ) return false;
            if ( a.exit_side <= 1) {
                return ( a.exit_place < b.exit_place );
            }
            return ( a.exit_place > b.exit_place );
        }
    };

    void printRay(Ray const &r){
        std::printf("Ray: tile=%u, centrifuge=%u, side=%u, place=%f\n",
                    r.tile, r.centrifuge, r.exit_side, r.exit_place);
    }
    void printVertex(FatVertex const &v){
        std::printf("Vertex: [%f,%f]x[%f,%f]\n", v[X][0],v[X][1],v[Y][0],v[Y][1] );
        for (unsigned i=0; i < v.rays.size(); i++){
            printRay(v.rays[i]);
        }
    }

    //TODO: use a partial order on input coming from the context + Try quadrant order just in case it's enough.
    //TODO: use monotonic assumption.
    void sortRays( FatVertex &v ){
        OptRect sep = isolateVertex(v);

        for (unsigned i=0; i < v.rays.size(); i++){
            v.rays[i].centrifuge = (tiles_data[ v.rays[i].tile ].fbox == v);
            v.rays[i].exit_time = v.rays[i].centrifuge ? 1 : 0 ;
        }

        for (unsigned i=0; i < v.rays.size(); i++){
            //TODO: don't convert each time!!!
            assert( v.rays[i].tile < tiles_data.size() );
            D2<SBasis> c = tileToSB( tiles_data[ v.rays[i].tile ] );

            for (unsigned side=0; side<4; side++){//scan X or Y direction, on level min or max...
                double level = sep->corner( side )[1-side%2];
                if (level != infinity() && level != -infinity() ){
                    std::vector<double> times = roots(c[1-side%2]-level);
                    if ( times.size() > 0 ) {
                        double t;
                        assert( v.rays[i].tile < tiles_data.size() );
                        if (tiles_data[ v.rays[i].tile ].fbox == v){
                            t = times.front();
                            if ( v.rays[i].exit_side > 3 || v.rays[i].exit_time > t ){
                                v.rays[i].setExitInfo( side, c[side%2](t), t);
                            }
                        }else{
                            t = times.back();
                            if ( v.rays[i].exit_side > 3 || v.rays[i].exit_time < t ){
                                v.rays[i].setExitInfo( side, c[side%2](t), t);
                            }
                        }
                    }
                }
            }
        }

        //Rk: at this point, side == 4 means the edge is contained in the intersection box (?)...;
        std::sort( v.rays.begin(), v.rays.end(), ExitOrder() );
    }


    //-------------------------------
    //-- intialize all data.
    //-------------------------------

    Sweeper(){}
    Sweeper(PathVector const &input_paths, Dim2 sweep_dir, double tolerance=1e-5){
        paths = input_paths;//use a ptr...
        dim = sweep_dir;
        tol = tolerance;

        //split paths into monotonic tiles
        createMonotonicTiles();

        //split at tiles intersections
        splitIntersectingTiles();

        //handle overlapping end boxes/and tiles traversing boxes.
        do{
            vtxboxes = collectBoxes();
        }while ( splitTilesThroughFatPoints(vtxboxes) );

        enlargeTilesEnds(vtxboxes);

        //now create the pointers to the tiles.
        tiles = std::vector<unsigned>(tiles_data.size(), 0);
        for (unsigned i=0; i<tiles_data.size(); i++){
            tiles[i] = i;
        }
        sortTiles();

        //initialize the context.
        if (tiles_data.size()>0){
            context.clear();
            context.last_pos = tiles_data[tiles.front()].min();
            context.last_pos[dim] -= 1;
            context.pending_vertex = FatVertex();
            context.pending_event = Event();
        }

//        std::printf("Sweeper initialized (%u tiles)\n", tiles_data.size());
    }


    //-------------------------------
    //-- Event walk.
    //-------------------------------


    Event getNextEvent(){
//        std::printf("getNextEvent():\n");

//        std::printf("initial contex:\n");
//        printContext();
        Event old_event = context.pending_event, event;
//        std::printf("apply old event\n");
        applyEvent(context.pending_event);
//        printContext();

        if (context.pending_vertex.rays.size()== 0){
//            std::printf("cook up a new vertex\n");

            //find the edges at the next vertex.
            //TODO: implement this as a lower bound!!
            std::vector<unsigned>::iterator low, high;
            //Warning: bad looking test, but make sure we advance even in case of 0 width boxes...
            for ( low = tiles.begin(); low != tiles.end() && 
                      ( tiles_data[*low].state==1 || Point::LexOrderRt(dim)(tiles_data[*low].cur_box().min(), context.last_pos) ); low++){}

            if ( low == tiles.end() ){
//                std::printf("no more event found\n");
                return(Event());
            }
            Rect pos = tiles_data[ *low ].cur_box();
            context.last_pos = pos.min();
            context.last_pos[1-dim] = pos.max()[1-dim];

//            printContext();
//            std::printf("purgeDeadTiles\n");
            purgeDeadTiles();
//            printContext();

            FatVertex v(pos);
            high = low;
            do{
//                v.rays.push_back( Ray(*high, !tiles_data[*high].open) );
                v.rays.push_back( Ray(*high, tiles_data[*high].state!=0) );
                high++;
            }while( high != tiles.end() && tiles_data[ *high ].cur_box()==pos );

//            std::printf("sortRays\n");
            sortRays(v);

            //Look for an opened tile
            unsigned i=0;

            for( i=0; i<v.rays.size(); ++i){assert( v.rays[i].tile<tiles_data.size() );}
//            for( i=0; i<v.rays.size() && !tiles_data[ v.rays[i].tile ].open; ++i){}
            for( i=0; i<v.rays.size() && tiles_data[ v.rays[i].tile ].state!=0; ++i){}

            //if there are only openings:
            if (i == v.rays.size() ){
//                std::printf("only openings!\n");
                event.insert_at = contextRanking(pos.min());
            }
            //if there is at least one closing: make it first, and catch 'insert_at'.
            else{
//                std::printf("not only openings\n");
                if( i > 0 ){
                    std::vector<Ray> head;
                    head.assign  ( v.rays.begin(), v.rays.begin()+i );
                    v.rays.erase ( v.rays.begin(), v.rays.begin()+i );
                    v.rays.insert( v.rays.end(), head.begin(), head.end());
                }
                //assert( tiles_data[ v.rays[0].tile ].open );
                assert( tiles_data[ v.rays[0].tile ].state==0 );
                event.insert_at = context.find( v.rays.front().tile )+1;
//                event.erase_at = context.find( v.rays.front().tile );
//                std::printf("at least one closing!\n");
            }
            context.pending_vertex = v;
        }else{
//            std::printf("continue biting exiting vertex\n");
            event.tile = context.pending_vertex.rays.front().tile;
            event.insert_at = old_event.insert_at;
//             if (old_event.opening){
//                 event.insert_at++;
//             }else{
//                 if (old_event.erase_at < old_event.insert_at){
//                     event.insert_at--;
//                 }
//             }
            event.insert_at++;
        }
        event.tile = context.pending_vertex.rays.front().tile;
//        event.opening = !tiles_data[ event.tile ].open;
        event.opening = tiles_data[ event.tile ].state!=0;
        event.to_be_continued = context.pending_vertex.rays.size()>1;
//        if ( !event.opening ) event.erase_at = context.find(event.tile);

        context.pending_vertex.rays.erase(context.pending_vertex.rays.begin());
        context.pending_event = event;
//        printEvent(event);
        return event;
    }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
