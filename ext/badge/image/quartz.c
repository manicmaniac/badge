#include <ruby.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageiO/ImageIO.h>

struct image_data {
    CGImageRef cgimage;
};

// Taken from https://stackoverflow.com/a/26365180/6918498
static CGBitmapInfo normalizeBitmapInfo(CGBitmapInfo oldBitmapInfo) {
    //extract the alpha info by resetting everything else
    CGImageAlphaInfo alphaInfo = oldBitmapInfo & kCGBitmapAlphaInfoMask;

    //Since iOS8 it's not allowed anymore to create contexts with unmultiplied Alpha info
    if (alphaInfo == kCGImageAlphaLast) {
        alphaInfo = kCGImageAlphaPremultipliedLast;
    }
    if (alphaInfo == kCGImageAlphaFirst) {
        alphaInfo = kCGImageAlphaPremultipliedFirst;
    }

    //reset the bits
    CGBitmapInfo newBitmapInfo = oldBitmapInfo & ~kCGBitmapAlphaInfoMask;

    //set the bits to the new alphaInfo
    newBitmapInfo |= alphaInfo;

    return newBitmapInfo;
}

static void free_image_data(struct image_data *image_data_ptr) {
    CGImageRelease(image_data_ptr->cgimage);
    free(image_data_ptr);
}

static const rb_data_type_t image_data_type = {
    "image_data",
    {0, (void (*)(void *))free_image_data, 0, {0, 0}},
    0, 0,
    RUBY_TYPED_FREE_IMMEDIATELY
};

static CGImageRef badge_image_get_cgimage(VALUE self) {
    struct image_data *image_data_ptr;

    TypedData_Get_Struct(self, struct image_data, &image_data_type, image_data_ptr);
    return image_data_ptr->cgimage;
}

static void badge_image_set_cgimage(VALUE self, CGImageRef cgimage) {
    struct image_data *image_data_ptr;

    TypedData_Get_Struct(self, struct image_data, &image_data_type, image_data_ptr);
    image_data_ptr->cgimage = cgimage;
}

static VALUE badge_image_initialize(VALUE self, VALUE path, VALUE tempfile) {
    CGDataProviderRef imageDataProvider = CGDataProviderCreateWithFilename(StringValuePtr(path));
    CGImageRef cgimage = CGImageCreateWithPNGDataProvider(imageDataProvider, NULL, false, kCGRenderingIntentDefault);
    CGDataProviderRelease(imageDataProvider);
    badge_image_set_cgimage(self, cgimage);
    rb_iv_set(self, "@path", path);
    rb_iv_set(self, "@tempfile", tempfile);
    return self;
}

static VALUE badge_image_alloc(VALUE klass) {
    struct image_data *image_data_ptr = NULL;
    return TypedData_Make_Struct(klass, struct image_data, &image_data_type, image_data_ptr);
}

static VALUE badge_image_s_open_block(VALUE file, VALUE klass) {
    VALUE Tempfile = rb_const_get(klass, rb_intern("Tempfile"));
    VALUE tempfile = rb_funcall(Tempfile, rb_intern("new"), 1, rb_str_new_literal("badge-image-quartz"));
    rb_funcall(tempfile, rb_intern("binmode"), 0);
    VALUE IO = rb_const_get(klass, rb_intern("IO"));
    rb_funcall(file, rb_intern("rewind"), 0);
    rb_funcall(IO, rb_intern("copy_stream"), 2, file, tempfile);
    rb_funcall(tempfile, rb_intern("rewind"), 0);
    VALUE path = rb_funcall(tempfile, rb_intern("path"), 0);
    VALUE self = badge_image_alloc(klass);
    return badge_image_initialize(self, path, tempfile);
}

static VALUE badge_image_s_open(VALUE klass, VALUE url) {
    VALUE is_svg = rb_funcall(url, rb_intern("end_with?"), 1, rb_str_new_literal(".svg"));
    if (rb_equal(is_svg, Qtrue)) {
        rb_raise(rb_eRuntimeError, "invalid image format %s", StringValuePtr(url));
        return Qnil;
    }
    VALUE URI = rb_const_get(klass, rb_intern("URI"));
    return rb_block_call(URI, rb_intern("open"), 1, &url, badge_image_s_open_block, klass);
}

static VALUE badge_image_s_load(VALUE klass, VALUE path) {
    VALUE self = badge_image_alloc(klass);
    return badge_image_initialize(self, path, Qnil);
}

static VALUE badge_image_colorspace(VALUE self, VALUE colorspace) {
    CFStringRef spaceName = CFStringCreateWithCStringNoCopy(NULL, StringValuePtr(colorspace), kCFStringEncodingASCII, kCFAllocatorNull);
    CGColorSpaceRef space = CGColorSpaceCreateWithName(spaceName);
    CGImageRef cgimage = badge_image_get_cgimage(self);
    CGImageRef new_cgimage = CGImageCreateCopyWithColorSpace(cgimage, space);
    CGImageRelease(cgimage);
    badge_image_set_cgimage(self, new_cgimage);
    return self;
}

static VALUE badge_image_format(VALUE self, VALUE format) {
    char *formatCString = StringValuePtr(format);
    if (strncmp("png", formatCString, 3) != 0) {
        rb_raise(rb_eRuntimeError, "invalid image format %s", formatCString);
    }
    return self;
}

static VALUE badge_image_resize(int argc, VALUE *argv, VALUE self) {
    if (argc < 2 || argc > 3) {
        rb_raise(rb_eArgError, "wrong number of arguments (given %d, expected 2..3)", argc);
        return self;
    }
    VALUE width = argv[0];
    VALUE height = argv[1];
    VALUE strategy = (argc == 3) ? argv[2] : Qnil;

    CGImageRef cgimage = badge_image_get_cgimage(self);
    if (!cgimage) puts("cgimage is nil");
    double aspectRatio = (double)CGImageGetHeight(cgimage) / (double)CGImageGetWidth(cgimage);
    CGRect contextRect = CGRectMake(0, 0, (CGFloat)NUM2DBL(width), (CGFloat)floor(NUM2DBL(height) * aspectRatio));
    CGContextRef context = CGBitmapContextCreate(
        NULL,
        (size_t)contextRect.size.width,
        (size_t)contextRect.size.height,
        CGImageGetBitsPerComponent(cgimage),
        0,
        CGImageGetColorSpace(cgimage),
        normalizeBitmapInfo(CGImageGetAlphaInfo(cgimage))
    );
    CGContextSetInterpolationQuality(context, kCGInterpolationHigh);
    CGContextDrawImage(context, contextRect, cgimage);
    CGImageRelease(cgimage);
    CGImageRef resizedCGImage = CGBitmapContextCreateImage(context);
    if (!resizedCGImage) puts("resizedCGImage is nil");
    CGContextRelease(context);
    badge_image_set_cgimage(self, resizedCGImage);
    return self;
}

static VALUE badge_image_write(VALUE self, VALUE path) {
    CFStringRef outputPath = CFStringCreateWithFileSystemRepresentation(NULL, StringValuePtr(path));
    CFURLRef outputURL = CFURLCreateWithFileSystemPath(NULL, outputPath, kCFURLPOSIXPathStyle, false);
    CFRelease(outputPath);
    CGImageDestinationRef destination = CGImageDestinationCreateWithURL(outputURL, CFSTR("public.png"), 1, NULL);
    CFRelease(outputURL);
    CGImageRef cgimage = badge_image_get_cgimage(self);
    CGImageDestinationAddImage(destination, cgimage, NULL);
    CGImageDestinationFinalize(destination);
    CFRelease(destination);
    return Qnil;
}

static VALUE badge_image_width(VALUE self) {
    CGImageRef cgimage = badge_image_get_cgimage(self);
    CGFloat width = CGImageGetWidth(cgimage);
    return rb_float_new(width);
}

static VALUE badge_image_height(VALUE self) {
    CGImageRef cgimage = badge_image_get_cgimage(self);
    CGFloat height = CGImageGetHeight(cgimage);
    return rb_float_new(height);
}

static VALUE badge_image_composite(int argc, VALUE *argv, VALUE self) {
    if (argc < 3 || argc > 4) {
        rb_raise(rb_eArgError, "wrong number of arguments (given %d, expected 3..4)", argc);
        return self;
    }
    VALUE overlay = argv[0];
    VALUE alpha_channel = argv[1];
    VALUE gravity = argv[2];
    VALUE geometry = (argc == 4) ? argv[3] : Qnil;

    if (!NIL_P(alpha_channel)) {
        rb_warn("alpha_channel is not supported in Image::Quartz. Install ImageMagick or GraphicsMagick to use this feature.");
    }
    if (!NIL_P(gravity)) {
        rb_warn("alpha_channel is not supported in Image::Quartz. Install ImageMagick or GraphicsMagick to use this feature.");
    }
    if (!NIL_P(geometry)) {
        rb_warn("alpha_channel is not supported in Image::Quartz. Install ImageMagick or GraphicsMagick to use this feature.");
    }
    CGImageRef cgimage = badge_image_get_cgimage(self);
    CGImageRef overlayCGImage = badge_image_get_cgimage(overlay);
    CGRect contextRect = CGRectMake(0, 0, CGImageGetWidth(cgimage), CGImageGetHeight(cgimage));
    CGContextRef context = CGBitmapContextCreate(
        NULL,
        (size_t)contextRect.size.width,
        (size_t)contextRect.size.height,
        CGImageGetBitsPerComponent(cgimage),
        0,
        CGImageGetColorSpace(cgimage),
        normalizeBitmapInfo(CGImageGetAlphaInfo(cgimage))
    );
    CGContextSetInterpolationQuality(context, kCGInterpolationHigh);
    CGContextDrawImage(context, contextRect, cgimage);
    CGImageRelease(cgimage);
    CGFloat overlayCGImageWidth = (CGFloat)CGImageGetWidth(overlayCGImage);
    CGFloat overlayCGImageHeight = (CGFloat)CGImageGetHeight(overlayCGImage);
    CGRect overlayRect = CGRectMake((contextRect.size.width - overlayCGImageWidth) / 2, contextRect.size.height - overlayCGImageHeight, overlayCGImageWidth, overlayCGImageHeight);
    CGContextDrawImage(context, overlayRect, overlayCGImage);
    CGImageRef composedCGImage = CGBitmapContextCreateImage(context);
    CGContextRelease(context);
    badge_image_set_cgimage(self, composedCGImage);
    return self;
}

void Init_quartz(void) {
    rb_require("open-uri");
    rb_require("tempfile");
    rb_require("badge/image");

    VALUE Badge = rb_const_get(rb_cObject, rb_intern("Badge"));;
    VALUE Image = rb_const_get(Badge, rb_intern("Image"));
    VALUE Quartz = rb_define_class_under(Image, "Quartz", rb_cObject);
    rb_define_alloc_func(Quartz, badge_image_alloc);
    rb_define_singleton_method(Quartz, "open", badge_image_s_open, 1);
    rb_define_singleton_method(Quartz, "load", badge_image_s_load, 1);
    rb_define_attr(Quartz, "path", 1, 0);
    rb_define_method(Quartz, "initialize", badge_image_initialize, 1);
    rb_define_method(Quartz, "colorspace", badge_image_colorspace, 1);
    rb_define_method(Quartz, "format", badge_image_format, 1);
    rb_define_method(Quartz, "resize", badge_image_resize, -1);
    rb_define_method(Quartz, "write", badge_image_write, 1);
    rb_define_method(Quartz, "width", badge_image_width, 0);
    rb_define_method(Quartz, "height", badge_image_height, 0);
    rb_define_method(Quartz, "composite", badge_image_composite, -1);
}
