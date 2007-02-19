#include "sb-pw.h"

namespace Geom {

/**
 * returns true if the pw_sb meets some basic invariants.
 */
bool pw_sb::cheap_invariants() const {
    // segs between cuts
    if(!(segs.size() + 1 == cuts.size() || size() == 0 || cuts.size() == 0))
        return false;
    // cuts in order
    for(int i = 0; i < segs.size(); i++)
        if(cuts[i] >= cuts[i+1])
            return false;
    return true;
}

bool pw_sb::invariants() const {
    for(int i = 0; i<size(); i++) {
        
    }
}

/**SBasis elem_portion(const pw_sb &a, int i, double from, double to);
 * returns a portion of a piece of a pw_sb, given the piece's index and a to/from time.
 */
SBasis elem_portion(const pw_sb &a, int i, double from, double to) {
    double rwidth = 1 / (a.cuts[i+1] - a.cuts[i]);
    return portion( a[i], (from - a.cuts[i]) * rwidth, (to - a.cuts[i]) * rwidth );
}

/**pw_sb partition(const pw_sb &t, vector<double> const &c);
 * Further subdivides the pw_sb such that there is a cut at every value in c.
 * Precondition: c sorted lower to higher.
 * 
 * //Given pw_sb a and b:
 * pw_sb ac = a.partition(b.cuts);
 * pw_sb bc = b.partition(a.cuts);
 * //ac.cuts should be equivalent to bc.cuts
 */
pw_sb partition(const pw_sb &t, vector<double> const &c) {
    pw_sb ret = pw_sb();
    //just a bit of optimizing reservation
    ret.cuts.reserve(c.size() + t.cuts.size());
    ret.segs.reserve(c.size() + t.cuts.size() - 1);

    int si = 0, ci = 0;     //Segment index, Cut index

    //if the input cuts have something earlier than this pw_sb, add sections of zero
    while(c[ci] < t.cuts[0] && ci < c.size()) {
        ret.cuts.push_back(c[ci]);
        ret.segs.push_back(SBasis());
        ci++;
    }
    ret.cuts.push_back(t.cuts[0]);
    double prev = t.cuts[0];      //Previous cut made
    //Should have the cuts = segs + 1 invariant before/after every pass
    while(si < t.size() && ci <= c.size()) {
        if(ci == c.size() || c[ci] >= t.cuts[si + 1]) {  //no more cuts within this segment
            if(prev > t.cuts[si]) {      //need to push final portion of segment
                ret.segs.push_back(portion(t[si], (prev - t.cuts[si]) / (t.cuts[si+1] - t.cuts[si]), 1.0));
            } else {                     //plain copy of last segment is fine
                ret.segs.push_back(t[si]);
            }
            ret.cuts.push_back(t.cuts[si + 1]);
            prev = t.cuts[si + 1];
            si++;
        } else if(c[ci] == t.cuts[si]){                  //coincident
            //Already finalized the seg with the code immediately above
            ci++;
        } else {                                         //plain old subdivision
	    ret.segs.push_back(elem_portion(t, si, prev, c[ci]));
            ret.cuts.push_back(c[ci]);
            prev = c[ci];
            ci++;
        }
    }
    while(ci < c.size()) { //input cuts extend further than this pw_sb, add sections of zero
        if(c[ci] != prev) {
            ret.segs.push_back(SBasis());
            ret.cuts.push_back(c[ci]);
        }
        ci++;
    }
    return ret;
}

/**pw_sb portion(const pw_sb &a, double from, double to);
 * Returns a pw_sb with a defined domain of [from, to], cutting the end segments appropriately.
 * If to - from is negative, then the order and pieces are reversed.
 */
pw_sb portion(const pw_sb &a, double from, double to) {
    int fi = 0, ti = 0; //from/to indexes
    for(int i = 0; i < a.size(); i++) {
	if(a.cuts[i] < from && from < a.cuts[i+1]) fi = i; 
	if(a.cuts[i] < to && to < a.cuts[i+1]) ti = i;
    }
    if (fi > a.cuts[a.size()]) fi = a.segs.size() - 1;
    if (ti > a.cuts[a.size()]) ti = a.segs.size() - 1;

    pw_sb ret = pw_sb();
    
    if (fi < ti) {
	ret.segs.reserve(ti - fi + 1);
	ret.cuts.reserve(ti - fi + 2);
	//TODO: consider coincidince
	ret.cuts.push_back(from);
	ret.segs.push_back( portion(a[fi], (from - a.cuts[fi]) / (a.cuts[fi+1] - a.cuts[fi]), 1.0) );
        for(int i = fi + 1; i < ti; i++) {
	    ret.cuts.push_back(a.cuts[i]);
	    ret.segs.push_back(a.segs[i]);
        }
	ret.cuts.push_back(a.cuts[ti]);
	ret.segs.push_back( portion(a[ti], 0.0, (to - a.cuts[ti]) / (a.cuts[ti+1] - a.cuts[ti])) );
	ret.cuts.push_back(a.cuts[ti + 1]);
    } else if (fi > ti) {
	ret.segs.reserve(fi - ti + 1);
	ret.cuts.reserve(fi - ti + 2);
	//TODO: use above as model.  Use SBasis reverse(SBasis) function
        for(int i = fi - 1; i < ti; i--) {
	    
        }
    } else {
	ret.segs.reserve(1);
	ret.cuts.reserve(2);
        ret.cuts.push_back(from);
	ret.segs.push_back(elem_portion(a, fi, from, to));
	ret.cuts.push_back(to);
    }
}

//pw_sb operator+(BezOrd b, SBasis a)

pw_sb operator-(pw_sb const &a) {
    pw_sb ret = pw_sb();
    for(int i = 0; i < a.size();i++) {
        ret.segs.push_back( - a[i] );
        ret.cuts.push_back( a.cuts[i] );
    }
    return ret;
}

pw_sb operator-(BezOrd const &b, const pw_sb&a) {
    pw_sb ret = pw_sb();
    for(int i = 0; i < a.segs.size();i++) {
        ret.segs.push_back( b - a[i] );
        ret.cuts.push_back( a.cuts[i] );
    }
    return ret;
}

pw_sb operator+=(pw_sb& a, const BezOrd& b) {
    for(int i = 0; i < a.segs.size();i++) {
        a[i] += b;
    }
    return a;
}
pw_sb operator+=(pw_sb& a, double b) {
    for(int i = 0; i < a.segs.size();i++) {
        a[i] += b;
    }
    return a;
}
pw_sb operator-=(pw_sb& a, const BezOrd& b) {
    for(int i = 0; i < a.segs.size();i++) {
        a[i] += b;
    }
    return a;
}
pw_sb operator-=(pw_sb& a, double b) {
    for(int i = 0;i < a.segs.size();i++) {
        a[i] -= b;
    }
    return a;
}

// Semantically-correct zipping of pw_sbs, with an arbitrary operation
template <typename F>
inline pw_sb ZipSBWith(F f, pw_sb const &a, pw_sb const &b) {
    pw_sb pa = partition(a, b.cuts), pb = partition(b, a.cuts);
    pw_sb ret = pw_sb();
    assert(pa.size() == pb.size());
    for (int i = 0; i < pa.size(); i++) {
        ret.segs.push_back(f.op(pa[i], pb[i]));
        ret.cuts.push_back(pa.cuts[i]);
    }
    ret.cuts.push_back(pa.cuts[pa.size()]);
    return ret;
}

//Dummy structs
struct sbasis_add{SBasis op(SBasis const &a, SBasis const &b) {return a+b;} };
struct sbasis_sub{SBasis op(SBasis const &a, SBasis const &b) {return a-b;} };
struct sbasis_mul{SBasis op(SBasis const &a, SBasis const &b) {return a*b;} };
struct sbasis_div{
    int k;
    sbasis_div(int n) { k = n; }
    SBasis op(SBasis const &a, SBasis const &b) {return divide(a, b, k);}
};

pw_sb operator+(pw_sb const &a, pw_sb const &b) { return ZipSBWith(sbasis_add(), a, b); }

pw_sb operator-(pw_sb const &a, pw_sb const &b) { return ZipSBWith(sbasis_sub(), a, b); }

pw_sb multiply(pw_sb const &a, pw_sb const &b) { return ZipSBWith(sbasis_mul(), a, b); }
pw_sb divide(pw_sb const &a, pw_sb const &b, int k) { return ZipSBWith(sbasis_div(k), a, b);}

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
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
