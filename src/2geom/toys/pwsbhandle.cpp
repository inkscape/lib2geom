class PWSBHandle : public Handle{
public:
    unsigned handles_per_curve, curve_size, segs;
    PWSBHandle()  {}
    PWSBHandle(unsigned cs, unsigned segs) :handles_per_curve(cs*segs),curve_size(cs), segs(segs) {}
    std::vector<Geom::Point> pts;
    virtual void draw(cairo_t *cr, bool annotes = false);
  
    virtual void* hit(Geom::Point mouse);
    virtual void move_to(void* hit, Geom::Point om, Geom::Point m);
    void push_back(double x, double y) {pts.push_back(Geom::Point(x,y));}
    Piecewise<SBasis> value(double y_0=150) {
        Piecewise<SBasis> pws;
	Point* base = &pts[0];
	for(unsigned i = 0; i < handles_per_curve; i+=curve_size) {
	    pws.push_cut(base[i][0]);
	    //Bad hack to move 0 to 150
	    for(unsigned j = i; j < i + curve_size; j++)
		base[j] = Point(base[j][0], base[j][1] - y_0);
	    pws.push_seg( Geom::handles_to_sbasis(base+i, curve_size-1)[1]);
	    for(unsigned j = i; j < i + curve_size; j++)
		base[j] = Point(base[j][0], base[j][1] + y_0);
	}
	pws.push_cut(base[handles_per_curve - 1][0]);
	assert(pws.invariants());
	return pws;
    }
    virtual void load(FILE* f);
    virtual void save(FILE* f);
};

void PWSBHandle::draw(cairo_t *cr, bool /*annotes*/) {
    for(unsigned i = 0; i < pts.size(); i++) {
	draw_circ(cr, pts[i]);
    }
}

void* PWSBHandle::hit(Geom::Point mouse) {
    for(unsigned i = 0; i < pts.size(); i++) {
	if(Geom::distance(mouse, pts[i]) < 5)
	    return (void*)(&pts[i]);
    }
    return 0;
}

void PWSBHandle::move_to(void* hit, Geom::Point /*om*/, Geom::Point m) {
    if(hit) {
	*(Geom::Point*)hit = m;
	Point* base = &pts[0];
	for(unsigned i = curve_size; i < handles_per_curve; i+=curve_size) {
	    base[i-1][0] = base[i][0];
	}
	for(unsigned i = 0; i < handles_per_curve; i+=curve_size) {
	    for(unsigned j = 1; j < (curve_size-1); j++) {
                double t = float(j)/(curve_size-1);
		base[i+j][0] = (1 - t)*base[i][0] + t*base[i+curve_size-1][0];
            }
	}
    }
}

void PWSBHandle::load(FILE* f) {
    unsigned n = 0;
    assert(3 == fscanf(f, "%d %d %d\n", &curve_size, &segs, &n));
    assert(n == curve_size*segs);
    pts.clear();
    for(unsigned i = 0; i < n; i++) {
	pts.push_back(read_point(f));
    }
}

void PWSBHandle::save(FILE* f) {
    fprintf(f, "%d %d %lu\n", curve_size, segs, pts.size());
    for(unsigned i = 0; i < pts.size(); i++) {
	fprintf(f, "%lf %lf\n", pts[i][0], pts[i][1]);
    }
}
