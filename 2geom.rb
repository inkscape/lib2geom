require 'rubygems'
require 'inline'

LIB_DIR = File.dirname( File.expand_path( $0 ) )
INCLUDE_DIR = LIB_DIR

class Inline::C
  def lib2geom_prologue
    add_compile_flags "-I#{ INCLUDE_DIR }", '-x c++', '-lstdc++'
    add_link_flags "-L#{ LIB_DIR }", '-l2geom'

    include '"path.h"'
    include '"path-builder.h"'

    prefix <<-EOS
      namespace {

      template <typename T>
      inline void do_delete(void *ptr) {
        delete static_cast<T *>(ptr);
      }

      inline VALUE Geom_module() {
        static VALUE mod=rb_const_get(rb_cObject, rb_intern("Geom"));
        return mod;
      }

      inline VALUE SubPath_class() {
        static VALUE klass=rb_const_get(Geom_module(), rb_intern("SubPath"));
        return klass;
      }

      inline VALUE Path_class() {
        static VALUE klass=rb_const_get(Geom_module(), rb_intern("Path"));
        return klass;
      }

      inline VALUE Elem_class() {
        static VALUE klass=rb_const_get(SubPath_class(), rb_intern("Elem"));
        return klass;
      }

      }
    EOS
  end
end

module Geom

class Path
  include Enumerable

  inline do |builder|
    builder.lib2geom_prologue
    builder.include '<vector>'

    builder.c_raw_singleton <<-EOS
      static VALUE _new(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        Path *path = new Path();
        return Data_Wrap_Struct(self, NULL, &do_delete<Path>, path);
      }
    EOS

    builder.c_raw <<-EOS
      static VALUE each(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        Path *path;
        Data_Get_Struct(self, Path, path);
        for ( std::vector<SubPath>::iterator iter = path->subpaths.begin() ;
              iter != path->subpaths.end() ; ++iter )
        {
          SubPath *subpath = new SubPath(*iter);
          rb_yield(Data_Wrap_Struct(SubPath_class(), NULL, &do_delete<SubPath>, subpath));
        }
        return self;
      }
    EOS

    builder.c_raw <<-EOS
      static VALUE size(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        Path *path;
        Data_Get_Struct(self, Path, path);
        return INT2FIX(path->subpaths.size());
      }
    EOS
  end

  alias length size

  class << self
    alias new _new
    alias allocate _new
  end
end

class SubPath
  inline do |builder|
    builder.lib2geom_prologue

    builder.c_raw_singleton <<-EOS
      static VALUE _new(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        SubPath *subpath = new SubPath();
        return Data_Wrap_Struct(self, NULL, &do_delete<SubPath>, subpath);
      }
    EOS

    builder.c_raw <<-EOS
      static VALUE each(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        return self;
      }
    EOS

    builder.c_raw <<-EOS
      static VALUE _closed(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        SubPath *subpath;
        Data_Get_Struct(self, SubPath, subpath);
        return ( subpath->closed ? Qtrue : Qfalse );
      }
    EOS
  end

  class << self
    alias new _new
    alias allocate _new
  end

  alias closed? _closed
end

class PathBuilder
  inline do |builder|
    builder.lib2geom_prologue

    builder.c_raw_singleton <<-EOS
      static VALUE _new(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        PathBuilder *builder = new PathBuilder();
        return Data_Wrap_Struct(self, NULL, &do_delete<PathBuilder>, builder);
      }
    EOS

    builder.c_raw <<-EOS
      static VALUE peek(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        PathBuilder *builder;
        Data_Get_Struct(self, PathBuilder, builder);
        Path *path = new Path(builder->peek());
        return Data_Wrap_Struct(Path_class(), NULL, &do_delete<Path>, path);
      }
    EOS

    builder.c_raw <<-EOS
      static VALUE start_subpath(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        PathBuilder *builder;
        Data_Get_Struct(self, PathBuilder, builder);
        builder->start_subpath(Point(rb_num2dbl(argv[0]), rb_num2dbl(argv[1])));
        return self;
      }
    EOS

    builder.c_raw <<-EOS
      static VALUE push_line(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        PathBuilder *builder;
        Data_Get_Struct(self, PathBuilder, builder);
        builder->push_line(Point(rb_num2dbl(argv[0]), rb_num2dbl(argv[1])));
        return self;
      }
    EOS

    builder.c_raw <<-EOS
      static VALUE close_subpath(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        PathBuilder *builder;
        Data_Get_Struct(self, PathBuilder, builder);
        builder->close_subpath();
        return self;
      }
    EOS
  end

  class << self
    alias new _new
    alias allocate _new
  end

  class Elem
  end
end

end
