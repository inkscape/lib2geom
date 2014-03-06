import gdb.printing
import math

def xstr(val):
    if val.is_optimized_out:
        return "<optimized out>"
    else:
        return str(val)

class GeomPointPrinter:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        return "(" + xstr(self.val["_pt"][0]) + ", " + xstr(self.val["_pt"][1]) + ")"

class GeomIntervalPrinter:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        return "[" + xstr(self.val["_b"][0]) + ", " + xstr(self.val["_b"][1]) + ")"

class GeomOptIntervalPrinter:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        if self.val["m_initialized"]:
            addr = self.val["m_storage"]["dummy_"]["data"].address
            if self.val.type == gdb.lookup_type('Geom::OptInterval'):
                return str(addr.cast(gdb.lookup_type('Geom::Interval').pointer()).dereference())
            else:
                return str(addr.cast(gdb.lookup_type('Geom::GenericInterval<int>').pointer()).dereference())
        else:
            return "empty"

class GeomRectPrinter:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        return str(self.val["f"][0]) + " x " + str(self.val["f"][1])

class GeomOptRectPrinter:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        if self.val["m_initialized"]:
            addr = self.val["m_storage"]["dummy_"]["data"].address
            if self.val.type == gdb.lookup_type('Geom::OptRect'):
                return str(addr.cast(gdb.lookup_type('Geom::Rect').pointer()).dereference())
            else:
                return str(addr.cast(gdb.lookup_type('Geom::GenericRect<int>').pointer()).dereference())
        else:
            return "empty"

class GeomAffinePrinter:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        return ("\n | %8f  %8f  0 |" +
                "\n | %8f  %8f  0 |" +
                "\n | %8f  %8f  1 |") % tuple(self.val["_c"][x] for x in range(0,6))

class GeomTranslatePrinter:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        return "translate by " + str(self.val["vec"])

class GeomScalePrinter:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        return "scale by " + str(self.val["vec"])

class GeomRotatePrinter:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        x = self.val["vec"][0]
        y = self.val["vec"][1]
        angle = math.atan2(y, x) / math.pi * 180.0
        return "rotate by %s = %f degrees" % (str(self.val["vec"]), angle)

class GeomZoomPrinter:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        return "zoom %f%% at %s" % (self.val["_scale"] * 100, str(self.val["_trans"]))


def lib2geom_pretty_printer():
	pp = gdb.printing.RegexpCollectionPrettyPrinter("lib2geom")
	pp.add_printer('Geom::Point', '^Geom::(Int)?Point$', GeomPointPrinter)
	pp.add_printer('Geom::Interval', '^(Geom::Interval|Geom::GenericInterval<int>)$', GeomIntervalPrinter)
	pp.add_printer('Geom::OptInterval', '^(Geom::OptInterval|Geom::GenericOptInterval<int>)$', GeomOptIntervalPrinter)
	pp.add_printer('Geom::Rect', '^(Geom::Rect|Geom::GenericRect<int>)$', GeomRectPrinter)
	pp.add_printer('Geom::OptRect', '^(Geom::OptRect|Geom::GenericOptRect<int>)$', GeomOptRectPrinter)
	pp.add_printer('Geom::Affine', '^Geom::Affine$', GeomAffinePrinter)
	pp.add_printer('Geom::Translate', '^Geom::Translate$', GeomTranslatePrinter)
	pp.add_printer('Geom::Scale', '^Geom::Scale', GeomScalePrinter)
	pp.add_printer('Geom::Rotate', '^Geom::Rotate$', GeomRotatePrinter)
	pp.add_printer('Geom::Zoom', '^Geom::Zoom$', GeomZoomPrinter)
	return pp

def register_lib2geom_printers():
	gdb.printing.register_pretty_printer(
		gdb.current_objfile(),
		lib2geom_pretty_printer())
