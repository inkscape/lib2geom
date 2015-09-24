#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-2d.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/bezier-to-sbasis.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

#include <vector>
using std::vector;
using namespace Geom;

class NormalBundle : public std::vector<D2<SBasis2d> >{
public:
    vector<double> lengths;
    NormalBundle(){lengths.push_back(0.);}
    void setBase(D2<SBasis> const &B, double tol);
    void draw(cairo_t* cr, unsigned NbSections =5,unsigned NbFibre=5);
};
vector<D2<SBasis> > compose(NormalBundle const &NB, 
				    D2<SBasis> const &Binit,
				    Geom::Point Origin=Geom::Point(0,0));

//--------------------------------------------

void SBasis1d_to_2d(D2<SBasis> C0,
		    D2<SBasis> C1,
		    D2<SBasis2d> &S){
    for(unsigned dim = 0; dim < 2; dim++) {
//**** C0 and C1 should have the same size...
        for (unsigned i=C0[dim].size();i<C1[dim].size(); i++)
            C0[dim].push_back(Linear(0));
        for (unsigned i=C1[dim].size();i<C0[dim].size(); i++)
            C1[dim].push_back(Linear(0));
        S[dim].clear();
        S[dim].us = C0[dim].size();
        S[dim].vs = 1;
        for(unsigned v = 0; v < S[dim].vs; v++)
            for(unsigned u = 0; u < S[dim].us; u++)
                S[dim].push_back(Linear2d(C0[dim][u][0],C0[dim][u][1],
                                          C1[dim][u][0],C1[dim][u][1]));
    }
}

void NormalBundle::setBase(D2<SBasis> const &B, double tol=0.01) {

  D2<SBasis> dB = derivative(B);
  vector<double> cuts;
  Piecewise<D2<SBasis> > unitV=unitVector(dB);

  //TODO: clean this up, use arc_length_parametrization...
  cuts=unitV.cuts;
  double t0=0,t1,L=0;
  for(unsigned i=1;i<cuts.size();i++){
    t1=cuts[i];
    D2<SBasis> subB=compose(B,Linear(t0,t1));
    D2<SBasis2d> S;
    SBasis1d_to_2d(subB, subB+rot90(unitV[i]), S);
    push_back(S);
    
    SBasis s=integral(dot(compose(dB,Linear(t0,t1)),unitV[i]));
    L+=(s(1)-s(0))*(t1-t0);
    lengths.push_back(L);
    
    t0=t1;
  }
}

void NormalBundle::draw(cairo_t *cr, unsigned NbLi, unsigned NbCol) {
    D2<SBasis> B;
    vector<D2<SBasis> > tB;
    //Geom::Point Seg[2];
    B[1]=Linear(-100,100);
    double width=*(lengths.rbegin());
    if (NbCol>0)
        for(unsigned ui = 0; ui <= NbCol; ui++) {
            B[0]=Linear(ui*width/NbCol);
            tB = compose(*this,B);
            if (tB.size()>0) cairo_d2_sb(cr, tB[0]);
        }

    B[0]=SBasis(Linear(0,1));
    for(unsigned ui = 0; ui <= NbLi; ui++) {
        B[1]=Linear(-100+ui*200/NbLi);
        for(unsigned i = 0; i <size(); i++) {
            D2<SBasis> section=composeEach((*this)[i],B);
            cairo_d2_sb(cr, section);
        }
    }
}


vector<D2<SBasis> > compose(NormalBundle const &NB, 
				    D2<SBasis> const &Binit,
				    Geom::Point Origin){
    vector<D2<SBasis> > result;
    D2<SBasis> B=Binit;
    D2<SBasis> Bcut;
    vector<double> Roots;
    std::map<double,unsigned> Cuts;
    unsigned idx;

    B = B + (-Origin);

    //--Find intersections with fibers over segment ends.
    for(unsigned i=0; i<=NB.size();i++){
        Roots=roots(B[0]);
        for (vector<double>::iterator root=Roots.begin();
             root!=Roots.end();root++)
            Cuts[*root]=i;
        if((Cuts.count(0.)==0) and 
           ((B[0].valueAt(0.)<=0) or i==NB.size()))
            Cuts[0.]=i;
        if((Cuts.count(1.)==0) and 
           ((B[0].valueAt(1.)<=0) or i==NB.size()))
            Cuts[1.]=i;
        if (i<NB.size())
            B[0]-=(NB.lengths[i+1]-NB.lengths[i]);
    }
    B[0]+=*(NB.lengths.rbegin());

    //-- Compose each piece with the relevant sbasis2d.
    // TODO: use a uniform parametrization of the base.
    std::map<double,unsigned>::iterator cut=Cuts.begin();
    std::map<double,unsigned>::iterator next=cut; next++;
    while(next!=Cuts.end()){
        double t0=(*cut).first;
        unsigned  idx0=(*cut).second;
        double t1=(*next).first;
        unsigned  idx1=(*next).second;
        if (idx0 != idx1){
            idx=std::min(idx0,idx1);
        } else if(B[0]((t0+t1)/2) < NB.lengths[idx0]) { // we have a left 'bump',
            idx=idx0-1;
        } else if(B[0]((t0+t1)/2) == NB.lengths[idx0]) { //we have a vertical segment!...
            idx= (idx0==NB.size())? idx0-1:idx0;
        } else                                                  //we have a right 'bump'.
            idx=idx0;

        //--trim version...
        if (idx>=0 and idx<NB.size()) {
            for (unsigned dim=0;dim<2;dim++)
                Bcut[dim]=compose(B[dim], Linear(t0,t1));
            double width=NB.lengths[idx+1]-NB.lengths[idx];
            Bcut[0]=compose(Linear(-NB.lengths[idx]/width,
                                   (1-NB.lengths[idx])/width),Bcut[0]);
            Bcut = composeEach(NB[idx], Bcut);
            result.push_back(Bcut);
        }
        cut++;
        next++;
    }
    return(result);
}



class NormalBundleToy: public Toy {
    PointSetHandle B_handle;
    PointSetHandle P_handle;
    PointHandle O_handle;
    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        D2<SBasis> B = B_handle.asBezier();
        D2<SBasis> P = P_handle.asBezier();
        Geom::Point O = O_handle.pos;
    
        NormalBundle NBdle;
        NBdle.setBase(B);
        Geom::Point Oo(O[0]+*(NBdle.lengths.rbegin()),O[1]);

        vector<D2<SBasis> > Q=compose(NBdle,P,O);

        cairo_set_line_width (cr, 0.5);
        //Base lines
        cairo_set_source_rgba (cr, 0.9, 0., 0., 1);
        cairo_d2_sb(cr, B);
        draw_line_seg(cr, O, Oo);
        cairo_stroke(cr);

        //Sections    
        cairo_set_source_rgba (cr, 0, 0, 0.9, 1);
        cairo_d2_sb(cr, P);
        for (unsigned i=0;i<Q.size();i++){
            cairo_d2_sb(cr, Q[i]);
        }
        cairo_stroke(cr);

        //Normal bundle    
        cairo_set_source_rgba (cr, 0., 0., 0., 1);
        NBdle.draw(cr,3,5);
        cairo_stroke(cr);


        Toy::draw(cr, notify, width, height, save,timer_stream);
    }        

public:
    NormalBundleToy(){
        if(handles.empty()) {
            handles.push_back(&B_handle);
            handles.push_back(&P_handle);
            handles.push_back(&O_handle);
            for(unsigned i = 0; i < 4; i++)
                B_handle.push_back(200+50*i,400);
            for(unsigned i = 0; i < 4; i++)
                P_handle.push_back(100+uniform()*400,
                                   150+uniform()*100);
            O_handle.pos = Geom::Point(200,200);
        }
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new NormalBundleToy);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

 	  	 
