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
        pw_sb ret = pw_sb();
        //just a bit of optimizing reservation
        ret.cuts.reserve(c.size() + cuts.size());
        ret.segs.reserve(c.size() + cuts.size() - 1);
 
       //Segment index, Cut index
        int si = 0, ci = 0;

        //if the input cuts have something earlier than this pw_sb, add sections of zero
        while(ci < c.size() && c[ci] < cuts[0]) {
            ret.cuts.push_back(c[ci]);
            prev = c[ci];
            ret.segs.push_back(SBasis());
            ci++;
        }
        ret.cuts.push_back(cuts[0]);
        double prev = cuts[0];
        while(si < segs.size() && ci < c.size()) {
            if(c[ci] >= cuts[si + 1]) {  //no more cuts within this segment
                if(prev > cuts[si]) {
                    ret.segs.push_back(segs[si].portion( (prev - cuts[si]) / (cuts[si+1] - cuts[si]), 1);
                } else {
                    ret.segs.push_back(segs[si]);
                }
                si++;
                ret.cuts.push_back(cuts[si]);
                prev = cuts[si];
            } else if(c[ci] == cuts[si]) { //coincident
                //Already finalized the seg with the code immediately above
                ci++;
            } else {
                double rwidth = 1 / (cuts[si+1] - cuts[si]);
                ret.cuts.push_back(c[ci]);
                prev = c[ci];
                ret.segs.push_back(segs[si].portion( (prev - cuts[si]) * rwidth, (c[ci] - cuts[si]) * rwidth ));
                ci++;
            }
        }
        //if the input cuts extend further than this pw_sb, add sections of zero
        while(ci < c.size()) {
            ret.cuts.push_back(c[ci]);
            ret.segs.push_back(SBasis());
            ci++;
        }
        return ret;
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

    /*splitSeg(int i, double t) {
        SBasis a, b;
        segs[i].split((t-cuts[i]) / (cuts[i+1]-cuts[i]), a, b);
        vector<SBasis>::iterator it = segs.begin() + i
        segs.remove(it);
        segs.insert(it, a);
        segs.insert(it+1, b);
        cuts.insert(cuts.begin() + i + 1, c[ci]);
    }*/

}

#endif //SEEN_GEOM_SB_PW_H
