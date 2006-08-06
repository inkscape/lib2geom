require 'rubygems'
require 'inline'

module Geom

LIB_DIR = File.dirname( File.expand_path( $0 ) )
INCLUDE_DIR = LIB_DIR

class Path
  inline do |builder|
    builder.include '"path.h"'

    builder.add_compile_flags "-I#{ INCLUDE_DIR }", '-x c++', '-lstdc++'
    builder.add_link_flags "-L#{ LIB_DIR }", '-l2geom'

    builder.prefix "
      template <typename T>
      static void do_delete(void *ptr) {
        delete static_cast<T *>(ptr);
      }
    "

    builder.c_raw_singleton "
      static VALUE _new(int argc, VALUE *argv, VALUE self) {
        Geom::Path *path = new Geom::Path();
        return Data_Wrap_Struct(self, NULL, &do_delete<Geom::Path>, path);
      }
    "
  end

  class << self
    alias new _new
    alias allocate _new
  end
end

end
