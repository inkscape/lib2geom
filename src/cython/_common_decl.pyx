cdef object wrap_vector_double(vector[double] v):
    r = [] 
    cdef unsigned int i
    for i in range(v.size()):
        r.append(v[i])
    return r

cdef vector[double] make_vector_double(object l):
    cdef vector[double] ret
    for i in l:
        ret.push_back( float(i) )
    return ret
