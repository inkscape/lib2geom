/*
 * Show off crossings between two Bezier curves.
 * The intersection points are found by using Bezier clipping.
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


#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <2geom/geom.h>
#include <2geom/d2.h>
#include <2geom/bezier.h>
#include <2geom/interval.h>
#include <2geom/crossing.h>


#include <vector>
#include <utility>

namespace Geom
{


namespace detail
{

void base_line (std::vector<double> & bl, std::vector<Point> const& A)
{
    size_t n = A.size() - 1;
    bl[0] = A[n][Y] - A[0][Y];
    bl[1] = A[0][X] - A[n][X];
    bl[2] = cross(A[0], A[n]);
    double l = std::sqrt(bl[0] * bl[0] + bl[1] * bl[1]);
    assert (l != 0);
    bl[0] /= l;
    bl[1] /= l;
    bl[2] /= l;
}


double distance (Point const& P, std::vector<double> const& bl)
{
    return bl[X] * P[X] + bl[Y] * P[Y] + bl[2];
}


void fat_line_bounds (Interval& bound,
                      std::vector<Point> const& A,
                      std::vector<double> const& bl)
{
    bound[0] = 0;
    bound[1] = 0;
    double d;
    size_t n = A.size()-1;
    for (size_t i = 1; i < n; ++i)
    {
        d = distance(A[i], bl);
        if (bound[0] > d)
        {
            bound[0] = d;
        }
        if (bound[1] < d)
        {
            bound[1] = d;
        }
    }
}

double intersect (Point const& p1, Point const& p2, double y)
{
    // we are sure that p2[Y] != p1[Y] because this routine is called
    // only when the lower or upper bound is crossed
    double s = (y - p1[Y]) / (p2[Y] - p1[Y]);
    return (p2[X]-p1[X])*s + p1[X];
}


void clip (Interval& dom,
           std::vector<Point> const& B,
           std::vector<double> const& bl,
           Interval const& bound)
{
    double n = B.size() - 1;  // number of sub-intervals
    std::vector<Point> D;     // distance curve control points
    D.reserve (B.size());
    double d;
    for (size_t i = 0; i < B.size(); ++i)
    {
        d = distance (B[i], bl);
        D.push_back (Point(i/n, d));
    }
    ConvexHull chD(D);
    std::vector<Point> const& p = chD.boundary; // convex hull vertices


    bool plower, phigher;
    bool clower, chigher;
    double t, tmin = 1, tmax = 0;
    //std::cerr << "bound : " << bound.min() << ", " << bound.max() << std::endl;

    plower = (p[0][Y] < bound.min());
    phigher = (p[0][Y] > bound.max());
    if (!(plower || phigher))  // inside the fat line
    {
        if (tmin > p[0][X])  tmin = p[0][X];
        if (tmax < p[0][X])  tmax = p[0][X];
        //std::cerr << "0 : inside " << p[0] << " : tmin = " << tmin << ", tmax = " << tmax << std::endl;
    }

    for (size_t i = 1; i < p.size(); ++i)
    {
        clower = (p[i][Y] < bound.min());
        chigher = (p[i][Y] > bound.max());
        if (!(clower || chigher))  // inside the fat line
        {
            if (tmin > p[i][X])  tmin = p[i][X];
            if (tmax < p[i][X])  tmax = p[i][X];
            //std::cerr << i << " : inside " << p[i] << " : tmin = " << tmin << ", tmax = " << tmax << std::endl;
        }
        if (clower != plower)  // cross the lower bound
        {
            t = intersect(p[i-1], p[i], bound.min());
            if (tmin > t)  tmin = t;
            if (tmax < t)  tmax = t;
            plower = clower;
            //std::cerr << i << " : lower " << p[i] << " : tmin = " << tmin << ", tmax = " << tmax << std::endl;
        }
        if (chigher != phigher)  // cross the upper bound
        {
            t = intersect(p[i-1], p[i], bound.max());
            if (tmin > t)  tmin = t;
            if (tmax < t)  tmax = t;
            phigher = chigher;
            //std::cerr << i << " : higher " << p[i] << " : tmin = " << tmin << ", tmax = " << tmax << std::endl;
        }
    }

    // we have to test the closing segment for intersection
    size_t last = p.size() - 1;
    clower = (p[0][Y] < bound.min());
    chigher = (p[0][Y] > bound.max());
    if (clower != plower)  // cross the lower bound
    {
        t = intersect(p[last], p[0], bound.min());
        if (tmin > t)  tmin = t;
        if (tmax < t)  tmax = t;
        //std::cerr << "0 : lower " << p[0] << " : tmin = " << tmin << ", tmax = " << tmax << std::endl;
    }
    if (chigher != phigher)  // cross the upper bound
    {
        t = intersect(p[last], p[0], bound.max());
        if (tmin > t)  tmin = t;
        if (tmax < t)  tmax = t;
        //std::cerr << "0 : higher " << p[0] << " : tmin = " << tmin << ", tmax = " << tmax << std::endl;
    }

    dom[0] = tmin;
    dom[1] = tmax;
}


void portion (std::vector<Point> & B, double t0, double t1)
{
    Bezier::Order bo(B.size()-1);
    Bezier Bx(bo), By(bo);
    for (size_t i = 0; i < B.size(); ++i)
    {
        Bx[i] = B[i][X];
        By[i] = B[i][Y];
    }
    Bx = portion(Bx, t0, t1);
    By = portion(By, t0, t1);
    assert (Bx.size() == By.size());
    B.resize(Bx.size());
    for (size_t i = 0; i < Bx.size(); ++i)
    {
        B[i][X] = Bx[i];
        B[i][Y] = By[i];
    }
}

void map_to(Interval & J, Interval const& I)
{
    double length = J.extent();
    J[1] = I.max() * length + J[0];
    J[0] = I.min() * length + J[0];
}

double angle (std::vector<Point> const& A)
{
    size_t n = A.size() -1;
    double a = std::atan2(A[n][Y] - A[0][Y], A[n][X] - A[0][X]);
    return (180 * a / M_PI);
}

void iterate (std::vector<Interval>& domsA,
              std::vector<Interval>& domsB,
              std::vector<Point> const& A,
              std::vector<Point> const& B,
              Interval const& domA,
              Interval const& domB)
{
//    std::cerr << ">> recursive call <<" << std::endl;
//    std::cerr << "dom(A) : " << domA.min() << ", " << domA.max() << std::endl;
//    std::cerr << "dom(B) : " << domB.min() << ", " << domB.max() << std::endl;
//    std::cerr << "angle(A) : " << angle(A) << std::endl;
//    std::cerr << "angle(B) : " << angle(B) << std::endl;

    const double precision = 1e-5;

    if (domA.extent() < precision && domB.extent() < precision)
    {
        return;
    }

    std::vector<Point> pA = A;
    std::vector<Point> pB = B;
    std::vector<Point>* C1 = &pA;
    std::vector<Point>* C2 = &pB;

    Interval dompA = domA;
    Interval dompB = domB;
    Interval* dom1 = &dompA;
    Interval* dom2 = &dompB;

    std::vector<double> bl(3);
    Interval bound, dom;

    size_t iter = 0;
    while (++iter < 1000 && (dompA.extent() >= precision || dompB.extent() >= precision))
    {
//        std::cerr << "iter: " << iter << std::endl;

        base_line(bl, *C1);
        fat_line_bounds(bound, *C1, bl);
        clip(dom, *C2, bl, bound);

        if (dom.min() > dom.max())
        {
//            std::cerr << "dom: empty" << std::endl;
            return;
        }
//        std::cerr << "dom : " << dom.min() << ", " << dom.max() << std::endl;
        if (!(dom.min() == 0 && dom.max() == 1))
        {
            portion(*C2, dom.min(), dom.max());
        }
        map_to(*dom2, dom);
        if (dom.extent() > 0.8)
        {
//            std::cerr << "clipping less than 20% : " << dom.extent() << std::endl;
            std::vector<Point> pC1, pC2;
            Interval first_half(0.0, 0.5);
            Interval second_half(0.5, 1.0);
            Interval dompC1, dompC2, dompC;
            if (dompA.extent() > dompB.extent())
            {
                pC1 = pC2 = pA;
                portion(pC1, 0.0, 0.5);
                portion(pC2, 0.5, 1.0);
                dompC1 = dompC2 = dompA;
                map_to(dompC1, first_half);
                map_to(dompC2, second_half);
                iterate(domsA, domsB, pC1, pB, dompC1, dompB);
                iterate(domsA, domsB, pC2, pB, dompC2, dompB);
            }
            else
            {
                pC1 = pC2 = pB;
                portion(pC1, 0.0, 0.5);
                portion(pC2, 0.5, 1.0);
                dompC1 = dompC2 = dompB;
                map_to(dompC1, first_half);
                map_to(dompC2, second_half);
//                iterate(domsA, domsB, pA, pC1, dompA, dompC1);
//                iterate(domsA, domsB, pA, pC2, dompA, dompC2);
                iterate(domsB, domsA, pC1, pA, dompC1, dompA);
                iterate(domsB, domsA, pC2, pA, dompC2, dompA);
            }
            return;
        }
        std::swap(C1, C2);
        std::swap(dom1, dom2);
//        std::cerr << "dom(pA) : " << dompA.min() << ", " << dompA.max() << std::endl;
//        std::cerr << "dom(pB) : " << dompB.min() << ", " << dompB.max() << std::endl;
    }
    domsA.push_back(dompA);
    domsB.push_back(dompB);
}


}  // end namespace detail


}  // end namespace Geom


using namespace Geom;


void draw_line (cairo_t *cr, std::vector<double> const& l,
                int ox, int oy, size_t w, size_t h)
{
    int mx = ox + static_cast<int>(w);
    int my = oy + static_cast<int>(h);
    for (int i = ox; i < mx; ++i)
        for (int j = oy; j < my; ++j)
        {
            if (are_near(detail::distance(Point(i,j), l), 0, 1))
            {
                cairo_move_to(cr, Point(i,j));
                cairo_line_to(cr, Point(i+0.1,j+0.1));
            }
        }
}

void draw_distance (cairo_t* cr,
                    std::vector<Point> const& B,
                    std::vector<double> const& l,
                    Interval const& mm)
{
    Point O(50, mm.max()+ 100);
    double scale  = 200.0;
    cairo_set_line_width (cr, 0.3);
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1);
    cairo_move_to(cr, O);
    cairo_line_to(cr, O + Point(scale, 0));
    cairo_move_to(cr, O + Point(0, -mm.max()));
    cairo_line_to(cr, O + Point(scale, -mm.max()));
    cairo_move_to(cr, O + Point(0, -mm.min()));
    cairo_line_to(cr, O + Point(scale, -mm.min()));
    cairo_stroke(cr);

    size_t n = B.size();
    std::vector<Point> D(n);
    double d;
    for (size_t i = 0; i < n; ++i)
    {
        d = detail::distance(B[i], l);
        D[i] = Point((scale*i)/(n-1), -d) + O;
    }

    D2<SBasis> Dist = handles_to_sbasis(D, D.size()-1);
    cairo_set_line_width (cr, 0.3);
    cairo_set_source_rgba (cr, 0.8, 0.0, 0.0, 1);
    cairo_md_sb(cr, Dist);
    cairo_stroke(cr);

    cairo_set_line_width (cr, 0.3);
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.7, 1);
    cairo_move_to(cr, D[0]);
    for (size_t i = 1; i < n; ++i)
    {
        cairo_line_to(cr, D[i]);
    }
    cairo_line_to(cr, D[0]);
    cairo_stroke(cr);

    cairo_set_line_width (cr, 0.3);
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1);
    for (size_t i = 0; i < n; ++i)
    {
        draw_handle(cr, D[i]);
    }
    cairo_stroke(cr);

    ConvexHull ch(D);
    std::vector<Point> chD;
    chD.reserve(ch.boundary.size());
    std::vector<Point>::iterator it = ch.boundary.begin();
    while (*(it++) != D[0])
    {}
    --it;
    chD.insert(chD.begin(), it, ch.boundary.end());
    chD.insert(chD.end(), ch.boundary.begin(), it);

    cairo_set_line_width (cr, 0.4);
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1);
    cairo_move_to(cr, chD[0]);
    draw_text(cr, chD[0], "d0");
    for (size_t i = 1; i < chD.size(); ++i)
    {
        cairo_line_to(cr, chD[i]);
        if (i == 1)
            draw_text(cr, chD[i], "d1");
    }
    cairo_line_to(cr, chD[0]);
    cairo_stroke(cr);
}



class CurveIntersect : public Toy
{

    void draw( cairo_t *cr, std::ostringstream *notify,
               int width, int height, bool save )
    {
        cairo_set_line_width (cr, 0.3);
        cairo_set_source_rgba (cr, 0.8, 0., 0, 1);
        D2<SBasis> A = pshA.asBezier();
        cairo_md_sb(cr, A);
        cairo_stroke(cr);
        cairo_set_source_rgba (cr, 0.0, 0., 0, 1);
        D2<SBasis> B = pshB.asBezier();
        cairo_md_sb(cr, B);
        cairo_stroke(cr);

        Interval domA(0,1);
        Interval domB(0,1);
        std::vector<Interval> domsA, domsB;
        detail::iterate(domsA, domsB, pshA.pts, pshB.pts, domA, domB);
        assert (domsA.size() == domsB.size());
        for (size_t i = 0; i < domsA.size(); ++i)
        {
            Interval& domA = domsA[i];
            Interval& domB = domsB[i];
            std::cerr << i << " : domA : " << domA.min() << ", " << domA.max() << std::endl;
            std::cerr << i << " : domB : " << domB.min() << ", " << domB.max() << std::endl;

//            Rect rectA(A(domA.min()), A(domA.max()));
//            Rect rectB(B(domB.min()), B(domB.max()));
//
//            cairo_set_line_width (cr, 0.3);
//            cairo_set_source_rgba (cr, 0.8, 0.0, 0, 1);
//            cairo_rectangle(cr, rectA);
//            cairo_stroke(cr);
//            cairo_set_source_rgba (cr, 0.0, 0.0, 0, 1);
//            cairo_rectangle(cr, rectB);
//            cairo_stroke(cr);

            cairo_set_line_width (cr, 0.3);
            cairo_set_source_rgba (cr, 0.0, 0.0, 0.7, 1);
            draw_handle(cr, A(domA.middle()));
            draw_handle(cr, B(domB.middle()));
            cairo_stroke(cr);
        }

//        std::vector<double> bl;
//        detail::base_line(bl, pshA.pts);
//
//        Interval mm;
//        detail::fat_line_bounds(mm, pshA.pts, bl);
//
//        Interval dom;
//        detail::clip(dom, pshB.pts, bl, mm);
//        std::cerr << "lt = " << dom.min() << std::endl;
//        std::cerr << "rt = " << dom.max() << std::endl;
//
//        if (dom.min() <= dom.max())
//        {
//            pshC.pts = pshB.pts;
//            detail::portion(pshC.pts, dom.min(), dom.max());
//            cairo_set_line_width (cr, 0.6);
//            cairo_set_source_rgba (cr, 0.0, 0.0, 0.8, 1);
//            D2<SBasis> C = pshC.asBezier();
//            cairo_md_sb(cr, C);
//            cairo_stroke(cr);
//        }

//        draw_distance(cr, pshB.pts, bl, mm);


//        cairo_set_line_width (cr, 0.3);
//        cairo_set_source_rgba (cr, 0.0, 0.0, 0.8, 1);
//        draw_line(cr, bl, 0, 0, width, height);
//        bl[2] -= mm.min();
//        draw_line(cr, bl, 0, 0, width, height);
//        bl[2] -= (mm.extent());
//        draw_line(cr, bl, 0, 0, width, height);
//        bl[2] += mm.max();
//        cairo_stroke(cr);

//        intersect(xs, A, B);
//        for (size_t i = 0; i < xs.size(); ++i)
//        {
//            draw_handle(cr, B(xs[i].tb));
//        }

        Toy::draw(cr, notify, width, height, save);
    }


public:
    CurveIntersect(unsigned int _A_bez_ord, unsigned int _B_bez_ord)
        : A_bez_ord(_A_bez_ord), B_bez_ord(_B_bez_ord)
    {
        handles.push_back(&pshA);
        for (unsigned int i = 0; i <= A_bez_ord; ++i)
            pshA.push_back(Geom::Point(uniform()*400, uniform()*400)+Point(200,200));
        handles.push_back(&pshB);
        for (unsigned int i = 0; i <= B_bez_ord; ++i)
            pshB.push_back(Geom::Point(uniform()*400, uniform()*400)+Point(200,200));

    }

private:
    unsigned int A_bez_ord, B_bez_ord;
    PointSetHandle pshA, pshB, pshC;
};


int main(int argc, char **argv)
{
    unsigned int A_bez_ord = 4;
    unsigned int B_bez_ord = 6;
    if(argc > 1)
        sscanf(argv[1], "%d", &A_bez_ord);
    if(argc > 2)
        sscanf(argv[2], "%d", &B_bez_ord);


    init( argc, argv, new CurveIntersect(A_bez_ord, B_bez_ord), 800, 800);
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


