#include <iostream>
#include <2geom/path.h>
#include <2geom/svg-path-parser.h>
#include <2geom/pathvector.h>
#include <2geom/sbasis-geometric.h>

#define SWEEP_GRAPH_DEBUG
#include <2geom/toposweep.cpp>

#include <cstdlib>

#include <2geom/toys/path-cairo.h>
#include <2geom/toys/toy-framework-2.h>

#include <algorithm>
#include <queue>
#include <functional>
#include <limits>

using namespace Geom;

void draw_node(cairo_t *cr, Point h) {
    int x = int(h[Geom::X]);
    int y = int(h[Geom::Y]);
    cairo_new_sub_path(cr);
    cairo_arc(cr, x, y, 2, 0, M_PI*2);
}

void draw_section(cairo_t *cr, Section const &s, PathVector const &ps) {
    Curve *curv = s.get_portion(ps);
    cairo_curve(cr, *curv);
    draw_node(cr, curv->initialPoint());
    draw_node(cr, curv->finalPoint());
    cairo_stroke(cr);
    delete curv;
}

void draw_graph(cairo_t *cr, Graph const &vertices) {
    for(unsigned i = 0; i < vertices.size(); i++) {
        cairo_set_source_rgba(cr, colour::from_hsl(i*0.5, 1, 0.5, 0.75));
        for(unsigned j = 0; j < vertices[i]->enters.size(); j++) {
            draw_ray(cr, vertices[i]->avg, 10*unit_vector(vertices[i]->enters[j].other->avg - vertices[i]->avg));
            cairo_stroke(cr);
        }
        for(unsigned j = 0; j < vertices[i]->exits.size(); j++) {
            draw_ray(cr, vertices[i]->avg, 20*unit_vector(vertices[i]->exits[j].other->avg - vertices[i]->avg));
            cairo_stroke(cr);
        }
    }
}

unsigned find_vert(Graph const &vertices, Vertex const *v) {
    return std::find(vertices.begin(), vertices.end(), v) - vertices.begin();
}

void write_graph(Graph const &vertices) {
    for(unsigned i = 0; i < vertices.size(); i++) {
        std::cout << i << " " << vertices[i]->avg << " [";
        for(unsigned j = 0; j < vertices[i]->enters.size(); j++)
            std::cout << find_vert(vertices, vertices[i]->enters[j].other) << ", ";
        std::cout  << "|";
        for(unsigned j = 0; j < vertices[i]->exits.size(); j++)
            std::cout << find_vert(vertices, vertices[i]->exits[j].other) << ", ";
        std::cout << "]\n";
    }
    std::cout << "=======\n";
}

void draw_edges(cairo_t *cr, std::vector<Edge> const &edges, PathVector const &ps, double t) {
    for(unsigned j = 1; j < edges.size(); j++) {
        const Section * const s1 = edges[j-1].section,
                       * const s2 = edges[j].section;
        Point fp = s1->curve.get(ps)(lerp(t, s1->f, s1->t));
        Point tp = s2->curve.get(ps)(lerp(t, s2->f, s2->t));
        draw_ray(cr, fp, tp - fp);
        cairo_stroke(cr);
    }
}

void draw_edge_orders(cairo_t *cr, std::vector<Vertex*> const &vertices, PathVector const &ps) {
    for(unsigned i = 0; i < vertices.size(); i++) {
        cairo_set_source_rgba(cr, colour::from_hsl(i*0.5, 1, 0.5, 0.75));
        draw_edges(cr, vertices[i]->enters, ps, 0.6);
        draw_edges(cr, vertices[i]->exits, ps, 0.4);
    }
}

void draw_areas(cairo_t *cr, PathVector const &pa, Areas const &areas) {
    PathVector ps = areas_to_paths(pa, areas);
    for(unsigned i = 0; i < ps.size(); i++) {
        double area;
        Point centre;
        Geom::centroid(ps[i].toPwSb(), centre, area);
        double d = 5.;
        if(area < 0) cairo_set_dash(cr, &d, 1, 0);
        cairo_path(cr, ps[i]);
        cairo_stroke(cr);
        cairo_set_dash(cr, &d, 0, 0);
    }
}

class SweepWindow: public Toy {
    vector<Path> path, path2;
    std::vector<Toggle> toggles;
    PointHandle p, p2;
    std::vector<colour> colours;
    virtual void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save, std::ostringstream *timer_stream) {

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_set_line_width(cr, 3);

        monoss.clear();
        contexts.clear();
        chopss.clear();
        
        vector<Path> pa = path;
        concatenate(pa, path2 + p2.pos);
        
        Graph output = sweep_graph(pa,X);
        
        int cix = (int) p.pos[X] / 10;
        if(cix >= 0 && cix < (int)contexts.size()) {
            while(colours.size() < contexts[cix].size()) {
                double c = colours.size();
                colours.push_back(colour::from_hsl(c*0.5, 1, 0.5, 0.75));
            }
            /* for(unsigned i = 0; i < contexts[cix].size(); i++) {
                cairo_set_source_rgba(cr, colours[i]);
                draw_section(cr, contexts[cix][i], pa);
                draw_number(cr, contexts[cix][i].curve.get(pa)
                                ((contexts[cix][i].t + contexts[cix][i].f) / 2), i);
                cairo_stroke(cr);
            } */
            cairo_set_source_rgba(cr, 0,0,0,1);
            /* for(unsigned i = 0; i < monoss[cix].size(); i++) {
                draw_section(cr, monoss[cix][i], path);
                cairo_fill(cr);
            } */
            /* cairo_set_source_rgba(cr,0,0,0,0.5);
            cairo_set_line_width(cr, 10);
            std::cout << "!!!!!!!!!!! " << chopss[cix].size() << std::endl;
            for(unsigned i = 0; i < chopss[cix].size(); i++) {
                draw_section(cr, chopss[cix][i], path);
                cairo_stroke(cr);
            } */
        }
        
        *notify << cix << std::endl;
        
        //draw_graph(cr, output);
        
        cairo_set_line_width(cr, 1);
        //draw_edge_orders(cr, output, pa);
        
        //remove_vestigial_verts(output);
        double_whiskers(output);
        write_graph(output);
        Areas areas = traverse_areas(output);
        remove_area_whiskers(areas);
        areas = filter_areas(pa, areas, UnionOp(path.size(), false, false));
        
        draw_areas(cr, pa, areas);
        /*for(unsigned i = 0; i < areas.size(); i++) {
            for(unsigned j = 0; j < areas[i].size(); j++) {
                std::cout << areas[i][j] << ", ";
            }
            std::cout << std::endl;
        }*/
        
        /*std::vector<std::vector<Curve*> > curves = unio(path, true, path2 + p2.pos, true);
        
        for(unsigned i = 0; i < curves.size(); i++) {
            for(unsigned j = 0; j < curves[i].size(); j++) {
                cairo_curve(cr, *curves[i][j]);
                cairo_stroke(cr);
            }
        } */
        
        
        //unsigned area_ix = cix < (int)areas.size() ? cix : areas.size() - 1;
        /*cairo_set_line_width(cr, 5);
        cairo_set_source_rgba(cr, 1, 1,0,1);
        for(unsigned area_ix = 0; area_ix < areas.size(); area_ix++) {
            for(unsigned i = 0; i < areas[area_ix].size(); i++) {
                draw_section(cr, *areas[area_ix][i], pa);
            }
        }*/
        
        /*
        cairo_set_line_width(cr, 1);
        cairo_set_source_rgba(cr, 0, 0, 0, 1);
        for(unsigned i = 0; i < visiteds[area_ix].size(); i++) {
            for(unsigned j = 0; j < visiteds[area_ix][i].size(); j++) {
                if(visiteds[area_ix][i][j]) {
                    const Section *s = output[i]->lookup(j).section;
                    draw_ray(cr, output[i]->avg, lerp(0.5, s->curve.get(pa)(lerp(0.35, s->f, s->t)) - output[i]->avg, Point()));
                    cairo_stroke(cr);
                }
            }
        } */
        
        free_graph(output);
        
        /*
        std::vector<Edge> sects = fill(output, X);
        for(unsigned i = 0; i < sects.size(); i++) {
            cairo_stroke(cr);
            Section s = output.sections[sects[i].section];
            draw_section(cr, s, path);
            draw_number(cr, s.curve.get(path)((s.f + s.t) / 2), i);
        } */
        
        Toy::draw(cr, notify, width, height, save,timer_stream);
    }

    void mouse_pressed(GdkEventButton* e) {
        toggle_events(toggles, e);
        Toy::mouse_pressed(e);
    }
    
    void key_hit(GdkEventKey* e) {
        if(e->keyval == 'a') p.pos[X] = 0;
        else if(e->keyval == '[') p.pos[X] -= 10;
        else if(e->keyval == ']') p.pos[X] += 10;
        if (p.pos[X] < 0) {
            p.pos[X] = 0;
        }
        redraw();
    }
    
    public:
    SweepWindow () {}
    void first_time(int argc, char** argv) {
        const char *path_name="sanitize_examples.svgd",
                     *path2_name="sanitize_examples.svgd";
        if(argc > 1)
            path_name = argv[1];
        if(argc > 2)
            path2_name = argv[2];
        path = read_svgd(path_name); //* Scale(3);
        path2 = read_svgd(path2_name);
        OptRect bounds = bounds_exact(path);
        if(bounds) path += Point(10,10)-bounds->min();
        bounds = bounds_exact(path2);
        if(bounds) path2 += Point(10,10)-bounds->min();
        p = PointHandle(Point(100,300));
        handles.push_back(&p);
        p2 = PointHandle(Point(200,300));
        handles.push_back(&p2);
    }
};

int main(int argc, char **argv) {
    init(argc, argv, new SweepWindow());
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4:encoding=utf-8:textwidth=99 :
