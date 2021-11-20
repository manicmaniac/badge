#include <ruby.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageiO/ImageIO.h>

static VALUE badge_image_open(VALUE self, VALUE url) {
    return T_NIL;
}

static VALUE badge_image_load(VALUE self, VALUE path) {
    return T_NIL;
}

static VALUE badge_image_colorspace(VALUE self, VALUE colorspace) {
    return T_NIL;
}

static VALUE badge_image_format(VALUE self, VALUE format) {
    return T_NIL;
}

static VALUE badge_image_resize(VALUE self, VALUE resize) {
    return T_NIL;
}

static VALUE badge_image_write(VALUE self, VALUE write) {
    return T_NIL;
}

static VALUE badge_image_path(VALUE self) {
    return T_NIL;
}

static VALUE badge_image_width(VALUE self) {
    return T_NIL;
}

static VALUE badge_image_height(VALUE self) {
    return T_NIL;
}

static VALUE badge_image_composite(VALUE self, VALUE overlay, VALUE alpha_channel, VALUE gravity, VALUE geometry) {
    return T_NIL;
}

void Init_quartz(void) {
    VALUE Image = rb_define_class("Image", rb_cObject);
    rb_define_singleton_method(Image, "open", badge_image_open, 1);
    rb_define_singleton_method(Image, "load", badge_image_load, 1);
    rb_define_method(Image, "colorspace", badge_image_colorspace, 1);
    rb_define_method(Image, "format", badge_image_format, 1);
    rb_define_method(Image, "resize", badge_image_resize, 1);
    rb_define_method(Image, "write", badge_image_write, 1);
    rb_define_method(Image, "path", badge_image_path, 0);
    rb_define_method(Image, "width", badge_image_width, 0);
    rb_define_method(Image, "height", badge_image_height, 0);
    rb_define_method(Image, "composite", badge_image_composite, 4);
}
