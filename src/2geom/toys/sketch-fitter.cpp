/* 
 * sb-to-bez Toy - Tests convertions from sbasis to cubic bezier.
 *
 * Copyright 2007 jf barraud.
 * 2008 njh
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
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
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

// mainly experimental atm...
// do not expect to find anything understandable here atm. 

#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/sbasis-geometric.h>
#include <2geom/basic-intersection.h>

#include <2geom/circle.h>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#define ZERO 1e-7

using std::vector;
using namespace Geom;
using namespace std;

#include <stdio.h>
#include <gsl/gsl_poly.h>

std::vector<Point> neighbors(std::vector<Point> const &pts, unsigned idx, double radius){
    std::vector<Point> res;
    Point p0 = pts[idx];
    for (unsigned i = 0; i<pts.size(); i++){
        Point p = pts[i];
        if ( L2(p-p0) < radius ) res.push_back(p);
    }
    return res;
}

double curvature(Point const &a, Point const &b, Point const &c){
    Line med_ab = Line( (a+b)/2, (a+b)/2+rot90(b-a) );
    Line med_bc = Line( (b+c)/2, (b+c)/2+rot90(c-b) );
    OptCrossing o = intersection(med_ab, med_bc);
    if (o){
        Point oo = med_ab.pointAt(o->ta);
        return(1./L2(oo-a));
    }
    else 
        return 0;
}

double avarageCurvature(std::vector<Point> const &pts, unsigned idx, double radius){
    std::vector<Point> ngbrs = neighbors(pts, idx, radius);
    if (ngbrs.size()<3) return 0;
    double k=0;
    double mass = 0;
    for (unsigned i=0; i<5; i++){
        unsigned ia,ib,ic;
        ia = rand()%ngbrs.size();
        while (ib == ia) 
            ib = rand()%ngbrs.size();
        while (ic == ia || ic == ib) 
            ic = rand()%ngbrs.size();
        k += curvature(pts[ia],pts[ib],pts[ic]);
        mass += 1; //smaller mass to closer triplets?
    }
    k /= mass;
    return k;
}

Point massCenter(std::vector<Point> const &pts){
    Point g = Point(0,0);
    for (unsigned i=0; i<pts.size(); i++){
        g += pts[i]/pts.size();
    }
    return g;
}

Line meanSquareLine(std::vector<Point> const &pts){
    Point g = massCenter(pts);
    double a = 0, b = 0, c = 0;
    for (unsigned i=0; i<pts.size(); i++){
        a += (pts[i][Y]-g[Y])*(pts[i][Y]-g[Y]);
        b +=-(pts[i][X]-g[X])*(pts[i][Y]-g[Y]);
        c += (pts[i][X]-g[X])*(pts[i][X]-g[X]);
    }
    double eigen = ( (a+c) - sqrt((a-c)*(a-c)+4*b*b) )/2; 
    Point u(-b,a-eigen);
    return Line(g, g+u);
}

void tighten(std::vector<Point> &pts, double radius, bool linear){
    std::vector<Point> res = pts;
    for (unsigned i=0; i<pts.size(); i++){
        std::vector<Point> ngbrs = neighbors(pts,i,radius);
        if (linear){
            Line d = meanSquareLine(ngbrs);
            Point proj = projection( pts[i], d );
            double t = 2./3.;
            //pts[i] = pts[i]*(1-t) + proj*t;
            res[i] = pts[i]*(1-t) + proj*t;
        }else if (ngbrs.size()>=3) {
            Circle c(ngbrs);
            Point o = c.center();
            double r = c.ray();
            //pts[i] = o + unit_vector(pts[i]-o)*r;
            res[i] = o + unit_vector(pts[i]-o)*r;
        }
    }
    pts = res;
}

double dist_to(std::vector<Point> const &pts, Point const &p, unsigned *idx=NULL){
    double d,d_min = std::numeric_limits<float>::infinity();
    if (idx) *idx = pts.size();
    for (unsigned i = 0; i<pts.size(); i++){
        d = L2(pts[i]-p);
        if ( d < d_min ){
            d_min = d;
            if (idx) *idx = i;
        }
    }
    return d_min;
}

void fuse_close_points(std::vector<Point> &pts, double dist_min){
    if (pts.size()==0) return;
    std::vector<Point> reduced_pts;
    reduced_pts.push_back(pts[0]);
    for (unsigned i = 0; i<pts.size(); i++){
        double d = dist_to(reduced_pts, pts[i]);
        if ( d > dist_min ) reduced_pts.push_back(pts[i]);
    }
    pts = reduced_pts;
    return;
}


unsigned nearest_after(std::vector<Point>const &pts, unsigned idx, double *dist = NULL){
    if ( idx >= pts.size()-1 ) return pts.size();
    Point p = pts[idx];
    unsigned res = idx+1;
    double d_min = L2(p-pts[res]);
    for (unsigned i=idx+2; i<pts.size(); i++){
        double d = L2(p-pts[i]);
        if (d < d_min) {
            d_min = d;
            res = i;
        }
    }
    if (dist) *dist = d_min;
    return res;
}

//TEST ME: use direction information to separate exaeco?
void sort_nearest(std::vector<Point> &pts, double no_longer_than = 0){
    double d;
    Point p;
    for (unsigned i=0; i<pts.size()-1; i++){
        unsigned j = nearest_after(pts,i,&d);
        if (no_longer_than >0.1 && d > no_longer_than){
            pts.erase(pts.begin()+i+1, pts.end());
            return;
        }
        p = pts[i+1];
        pts[i+1] = pts[j];
        pts[j] = p;
    }
}
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------


std::vector<Point> eat(std::vector<Point> const &pts, double sampling){
    std::vector<bool> visited(pts.size(),false);
    std::vector<Point> res;
    Point p = pts.front();
    //Point q = p;
    res.push_back(p);
    while(true){
        double num_nghbrs = 0;
        double dir = 0;
        Point next(0,0);
        for(unsigned i = 0; i < pts.size(); i++) {
            if (!visited[i] && L2(pts[i]-p)<sampling){
                //TODO: rotate pts[i] so that last step was in dir -pi...
                //dir += atan2(pts[i]-p);
                visited[i] = true;
                next+= pts[i]-p;
                num_nghbrs += 1;
            }
        }
        if (num_nghbrs == 0) break;
        //q=p;
        next *= 1./num_nghbrs;
        p += next;
        res.push_back(p);
    }
    return res;
}





//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

double exp_rescale(double x)
{
    return pow(10, x*5-2);
}
std::string exp_formatter(double x)
{
    return default_formatter(exp_rescale(x));
}

class SketchFitterToy: public Toy {

    enum menu_item_t
    {
        SHOW_MENU = 0,
        TEST_TIGHTEN,
        TEST_EAT_BY_STEP,
        TEST_TIGHTEN_EAT,
        TEST_CURVATURE,
        TEST_SORT,
        TEST_NUMERICAL,
        TOTAL_ITEMS // this one must be the last item
    };

    enum handle_label_t
    {
    };

    enum toggle_label_t
    {
    };
    enum tighten_toggle_label_t
    {
        LINEAR = 0,
    };

    enum slider_label_t
    {
        TIGHTEN_NBHD_SIZE = 0,
        TIGHTEN_ITERRATIONS = 1
    };

    static const char* menu_items[TOTAL_ITEMS];
    static const char keys[TOTAL_ITEMS];

    void fit_empty(){}
    virtual void first_time(int /*argc*/, char** /*argv*/)
    {
        draw_f = &SketchFitterToy::draw_menu;
        fit_f =  &SketchFitterToy::fit_empty;
    }

    void init_common()
    {
        set_common_control_geometry = true;
        set_control_geometry = true;

        sliders.clear();
        toggles.clear();
        handles.clear();
    }
    void init_common_ctrl_geom(cairo_t* /*cr*/, int /*width*/, int /*height*/, std::ostringstream* /*notify*/)
    {
        if ( set_common_control_geometry )
        {
            set_common_control_geometry = false;
        }
    }
    virtual void draw_common( cairo_t *cr, std::ostringstream *notify,
                              int width, int height, bool /*save*/, std::ostringstream *timer_stream )
    {
        init_common_ctrl_geom(cr, width, height, notify);
        if(!mouses.empty()) {
            cairo_move_to(cr, mouses[0]);
            for(unsigned i = 0; i < mouses.size(); i++) {
                cairo_line_to(cr, mouses[i]);
            }
            for(unsigned i = 0; i < mouses.size(); i++) {
                draw_cross(cr, mouses[i]);
            }
            cairo_set_source_rgba (cr, 0., 0., 0., .25);
            cairo_set_line_width (cr, 0.5);
            cairo_stroke(cr);
        }
        *notify << "Press SHIFT to continue sketching. 'Z' to apply changes";
    }


//-----------------------------------------------------------------------------------------
// Tighten: tries to move the points toward the common curve
//-----------------------------------------------------------------------------------------
    void init_tighten()
    {
        init_common();
        sliders.push_back(Slider(0., 1., 0, 0.7, "neighborhood size"));
        sliders[TIGHTEN_NBHD_SIZE].formatter(&exp_formatter);
        sliders.push_back(Slider(0, 10, 1, 1, "iterrations"));
        handles.push_back(&(sliders[TIGHTEN_NBHD_SIZE]));
        handles.push_back(&(sliders[TIGHTEN_ITERRATIONS]));

        toggles.push_back(Toggle("Linear",true));
        handles.push_back(&(toggles[0]));

    }
    void init_tighten_ctrl_geom(cairo_t* /*cr*/, std::ostringstream* /*notify*/, int width, int height)
    {
        if ( set_control_geometry ){
            set_control_geometry = false;
            sliders[TIGHTEN_NBHD_SIZE].geometry(Point(50, height - 35*2), 180);
            sliders[TIGHTEN_ITERRATIONS].geometry(Point(50, height - 35*3), 180);

            Point p(width-125, height - 50), d(100,25);
            toggles[0].bounds = Rect(p,     p + d);
        }
    }
    void fit_tighten(){
        improved_mouses = mouses;
        double radius = exp_rescale(sliders[0].value());
        for (unsigned i=1; i<=sliders[TIGHTEN_ITERRATIONS].value(); i++){
            tighten(improved_mouses, radius, toggles[0].on);
        }        
    }
    void draw_tighten(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        draw_common(cr, notify, width, height, save, timer_stream);
        init_tighten_ctrl_geom(cr, notify, width, height);

        if(!improved_mouses.empty()) {            
            cairo_move_to(cr, improved_mouses[0]);
            for(unsigned i = 0; i < improved_mouses.size(); i++) {
                draw_cross(cr, improved_mouses[i]);
            }
            cairo_set_source_rgba (cr, 1., 0., 0., 1);
            cairo_set_line_width (cr, .75);
            cairo_stroke(cr);
        }
    }

//-----------------------------------------------------------------------------------------
// Eat by step: eats the curve moving at each step in the avarage direction of the neighbors.
//-----------------------------------------------------------------------------------------
    void init_eat()
    {
        init_common();

        sliders.push_back(Slider(0., 1., 0, 0.7, "neighborhood size"));
        sliders[0].formatter(&exp_formatter);
        handles.push_back(&(sliders[0]));
    }
    void init_eat_ctrl_geom(cairo_t* /*cr*/, std::ostringstream* /*notify*/, int /*width*/, int height)
    {
        if ( set_control_geometry ){
            set_control_geometry = false;
            sliders[0].geometry(Point(50, height - 35*(0+2)), 180);
        }
    }
    void fit_eat(){
        double radius = exp_rescale(sliders[0].value());
        improved_mouses = mouses;

        tighten(improved_mouses, 20, true);
        
        stroke = Piecewise<D2<SBasis> >();
        improved_mouses = eat(improved_mouses, radius);
        Path p(improved_mouses[0]);
        for(unsigned i = 1; i < improved_mouses.size(); i++) {
            p.appendNew<LineSegment>(improved_mouses[i]);
        }
        stroke = p.toPwSb();
    }
    void draw_eat(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        draw_common(cr, notify, width, height, save, timer_stream);
        init_eat_ctrl_geom(cr, notify, width, height);
        if(!mouses.empty()) {
            cairo_pw_d2_sb(cr, stroke);
        }
        cairo_set_source_rgba (cr, 1., 0., 0., 1);
        cairo_set_line_width (cr, .75);
        cairo_stroke(cr);
    }

//-----------------------------------------------------------------------------------------
// Tighten + Eat
//-----------------------------------------------------------------------------------------
    void init_tighten_eat()
    {
        init_common();
        sliders.push_back(Slider(0., 1., 0, 0.7, "tightening neighborhood size"));
        sliders[0].formatter(&exp_formatter);
        sliders.push_back(Slider(0, 10, 1, 1, "tighten iterrations"));
        sliders.push_back(Slider(0., 1., 0, 0.7, "eating neighborhood size"));
        sliders[2].formatter(&exp_formatter);
        handles.push_back(&(sliders[0]));
        handles.push_back(&(sliders[1]));
        handles.push_back(&(sliders[2]));
    }
    void init_tighten_eat_ctrl_geom(cairo_t* /*cr*/, std::ostringstream* /*notify*/, int width, int height)
    {
        if ( set_control_geometry ){
            set_control_geometry = false;
            sliders[0].geometry(Point(50, height - 35*2), 180);
            sliders[1].geometry(Point(50, height - 35*3), 180);
            sliders[2].geometry(Point(50, height - 35*4), 180);
        }
    }
    void fit_tighten_eat(){
        improved_mouses = mouses;
        double radius = exp_rescale(sliders[0].value());
        for (unsigned i=1; i<=sliders[1].value(); i++){
            tighten(improved_mouses, radius, toggles[0].on);
        }
        stroke = Piecewise<D2<SBasis> >();
        radius = exp_rescale(sliders[2].value());
        improved_mouses = eat(improved_mouses, radius);
        Path p(improved_mouses[0]);
        for(unsigned i = 1; i < improved_mouses.size(); i++) {
            p.appendNew<LineSegment>(improved_mouses[i]);
        }
        stroke = p.toPwSb();
    }
    void draw_tighten_eat(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        draw_common(cr, notify, width, height, save, timer_stream);
        init_tighten_eat_ctrl_geom(cr, notify, width, height);

        if(!improved_mouses.empty()) {            
            cairo_pw_d2_sb(cr, stroke);
            cairo_set_source_rgba (cr, 1., 0., 0., 1);
            cairo_set_line_width (cr, .75);
            cairo_stroke(cr);
        }
    }

//-----------------------------------------------------------------------------------------
// Avarage curvature.
//-----------------------------------------------------------------------------------------
    void init_curvature()
    {
        init_common();

        sliders.push_back(Slider(0., 1., 0, 0.8, "neighborhood size"));
        sliders[0].formatter(&exp_formatter);
        sliders.push_back(Slider(0, 100, 0, 50, "Point chooser"));
        handles.push_back(&(sliders[0]));
        handles.push_back(&(sliders[1]));
    }
    void init_curvature_ctrl_geom(cairo_t* /*cr*/, std::ostringstream* /*notify*/, int /*width*/, int height)
    {
        if ( set_control_geometry ){
            set_control_geometry = false;
            sliders[0].geometry(Point(50, height - 60), 180);
            sliders[1].geometry(Point(50, height - 90), 180);
        }
    }
    void fit_curvature(){
        //Not implemented
    }
    void draw_curvature(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        draw_common(cr, notify, width, height, save, timer_stream);
        init_curvature_ctrl_geom(cr, notify, width, height);
        if(!mouses.empty()) {
            double radius = exp_rescale(sliders[0].value());
            unsigned i = unsigned( (mouses.size()-1)*sliders[1].value()/100. );
            std::vector<Point> ngbrs = neighbors(mouses,i,radius);
            if ( ngbrs.size()>2 ){
                draw_cross(cr, mouses[i]);
                Circle c(ngbrs);
                cairo_arc(cr, c.center(X), c.center(Y), c.ray(), 0, 2*M_PI);
                cairo_set_source_rgba (cr, 1., 0., 0., 1);
                cairo_set_line_width (cr, .75);
                cairo_stroke(cr);
            }
        }
    }

//-----------------------------------------------------------------------------------------
// Sort: tighten, then sort and eventually fuse.
//-----------------------------------------------------------------------------------------
    void init_sort()
    {
        init_common();
        sliders.push_back(Slider(0., 1., 0, 0.7, "tighten neighborhood size"));
        sliders[TIGHTEN_NBHD_SIZE].formatter(&exp_formatter);
        sliders.push_back(Slider(0, 10, 1, 1, "tighten iterrations"));
        sliders.push_back(Slider(0., 1., 0, 0.8, "no jump longer than"));
        sliders[2].formatter(&exp_formatter);
        sliders.push_back(Slider(0., 1., 0, 0.6, "fuse radius"));
        sliders[3].formatter(&exp_formatter);
        handles.push_back(&(sliders[TIGHTEN_NBHD_SIZE]));
        handles.push_back(&(sliders[TIGHTEN_ITERRATIONS]));
        handles.push_back(&(sliders[2]));
        handles.push_back(&(sliders[3]));

        toggles.push_back(Toggle("Linear",true));
        handles.push_back(&(toggles[0]));
    }
    void init_sort_ctrl_geom(cairo_t* /*cr*/, std::ostringstream* /*notify*/, int width, int height)
    {
        if ( set_control_geometry ){
            set_control_geometry = false;
            sliders[TIGHTEN_NBHD_SIZE].geometry(Point(50, height - 35*2), 180);
            sliders[TIGHTEN_ITERRATIONS].geometry(Point(50, height - 35*3), 180);
            sliders[2].geometry(Point(50, height - 35*4), 180);
            sliders[3].geometry(Point(50, height - 35*5), 180);

            Point p(width-125, height - 50), d(100,25);
            toggles[0].bounds = Rect(p,     p + d);
        }
    }
    void fit_sort(){
        improved_mouses = mouses;
        double radius = exp_rescale(sliders[TIGHTEN_NBHD_SIZE].value());
        for (unsigned i=1; i<=sliders[TIGHTEN_ITERRATIONS].value(); i++){
            tighten(improved_mouses, radius, toggles[0].on);
        }
        double max_jump = exp_rescale(sliders[2].value());
        sort_nearest(improved_mouses, max_jump);
        radius = exp_rescale(sliders[3].value());
        fuse_close_points(improved_mouses, radius);

    }
    void draw_sort(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        draw_common(cr, notify, width, height, save, timer_stream);
        init_sort_ctrl_geom(cr, notify, width, height);

        if(!improved_mouses.empty()) {            
            for(unsigned i = 0; i < improved_mouses.size(); i++) {
                draw_cross(cr, improved_mouses[i]);
            }
            cairo_move_to(cr, improved_mouses[0]);
            for(unsigned i = 1; i < improved_mouses.size(); i++) {
                cairo_line_to(cr, improved_mouses[i]);
            }
            cairo_set_source_rgba (cr, 1., 0., 0., 1);
            cairo_set_line_width (cr, .75);
            cairo_stroke(cr);
        }
    }


//-----------------------------------------------------------------------------------------
// Brutal optimization, number of segment fixed.
//-----------------------------------------------------------------------------------------
    void init_numerical()
    {
        init_common();
        sliders.push_back(Slider(0, 10, 1, 1, "Number of curves"));
        handles.push_back(&(sliders[0]));
    }
    void init_numerical_ctrl_geom(cairo_t* /*cr*/, std::ostringstream* /*notify*/, int /*width*/, int height)
    {
        if ( set_control_geometry ){
            set_control_geometry = false;
            sliders[0].geometry(Point(50, height - 35*(0+2)), 180);
        }
    }
    void fit_numerical(){
        //Not implemented
    }
    void draw_numerical(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {
        draw_common(cr, notify, width, height, save, timer_stream);
        init_numerical_ctrl_geom(cr, notify, width, height);
        if(!mouses.empty()) {            
            cairo_pw_d2_sb(cr, stroke);
            cairo_set_source_rgba (cr, 1., 0., 0., 1);
            cairo_set_line_width (cr, .75);
            cairo_stroke(cr);
        }
    }
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
  
public:
    vector<Point> mouses;
    int mouse_drag;
    vector<Point> improved_mouses;
    Piecewise<D2<SBasis > > stroke;
    
    void mouse_pressed(GdkEventButton* e) {
        //toggle_events(toggles, e);
	Toy::mouse_pressed(e);
	if(!selected) {
	    mouse_drag = 1;
            if (!(e->state & (GDK_SHIFT_MASK))){
                mouses.clear();
            }
	}
    }

    virtual void mouse_moved(GdkEventMotion* e) {
	if(mouse_drag) {
	    mouses.push_back(Point(e->x, e->y));
	    redraw();
	} else {
	    Toy::mouse_moved(e);
	}
    }

    virtual void mouse_released(GdkEventButton* e) {
        mouse_drag = 0;
        if(!mouses.empty()) {            
            (this->*fit_f)();
        }
        Toy::mouse_released(e);
    }

    void init_menu()
    {
        handles.clear();
        sliders.clear();
        toggles.clear();
    }
    void draw_menu( cairo_t * cr, std::ostringstream *notify,
                    int /*width*/, int /*height*/, bool /*save*/, std::ostringstream *timer_stream )
    {
        *notify << std::endl;
        for (int i = SHOW_MENU; i < TOTAL_ITEMS; ++i)
        {
            *notify << "   " << keys[i] << " -  " <<  menu_items[i] << std::endl;
        }
        if(!mouses.empty()) {
            cairo_move_to(cr, mouses[0]);
            for(unsigned i = 0; i < mouses.size(); i++) {
                cairo_line_to(cr, mouses[i]);
            }
            for(unsigned i = 0; i < mouses.size(); i++) {
                draw_cross(cr, mouses[i]);
            }
            cairo_set_source_rgba (cr, 0., 0., 0., .25);
            cairo_set_line_width (cr, 0.5);
            cairo_stroke(cr);
        }
    }

    void key_hit(GdkEventKey *e)
    {
        char choice = std::toupper(e->keyval);
        switch ( choice )
        {
            case 'A':
                init_menu();
                draw_f = &SketchFitterToy::draw_menu;
                break;
            case 'B':
                init_tighten();
                fit_f = &SketchFitterToy::fit_tighten;
                draw_f = &SketchFitterToy::draw_tighten;
                break;
            case 'C':
                init_eat();
                fit_f = &SketchFitterToy::fit_eat;
                draw_f = &SketchFitterToy::draw_eat;
                break;
            case 'D':
                init_tighten_eat();
                fit_f = &SketchFitterToy::fit_tighten_eat;
                draw_f = &SketchFitterToy::draw_tighten_eat;
                break;
            case 'E':
                init_sort();
                fit_f = &SketchFitterToy::fit_sort;
                draw_f = &SketchFitterToy::draw_sort;
                break;
            case 'F':
                init_curvature();
                fit_f = &SketchFitterToy::fit_curvature;
                draw_f = &SketchFitterToy::draw_curvature;
                break;
            case 'G':
                init_numerical();
                fit_f = &SketchFitterToy::fit_numerical;
                draw_f = &SketchFitterToy::draw_numerical;
                break;
            case 'Z':
                mouses = improved_mouses;
                break;
        }
        redraw();
    }

    virtual void draw( cairo_t *cr, std::ostringstream *notify,
                       int width, int height, bool save, std::ostringstream *timer_stream )
    {
        m_width = width;
        m_height = height;
        m_length = (m_width > m_height) ? m_width : m_height;
        m_length *= 2;
        (this->*draw_f)(cr, notify, width, height, save, timer_stream);
        Toy::draw(cr, notify, width, height, save, timer_stream);
    }


  public:
    SketchFitterToy()
    {
        srand ( time(NULL) );
    }

  private:
    typedef void (SketchFitterToy::* draw_func_t) (cairo_t*, std::ostringstream*, int, int, bool, std::ostringstream*);
    draw_func_t draw_f;
    typedef void (SketchFitterToy::* fit_func_t) ();
    fit_func_t fit_f;
    bool set_common_control_geometry;
    bool set_control_geometry;
    std::vector<Toggle> toggles;
    std::vector<Slider> sliders;
    double m_width, m_height, m_length;

}; // end class SketchFitterToy


const char* SketchFitterToy::menu_items[] =
{
    "show this menu",
    "tighten",
    "eat points step by step",
    "tighten + eat",
    "tighten + sort + fuse",
    "curvature",
    "numerical",
};

const char SketchFitterToy::keys[] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G'
};

int main(int argc, char **argv) {
    init(argc, argv, new SketchFitterToy);
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
// vim: filetype = cpp:expandtab:shiftwidth = 4:tabstop = 8:softtabstop = 4:encoding = utf-8:textwidth = 99 :
