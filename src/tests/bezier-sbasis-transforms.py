#!/usr/bin/python

from Numeric import *
from LinearAlgebra import *

pascals_triangle = []
rows_done = 0

def choose(n, k):
    # indexing is (0,0,), (1,0), (1,1), (2, 0)...
    # to get (i, j) i*(i+1)/2 + j
    global rows_done,pascals_triangle
    if(k < 0 or k > n):
        return 0;
    if(rows_done <= n):
        if(rows_done == 0):
            pascals_triangle.append(1);
            rows_done = 1;
        while(rows_done <= n):
            p = len(pascals_triangle) - rows_done;
            pascals_triangle.append(1);
            for i in range(rows_done-1):
                pascals_triangle.append(pascals_triangle[p] 
                                        + pascals_triangle[p+1]);
		p+=1
            pascals_triangle.append(1);
            rows_done +=1;
    row = (n*(n+1))/2;
    return pascals_triangle[row+k];

# http://www.research.att.com/~njas/sequences/A109954
def T(n, k):
    return ((-1)**(n+k))*choose(n+k+2, 2*k+2)

def inver(q):
    result = zeros((q+2,q+2))
    q2 = q/2+1
    for i in range(q2):
        for j in range(i+1):
            result[q/2-j][q/2-i] = T(i, j)
            result[q/2+j+2][q/2-i-1] = -T(i, j)
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

s = simple(7)
si = floor(inverse(s)+0.5)
print si.astype(Int)
print inver(7)
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
