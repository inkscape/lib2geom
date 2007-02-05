#ifndef SEEN_GEOM_SB_PW_H
#define SEEN_GEOM_SB_PW_H

#include "s-basis.h"
#include <vector>

using namespace std;

namespace Geom {

class pw_sb {
    vector<double> cuts;
    vector<SBasis> segs; 
    //segs[i] stretches from cuts[i] to cuts[i+1]. 

    pw_sb partition(vector<double> const &c) {
        ret = pw_sb(this);
        //Segment index, Cut index
        int si = 0, ci = 0;
        //if the input cuts have something earlier than this pw_sb, add sections of zero
        while(c[ci] < cuts[si] && ci < c.size()) {
            ret.segs.insert(segs.begin()+si, BezOrd(0, 0));
            ret.cuts.insert(cuts.begin()+si, c[ci]);
            si++; ci++;
        }
        while(si < segs.size() && ci < c.size()) {
            if(c[ci] > cuts[si]) {  //no more cuts within this segment; move-along
                si++;
            } else {
                SBasis a, b;
                ret.segs[si].split( (c[ci] - ret.cuts[si]) / (ret.cuts[si+1] - ret.cuts[si]), a, b);
                ret.segs.remove(ret.segs.begin() + si);
                ret.segs.insert(ret.segs.begin() + si, a);
                si++;
                ret.segs.insert(ret.segs.begin() + si, b);
                ret.cuts.insert(ret.cuts.begin() + si, c[ci]);
                ci++;
            }
        }
        //if the input cuts extend further than this pw_sb, add sections of zero
        while(ci < c.size()) {
            ret.segs.push_back(BezOrd(0, 0));
            ret.cuts.push_back(c[ci]);
            ci++;
        }
    }
};

pw_sb operator+(pw_sb const &a, pw_sb const &b) {

}

pw_sb compose(pw_sb const &a, pw_sb const &b) {

}

/* Mid-coding I realized this would need to output the mythical md_pw_sb

    explicit pw_sb(Path2::Path p) {
        for ( Sequence::iterator iter=p.first; iter!=p.last; ++iter ) {
            segs.push_back((*iter).sbasis());
        }
    }

*/

/*
class pw_sb {
    vector<pw_piece> pieces;

    const_iterator first() {pieces.begin();}
    const_iterator last()  {pieces.end();}

    pw_sb() : pieces() {}
};

class pw_piece {
    double start, end;
    SBasis seg;

    pw_piece(SBasis s, double st)            : seg(s), start(st) { end = t + 1; }
    pw_piece(SBasis s, double st, double en) : seg(s), start(st), end(en) {}
}
*/

}

#endif //SEEN_GEOM_SB_PW_H
