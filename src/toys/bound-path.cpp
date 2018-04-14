/*
 * Bounds Path and PathVector 
 *
 * Authors:
 *      Marco Cecchetti <mrcekets at gmail.com>
 * 
 * Copyright 2008  authors
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
 */

#include <2geom/d2.h>
#include <2geom/sbasis.h>
#include <2geom/path.h>
#include <2geom/pathvector.h>

#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>

using namespace Geom;

std::string option_formatter(double x)
{
    if (x == 0.0)
        return std::string("CURVE");
    if (x == 1.0)
        return std::string("PATH");
    if (x == 2.0)
        return std::string("PATHVECTOR");
    
    return std::string("");
}

class BoundsPath : public Toy
{
    enum { CURVE = 0, PATH, PATHVECTOR };
    enum { FAST = 0, EXACT = 1 };
    
  private:
    void draw( cairo_t *cr, std::ostringstream *notify, 
               int width, int height, bool save, std::ostringstream *timer_stream) override
    {
        cairo_set_line_width (cr, 0.3);
        m_selection_kind = (unsigned int) (sliders[0].value());
        
        for (unsigned int i = 0; i < m_pathvector_coll.size(); ++i)
        {
            cairo_set_source_rgba(cr, 0.0, 0.4*(i+1), 0.8/(i+1), 1.0);
            for (unsigned int j = 0; j < m_pathvector_coll[i].size(); ++j)
            {
                m_pathvector_coll[i][j].clear();
                for (unsigned int k = 0; k < m_curves_per_path; ++k)
                {
                    PointSetHandle psh;
                    psh.pts.resize(m_handles_per_curve);
                    for (unsigned int h = 0; h < m_handles_per_curve; ++h)
                    {
                        unsigned int kk = k * (m_handles_per_curve-1) + h;
                        psh.pts[h] = m_pathvector_coll_handles[i][j].pts[kk];
                    }
                    m_pathvector_coll[i][j].append(psh.asBezier());
                }
                cairo_path(cr, m_pathvector_coll[i][j]);
            }
            cairo_stroke(cr);
        }
        
        
        Rect bound;
        if ( (m_selection_kind == CURVE) && (m_selected_curve != -1) )
        {
            const Curve & curve = m_pathvector_coll[m_selected_pathvector][m_selected_path][m_selected_curve];
            bound = toggles[0].on ? curve.boundsExact()
                                  : curve.boundsFast();
        }
        else if ( (m_selection_kind == PATH) && (m_selected_path != -1) )
        {
            const Path & path = m_pathvector_coll[m_selected_pathvector][m_selected_path];
            bound = toggles[0].on ? *path.boundsExact()
                                  : *path.boundsFast();
        }
        else if ( (m_selection_kind == PATHVECTOR) && (m_selected_pathvector != -1) )
        {
            const PathVector & pathvector = m_pathvector_coll[m_selected_pathvector];
            bound = toggles[0].on ? *bounds_exact(pathvector)
                                  : *bounds_fast(pathvector);            
        }

        cairo_set_source_rgba(cr, 0.5, 0.0, 0.0, 1.0);
        cairo_set_line_width (cr, 0.4);
        cairo_rectangle(cr, bound.left(), bound.top(), bound.width(), bound.height());
        cairo_stroke(cr);
        
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }
    
    
    void mouse_pressed(GdkEventButton* e) override
    {
        Point pos(e->x, e->y);
        double d, t;
        double dist = 1e10;
        Rect bound;
        m_selected_pathvector = -1;
        m_selected_path = -1;
        m_selected_curve = -1;
        if (m_selection_kind == CURVE)
        {
            for (unsigned int i = 0; i < m_pathvector_coll.size(); ++i)
            {
                for (unsigned int j = 0; j < m_pathvector_coll[i].size(); ++j)
                {
                    for ( unsigned int k = 0; k <  m_pathvector_coll[i][j].size(); ++k)
                    {
                        const Curve & curve = m_pathvector_coll[i][j][k];
                        bound = toggles[0].on ? curve.boundsExact()
                                              : curve.boundsFast();
                        d = distanceSq(pos, bound);
                        if ( are_near(d, 0) )
                        {
                            t = curve.nearestTime(pos);
                            d = distanceSq(pos, curve.pointAt(t));
                            if (d < dist)
                            {
                                dist = d;
                                m_selected_pathvector = i;
                                m_selected_path = j;
                                m_selected_curve = k;
                            }
                        }
                    }
                }
            }
            //std::cerr << "m_selected_path = " << m_selected_path << std::endl;
            //std::cerr << "m_selected_curve = " << m_selected_curve << std::endl;  
        }
        else if (m_selection_kind == PATH)
        {
            for (unsigned int i = 0; i < m_pathvector_coll.size(); ++i)
            {
                for (unsigned int j = 0; j < m_pathvector_coll[i].size(); ++j)
                {
                    const Path & path = m_pathvector_coll[i][j];
                    bound = toggles[0].on ? *path.boundsExact()
                                          : *path.boundsFast();
                    d = distanceSq(pos, bound);
                    if ( are_near(d, 0) )
                    {
                        t = path.nearestTime(pos).asFlatTime();
                        d = distanceSq(pos, path.pointAt(t));
                        if (d < dist)
                        {
                            dist = d;
                            m_selected_pathvector = i;
                            m_selected_path = j; 
                        }
                    }
                }
            }
        }
        else if (m_selection_kind == PATHVECTOR)
        {
            for (unsigned int i = 0; i < m_pathvector_coll.size(); ++i)
            {
                const PathVector & pathvector = m_pathvector_coll[i];
                bound = toggles[0].on ? *bounds_exact(pathvector)
                                      : *bounds_fast(pathvector);
                d = distanceSq(pos, bound);
                if ( are_near(d, 0) )
                {
                    for (unsigned int j = 0; j < m_pathvector_coll[i].size(); ++j)
                    {
                        const Path & path = m_pathvector_coll[i][j];
                        t = path.nearestTime(pos).asFlatTime();
                        d = distanceSq(pos, path.pointAt(t));
                        if (d < dist)
                        {
                            dist = d;
                            m_selected_pathvector = i;
                        }
                    }                    
                }
            }
        }           

        Toy::mouse_pressed(e);
    }
 
  public:
    BoundsPath()
    {
        m_total_pathvectors = 2;
        m_paths_per_vector = 2;
        m_curves_per_path = 3;
        m_handles_per_curve = 4;
        
        m_selection_kind = CURVE;
        m_selected_pathvector = -1;
        m_selected_path = -1;
        m_selected_curve = -1;
        m_handles_per_path = m_curves_per_path * (m_handles_per_curve-1) + 1;
        
        m_pathvector_coll_handles.resize(m_total_pathvectors);
        m_pathvector_coll.resize(m_total_pathvectors);
        for (unsigned int k = 0; k < m_total_pathvectors; ++k)
        {
            m_pathvector_coll_handles[k].resize(m_paths_per_vector);
            m_pathvector_coll[k].resize(m_paths_per_vector);
            for (unsigned int j = 0; j < m_paths_per_vector; ++j)
            {
                m_pathvector_coll_handles[k][j].pts.resize(m_handles_per_path);
                handles.push_back(&(m_pathvector_coll_handles[k][j]));
                for (unsigned int i = 0; i < m_handles_per_path; ++i)
                {
                    m_pathvector_coll_handles[k][j].pts[i] 
                        = Point(500*uniform() + 300*k, 300*uniform() + 80 + 200*j);
                }
            }
        }
        
        sliders.push_back(Slider(0, 2, 1, 0, "selection type"));
        sliders[0].geometry(Point(10, 20), 50, X);
        sliders[0].formatter(&option_formatter);
        
        Rect toggle_bound(Point(300,20), Point(390, 45));
        toggles.push_back(Toggle(toggle_bound, "fast/exact", EXACT));
        
        handles.push_back(&(sliders[0]));
        handles.push_back(&(toggles[0]));
    }
    
   
  private:
    unsigned int m_total_pathvectors;
    unsigned int m_paths_per_vector;
    unsigned int m_curves_per_path;
    unsigned int m_handles_per_curve;
    unsigned int m_handles_per_path;
    std::vector<PathVector>  m_pathvector_coll;
    std::vector< std::vector<PointSetHandle> > m_pathvector_coll_handles;
//    PathVector m_pathvector;
//    std::vector<PointSetHandle> m_pathvector_handles;
    int m_selected_curve;
    int m_selected_path;
    int m_selected_pathvector;
    unsigned int m_selection_kind;
    std::vector<Slider> sliders;
    std::vector<Toggle> toggles;
};


int main(int argc, char **argv) 
{   
    init( argc, argv, new BoundsPath(), 800, 600 );
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
