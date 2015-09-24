from Tkinter import *

import math

import cy2geom
from cy2geom import Point, Path


def Nagon(N):
    """Return N-agon with side of length 1."""
    side = cy2geom.LineSegment(Point(-0.5, 0), Point(0.5, 0))
    angle = 2*math.pi/N
    distance_to_center = 0.5 / math.tan(math.pi/N)
    return Path.fromList(
        [ side.transformed( 
            cy2geom.Translate(Point(0, -distance_to_center))*
            cy2geom.Rotate(angle*i)) 
            for i in range(N)
        ], 
        stitching = Path.STITCH_DISCONTINUOUS, 
        closed = True )

def draw(c, dt=0.001, batch=10, scale=20, x_offset = 400, y_offset = 300):
    """Draw curve or path."""
    master = Tk()
    w = Canvas(master, width=800, height=600)
    w.pack()
    n = 0
    t = 0
    points = []
    
    if isinstance(c, Path):
        maxt = c.size_default()
    else:
        maxt = 1
    
    while (t < maxt):
        t = n*dt
        n+=1
        p = c(t)
        points.extend( [ p.x * scale + x_offset, p.y * scale + y_offset] )
        
    while points:
        draw_points = tuple(points[:batch*2])
        if len(points) == 2:
            break
        del points[:batch*2]
        
        l = w.create_line(*draw_points)
        w.grid()

    master.mainloop()
