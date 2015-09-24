#!/usr/bin/python

from Numeric import *
from LinearAlgebra import *

pascals_triangle = []
rows_done = 0

def choose(n, k):
    r = 1
    for i in range(1,k+1):
        r *= n-k+i
        r /= i
    return r

# http://www.research.att.com/~njas/sequences/A109954
def T(n, k):
    return ((-1)**(n+k))*choose(n+k+2, 2*k+2)

def inver(q):
    result = zeros((q+2,q+2))
    q2 = q/2+1
    for i in range(q2):
        for j in range(i+1):
            val = T(i,j)
            result[q/2-j][q/2-i] = val
            result[q/2+j+2][q/2+i+2] = val
            result[q/2+j+2][q/2-i-1] = -val
            if q/2+i+3 < q+2:
                result[q/2-j][q/2+i+3] = -val
            
    for i in range(q+2):
        result[q2][i] = [1,-1][(i-q2)%2]
    return result
        
def simple(q):
    result = zeros((q+2,q+2))
    for i in range(q/2+1):
        for j in range(q+1):
            result[j][i] = choose(q-2*i, j-i)
            result[j+1][q-i+1] = choose(q-2*i, j-i)
    result[q/2+1][q/2+1] = 1
    return result

print "The aim of the game is to work out the correct indexing to make the two matrices match :)"

s = simple(4)
si = floor(inverse(s)+0.5)
print si.astype(Int)
print inver(4)
exit(0)
print "<html><head><title></title></head><body>"

def arrayhtml(a):
    s = "<table>"
    r,c = a.shape
    for i in range(r):
        s += "<tr>";
        for j in range(c):
            s += "<td>%g</td>" % a[i,j]
        s += "</tr>"
    s += "</table>"
    return s

for i in [21]:#range(1,13,2):
    s = simple(i)
    print "<h1>T<sup>-1</sup> = </h1>"
    print arrayhtml(s)
    print "<h1>T = </h1>"
    print arrayhtml(floor(inverse(s)+0.5))

print "</body></html>"
