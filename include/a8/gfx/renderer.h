#ifndef A8_GFX_RENDERER_H_
#define A8_GFX_RENDERER_H_

#include "a8/gfx/common.h"

#include "nanovg.h"

namespace a8::gfx {

class Renderer {
  static NVGcolor rgb(unsigned char r, unsigned char g, unsigned char b) {
    return nvgRGB(r, g, b);
  }
  static NVGcolor rgb(float r, float g, float b) {
    nvgRGBf(r, g, b);
  }
  static NVGcolor rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    nvgRGBA(r, g, b, a);
  }
  static NVGcolor rgba(float r, float g, float b, float a) {
    nvgRGBAf(r, g, b, a);
  }
  static NVGcolor lerpRGBA(NVGcolor c0, NVGcolor c1, float u) {
    nvgLerpRGBA(c0, c1, u);
  }
  static NVGcolor transRGBA(NVGcolor c0, unsigned char a) {
    nvgTransRGBA(c0, a);
  }
  static NVGcolor transRGBA(NVGcolor c0, float a) {
    nvgTransRGBAf(c0, a);
  }
  static NVGcolor hsl(float h, float s, float l) {
    nvgHSL(h, s, l);
  }
  static NVGcolor hsla(float h, float s, float l, unsigned char a) {
    nvgHSLA(h, s, l, a);
  }
  static void transformIdentity(float* dst) {
    nvgTransformIdentity(dst);
  }
  static void transformTranslate(float* dst, float tx, float ty) {
    nvgTransformTranslate(dst, tx, ty);
  }
  static void transformScale(float* dst, float sx, float sy) {
    nvgTransformScale(dst, sx, sy);
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
    nvgTransformInverse(dst, src);
  }
  static void transformPoint(float* dstx, float* dsty, const float* xform, float srcx, float srcy) {
    nvgTransformPoint(dstx, dsty, xform, srcx, srcy);
  }
  static float degToRad(float deg) {
    nvgDegToRad(deg);
  }
  static float radToDeg(float rad) {
    nvgRadToDeg(rad);
  }

  virtual void beginFrame(float windowWidth, float windowHeight, float devicePixelRatio) = 0;
  virtual void cancelFrame() = 0;
  virtual void endFrame() = 0;
  virtual void globalCompositeOperation(int op) = 0;
  virtual void globalCompositeBlendFunc(int sfactor, int dfactor) = 0;
  virtual void globalCompositeBlendFuncSeparate(int srcRGB, int dstRGB, int srcAlpha, int dstAlpha) = 0;
  virtual void save() = 0;
  virtual void restore() = 0;
  virtual void reset() = 0;
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
  virtual void resetTransform() = 0;
  virtual void transform(float a, float b, float c, float d, float e, float f) = 0;
  virtual void translate(float x, float y) = 0;
  virtual void rotate(float angle) = 0;
  virtual void skewX(float angle) = 0;
  virtual void skewY(float angle) = 0;
  virtual void scale(float x, float y) = 0;
  virtual void currentTransform(float* xform) = 0;
  virtual int createImage(const char* filename, int imageFlags) = 0;
  virtual int createImageMem(int imageFlags, unsigned char* data, int ndata) = 0;
  virtual int createImageRGBA(int w, int h, int imageFlags, const unsigned char* data) = 0;
  virtual void updateImage(int image, const unsigned char* data) = 0;
  virtual void imageSize(int image, int* w, int* h) = 0;
  virtual void deleteImage(int image) = 0;
  virtual NVGpaint linearGradient(float sx, float sy, float ex, float ey, NVGcolor icol, NVGcolor ocol) = 0;
  virtual NVGpaint boxGradient(float x, float y, float w, float h, float r, float f, NVGcolor icol, NVGcolor ocol) = 0;
  virtual NVGpaint radialGradient(float cx, float cy, float inr, float outr, NVGcolor icol, NVGcolor ocol) = 0;
  virtual NVGpaint imagePattern(float ox, float oy, float ex, float ey, float angle, int image, float alpha) = 0;
  virtual void scissor(float x, float y, float w, float h) = 0;
  virtual void intersectScissor(float x, float y, float w, float h) = 0;
  virtual void resetScissor() = 0;
  virtual void beginPath() = 0;
  virtual void moveTo(float x, float y) = 0;
  virtual void lineTo(float x, float y) = 0;
  virtual void bezierTo(float c1x, float c1y, float c2x, float c2y, float x, float y) = 0;
  virtual void quadTo(float cx, float cy, float x, float y) = 0;
  virtual void arcTo(float x1, float y1, float x2, float y2, float radius) = 0;
  virtual void closePath() = 0;
  virtual void pathWinding(int dir) = 0;
  virtual void arc(float cx, float cy, float r, float a0, float a1, int dir) = 0;
  virtual void rect(float x, float y, float w, float h) = 0;
  virtual void roundedRect(float x, float y, float w, float h, float r) = 0;
  virtual void roundedRectVarying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft) = 0;
  virtual void ellipse(float cx, float cy, float rx, float ry) = 0;
  virtual void circle(float cx, float cy, float r) = 0;
  virtual void fill() = 0;
  virtual void stroke() = 0;
  virtual int createFont(const char* name, const char* filename) = 0;
  virtual int createFontMem(const char* name, unsigned char* data, int ndata, int freeData) = 0;
  virtual int findFont(const char* name) = 0;
  virtual int addFallbackFontId(int baseFont, int fallbackFont) = 0;
  virtual int addFallbackFont(const char* baseFont, const char* fallbackFont) = 0;
  virtual void fontSize(float size) = 0;
  virtual void fontBlur(float blur) = 0;
  virtual void textLetterSpacing(float spacing) = 0;
  virtual void textLineHeight(float lineHeight) = 0;
  virtual void textAlign(int align) = 0;
  virtual void fontFaceId(int font) = 0;
  virtual void fontFace(const char* font) = 0;
  virtual float text(float x, float y, const char* string, const char* end) = 0;
  virtual void textBox(float x, float y, float breakRowWidth, const char* string, const char* end) = 0;
  virtual float textBounds(float x, float y, const char* string, const char* end, float* bounds) = 0;
  virtual void textBoxBounds(float x, float y, float breakRowWidth, const char* string, const char* end, float* bounds) = 0;
  virtual int textGlyphPositions(float x, float y, const char* string, const char* end, NVGglyphPosition* positions, int maxPositions) = 0;
  virtual void textMetrics(float* ascender, float* descender, float* lineh) = 0;
  virtual int textBreakLines(const char* string, const char* end, float breakRowWidth, NVGtextRow* rows, int maxRows) = 0;
  virtual void debugDumpPathCache() = 0;
};

} // namespace a8::gfx

#endif // A8_GFX_RENDERER_H_

