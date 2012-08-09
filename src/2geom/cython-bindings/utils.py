from Tkinter import *
from cy2geom import Path


def draw(c, dt=0.001, batch=10, scale=20, x_offset = 400, y_offset = 300):
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
