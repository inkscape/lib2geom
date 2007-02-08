#include "sb-pw.h"

namespace Geom {
pw_sb partition(const pw_sb &t, vector<double> const &c) {
    pw_sb ret = pw_sb();
    //just a bit of optimizing reservation
    ret.cuts.reserve(c.size() + t.cuts.size());
    ret.segs.reserve(c.size() + t.cuts.size() - 1);

    int si = 0, ci = 0;     //Segment index, Cut index

    //if the input cuts have something earlier than this pw_sb, add sections of zero
    while(ci < c.size() && c[ci] < t.cuts[0]) {
        ret.cuts.push_back(c[ci]);
        ret.segs.push_back(SBasis());
        ci++;
    }
    ret.cuts.push_back(t.cuts[0]);
    double prev = t.cuts[0];      //Previous cut made
    while(si < t.segs.size() && ci < c.size()) {
        if(c[ci] >= t.cuts[si + 1]) {  //no more cuts within this segment
            if(prev > t.cuts[si]) {
                ret.segs.push_back(portion(t.segs[si], (prev - t.cuts[si]) / (t.cuts[si+1] - t.cuts[si]), 1));
            } else {
                ret.segs.push_back(t.segs[si]);
            }
            si++;
            ret.cuts.push_back(t.cuts[si]);
            prev = t.cuts[si];
        } else if(c[ci] == t.cuts[si]) { //coincident
            //Already finalized the seg with the code immediately above
            ci++;
        } else {
            double rwidth = 1 / (t.cuts[si+1] - t.cuts[si]);
            ret.cuts.push_back(c[ci]);
            prev = c[ci];
            ret.segs.push_back(portion(t.segs[si], (prev - t.cuts[si]) * rwidth, (c[ci] - t.cuts[si]) * rwidth ));
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

/* This macro provides a mapping of operations on pw_sb pieces
 * It assumes that it is the contents of a function, and there is a pw_sb parameter, a.
 */
#define MapSB(op)                               \
    pw_sb ret = pw_sb();                        \
    for(int i = 0; i < ret.segs.size();i++) {   \
        ret.segs.push_back( op );               \
        ret.cuts.push_back( a.cuts[i] );        \
    }                                           \
    return ret;

//pw_sb operator+(BezOrd b, SBasis a)

pw_sb operator-(pw_sb const &a) { MapSB(- a.segs[i]) }
pw_sb operator-(BezOrd const &b, const pw_sb&a) { MapSB(b- a.segs[i]) }

pw_sb operator+=(pw_sb& a, const BezOrd& b) {
    for(int i = 0; i < a.segs.size();i++) {
        a.segs[i] += b;
    }
    return a;
}
pw_sb operator+=(pw_sb& a, double b) {
    for(int i = 0; i < a.segs.size();i++) {
        a.segs[i] += b;
    }
    return a;
}
pw_sb operator-=(pw_sb& a, const BezOrd& b) {
    for(int i = 0; i < a.segs.size();i++) {
        a.segs[i] += b;
    }
    return a;
}
pw_sb operator-=(pw_sb& a, double b) {
    for(int i = 0;i < a.segs.size();i++) {
        a.segs[i] -= b;
    }
    return a;
}

// Semantically-correct zipping of pw_sbs, with an arbitrary operation
template <typename F>
inline pw_sb ZipSBWith(pw_sb const &a, pw_sb const &b) {
  pw_sb pa = partition(a, b.cuts), pb = partition(b, a.cuts);
  pw_sb ret = pw_sb();
  for ( int i = 0 ; i < pa.segs.size() && i < pb.segs.size() ; i++ ) {
    ret.segs.push_back(F::op(pa.segs[i], pb.segs[i]));
    ret.cuts.push_back(pa.cuts[i]);
  }
  return ret;
}

//Dummy structs
struct sbasis_add{static SBasis op(SBasis const &a, SBasis const &b) {return a + b;} };
struct sbasis_sub{static SBasis op(SBasis const &a, SBasis const &b) {return a - b;} };
struct sbasis_mul{static SBasis op(SBasis const &a, SBasis const &b) {return a * b;} };

pw_sb operator+(pw_sb const &a, pw_sb const &b) { ZipSBWith<sbasis_add>(a, b); }
pw_sb operator-(pw_sb const &a, pw_sb const &b) { ZipSBWith<sbasis_sub>(a, b); }

pw_sb multiply(pw_sb const &a, pw_sb const &b) { ZipSBWith<sbasis_mul> (a, b); }

/*TODO: should k really be necessary?
template <int K>
struct sbasis_div{static SBasis op(SBasis const &a, SBasis const &b) {return divide(a, b, K);} };
pw_sb divide(pw_sb const &a, pw_sb const &b, int k) { ZipSBWith<sbasis_div<k> >(a, b);}
*/

pw_sb compose(pw_sb const &a, pw_sb const &b) {

}

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
