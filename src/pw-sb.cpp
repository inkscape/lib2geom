#include "pw-sb.h"
#include <iterator>
#include <map>

namespace Geom {

/**
 * returns true if the pw_sb meets some basic invariants.
 */
bool pw_sb::cheap_invariants() const {
    // segs between cuts
    if(!(segs.size() + 1 == cuts.size() || (segs.empty() && cuts.empty())))
        return false;
    // cuts in order
    for(int i = 0; i < segs.size(); i++)
        if(cuts[i] >= cuts[i+1])
            return false;
    return true;
}

/**SBasis elem_portion(const pw_sb &a, int i, double from, double to);
 * returns a portion of a piece of a pw_sb, given the piece's index and a to/from time.
 */
SBasis elem_portion(const pw_sb &a, int i, double from, double to) {
    double rwidth = 1 / (a.cuts[i+1] - a.cuts[i]);
    return portion( a[i], (from - a.cuts[i]) * rwidth, (to - a.cuts[i]) * rwidth );
}

/**pw_sb partition(const pw_sb &pw, vector<double> const &c);
 * Further subdivides the pw_sb such that there is a cut at every value in c.
 * Precondition: c sorted lower to higher.
 * 
 * //Given pw_sb a and b:
 * pw_sb ac = a.partition(b.cuts);
 * pw_sb bc = b.partition(a.cuts);
 * //ac.cuts should be equivalent to bc.cuts
 */
pw_sb partition(const pw_sb &pw, vector<double> const &c) {
    if(c.size() == 0) return pw_sb(pw);

    pw_sb ret = pw_sb();

    //just a bit of optimizing reservation
    ret.cuts.reserve(c.size() + pw.cuts.size());
    ret.segs.reserve(c.size() + pw.cuts.size() - 1);

    //0-length pw_sb is like a 0-length sbasis - equal to 0!
    if(pw.size() == 0) {
        for(int i = 0; i < c.size() - 1; i++) {
            ret.push_back(c[i], SBasis());
        }
        ret.cuts.push_back(c.back());
        return ret;
    }

    int si = 0, ci = 0;     //Segment index, Cut index

    //if the cuts have something earlier than the pw_sb, add portions of the first segment
    while(c[ci] < pw.cuts.front() && ci < c.size()) {
        bool isLast = (ci == c.size()-1 || c[ci + 1] >= pw.cuts.front());
        ret.push_back(c[ci], elem_portion(pw, 0, c[ci], isLast ? pw.cuts.front() : c[ci + 1]));
        ci++;
    }

    ret.cuts.push_back(pw.cuts[0]);
    double prev = pw.cuts[0];    //previous cut
    //Loop which handles cuts within the pw_sb domain
    //Should have the cuts = segs + 1 invariant
    while(si < pw.size() && ci <= c.size()) {
        if(ci == c.size() && prev <= pw.cuts[si]) { //cuts exhausted, straight copy the rest
            ret.segs.insert(ret.segs.end(), pw.segs.begin() + si, pw.segs.end());
            ret.cuts.insert(ret.cuts.end(), pw.cuts.begin() + si + 1, pw.cuts.end());
            return ret;
        }else if(ci == c.size() || c[ci] >= pw.cuts[si + 1]) {  //no more cuts within this segment, finalize
            if(prev > pw.cuts[si]) {      //segment already has cuts, so portion is required
                ret.segs.push_back(portion(pw[si], pw.segt(prev, si), 1.0));
            } else {                     //plain copy is fine
                ret.segs.push_back(pw[si]);
            }
            ret.cuts.push_back(pw.cuts[si + 1]);
            prev = pw.cuts[si + 1];
            si++;
        } else if(c[ci] == pw.cuts[si]){                  //coincident
            //Already finalized the seg with the code immediately above
            ci++;
        } else {                                         //plain old subdivision
            ret.push_back(elem_portion(pw, si, prev, c[ci]), c[ci]);
            prev = c[ci];
            ci++;
        }
    }

    while(ci < c.size()) { //input cuts extend further than this pw_sb, add sections of zero
        if(c[ci] != prev) {
            ret.push_back(elem_portion(pw, pw.size() - 1, prev, c[ci]), c[ci]);
            prev = c[ci];
        }
        ci++;
    }
    return ret;
}

/**pw_sb portion(const pw_sb &pw, double from, double to);
 * Returns a pw_sb with a defined domain of [min(from, to), max(from, to)].
 */
pw_sb portion(const pw_sb &pw, double from, double to) {
    pw_sb ret;

    double temp = from;
    from = min(from, to);
    to = max(temp, to);
    
    int i = pw.segn(from);
    ret.cuts.push_back(from);
    if(to < pw.cuts[i + 1]) {    //to/from inhabit the same segment
        ret.push_back(from, elem_portion(pw, i, from, to), to);
        return ret;
    }
    ret.push_back(from, portion( pw[i], pw.segt(from, i), 1.0 ), pw.cuts[i + 1]);
    i++;
    int fi = pw.segn(to, i);

    ret.segs.insert(ret.segs.end(), pw.segs.begin() + i, pw.segs.begin() + fi - 1);  //copy cuts
    ret.cuts.insert(ret.cuts.end(), pw.cuts.begin() + i + 1, pw.cuts.begin() + fi);  //and their ends
    ret.push_back( portion(pw[fi], 0.0, pw.segt(to, fi)), to);
    return ret;
}

vector<double> roots(const pw_sb &pw) {
    vector<double> ret;
    for(int i = 0; i < pw.size(); i++) {
        vector<double> sr = roots(pw[i]);
        for (int j = 0; j < sr.size(); j++) sr[j] = sr[j] * (pw.cuts[i + 1] - pw.cuts[i]) + pw.cuts[i];
        ret.insert(ret.end(), sr.begin(), sr.end());
    }
    return ret;
}

//pw_sb operator+(BezOrd b, SBasis a)

pw_sb operator-(pw_sb const &a) {
    pw_sb ret = pw_sb();
    for(int i = 0; i < a.size();i++)
        ret.push_back(a.cuts[i], - a[i]);
    ret.cuts.push_back(a.cuts.back());
    return ret;
}

pw_sb operator-(BezOrd const &b, const pw_sb&a) {
    //TODO: handle 0 length
    pw_sb ret = pw_sb();
    for(int i = 0; i < a.size();i++)
        ret.push_back( a.cuts[i], b - a[i] );
    ret.cuts.push_back(a.cuts.back());
    return ret;
}

pw_sb multiply(BezOrd const &b, const pw_sb&a) {
    //TODO: handle 0 length
    pw_sb ret = pw_sb();
    for(int i = 0; i < a.size();i++)
        ret.push_back( a.cuts[i], b * a[i] );
    ret.cuts.push_back( a.cuts.back() );
    return ret;
}

pw_sb operator+=(pw_sb& a, const BezOrd& b) {
    //TODO: handle 0 length
    for(int i = 0; i < a.size();i++) {
        a[i] += b;
    }
    return a;
}
pw_sb operator+=(pw_sb& a, double b) {
    if(a.size() == 0) a.push_back(0., BezOrd(b), 1.);

    for(int i = 0; i < a.size();i++) {
        a[i] += b;
    }
    return a;
}
pw_sb operator-=(pw_sb& a, const BezOrd& b) {
    //TODO: handle 0 length
    for(int i = 0; i < a.size();i++) {
        a[i] += b;
    }
    return a;
}
pw_sb operator-=(pw_sb& a, double b) {
    if(a.size() == 0) a.push_back(0., BezOrd(-b), 1.);

    for(int i = 0;i < a.size();i++) {
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


pw_sb compose(pw_sb const &f, SBasis  const &g){
  pw_sb result;
  
  //first check bounds...
  double M, m;
  bounds(g, m, M);
  if (M < f.cuts.front() || m > f.cuts.back()){
    int idx = (M < f.cuts.front()) ? 0 : f.cuts.size() - 2;
    double t0 = f.cuts[idx], width = f.cuts[idx+1] - t0;
    return (pw_sb) compose(BezOrd(-t0 / width, (1-t0) / width), g);
  }

  //-- collect all t / g(t)=f.cuts[idx] for some idx.
  // put them in a map:   t->idx.
  // Notice that t=0,1 recieve a special treatment:
  // 0->idx iff f.cuts[idx-1]<g(0)<=f.cuts[idx] (if out of range, f.cut[idx]=+/- infty)
  // the same for 1.
  std::map<double,int> cuts_pb; //pb stands for pullback
  vector<double> sols;
  for(int i=0; i<f.cuts.size();i++){
    sols=roots(g-BezOrd(f.cuts[i]));
    for (vector<double>::iterator root=sols.begin();root!=sols.end();root++)
      cuts_pb[*root]=i;
    if((cuts_pb.count(0.)==0) and (g[0][0]<=f.cuts[i]))
      cuts_pb[0.]=i;
    if((cuts_pb.count(1.)==0) and (g[0][1]<=f.cuts[i]))
      cuts_pb[1.]=i;
  }
  if(cuts_pb.count(0.)==0) cuts_pb[0.]=f.cuts.size();
  if(cuts_pb.count(1.)==0) cuts_pb[1.]=f.cuts.size();
  
  //-- Compose each piece with the relevant seg.
  result.cuts.push_back(0.);
  std::map<double,int>::iterator cut=cuts_pb.begin();
  std::map<double,int>::iterator next=cut; next++;
  while(next!=cuts_pb.end()){
    double t0=(*cut).first;
    int  idx0=(*cut).second;
    double t1=(*next).first;
    int  idx1=(*next).second;
    int  idx; //idx of the relevant f.segs
    if (std::max(idx0,idx1)==f.cuts.size()){ //g([t0,t1]) is above the top level,
      idx=f.cuts.size()-2;
    }else if (std::min(idx0,idx1)==-1){      //g([t0,t1]) is below the min level,
      idx=0;
    } else if (idx0 != idx1){                //g([t0,t1]) crosses from level idx0 to idx1,
      idx=std::min(idx0,idx1);
    } else if(g((t0+t1)/2) < f.cuts[idx0]) { //g([t0,t1]) is a 'U' under level idx0,
      idx=idx0-1;
    } else if(g((t0+t1)/2) > f.cuts[idx0]) { //g([t0,t1]) is a 'bump' over level idx0,
      idx=idx0;
    } else {                                 //g([t0,t1]) is contained in level idx0!...
      idx = (idx0==f.cuts.size())? idx0-1:idx0;
    }

    if (idx==-1) idx=0;
    if (idx>=f.cuts.size()-1) idx=f.cuts.size()-2;

    if (idx>=0 and idx<f.cuts.size()) {
      SBasis sub_g=compose(g, BezOrd(t0,t1));
      sub_g=compose(BezOrd(-f.cuts[idx]/(f.cuts[idx+1]-f.cuts[idx]),
			     (1-f.cuts[idx])/(f.cuts[idx+1]-f.cuts[idx])),sub_g);
      sub_g=compose(f[idx],sub_g);
      result.cuts.push_back(t1);
      result.segs.push_back(sub_g);
    }
    cut++;
    next++;
  }
  return(result);
} 

pw_sb compose(pw_sb const &f, pw_sb const &g){
  pw_sb result;
  for(int i = 0; i < g.segs.size(); i++){
    pw_sb fgi=compose(f, g.segs[i]);
    double t0 = g.cuts[i], t1 = g.cuts[i+1];
    for(int j = 0; j < fgi.cuts.size(); j++){
      fgi.cuts[j]= t0 + fgi.cuts[j] * (t1-t0);
    }
    result.append(fgi);
  }
}

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
