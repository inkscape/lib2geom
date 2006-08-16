require 'rubygems'
require 'inline'
require 'cairo'

LIB_DIR = File.dirname( File.expand_path( $0 ) )
INCLUDE_DIR = LIB_DIR

class Inline::C
  @@geom_type_converters_registered = false

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

      inline VALUE Cairo_module() {
        static VALUE mod=rb_const_get(rb_cObject, rb_intern("Cairo"));
        return mod;
      }

      inline VALUE Cairo_Context_class() {
        static VALUE klass=rb_const_get(Cairo_module(), rb_intern("Context"));
        return klass;
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

      inline VALUE path_to_value(Geom::Path *path) {
        return Data_Wrap_Struct(Path_class(), NULL, &do_delete<Geom::Path>, path);
      }

      inline Geom::Path *value_to_path(VALUE value) {
        Geom::Path *path;
        Data_Get_Struct(value, Geom::Path, path);
        return path;
      }

      }
    EOS

    unless @@geom_type_converters_registered
      add_type_converter "VALUE", '', ''
      add_type_converter "Geom::Path *", 'value_to_path', 'path_to_value'
      @@geom_type_converters_registered = true
    end
  end
end

module Cairo
class Context
  inline do |builder|
    builder.lib2geom_prologue
    builder.include '"path-cairo.h"'

    builder.c <<-EOS
      static VALUE path(Geom::Path *path) {
        cairo_t *cr;
        Data_Get_Struct(self, cairo_t, cr);
        cairo_path(cr, *path);
        return self;
      }
    EOS
  end
end
end

module Geom

class Path
  include Enumerable

  inline do |builder|
    builder.lib2geom_prologue
    builder.include '<vector>'

    builder.c_singleton <<-EOS
      static Geom::Path *_new() {
        return (new Geom::Path());
      }
    EOS

    builder.c <<-EOS
      static VALUE each() {
        Geom::Path *path=value_to_path(self);
        for ( std::vector<Geom::SubPath>::iterator iter = path->subpaths.begin() ;
              iter != path->subpaths.end() ; ++iter )
        {
          Geom::SubPath *subpath = new Geom::SubPath(*iter);
          rb_yield(Data_Wrap_Struct(SubPath_class(), NULL, &do_delete<Geom::SubPath>, subpath));
        }
        return self;
      }
    EOS

    builder.c <<-EOS
      static int size() {
        return value_to_path(self)->subpaths.size();
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

    builder.c <<-EOS
      static VALUE _closed() {
        Geom::SubPath *subpath;
        Data_Get_Struct(self, Geom::SubPath, subpath);
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
