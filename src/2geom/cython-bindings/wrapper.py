import os
import sys
import argparse

from pygccxml import parser, declarations

class wType:
    def __init__(self, c_str):
        self.c_str = c_str
        self.wrapped_types = ["bint", "double", "int", "unsigned int",
                            "Coord", "IntCoord", "Dim2", "size_t"]

    def get_C(self):
        """returns plain c string"""
        return self.c_str

    def get_Cython_type(self):
        """Returns corresponding Cython type string"""
        typestr= self.get_C()
        typestr= typestr.replace(" const ", " ")
        if typestr.find("::Geom::")==0:
            typestr = typestr[len("::Geom::"):]
        if typestr == "bool":
            typestr = "bint"
        return typestr
        
    def get_Python_function_argument(self):
        """Returns argument type to Python function call"""

        typestr = self.get_Cython_type()
        if typestr[-1]=="&":
            typestr = typestr[:-2]
        if typestr in self.wrapped_types:
            return typestr+" "
        return "cy_"+typestr+" "
    def get_Python_return(self):
        """ Returns a string used to wrap a function returning this type
        returns a string to be called with format( 'function call to c function ')
        """
        typestr = self.get_Cython_type()
        
        if typestr[-1] == "&":
            typestr = typestr[:-2]
        if typestr in self.wrapped_types:
            return "return {}"
        if typestr == "void":
            return "{}"
        if typestr[-1] == "*":
            typestr = typestr[:-2]+"_p"
        return "return wrap_"+typestr+"({})"
        
    def get_wrap_method(self, delim):
        """Returns wrap method in a form of a list of lines"""
        ret = []
        typestr = self.get_Cython_type()
        
        if typestr[-1] == "&":
            typestr = typestr[:-2]
        if typestr in self.wrapped_types:
            return []
        if typestr == "void":
            return []
        if typestr[-1] == "*":
            typestr = typestr[:-2]+"_p"
        ret.append("cdef cy_{} wrap_{}({} p):".format(typestr, typestr, typestr))
        ret.append(delim+"cdef {} * retp = new {}()".format(typestr, typestr))
        ret.append(delim+"retp[0] = p")
        ret.append(delim+"cdef cy_{} r = cy_{}.__new__(cy_{})".format(typestr, typestr, typestr))
        ret.append(delim+"r.thisptr = retp")
        ret.append(delim+"return r")
        return ret
        
    def get_Python_pass_argument(self):
        """ Returns argument to be passed to C function, to be called with .format(arg_name)"""
        typestr = self.get_Cython_type()
        if typestr[-1]=="&":
            typestr = typestr[:-2]
        if typestr in self.wrapped_types:
            return "{}"
        if typestr[-1] == "*":
            return "{}.thisptr"
        return "deref( {}.thisptr )"

class CythonWrapper:
    def __init__(self, gn):
        self.global_namespace = gn
        self.delim = '    '
    
    def wrap_constructor(self, constructor, types_dict):
        pxd_lines = []
        pyx_lines = []
        
        pxd_arguments = []
        for argument_type in constructor.argument_types:
            pxd_arguments.append( types_dict[argument_type.decl_string].get_Cython_type() )
        pxd_argument_str = "({})".format(", ".join(pxd_arguments))
        
        pxd_function_str = constructor.name+pxd_argument_str
        pxd_lines.append(self.delim*2+pxd_function_str)
        
        #python function
        arguments_python = ["self"]
        for argument in constructor.arguments:
            arguments_python.append(types_dict[argument.type.decl_string].get_Python_function_argument()+argument.name)
        declaration = "def {}".format("__init__")+"({}):".format(", ".join(arguments_python))
        
        arguments_cython = []
        for argument in constructor.arguments:
            arguments_cython.append(types_dict[argument.type.decl_string].get_Python_pass_argument().format(argument.name))
        function_call = "new "+constructor.name+"({})".format(", ".join(arguments_cython))
        
        return_statement = "self.thisptr = {}".format(function_call)
        pyx_lines.append(self.delim+declaration)
        pyx_lines.append(self.delim*2 + return_statement)
        
        return (pxd_lines, pyx_lines)

    def wrap_operator(self, operator, types_dict):
        pxd_lines = []
        pyx_lines = []

        pxd_arguments = []
        for argument_type in operator.argument_types:
            pxd_arguments.append( types_dict[argument_type.decl_string].get_Cython_type() )
        pxd_argument_str = "({})".format(", ".join(pxd_arguments))
        boost_wrapped = ["operator{}=".format(i) for i in ["+", "-", "*", "/", "|", "&"]]
        pxd_function_str = types_dict[operator.return_type.decl_string].get_Cython_type()+\
                            " "+operator.name+pxd_argument_str
        
        if not (operator.name in boost_wrapped):
            pxd_lines.append(self.delim*2+pxd_function_str)

        else:
            pxd_lines.append(self.delim*2+"#"+pxd_function_str)

            pxd_function_str = types_dict[operator.return_type.decl_string].get_Cython_type()+\
            " "+operator.name[:-1]+pxd_argument_str
            pxd_lines.append(self.delim*2+pxd_function_str)


        special_methods = {
            "+" : "__add__",
            "-" : "__sub__",
            "*" : "__mul__",
            "/" : "__div__",
            "|" : "__or__",
            "&" : "__and__",
            "[]": "__getitem__",
            "()": "__call__",
            "==": "__richcmp__",
            "!=": "__richcmp__"
        }
        
        if operator.name in boost_wrapped:
            operation = operator.name[-2]
        else:
            operation = operator.name[ len("operator"): ]
            
        if operation in special_methods:
            python_name = special_methods[operation]
        else:
            python_name = operator.name
        
        arguments_python = ["self"]
        for argument in operator.arguments:
            arguments_python.append(types_dict[argument.type.decl_string].get_Python_function_argument()+argument.name)
        declaration = "def {}".format(python_name)+"({}):".format(", ".join(arguments_python))
        
        arguments_cython = []
        for argument in operator.arguments:
            arguments_cython.append(types_dict[argument.type.decl_string].get_Python_pass_argument().format(argument.name))
        function_call = "deref( self.thisptr ) "+operation+" {}".format(", ".join(arguments_cython))
        
        return_statement = types_dict[operator.return_type.decl_string].get_Python_return().format(function_call)
        
        if (operator.arguments == []) and (operation == '-'):
            declaration = "def __neg__(self):"
        pyx_lines.append(self.delim+declaration)
        pyx_lines.append(self.delim*2 + return_statement)

        return (pxd_lines, pyx_lines)

    def wrap_method(self, method, types_dict):
        pxd_lines = []
        pyx_lines = []
        
        pxd_arguments = []
        for argument_type in method.argument_types:
            pxd_arguments.append( types_dict[argument_type.decl_string].get_Cython_type() )
        pxd_argument_str = "({})".format(", ".join(pxd_arguments))
        
        
        pxd_function_str = types_dict[method.return_type.decl_string].get_Cython_type()+\
                            " "+method.name+pxd_argument_str
        pxd_lines.append(self.delim*2+pxd_function_str)
        
        #python function
        arguments_python = ["self"]
        for argument in method.arguments:
            arguments_python.append(types_dict[argument.type.decl_string].get_Python_function_argument()+argument.name)
        declaration = "def {}".format(method.name)+"({}):".format(", ".join(arguments_python))
        
        arguments_cython = []   
        for argument in method.arguments:
            arguments_cython.append(types_dict[argument.type.decl_string].get_Python_pass_argument().format(argument.name))
        function_call = "self.thisptr."+method.name+"({})".format(", ".join(arguments_cython))
        
        return_statement = types_dict[method.return_type.decl_string].get_Python_return().format(function_call)
        pyx_lines.append(self.delim+declaration)
        pyx_lines.append(self.delim*2 + return_statement)
        
        return (pxd_lines, pyx_lines)
    
    def wrap_free_function(self, function, types_dict):
        pxd_lines = []
        pyx_lines = []
        decl_lines = []
                                
        pxd_arguments = []
        for argument_type in function.argument_types:
            pxd_arguments.append( types_dict[argument_type.decl_string].get_Cython_type() )
        pxd_argument_str = "({})".format(", ".join(pxd_arguments))
        
        
        pxd_function_str = types_dict[function.return_type.decl_string].get_Cython_type()+\
                            " "+function.name+pxd_argument_str
        pxd_lines.append(self.delim+pxd_function_str)
        
        #python function
        arguments_python = []
        for argument in function.arguments:
            arguments_python.append(types_dict[argument.type.decl_string].get_Python_function_argument()+argument.name)
        declaration = "def cy_{}".format(function.name)+"({}):".format(", ".join(arguments_python))
        
        decl_lines.append("from _cy_??? import cy_{0} as {0}".format(function.name))
        
        arguments_cython = []   
        for argument in function.arguments:

            arguments_cython.append(types_dict[argument.type.decl_string].get_Python_pass_argument().format(argument.name))
        function_call = function.name+"({})".format(", ".join(arguments_cython))
        
        return_statement = types_dict[function.return_type.decl_string].get_Python_return().format(function_call)
        pyx_lines.append(declaration)
        pyx_lines.append(self.delim + return_statement)
        
        return (pxd_lines, pyx_lines, decl_lines)

    
    def wrap_class(self, class_name, class_file):
        pxd_lines = []
        pyx_lines = []
        decl_lines = []
        other_lines = []

        pxd_lines.append("cdef extern from \"2geom/{}\" namespace \"Geom\":".format(class_file))
        pxd_lines.append(self.delim+"cdef cppclass {}:".format(class_name))
        pyx_lines.append("cdef class cy_{}:".format(class_name))
        pyx_lines.append(self.delim+"cdef {}* thisptr".format(class_name))
        #get_types
        types_dict = self.collect_types(class_name)
        #parse
        c_class = self.global_namespace.namespace(name = "Geom").class_(name=class_name)

        for member in c_class.get_members():
            if isinstance(member, declarations.calldef.constructor_t):
                pxd, pyx = self.wrap_constructor(member, types_dict)
                pxd_lines += pxd
                pyx_lines += pyx
            elif isinstance(member, declarations.calldef.member_operator_t):
                pxd, pyx = self.wrap_operator(member, types_dict)
                pxd_lines += pxd
                pyx_lines += pyx
            elif isinstance(member, declarations.calldef.member_function_t):
                pxd, pyx = self.wrap_method(member, types_dict)
                pxd_lines += pxd
                pyx_lines += pyx
                
        pyx_lines.append("#free functions:")
        for ff in self.global_namespace.namespace(name = "Geom").free_funs():
            if os.path.basename(ff.location.file_name) == class_file:
                pxd, pyx, decl = self.wrap_free_function(ff, types_dict)
                pxd_lines += map(lambda x: x[4:], pxd)
                pyx_lines += pyx
                decl_lines += decl
                
        for ff in self.global_namespace.namespace(name = "Geom").free_operators():
            if os.path.basename(ff.location.file_name) == class_file:
                pxd, pyx = self.wrap_operator(ff, types_dict)
                pxd_lines += map(lambda x: x[4:], pxd)
                pyx_lines += pyx
   
        pyx_lines.append("")
        for ctype in types_dict:
            wtype = types_dict[ctype]
            for line in wtype.get_wrap_method(self.delim):
                pyx_lines.append(line)
        print "------"
        for i in pxd_lines:
            print i
        print "------"
        for i in pyx_lines:
            print i
        print "------"
        for i in decl_lines:
            print i
            
           
    def collect_types(self, class_name):
        types_set = set()
        c_class = self.global_namespace.namespace(name = "Geom").class_(name=class_name)
        for member in c_class.get_members():
            if any([isinstance(member, declarations.calldef.constructor_t),
                    isinstance(member, declarations.calldef.member_operator_t),
                    isinstance(member, declarations.calldef.member_function_t)]):
                for a_type in member.argument_types:
                    types_set.add(a_type.decl_string) 
                if isinstance(member, declarations.calldef.member_operator_t) or isinstance(member, declarations.calldef.member_function_t):
                    types_set.add(member.return_type.decl_string)
        for ff in list(self.global_namespace.namespace(name = "Geom").free_funs()) + list(self.global_namespace.namespace(name = "Geom").free_operators()):
            for a_type in ff.argument_types:
                types_set.add(a_type.decl_string)
                types_set.add(ff.return_type.decl_string)
        types_dict = dict()
        
        for f_type in types_set:

            types_dict[f_type] = wType(f_type)
        return types_dict

def main():

    #set up argument parser
    cmd_parser = argparse.ArgumentParser()
    cmd_parser.add_argument('--lib2geom_dir', action = 'store')
    cmd_parser.add_argument('--file_name', action = 'store')
    cmd_parser.add_argument('--class_name', action = 'store')
    cmd_args = cmd_parser.parse_args()
    
    #set up pygccxml
    includes = [cmd_args.lib2geom_dir, 
                os.path.join(cmd_args.lib2geom_dir, 'src'), 
                "/usr/include/boost"]

    config = parser.config_t(compiler='gcc', 
                            include_paths=includes, 
                            cflags="")
    
    file_to_parse = [os.path.join(cmd_args.lib2geom_dir, "src", "2geom", cmd_args.file_name)] 
    
    decls = parser.parse( file_to_parse, config )
    global_namespace = declarations.get_global_namespace( decls )
    wrapper = CythonWrapper(global_namespace)
    wrapper.wrap_class(cmd_args.class_name, cmd_args.file_name)

main()

#run with
#python2 wrapper.py --lib2geom_dir ../../.. --file_name int-point.h --class_name IntPoint
#from cython-bindings directory
