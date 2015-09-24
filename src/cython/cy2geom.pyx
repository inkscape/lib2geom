#Axis specifiers for Dim2
X = 0
Y = 1

from _cy_primitives import cy_Angle as Angle
from _cy_primitives import cy_AngleInterval as AngleInterval
from _cy_primitives import cy_Point as Point
from _cy_primitives import cy_Line as Line
from _cy_primitives import cy_Ray as Ray
from _cy_primitives import cy_IntPoint as IntPoint


from _cy_rectangle import cy_Interval as Interval
from _cy_rectangle import cy_OptInterval as OptInterval
from _cy_rectangle import cy_IntInterval as IntInterval
from _cy_rectangle import cy_OptIntInterval as OptIntInterval

from _cy_rectangle import cy_GenericInterval as GenericInterval
from _cy_rectangle import cy_GenericOptInterval as GenericOptInterval

from _cy_rectangle import cy_GenericRect as GenericRect

from _cy_rectangle import cy_Rect as Rect
from _cy_rectangle import cy_OptRect as OptRect
from _cy_rectangle import cy_IntRect as IntRect
from _cy_rectangle import cy_OptIntRect as OptIntRect


from _cy_affine import cy_Affine as Affine
from _cy_affine import cy_Translate as Translate
from _cy_affine import cy_Rotate as Rotate
from _cy_affine import cy_VShear as VShear
from _cy_affine import cy_HShear as HShear
from _cy_affine import cy_Scale as Scale
from _cy_affine import cy_Zoom as Zoom

from _cy_affine import cy_Eigen as Eigen

from _cy_curves import cy_Curve as Curve

from _cy_curves import cy_Linear as Linear
from _cy_curves import cy_SBasis as SBasis
from _cy_curves import cy_SBasisCurve as SBasisCurve

from _cy_curves import cy_Bezier as Bezier
from _cy_curves import cy_BezierCurve as BezierCurve
from _cy_curves import cy_LineSegment as LineSegment
from _cy_curves import cy_QuadraticBezier as QuadraticBezier
from _cy_curves import cy_CubicBezier as CubicBezier

from _cy_curves import cy_HLineSegment as HLineSegment
from _cy_curves import cy_VLineSegment as VLineSegment

from _cy_curves import cy_EllipticalArc as EllipticalArc
#Wrap this? It doesn't fit into python's dynamic nature and 
#BezierCurve covers most of it's functionality 
#Maybe implement constructors for BezierCurve similar to those
#seen in BezierCurveN
#TODO
#from _cy_curves import cy_BezierCurveN as BezierCurveN

from _cy_curves import cy_lerp as lerp
from _cy_curves import cy_reverse as reverse
#~ from _cy_curves import cy_level_sets as level_sets


from _cy_path import cy_Path as Path


from _cy_conicsection import cy_Circle as Circle
from _cy_conicsection import cy_Ellipse as Ellipse
