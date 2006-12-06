#include "s-basis.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "multidim-sbasis.h"
#include "s-basis-2d.h"

#include "path.h"
#include "path-cairo.h"
#include "path-builder.h"

#include <map>
#include <iterator>
#include "translate.h"
#include "translate-ops.h"

#include "toy-framework.h"

using std::vector;

class NormalBundle : public std::vector<vector<SBasis2d> >{
public:
    vector<double> lengths;
    NormalBundle(){lengths.push_back(0.);}
    void setBase(multidim_sbasis<2> const &B, double tol);
    void draw(cairo_t* cr, int NbSections =5,int NbFibre=5);
};
vector<multidim_sbasis<2> > compose(NormalBundle const &NB, 
				    multidim_sbasis<2> const &Binit,
				    Geom::Point Origin=Geom::Point(0,0));


//--------------------------------------------

void SBasis1d_to_2d(multidim_sbasis<2> C0,
		    multidim_sbasis<2> C1,
		    vector<SBasis2d> &S){
    for(int dim = 0; dim < 2; dim++) {
//**** C0 and C1 should have the same size...
        for (int i=C0[dim].size();i<C1[dim].size(); i++)
            C0[dim].push_back(BezOrd(0));
        for (int i=C1[dim].size();i<C0[dim].size(); i++)
            C1[dim].push_back(BezOrd(0));
        S[dim].clear();
        S[dim].us = C0[dim].size();
        S[dim].vs = 1;
        for(int v = 0; v < S[dim].vs; v++)
            for(int u = 0; u < S[dim].us; u++)
                S[dim].push_back(BezOrd2d(C0[dim][u][0],C0[dim][u][1],
                                          C1[dim][u][0],C1[dim][u][1]));
    }
}

void NormalBundle::setBase(multidim_sbasis<2> const &B, double tol=0.1) {
    multidim_sbasis<2> dB;
    dB = derivative(B);
    SBasis arc = dot(dB, dB);

    double err = 0;
    double ss = 0.25;
    for(int i = 1; i < arc.size(); i++) {
        err += fabs(Hat(arc[i]))*ss;
        ss *= 0.25;
    }
    double le = fabs(arc[0][0]) - err;
    double re = fabs(arc[0][1]) - err;
    err /= std::max(arc[0][0], arc[0][1]);
    if(err > tol) {
        const int N = 2;
        for(int subdivi = 0; subdivi < N; subdivi++) {
            double dsubu = 1./N;
            double subu = dsubu*subdivi;
            multidim_sbasis<2> Bp;
            for(int dim = 0; dim < 2; dim++) 
                Bp[dim] = compose(B[dim], BezOrd(subu, dsubu+subu));
            setBase(Bp, tol);
        }
    }
    else {
        arc = sqrt(arc, 2);
        multidim_sbasis<2> offset;
        double dist=-1.;
        //-- the two coordinates should have the same degree!!!
        for (int i=dB[0].size();i<dB[1].size(); i++)
            dB[0].push_back(BezOrd(0));
        for (int i=dB[1].size();i<dB[0].size(); i++)
            dB[1].push_back(BezOrd(0));
        for (int dim = 0; dim < 2; dim++) {
            double sgn = dim?-1:1;
            offset[dim] = B[dim] + divide(dist*sgn*dB[1-dim],arc, 2);
        }
        vector<SBasis2d> S(2);
        SBasis1d_to_2d(B, offset, S);
        push_back(S);
        lengths.push_back(*(lengths.rbegin())+(arc.point_at(0.)+arc.point_at(1.))/2);
    }
}

void NormalBundle::draw(cairo_t *cr, int NbLi, int NbCol) {
    multidim_sbasis<2> B;
    vector<multidim_sbasis<2> > tB;
    Geom::Point Seg[2];
    B[1]=BezOrd(-100,100);
    double width=*(lengths.rbegin());
    if (NbCol>0)
        for(int ui = 0; ui <= NbCol; ui++) {
            B[0]=BezOrd(ui*width/NbCol);
            tB = compose(*this,B);
            if (tB.size()>0) cairo_md_sb(cr, tB[0]);
        }

    B[0]=SBasis(BezOrd(0,1));
    for(int ui = 0; ui <= NbLi; ui++) {
        B[1]=BezOrd(-100+ui*200/NbLi);
        for(int i = 0; i <size(); i++) {
            multidim_sbasis<2> section=compose( (*this)[i],B);
            cairo_md_sb(cr, section);
        }
    }
}


vector<multidim_sbasis<2> > compose(NormalBundle const &NB, 
				    multidim_sbasis<2> const &Binit,
				    Geom::Point Origin){
    vector<multidim_sbasis<2> > result;
    multidim_sbasis<2> B=Binit;
    multidim_sbasis<2> Bcut;
    vector<double> Roots;
    std::map<double,int> Cuts;
    int idx;

    B[0]-=Origin[0];
    B[1]-=Origin[1];

    //--Find intersections with fibers over segment ends.
    for(int i=0; i<=NB.size();i++){
        Roots=roots(B[0]);
        for (vector<double>::iterator root=Roots.begin();
             root!=Roots.end();root++)
            Cuts[*root]=i;
        if((Cuts.count(0.)==0) and 
           ((B[0].point_at(0.)<=0) or i==NB.size()))
            Cuts[0.]=i;
        if((Cuts.count(1.)==0) and 
           ((B[0].point_at(1.)<=0) or i==NB.size()))
            Cuts[1.]=i;
        if (i<NB.size())
            B[0]-=(NB.lengths[i+1]-NB.lengths[i]);
    }
    B[0]+=*(NB.lengths.rbegin());

    //-- Compose each piece with the relevant sbasis2d.
    std::map<double,int>::iterator cut=Cuts.begin();
    std::map<double,int>::iterator next=cut; next++;
    while(next!=Cuts.end()){
        double t0=(*cut).first;
        int  idx0=(*cut).second;
        double t1=(*next).first;
        int  idx1=(*next).second;
        if (idx0 != idx1){
            idx=std::min(idx0,idx1);
        } else if(B[0].point_at((t0+t1)/2) < NB.lengths[idx0]) { // we have a left 'bump',
            idx=idx0-1;
        } else if(B[0].point_at((t0+t1)/2) == NB.lengths[idx0]) { //we have a vertical segment!...
            idx= (idx0==NB.size())? idx0-1:idx0;
        } else                                                  //we have a right 'bump'.
            idx=idx0;

        //--trim version...
        if (idx>=0 and idx<NB.size()) {
            for (int dim=0;dim<2;dim++)
                Bcut[dim]=compose(B[dim], BezOrd(t0,t1));
            double width=NB.lengths[idx+1]-NB.lengths[idx];
            Bcut[0]=compose(BezOrd(-NB.lengths[idx]/width,
                                   (1-NB.lengths[idx])/width),Bcut[0]);
            Bcut=compose(NB[idx],Bcut);
            result.push_back(Bcut);
        }
        cut++;
        next++;
    }
    return(result);
}



class NormalBundleToy: public Toy {

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save) {
        multidim_sbasis<2> B = bezier_to_sbasis<2, 3>(handles.begin());
        multidim_sbasis<2> P = bezier_to_sbasis<2, 3>(handles.begin()+4);
        Geom::Point O = *(handles.begin()+8);
    
        NormalBundle NBdle;
        NBdle.setBase(B);
        Geom::Point Oo(O[0]+*(NBdle.lengths.rbegin()),O[1]);

        vector<multidim_sbasis<2> > Q=compose(NBdle,P,O);

        cairo_set_line_width (cr, 0.5);
        //Base lines
        cairo_set_source_rgba (cr, 0.9, 0., 0., 1);
        cairo_md_sb(cr, B);
        draw_line_seg(cr, O, Oo);
        cairo_stroke(cr);

        //Sections    
        cairo_set_source_rgba (cr, 0, 0, 0.9, 1);
        cairo_md_sb(cr, P);
        for (int i=0;i<Q.size();i++){
            cairo_md_sb(cr, Q[i]);
        }
        cairo_stroke(cr);

        //Normal bundle    
        cairo_set_source_rgba (cr, 0., 0., 0., 1);
        NBdle.draw(cr,3,5);
        cairo_stroke(cr);


        Toy::draw(cr, notify, width, height, save);
    }        

public:
    NormalBundleToy(){
        if(handles.empty()) {
            for(int i = 0; i < 4; i++)
                handles.push_back(Geom::Point(200+50*i,400));
            for(int i = 0; i < 4; i++)
                handles.push_back(Geom::Point(100+uniform()*400,
                                              150+uniform()*100));
            handles.push_back(Geom::Point(200,200));
        }
    }
};

int main(int argc, char **argv) {
    init(argc, argv, "normal-bundle", new NormalBundleToy);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

 	  	 
