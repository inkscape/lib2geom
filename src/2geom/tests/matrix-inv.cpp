#include <2geom/d2.h>
#include <2geom/transforms.h>

#include <2geom/path.h>
#include <2geom/svg-path-parser.h>

#include <vector>
using std::vector;
using namespace Geom;


int main() {
    //init(argc, argv, new Sb2d2);
    Geom::Affine test_matrix2[2] = {Geom::Affine( 0.001, 0.0, 0.0, 
                                                  0.001, 0.0, 0.0 ),
                                    Geom::Affine( 0.001, 0.001-1e-6, 0.001, 
                                                  0.001, 0.0, 0.0 )};
    for(int i = 0; i < 2; i++) {
        std::cout << "iteration: " << i << std::endl;
       Geom::Affine test_inverse2 = test_matrix2[i].inverse();
       std::cout << test_matrix2[i] << std::endl;
       std::cout << Point(0,0)*test_matrix2[i] << std::endl;
       std::cout << Point(1,0)*test_matrix2[i] << std::endl;
       std::cout << Point(0,1)*test_matrix2[i] << std::endl;
       std::cout << (Point(1,0)*test_matrix2[i])*test_inverse2 << std::endl;
       std::cout << (Point(0,1)*test_matrix2[i])*test_inverse2 << std::endl;
       std::cout << "det=" << test_matrix2[i].det() << std::endl;
       std::cout << test_inverse2 << std::endl;
       std::cout << test_inverse2*test_matrix2[i] << std::endl;
       std::cout << test_matrix2[i]*test_inverse2 << std::endl;
    }
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
