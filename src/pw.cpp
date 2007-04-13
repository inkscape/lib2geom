/*
 * pw.cpp - Piecewise function class
 *
 * Copyright 2007 Michael Sloan <mgsloan@gmail.com>
 * Copyright 2007 J. F. Barraud
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, output to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 */

#include "pw.h"
#include <iterator>
#include <map>

namespace Geom {

Piecewise<SBasis> divide(Piecewise<SBasis> const &a, Piecewise<SBasis> const &b, unsigned k) {
    Piecewise<SBasis> pa = partition(a, b.cuts), pb = partition(b, a.cuts);
    Piecewise<SBasis> ret = Piecewise<SBasis>();
    assert(pa.size() == pb.size());
    ret.cuts = pa.cuts;
    for (int i = 0; i < pa.size(); i++)
        ret.push_seg(divide(pa[i], pb[i], k));
    return ret;
}

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

Piecewise<SBasis> compose(Piecewise<SBasis> const &f, SBasis  const &g){
  Piecewise<SBasis> result;

  if (f.size()==0) return result;
  if (f.size()==1){
      double t0 = f.cuts[0], width = f.cuts[1] - t0;
      return (Piecewise<SBasis>) f.segs[0](compose(Linear(-t0 / width, (1-t0) / width), g));
  }

  //first check bounds...
  Interval bs = g.boundsFast();
  if (bs.max() < f.cuts.front() || bs.min() > f.cuts.back()){
      //TODO: use segN
      int idx = (bs.max() < f.cuts[1]) ? 0 : f.cuts.size()-2;
      double t0 = f.cuts[idx], width = f.cuts[idx+1] - t0;
      return (Piecewise<SBasis>) f.segs[idx](compose(Linear(-t0 / width, (1-t0) / width), g));  //TODO rewrite with portion/subdivide/whatever
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

Piecewise<SBasis> compose(Piecewise<SBasis> const &f, Piecewise<SBasis> const &g){
  Piecewise<SBasis> result;
  for(int i = 0; i < g.segs.size(); i++){
    Piecewise<SBasis> fgi=compose(f, g.segs[i]);
    double t0 = g.cuts[i], t1 = g.cuts[i+1];
    for(int j = 0; j < fgi.cuts.size(); j++){
      fgi.cuts[j]= t0 + fgi.cuts[j] * (t1-t0);
    }
    //append
    result.cuts.insert(result.cuts.end(), fgi.cuts.begin() + 1, fgi.cuts.end());
    result.segs.insert(result.segs.end(), fgi.segs.begin(), fgi.segs.end());
  }
}

Piecewise<SBasis> integral(Piecewise<SBasis> const &a) {
    Piecewise<SBasis> result;
    result.segs.resize(a.segs.size());
    result.cuts.insert(result.cuts.end(), a.cuts.begin(), a.cuts.end());
    for(int i = 0; i < a.segs.size(); i++){
        result.segs[i] = integral(a.segs[i]);
        // Need some kind off offset to share the constant over all segs
    }
    return result;
}

Piecewise<SBasis> derivative(Piecewise<SBasis> const &a) {
    Piecewise<SBasis> result;
    result.segs.resize(a.segs.size());
    result.cuts.insert(result.cuts.end(), a.cuts.begin(), a.cuts.end());
    for(int i = 0; i < a.segs.size(); i++){
        result.segs[i] = derivative(a.segs[i]);
    }
    return result;
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
