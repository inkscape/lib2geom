#!/usr/bin/python

def choose(n, k):
    r = 1
    for i in range(1, k+1):
        r = (r*(n-k+i))/i
    return r

def W(n, j, k):
    q = (n+1)/2.
    if((n & 1) == 0 and j == q and k == q):
        return 1
    if(k > n-k):
        return W(n, n-j, n-k)
    assert((k <= q))
    if(k >= q):
        return 0
    # assert(!(j >= n-k));
    if(j >= n-k):
        return 0
    # assert(!(j < k));
    if(j < k):
        return 0
    return float(choose(n-2*k-1, j-k)) / choose(n,j);

# this produces a degree 2q bezier from a degree k sbasis
def sbasis_to_bezier(B, q):
    if(q == 0):
        q = len(B)
    n = q*2
    result = [0.0 for i in range(n)]
    if(q > len(B)):
        q = len(B)
    n -= 1
    for k in range(q):
        for j in range(n-k+1):
            result[j] += (W(n, j, k)*B[k][0] +
                          W(n, n-j, k)*B[k][1])
    return result

def lerp(a, b, t):
    return (1-t)*a + t*b
