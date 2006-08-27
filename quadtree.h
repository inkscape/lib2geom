#include <vector>

class Quad{
public:
    Quad* children[4];
    std::vector<int> data;
    Quad() {
        for(int i = 0; i < 4; i++)
            children[i] = 0;
    }
};

class QuadTree{
public:
    Quad* root;
    double scale;
    double bx0, bx1;
    double by0, by1;

    QuadTree() : root(0), scale(1) {}
    
    Quad* search(double x0, double y0, double x1, double y1) {
        Quad *q = root;
        
        double bxx0 = bx1, bxx1 = bx1;
        double byy0 = by1, byy1 = by1;
        while(q) {
            double cx = (bxx0 + bxx1)/2;
            double cy = (byy0 + byy1)/2;
            unsigned i = 0;
            if(x0 >= cx) {
                i += 1;
                bxx0 = cx; // zoom in a quad
            } else if(x1 <= cx) {
                bxx1 = cx;
            } else
                break;
            if(y0 >= cy) {
                i += 2;
                byy0 = cy;
            } else if(y1 <= cy) {
                byy1 = cy;
            } else
                break;
            
            assert(i < 4);
            Quad *qq = q->children[i];
            if(qq == 0)
                break; // last non-null
            q = qq;
        }
        return q;
    }
    
    void
    insert(double x0, double y0, double x1, double y1, int shape) {
        // loop until a quad would break the box.
        if(root == 0) {
            root = new Quad;
        }
        Quad *q = root;
        
        double bxx0 = bx0, bxx1 = bx1;
        double byy0 = by0, byy1 = by1;
        while(q) {
            double cx = (bxx0 + bxx1)/2;
            double cy = (byy0 + byy1)/2;
            unsigned i = 0;
            assert(x0 >= bxx0);
            assert(x1 <= bxx1);
            assert(y0 >= byy0);
            assert(y1 <= byy1);
            if(x0 >= cx) {
                i += 1;
                bxx0 = cx; // zoom in a quad
            } else if(x1 <= cx) {
                bxx1 = cx;
            } else
                break;
            if(y0 >= cy) {
                i += 2;
                byy0 = cy;
            } else if(y1 <= cy) {
                byy1 = cy;
            } else
                break;
            
            assert(i < 4);
            Quad *qq = q->children[i];
            if(qq == 0) {
                qq = new Quad;
                q->children[i] = qq;
            }
            q = qq;
        }
        q->data.push_back(shape);
    }
};


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(substatement-open . 0))
  indent-tabs-mode:nil
  c-brace-offset:0
  fill-column:99
  End:
  vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/

