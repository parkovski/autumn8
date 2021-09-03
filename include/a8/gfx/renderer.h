#ifndef A8_GFX_RENDERER_H_
#define A8_GFX_RENDERER_H_

#include "common.h"

#include <nanovg.h>
#include <string_view>
#include <string>

namespace a8::gfx {

class Renderer {
public:
  virtual ~Renderer() = 0;

  // Setup {{{
  virtual void beginFrame(float windowWidth, float windowHeight,
                          float devicePixelRatio) = 0;

  void beginFrame(Size windowSize, float devicePixelRatio) {
    beginFrame(windowSize.width, windowSize.height, devicePixelRatio);
  }

  virtual void cancelFrame() = 0;

  virtual void endFrame() = 0;

  virtual void globalCompositeOperation(int op) = 0;

  virtual void globalCompositeBlendFunc(int sfactor, int dfactor) = 0;

  virtual void globalCompositeBlendFuncSeparate(int srcRGB, int dstRGB,
                                                int srcAlpha, int dstAlpha) = 0;

  virtual void save() = 0;

  virtual void restore() = 0;

  virtual void reset() = 0;
  // }}}

  // Drawing configuration {{{
  virtual void shapeAntiAlias(int enabled) = 0;

  virtual void strokeColor(NVGcolor color) = 0;

  virtual void strokePaint(NVGpaint paint) = 0;

  virtual void fillColor(NVGcolor color) = 0;

  virtual void fillPaint(NVGpaint paint) = 0;

  virtual void miterLimit(float limit) = 0;

  virtual void strokeWidth(float size) = 0;

  virtual void lineCap(int cap) = 0;

  virtual void lineJoin(int join) = 0;

  virtual void globalAlpha(float alpha) = 0;
  // }}}

  // Transform/translate {{{
  virtual void resetTransform() = 0;

  virtual void transform(float a, float b, float c, float d, float e,
                         float f) = 0;

  virtual void translate(float x, float y) = 0;

  void translate(Point p) {
    translate(p.x, p.y);
  }

  virtual void rotate(float angle) = 0;

  virtual void skewX(float angle) = 0;

  virtual void skewY(float angle) = 0;

  virtual void scale(float x, float y) = 0;

  void scale(Size size) {
    scale(size.width, size.height);
  }

  virtual void currentTransform(float* xform) = 0;
  // }}}

  // Image {{{
  virtual int createImage(const char* filename, int imageFlags) = 0;

  int createImage(const std::string &filename, int imageFlags) {
    return createImage(filename.c_str(), imageFlags);
  }

  int createImage(std::string_view filename, int imageFlags) {
    if (filename.back() == 0) {
      return createImage(filename.data(), imageFlags);
    } else {
      std::string filenameWithNull;
      filenameWithNull.reserve(filename.length() + 1);
      filenameWithNull.append(filename).push_back('\0');
      return createImage(filenameWithNull, imageFlags);
    }
  }

  virtual int createImageMem(int imageFlags, unsigned char* data,
                             int ndata) = 0;

  virtual int createImageRGBA(int w, int h, int imageFlags,
                              const unsigned char* data) = 0;

  virtual void updateImage(int image, const unsigned char* data) = 0;

  virtual void imageSize(int image, int* w, int* h) = 0;

  virtual void deleteImage(int image) = 0;
  // }}}

  // Gradient/pattern {{{
  virtual NVGpaint linearGradient(float sx, float sy, float ex, float ey,
                                  NVGcolor icol, NVGcolor ocol) = 0;

  virtual NVGpaint boxGradient(float x, float y, float w, float h, float r,
                               float f, NVGcolor icol, NVGcolor ocol) = 0;

  virtual NVGpaint radialGradient(float cx, float cy, float inr, float outr,
                                  NVGcolor icol, NVGcolor ocol) = 0;

  virtual NVGpaint imagePattern(float ox, float oy, float ex, float ey,
                                float angle, int image, float alpha) = 0;
  // }}}

  // Scissor {{{
  virtual void scissor(float x, float y, float w, float h) = 0;

  virtual void intersectScissor(float x, float y, float w, float h) = 0;

  virtual void resetScissor() = 0;
  // }}}

  // Path {{{
  virtual void beginPath() = 0;

  virtual void moveTo(float x, float y) = 0;

  virtual void lineTo(float x, float y) = 0;

  virtual void bezierTo(float c1x, float c1y, float c2x, float c2y,
                        float x, float y) = 0;

  virtual void quadTo(float cx, float cy, float x, float y) = 0;

  virtual void arcTo(float x1, float y1, float x2, float y2, float radius) = 0;

  virtual void closePath() = 0;

  virtual void pathWinding(int dir) = 0;

  virtual void arc(float cx, float cy, float r, float a0, float a1,
                   int dir) = 0;

  virtual void rect(float x, float y, float w, float h) = 0;

  virtual void roundedRect(float x, float y, float w, float h, float r) = 0;

  virtual void roundedRectVarying(float x, float y, float w, float h,
                                  float radTopLeft, float radTopRight,
                                  float radBottomRight,
                                  float radBottomLeft) = 0;

  virtual void ellipse(float cx, float cy, float rx, float ry) = 0;

  virtual void circle(float cx, float cy, float r) = 0;

  virtual void fill() = 0;

  virtual void stroke() = 0;
  // }}}

  // Font {{{
  virtual int createFont(const char* name, const char* filename) = 0;

  int createFont(const std::string &name, const std::string &filename) {
    return createFont(name.c_str(), filename.c_str());
  }

  virtual int createFontMem(const char* name, unsigned char* data, int ndata,
                            int freeData) = 0;

  int createFontMem(const std::string &name, unsigned char *data, int ndata,
                    int freeData) {
    return createFontMem(name.c_str(), data, ndata, freeData);
  }

  virtual int findFont(const char* name) = 0;

  int findFont(const std::string &name) {
    return findFont(name.c_str());
  }

  virtual int addFallbackFontId(int baseFont, int fallbackFont) = 0;

  virtual int addFallbackFont(const char* baseFont,
                              const char* fallbackFont) = 0;

  int addFallbackFont(const std::string &baseFont,
                      const std::string &fallbackFont) {
    return addFallbackFont(baseFont.c_str(), fallbackFont.c_str());
  }

  virtual void fontSize(float size) = 0;

  virtual void fontBlur(float blur) = 0;

  virtual void textLetterSpacing(float spacing) = 0;

  virtual void textLineHeight(float lineHeight) = 0;

  virtual void textAlign(int align) = 0;

  virtual void fontFaceId(int font) = 0;

  virtual void fontFace(const char* font) = 0;

  void fontFace(const std::string &font) {
    fontFace(font.c_str());
  }
  // }}}

  // Text {{{
  virtual float text(float x, float y, const char* string, const char* end) = 0;

  float text(float x, float y, std::string_view string) {
    const char *s = string.data();
    return text(x, y, s, s + string.length());
  }

  virtual void textBox(float x, float y, float breakRowWidth,
                       const char* string, const char* end) = 0;

  void textBox(float x, float y, float breakRowWidth, std::string_view string) {
    const char *s = string.data();
    textBox(x, y, breakRowWidth, s, s + string.length());
  }

  virtual float textBounds(float x, float y,
                           const char* string, const char* end,
                           float* bounds) = 0;

  float textBounds(float x, float y, std::string_view string, float *bounds) {
    const char *s = string.data();
    return textBounds(x, y, s, s + string.length(), bounds);
  }

  virtual void textBoxBounds(float x, float y, float breakRowWidth,
                             const char* string, const char* end,
                             float* bounds) = 0;

  virtual int textGlyphPositions(float x, float y,
                                 const char* string, const char* end,
                                 NVGglyphPosition* positions,
                                 int maxPositions) = 0;

  virtual void textMetrics(float* ascender, float* descender, float* lineh) = 0;

  virtual int textBreakLines(const char* string, const char* end,
                             float breakRowWidth, NVGtextRow* rows,
                             int maxRows) = 0;
  // }}}

  // Debugging {{{
  virtual void debugDumpPathCache() = 0;
  // }}}

  // Static methods {{{
  static NVGcolor rgb(unsigned char r, unsigned char g, unsigned char b) {
    return nvgRGB(r, g, b);
  }

  static NVGcolor rgbf(float r, float g, float b) {
    return nvgRGBf(r, g, b);
  }

  static NVGcolor rgba(unsigned char r, unsigned char g, unsigned char b,
                       unsigned char a) {
    return nvgRGBA(r, g, b, a);
  }

  static NVGcolor rgbaf(float r, float g, float b, float a) {
    return nvgRGBAf(r, g, b, a);
  }

  static NVGcolor lerpRGBA(NVGcolor c0, NVGcolor c1, float u) {
    return nvgLerpRGBA(c0, c1, u);
  }

  static NVGcolor transRGBA(NVGcolor c0, unsigned char a) {
    return nvgTransRGBA(c0, a);
  }

  static NVGcolor transRGBA(NVGcolor c0, float a) {
    return nvgTransRGBAf(c0, a);
  }

  static NVGcolor hsl(float h, float s, float l) {
    return nvgHSL(h, s, l);
  }

  static NVGcolor hsla(float h, float s, float l, unsigned char a) {
    return nvgHSLA(h, s, l, a);
  }

  static void transformIdentity(float* dst) {
    nvgTransformIdentity(dst);
  }

  static void transformTranslate(float* dst, float tx, float ty) {
    nvgTransformTranslate(dst, tx, ty);
  }

  static void transformTranslate(float* dst, Point t) {
    transformTranslate(dst, t.x, t.y);
  }

  static void transformScale(float* dst, float sx, float sy) {
    nvgTransformScale(dst, sx, sy);
  }

  static void transformScale(float* dst, Size s) {
    transformScale(dst, s.width, s.height);
  }

  static void transformRotate(float* dst, float a) {
    nvgTransformRotate(dst, a);
  }

  static void transformSkewX(float* dst, float a) {
    nvgTransformSkewX(dst, a);
  }

  static void transformSkewY(float* dst, float a) {
    nvgTransformSkewY(dst, a);
  }

  static void transformMultiply(float* dst, const float* src) {
    nvgTransformMultiply(dst, src);
  }

  static void transformPremultiply(float* dst, const float* src) {
    nvgTransformPremultiply(dst, src);
  }

  static int transformInverse(float* dst, const float* src) {
    return nvgTransformInverse(dst, src);
  }

  static void transformPoint(float* dstx, float* dsty, const float* xform,
                             float srcx, float srcy) {
    nvgTransformPoint(dstx, dsty, xform, srcx, srcy);
  }

  static Point transformPoint(const float* xform, Point src) {
    Point dst;
    transformPoint(&dst.x, &dst.y, xform, src.x, src.y);
    return dst;
  }

  static float degToRad(float deg) {
    return nvgDegToRad(deg);
  }

  static float radToDeg(float rad) {
    return nvgRadToDeg(rad);
  }
  // }}}
};

class NvgRenderer : public Renderer {
  NVGcontext *_vg;

public:
  explicit NvgRenderer(NVGcontext *vg) noexcept
    : _vg{vg}
  {}

  void beginFrame(float windowWidth, float windowHeight,
                  float devicePixelRatio) override;
  void cancelFrame() override;
  void endFrame() override;
  void globalCompositeOperation(int op) override;
  void globalCompositeBlendFunc(int sfactor, int dfactor) override;
  void globalCompositeBlendFuncSeparate(int srcRGB, int dstRGB, int srcAlpha,
                                        int dstAlpha) override;
  void save() override;
  void restore() override;
  void reset() override;
  void shapeAntiAlias(int enabled) override;
  void strokeColor(NVGcolor color) override;
  void strokePaint(NVGpaint paint) override;
  void fillColor(NVGcolor color) override;
  void fillPaint(NVGpaint paint) override;
  void miterLimit(float limit) override;
  void strokeWidth(float size) override;
  void lineCap(int cap) override;
  void lineJoin(int join) override;
  void globalAlpha(float alpha) override;
  void resetTransform() override;
  void transform(float a, float b, float c, float d, float e, float f) override;
  void translate(float x, float y) override;
  void rotate(float angle) override;
  void skewX(float angle) override;
  void skewY(float angle) override;
  void scale(float x, float y) override;
  void currentTransform(float* xform) override;
  int createImage(const char* filename, int imageFlags) override;
  int createImageMem(int imageFlags, unsigned char* data, int ndata) override;
  int createImageRGBA(int w, int h, int imageFlags,
                      const unsigned char* data) override;
  void updateImage(int image, const unsigned char* data) override;
  void imageSize(int image, int* w, int* h) override;
  void deleteImage(int image) override;
  NVGpaint linearGradient(float sx, float sy, float ex, float ey,
                          NVGcolor icol, NVGcolor ocol) override;
  NVGpaint boxGradient(float x, float y, float w, float h, float r, float f,
                       NVGcolor icol, NVGcolor ocol) override;
  NVGpaint radialGradient(float cx, float cy, float inr, float outr,
                          NVGcolor icol, NVGcolor ocol) override;
  NVGpaint imagePattern(float ox, float oy, float ex, float ey, float angle,
                        int image, float alpha) override;
  void scissor(float x, float y, float w, float h) override;
  void intersectScissor(float x, float y, float w, float h) override;
  void resetScissor() override;
  void beginPath() override;
  void moveTo(float x, float y) override;
  void lineTo(float x, float y) override;
  void bezierTo(float c1x, float c1y, float c2x, float c2y,
                float x, float y) override;
  void quadTo(float cx, float cy, float x, float y) override;
  void arcTo(float x1, float y1, float x2, float y2, float radius) override;
  void closePath() override;
  void pathWinding(int dir) override;
  void arc(float cx, float cy, float r, float a0, float a1, int dir) override;
  void rect(float x, float y, float w, float h) override;
  void roundedRect(float x, float y, float w, float h, float r) override;
  void roundedRectVarying(float x, float y, float w, float h,
                          float radTopLeft, float radTopRight,
                          float radBottomRight, float radBottomLeft) override;
  void ellipse(float cx, float cy, float rx, float ry) override;
  void circle(float cx, float cy, float r) override;
  void fill() override;
  void stroke() override;
  int createFont(const char* name, const char* filename) override;
  int createFontMem(const char* name, unsigned char* data, int ndata,
                    int freeData) override;
  int findFont(const char* name) override;
  int addFallbackFontId(int baseFont, int fallbackFont) override;
  int addFallbackFont(const char* baseFont, const char* fallbackFont) override;
  void fontSize(float size) override;
  void fontBlur(float blur) override;
  void textLetterSpacing(float spacing) override;
  void textLineHeight(float lineHeight) override;
  void textAlign(int align) override;
  void fontFaceId(int font) override;
  void fontFace(const char* font) override;
  float text(float x, float y, const char* string, const char* end) override;
  void textBox(float x, float y, float breakRowWidth,
               const char* string, const char* end) override;
  float textBounds(float x, float y, const char* string, const char* end,
                   float* bounds) override;
  void textBoxBounds(float x, float y, float breakRowWidth,
                     const char* string, const char* end,
                     float* bounds) override;
  int textGlyphPositions(float x, float y, const char* string, const char* end,
                         NVGglyphPosition* positions,
                         int maxPositions) override;
  void textMetrics(float* ascender, float* descender, float* lineh) override;
  int textBreakLines(const char* string, const char* end, float breakRowWidth,
                     NVGtextRow* rows, int maxRows) override;
  void debugDumpPathCache() override;
};

} // namespace a8::gfx

#endif // A8_GFX_RENDERER_H_
