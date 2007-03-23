#include "pw-sb.h"
#include <iterator>
#include <map>

namespace Geom {

/**
 * returns true if the pw_sb meets some basic invariants.
 */
bool pw_sb::invariants() const {
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
    assert(i < a.size());
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
    if(c.empty()) return pw_sb(pw);

    pw_sb ret = pw_sb();
    //just a bit of optimizing reservation
    ret.cuts.reserve(c.size() + pw.cuts.size());
    ret.segs.reserve(c.size() + pw.cuts.size() - 1);

    //0-length pw_sb is like a 0-length sbasis - equal to 0!
    if(pw.empty()) {
        ret.cuts = c;
        for(int i = 0; i < c.size() - 1; i++)
            ret.push_seg(SBasis());
        return ret;
    }

    int si = 0, ci = 0;     //Segment index, Cut index

    //if the cuts have something earlier than the pw_sb, add portions of the first segment
    while(c[ci] < pw.cuts.front() && ci < c.size()) {
        bool isLast = (ci == c.size()-1 || c[ci + 1] >= pw.cuts.front());
        ret.push_cut(c[ci]);
        ret.push_seg( elem_portion(pw, 0, c[ci], isLast ? pw.cuts.front() : c[ci + 1]) );
        ci++;
    }

    ret.push_cut(pw.cuts[0]);
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
                ret.push_seg(portion(pw[si], pw.segt(prev, si), 1.0));
            } else {                     //plain copy is fine
                ret.push_seg(pw[si]);
            }
            ret.push_cut(pw.cuts[si + 1]);
            prev = pw.cuts[si + 1];
            si++;
        } else if(c[ci] == pw.cuts[si]){                  //coincident
            //Already finalized the seg with the code immediately above
            ci++;
        } else {                                         //plain old subdivision
            ret.push(elem_portion(pw, si, prev, c[ci]), c[ci]);
            prev = c[ci];
            ci++;
        }
    }

    while(ci < c.size()) { //input cuts extend further than this pw_sb, add sections of zero
        if(c[ci] > prev) {
            ret.push(elem_portion(pw, pw.size() - 1, prev, c[ci]), c[ci]);
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
    if(pw.empty()) return pw_sb();

    pw_sb ret;

    double temp = from;
    from = min(from, to);
    to = max(temp, to);
    
    int i = pw.segn(from);
    ret.push_cut(from);
    if(to < pw.cuts[i + 1]) {    //to/from inhabit the same segment
        ret.push(elem_portion(pw, i, from, to), to);
        return ret;
    }
    ret.push(portion( pw[i], pw.segt(from, i), 1.0 ), pw.cuts[i + 1]);
    i++;
    int fi = pw.segn(to, i);

    ret.segs.insert(ret.segs.end(), pw.segs.begin() + i, pw.segs.begin() + fi - 1);  //copy segs
    ret.cuts.insert(ret.cuts.end(), pw.cuts.begin() + i + 1, pw.cuts.begin() + fi);  //and their ends

    ret.push( portion(pw[fi], 0.0, pw.segt(to, fi)), to);

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

pw_sb operator+(pw_sb const &a, double b) {
    pw_sb ret = pw_sb();
    ret.cuts = a.cuts;
    for(int i = 0; i < a.size();i++)
        ret.push_seg(b + a[i]);
    return ret;
}

pw_sb operator-(pw_sb const &a) {
    pw_sb ret = pw_sb();
    ret.cuts = a.cuts;
    for(int i = 0; i < a.size();i++)
        ret.push_seg(- a[i]);
    return ret;
}

pw_sb operator+=(pw_sb& a, double b) {
    if(a.empty()) { a.push_cut(0.); a.push(Linear(b), 1.); return a; }

    for(int i = 0; i < a.size();i++)
        a[i] += b;
    return a;
}
pw_sb operator-=(pw_sb& a, double b) {
    if(a.empty()) { a.push_cut(0.); a.push(Linear(b), 1.); return a; }

    for(int i = 0;i < a.size();i++)
        a[i] -= b;
    return a;
}
pw_sb operator*=(pw_sb& a, double b) {
    if(a.empty()) return pw_sb();

    for(int i = 0; i < a.size();i++)
        a[i] *= b;
    return a;
}
pw_sb operator/=(pw_sb& a, double b) {
    //FIXME: b == 0?
    if(a.empty()) return pw_sb();

    for(int i = 0; i < a.size();i++)
        a[i] /= b;
    return a;
}


// Semantically-correct zipping of pw_sbs, with an arbitrary operation
template <typename F>
inline pw_sb ZipSBWith(F f, pw_sb const &a, pw_sb const &b) {
    pw_sb pa = partition(a, b.cuts), pb = partition(b, a.cuts);
    pw_sb ret = pw_sb();
    assert(pa.size() == pb.size());
    ret.cuts = pa.cuts;
    for (int i = 0; i < pa.size(); i++)
        ret.push_seg(f.op(pa[i], pb[i]));
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

//--Compose---------------
/* Quiet ugly atm.
   A vector<pairs<double,unsigned> > would be better than a map<double,unsigned>.
   But all this gives me the 'missing ;' blues! --I am not brave enough today-- jfb.
 */
static std::map<double,unsigned> vect_of_vect_to_map(std::vector<std::vector<double> >roots ){
    std::map<double,unsigned> result;
    for(unsigned i=0; i<roots.size();i++){
        for(unsigned j=0; j<roots[i].size();j++){
            result[roots[i][j]]=i;
        }
    }
    return(result);
}

pw_sb compose(pw_sb const &f, SBasis  const &g){
  pw_sb result;

  if (f.size()==0) return result;
  if (f.size()==1){
      double t0 = f.cuts[0], width = f.cuts[1] - t0;
      return (pw_sb) f.segs[0](compose(Linear(-t0 / width, (1-t0) / width), g));
  }

  //first check bounds...
  double M, m;
  bounds(g, m, M);
  if (M < f.cuts.front() || m > f.cuts.back()){
      int idx = (M < f.cuts[1]) ? 0 : f.cuts.size()-2;
      double t0 = f.cuts[idx], width = f.cuts[idx+1] - t0;
      return (pw_sb) f.segs[idx](compose(Linear(-t0 / width, (1-t0) / width), g));
  }

  //-- collect all t / g(t)=f.cuts[idx] for some idx (!= first and last).
  // put them in a map:   t->idx.
  std::vector<double> levels;//we can forget first and last cuts of f...
  levels.insert(levels.begin(),f.cuts.begin()+1,f.cuts.end()-1);
  std::map<double,unsigned> cuts_pb; //pb stands for pullback
  cuts_pb = vect_of_vect_to_map(multi_roots(g,levels));

  // Also map 0 and 1 to the first level above(or =) f(0) and f(1).
  if(cuts_pb.count(0.)==0){
      int i=0;
      while (i<levels.size()&&(g[0][0]>levels[i])) i++;
      cuts_pb[0.]=i;
  }
  if(cuts_pb.count(1.)==0){
      int i=0;
      while (i<levels.size()&&(g[0][1]>levels[i])) i++;
      cuts_pb[1.]=i;
  }
 
  //-- Compose each piece of g with the relevant seg of f.
  result.cuts.push_back(0.);
  std::map<double,unsigned>::iterator cut=cuts_pb.begin();
  std::map<double,unsigned>::iterator next=cut; next++;
  while(next!=cuts_pb.end()){
    double t0=(*cut).first;
    int  idx0=(*cut).second;
    double t1=(*next).first;
    int  idx1=(*next).second;
    int  idx; //idx of the relevant f.segs
    if (std::max(idx0,idx1)==levels.size()){ //g([t0,t1]) is above the top level,
      idx=levels.size()-1;
    } else if (idx0 != idx1){                //g([t0,t1]) crosses from level idx0 to idx1,
      idx=std::min(idx0,idx1);
    } else if(g((t0+t1)/2) < levels[idx0]) { //g([t0,t1]) is a 'U' under level idx0,
      idx=idx0-1;
    } else if(g((t0+t1)/2) > levels[idx0]) { //g([t0,t1]) is a 'bump' over level idx0,
      idx=idx0;
    } else {                                 //g([t0,t1]) is contained in level idx0!...
      idx = (idx0==levels.size())? idx0-1:idx0;
    }

    //move idx back from levels f.cuts 
    idx+=1;

    SBasis sub_g=compose(g, Linear(t0,t1));
    sub_g=compose(Linear(-f.cuts[idx]/(f.cuts[idx+1]-f.cuts[idx]),
			     (1-f.cuts[idx])/(f.cuts[idx+1]-f.cuts[idx])),sub_g);
    sub_g=compose(f[idx],sub_g);
    result.cuts.push_back(t1);
    result.segs.push_back(sub_g);
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
    //append
    result.cuts.insert(result.cuts.end(), fgi.cuts.begin() + 1, fgi.cuts.end());
    result.segs.insert(result.segs.end(), fgi.segs.begin(), fgi.segs.end());
  }
}

pw_sb integral(pw_sb const &a) {
    pw_sb result;
    result.segs.resize(a.segs.size());
    result.cuts.insert(result.cuts.end(), a.cuts.begin(), a.cuts.end());
    for(int i = 0; i < a.segs.size(); i++){
        result.segs[i] = integral(a.segs[i]);
        // Need some kind off offset to share the constant over all segs
    }
    return result;
}

pw_sb derivative(pw_sb const &a) {
    pw_sb result;
    result.segs.resize(a.segs.size());
    result.cuts.insert(result.cuts.end(), a.cuts.begin(), a.cuts.end());
    for(int i = 0; i < a.segs.size(); i++){
        result.segs[i] = derivative(a.segs[i]);
    }
    return result;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
