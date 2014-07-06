# 2geom.rb - a lib2geom binding for ruby
#
# Copyright 2006 MenTaLguY <mental@rydia.net>
# Copyright 2006 Nathan Hurst <njh@mail.csse.monash.edu.au>
#
# This library is free software; you can redistribute it and/or
# modify it either under the terms of the GNU Lesser General Public
# License version 2.1 as published by the Free Software Foundation
# (the "LGPL") or, at your option, under the terms of the Mozilla
# Public License Version 1.1 (the "MPL"). If you do not alter this
# notice, a recipient may use your version of this file under either
# the MPL or the LGPL.
#
# You should have received a copy of the LGPL along with this library
# in the file COPYING-LGPL-2.1; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
# You should have received a copy of the MPL along with this library
# in the file COPYING-MPL-1.1
#
# The contents of this file are subject to the Mozilla Public License
# Version 1.1 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
# OF ANY KIND, either express or implied. See the LGPL or the MPL for
# the specific language governing rights and limitations.

require 'rubygems'
require 'inline'
require 'cairo'

LIB_DIR = File.dirname( File.expand_path( $0 ) )
INCLUDE_DIR = LIB_DIR

class Inline::C
  @@geom_type_converters_registered = false

  def add_pkgconfig_flags( *modules )
    add_compile_flags *`pkg-config --cflags #{modules.join(' ')}`.split
    add_link_flags *`pkg-config --libs #{modules.join(' ')}`.split
  end

  def lib2geom_prologue
    add_compile_flags "-I#{ INCLUDE_DIR }", '-x c++', '-lstdc++'
    add_link_flags "-L#{ LIB_DIR }", '-l2geom'
    add_pkgconfig_flags "cairo", "pango", "gsl"

    include '"path.h"'
    include '"path-builder.h"'
    include '"poly.h"'

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

      inline VALUE Path_class() {
        static VALUE klass=rb_const_get(Geom_module(), rb_intern("Path"));
        return klass;
      }

      inline VALUE Arrangement_class() {
        static VALUE klass=rb_const_get(Geom_module(), rb_intern("Arrangement"));
        return klass;
      }

      inline VALUE Elem_class() {
        static VALUE klass=rb_const_get(Path_class(), rb_intern("Elem"));
        return klass;
      }

      inline VALUE Poly_class() {
        static VALUE klass=rb_const_get(Geom_module(), rb_intern("Poly"));
        return klass;
      }

      inline VALUE arrangement_to_value(Geom::Arrangement *arrangement) {
        return Data_Wrap_Struct(Arrangement_class(), NULL, &do_delete<Geom::Arrangement>, arrangement);
      }

      inline Geom::Arrangement *value_to_arrangement(VALUE value) {
        Geom::Arrangement *arrangement;
        Data_Get_Struct(value, Geom::Arrangement, arrangement);
        return arrangement;
      }

      inline VALUE poly_to_value(Poly *poly) {
        return Data_Wrap_Struct(Poly_class(), NULL, &do_delete<Poly>, poly);
      }

      inline Poly *value_to_poly(VALUE value) {
        Poly *poly;
        Data_Get_Struct(value, Poly, poly);
        return poly;
      }

      }
    EOS

    unless @@geom_type_converters_registered
      add_type_converter "VALUE", '', ''
      add_type_converter "Geom::Arrangement *", 'value_to_arrangement', 'arrangement_to_value'
      add_type_converter "Poly *", 'value_to_poly', 'poly_to_value'
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
      static VALUE arrangement(Geom::Arrangement *arrangement) {
        cairo_t *cr;
        Data_Get_Struct(self, cairo_t, cr);
        cairo_arrangement(cr, *arrangement);
        return self;
      }
    EOS
  end
end
end

module Geom

class Arrangement
  include Enumerable

  inline do |builder|
    builder.lib2geom_prologue
    builder.include '<vector>'

    builder.c_singleton <<-EOS
      static VALUE _new() {
        return arrangement_to_value(new Geom::Arrangement());
      }
    EOS

    builder.c <<-EOS
      static VALUE each() {
        Geom::Arrangement *arrangement=value_to_arrangement(self);
        for ( Geom::PathVector::const_iterator iter = arrangement->begin() ;
              iter != arrangement->end() ; ++iter )
        {
          Geom::Path *path = new Geom::Path(*iter);
          rb_yield(Data_Wrap_Struct(Path_class(), NULL, &do_delete<Geom::Path>, path));
        }
        return self;
      }
    EOS

    builder.c <<-EOS
      static int size() {
        Geom::Arrangement *arrangement=value_to_arrangement(self);
        return arrangement->end() - arrangement->begin();
      }
    EOS
  end

  alias length size

  class << self
    alias new _new
    alias allocate _new
  end
end

class Path
  inline do |builder|
    builder.lib2geom_prologue

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
        return self;
      }
    EOS

    builder.c <<-EOS
      static VALUE _closed() {
        Geom::Path *path;
        Data_Get_Struct(self, Geom::Path, path);
        return ( path->is_closed() ? Qtrue : Qfalse );
      }
    EOS
  end

  class << self
    alias new _new
    alias allocate _new
  end

  alias closed? _closed
end

class ArrangementBuilder
  inline do |builder|
    builder.lib2geom_prologue

    builder.c_raw_singleton <<-EOS
      static VALUE _new(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        ArrangementBuilder *builder = new ArrangementBuilder();
        return Data_Wrap_Struct(self, NULL, &do_delete<ArrangementBuilder>, builder);
      }
    EOS

    builder.c_raw <<-EOS
      static VALUE peek(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        ArrangementBuilder *builder;
        Data_Get_Struct(self, ArrangementBuilder, builder);
        Arrangement *arrangement = new Arrangement(builder->peek());
        return Data_Wrap_Struct(Arrangement_class(), NULL, &do_delete<Arrangement>, arrangement);
      }
    EOS

    builder.c_raw <<-EOS
      static VALUE start_path(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        ArrangementBuilder *builder;
        Data_Get_Struct(self, ArrangementBuilder, builder);
        builder->start_path(Point(rb_num2dbl(argv[0]), rb_num2dbl(argv[1])));
        return self;
      }
    EOS

    builder.c_raw <<-EOS
      static VALUE push_line(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        ArrangementBuilder *builder;
        Data_Get_Struct(self, ArrangementBuilder, builder);
        builder->push_line(Point(rb_num2dbl(argv[0]), rb_num2dbl(argv[1])));
        return self;
      }
    EOS

    builder.c_raw <<-EOS
      static VALUE close_path(int argc, VALUE *argv, VALUE self) {
        using namespace Geom;
        ArrangementBuilder *builder;
        Data_Get_Struct(self, ArrangementBuilder, builder);
        builder->close_path();
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

class Poly
  include Enumerable

  inline do |builder|
    builder.lib2geom_prologue

    builder.c_raw_singleton <<-EOS
      static VALUE _new(int argc, VALUE *argv, VALUE self) {
        Poly *poly=new Poly();
        for ( int i = 0 ; i < argc ; i++ ) {
          poly->push_back(NUM2DBL(argv[i]));
        }
        return poly_to_value(poly);
      }
    EOS

    builder.c <<-EOS
      static VALUE _dup() {
        return poly_to_value(new Poly(*value_to_poly(self)));
      }
    EOS

    builder.c <<-EOS
      static unsigned long degree() {
        return value_to_poly(self)->degree();
      }
    EOS

    builder.c <<-EOS
      static VALUE _add(Poly *poly) {
        return poly_to_value(new Poly(*value_to_poly(self) + *poly));
      }
    EOS

    builder.c <<-EOS
      static VALUE _subtract(Poly *poly) {
        return poly_to_value(new Poly(*value_to_poly(self) - *poly));
      }
    EOS

    builder.c <<-EOS
      static VALUE _multiply(Poly *poly) {
        return poly_to_value(new Poly(*value_to_poly(self) * *poly));
      }
    EOS

    builder.c <<-EOS
      static VALUE _normalize() {
        value_to_poly(self)->normalize();
        return self;
      }
    EOS

    builder.c <<-EOS
      static VALUE _monicify() {
        value_to_poly(self)->monicify();
        return self;
      }
    EOS

    builder.c <<-EOS
      static double eval(double x) {
        return value_to_poly(self)->eval(x);
      }
    EOS

    builder.c <<-EOS
      static VALUE each() {
        Poly *poly=value_to_poly(self);
        Poly::iterator iter;
        for ( iter = poly->begin() ; iter != poly->end() ; ++iter ) {
          rb_yield(rb_float_new(*iter));
        }
        return self;
      }
    EOS
  end

  alias dup _dup
  # fixme: proper clone behavior
  class << self ; alias clone dup ; end

  alias normalize! _normalize
  def normalize ; dup.normalize! ; end

  alias monicify! _monicify
  def monicify ; dup.monicify! ; end

  alias + _add
  alias - _subtract
  alias * _multiply
end

end

