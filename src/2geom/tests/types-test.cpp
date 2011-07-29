#include <iostream>
#include <iomanip>
#include <2geom/2geom.h>

using namespace Geom;

#define TYPE_SIZE(T) \
	do { std::cout << std::setw(20) << #T << "  " << sizeof(T) << "\n"; } while(0)

int main() {
	std::cout << "Type sizes\n";
	TYPE_SIZE(Dim2);
	TYPE_SIZE(Coord);
	TYPE_SIZE(IntCoord);
	TYPE_SIZE(Point);
	TYPE_SIZE(Interval);
	TYPE_SIZE(OptInterval);
	TYPE_SIZE(IntInterval);
	TYPE_SIZE(OptIntInterval);
	TYPE_SIZE(Rect);
	TYPE_SIZE(OptRect);
	TYPE_SIZE(IntRect);
	TYPE_SIZE(OptIntRect);
	TYPE_SIZE(Affine);
	TYPE_SIZE(Path);
	TYPE_SIZE(PathVector);
	std::cout << std::endl;
	TYPE_SIZE(Curve);
	TYPE_SIZE(BezierCurve);
	TYPE_SIZE(LineSegment);
	TYPE_SIZE(QuadraticBezier);
	TYPE_SIZE(CubicBezier);
	TYPE_SIZE(SBasisCurve);
	TYPE_SIZE(EllipticalArc);
	TYPE_SIZE(SVGEllipticalArc);
	std::cout << std::endl;

	return 0;
}
