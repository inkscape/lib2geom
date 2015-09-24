/*
 * Show off crossings between two D2<SBasis> curves.
 * The intersection points are found by using implicitization tecnique.
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


#include <toys/path-cairo.h>
#include <toys/toy-framework-2.h>
#include <2geom/d2.h>
#include <2geom/sbasis-poly.h>
#include <2geom/numeric/linear_system.h>
#include <2geom/symbolic/implicit.h>


using namespace Geom;

/*
 * helper routines
 */
void poly_to_mvpoly1(SL::MVPoly1 & p, Geom::Poly const& q)
{
    for (size_t i = 0; i < q.size(); ++i)
    {
        p.coefficient(i, q[i]);
    }
    p.normalize();
}

void mvpoly1_to_poly(Geom::Poly & p, SL::MVPoly1 const& q)
{
    p.resize(q.get_poly().size());
    for (size_t i = 0; i < q.get_poly().size(); ++i)
    {
        p[i] = q[i];
    }
}


/*
 * intersection_info
 * structure utilized to store intersection info
 *
 * p  - the intersection point
 * t0 - the parameter t value at which the first curve pass through p
 * t1 - the parameter t value at which the first curve pass through p
 */
struct intersection_info
{
    intersection_info()
    {}

    intersection_info(Point const& _p, Coord _t0, Coord _t1)
        : p(_p), t0(_t0), t1(_t1)
    {}

    Point p;
    Coord t0, t1;
};

typedef std::vector<intersection_info> intersections_info;



/*
 * intersection algorithm
 */
void intersect(intersections_info& xs,  D2<SBasis> const& A, D2<SBasis> const& B)
{
    using std::swap;

    // supposing implicitization the most expensive step
    // we perform a call to intersect with curve arguments swapped
    if (A[0].size() > B[0].size())
    {
        intersect(xs, B, A);
        for (size_t i = 0; i < xs.size(); ++i)
            swap(xs[i].t0, xs[i].t1);

        return;
    }

    // convert A from symmetric power basis to power basis
    Geom::Poly A0 = sbasis_to_poly(A[0]);
    Geom::Poly A1 = sbasis_to_poly(A[1]);

    // convert to MultiPoly type
    SL::MVPoly1 Af, Ag;
    poly_to_mvpoly1(Af, A0);
    poly_to_mvpoly1(Ag, A1);

    // compute a basis of the ideal related to the curve A
    // in vector form
    Geom::SL::basis_type b;
    // if we compute the micro-basis the bezout matrix is made up
    // by one only entry so we can't do the inversion step.
    if (A0.size() == 3)
    {
        make_initial_basis(b, Af, Ag);
    }
    else
    {
        microbasis(b, Af, Ag);
    }

    // we put the basis in of the form of two independent moving line
    Geom::SL::MVPoly3 p, q;
    basis_to_poly(p, b[0]);
    basis_to_poly(q, b[1]);

    // compute the Bezout matrix and the implicit equation of the curve A
    Geom::SL::Matrix<Geom::SL::MVPoly2> BZ = make_bezout_matrix(p, q);
    SL::MVPoly2 ic = determinant_minor(BZ);
    ic.normalize();


    // convert B from symmetric power basis to power basis
    Geom::Poly B0 = sbasis_to_poly(B[0]);
    Geom::Poly B1 = sbasis_to_poly(B[1]);

    // convert to MultiPoly type
    SL::MVPoly1 Bf, Bg;
    poly_to_mvpoly1(Bf, B0);
    poly_to_mvpoly1(Bg, B1);

    // evaluate the implicit equation of A on B
    // so we get an s(t) polynomial that give us
    // the t values for B at which intersection happens
    SL::MVPoly1 s = ic(Bf, Bg);

    // convert s(t) to Poly type, in order to use the real_solve function
    Geom::Poly z;
    mvpoly1_to_poly(z, s);

    // compute t values for the curve B at which intersection happens
    std::vector<double> sol = solve_reals(z);

    // filter the found solutions wrt the domain interval [0,1] of B
    // and compute the related point coordinates
    std::vector<double> pt;
    pt.reserve(sol.size());
    std::vector<Point> points;
    points.reserve(sol.size());
    for (size_t i = 0; i < sol.size(); ++i)
    {
        if (sol[i] >= 0 && sol[i] <= 1)
        {
            pt.push_back(sol[i]);
            points.push_back(B(pt.back()));
        }
    }

    // case: A is parametrized by polynomial of degree 1
    // we compute the t values of A at the intersection points
    // and filter the results wrt the domain interval [0,1]
    double t;
    xs.clear();
    xs.reserve(pt.size());
    if (A0.size() == 2)
    {
        for (size_t i = 0; i < points.size(); ++i)
        {
            t = (points[i][X] - A0[0]) / A0[1];
            if (t >= 0 && t <= 1)
            {
                xs.push_back(intersection_info(points[i], t, pt[i]));
            }
        }
        return;
    }

    // general case
    // we compute the value of the parameter t of A at each intersection point
    // and we filter the final result wrt the domain interval [0,1]
    // the computation is performed by using the inversion formula for each point
    // As reference see:
    // Sederberger - Computer Aided Geometric Design
    // par 16.5 - Implicitization and Inversion
    size_t n = BZ.rows();
    Geom::NL::Matrix BZN(n, n);
    Geom::NL::MatrixView BZV(BZN, 0, 0, n-1, n-1);
    Geom::NL::VectorView cv = BZN.column_view(n-1);
    Geom::NL::VectorView bv(cv, n-1);
    Geom::NL::LinearSystem ls(BZV, bv);
    for (size_t i = 0; i < points.size(); ++i)
    {
        // evaluate the first main minor of order n-1 at each intersection point
        polynomial_matrix_evaluate(BZN, BZ, points[i]);
        // solve the linear system with the powers of t as unknowns
        ls.SV_solve();
        // the last element contains the t value
        t = -ls.solution()[n-2];
        // filter with respect to the domain of A
        if (t >= 0 && t <= 1)
        {
            xs.push_back(intersection_info(points[i], t, pt[i]));
        }
    }
}



class IntersectImplicit : public Toy
{

    void draw( cairo_t *cr, std::ostringstream *notify,
               int width, int height, bool save, std::ostringstream *timer_stream)
    {
        cairo_set_line_width (cr, 0.3);
        cairo_set_source_rgba (cr, 0.8, 0., 0, 1);
        D2<SBasis> A = pshA.asBezier();
        cairo_d2_sb(cr, A);
        cairo_stroke(cr);
        cairo_set_source_rgba (cr, 0.0, 0., 0, 1);
        D2<SBasis> B = pshB.asBezier();
        cairo_d2_sb(cr, B);
        cairo_stroke(cr);

        intersect(xs, A, B);
        for (size_t i = 0; i < xs.size(); ++i)
        {
            draw_handle(cr, xs[i].p);
        }

        Toy::draw(cr, notify, width, height, save,timer_stream);
    }


public:
    IntersectImplicit(unsigned int _A_bez_ord, unsigned int _B_bez_ord)
        : A_bez_ord(_A_bez_ord), B_bez_ord(_B_bez_ord)
    {
        handles.push_back(&pshA);
        for (unsigned int i = 0; i <= A_bez_ord; ++i)
            pshA.push_back(Geom::Point(uniform()*400, uniform()*400));
        handles.push_back(&pshB);
        for (unsigned int i = 0; i <= B_bez_ord; ++i)
            pshB.push_back(Geom::Point(uniform()*400, uniform()*400));

    }

private:
    unsigned int A_bez_ord, B_bez_ord;
    PointSetHandle pshA, pshB;
    intersections_info xs;
};


int main(int argc, char **argv)
{
    unsigned int A_bez_ord = 4;
    unsigned int B_bez_ord = 6;
    if(argc > 1)
        sscanf(argv[1], "%d", &A_bez_ord);
    if(argc > 2)
        sscanf(argv[2], "%d", &B_bez_ord);


    init( argc, argv, new IntersectImplicit(A_bez_ord, B_bez_ord));
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
