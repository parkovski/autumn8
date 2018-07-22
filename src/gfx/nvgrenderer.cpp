#include "a8/gfx/renderer.h"

#include "nanovg.h"

namespace a8::gfx {

class NvgRenderer : public Renderer {
  NVGcontext *_vg;
public:
  explicit NvgRenderer(NVGcontext *vg) noexcept : _vg(vg) {}

  void beginFrame(float windowWidth, float windowHeight, float devicePixelRatio) override {
    nvgBeginFrame(_vg, windowWidth, windowHeight, devicePixelRatio);
  }

  void cancelFrame() override {
    nvgCancelFrame(_vg);
  }

  void endFrame() override {
    nvgEndFrame(_vg);
  }

  void globalCompositeOperation(int op) override {
    nvgGlobalCompositeOperation(_vg, op);
  }

  void globalCompositeBlendFunc(int sfactor, int dfactor) override {
    nvgGlobalCompositeBlendFunc(_vg, sfactor, dfactor);
  }

  void globalCompositeBlendFuncSeparate(int srcRGB, int dstRGB, int srcAlpha, int dstAlpha) override {
    nvgGlobalCompositeBlendFuncSeparate(_vg, srcRGB, dstRGB, srcAlpha, dstAlpha);
  }

  void save() override {
    nvgSave(_vg);
  }

  void restore() override {
    nvgRestore(_vg);
  }

  void reset() override {
    nvgReset(_vg);
  }

  void shapeAntiAlias(int enabled) override {
    nvgShapeAntiAlias(_vg, enabled);
  }

  void strokeColor(NVGcolor color) override {
    nvgStrokeColor(_vg, color);
  }

  void strokePaint(NVGpaint paint) override {
    nvgStrokePaint(_vg, paint);
  }

  void fillColor(NVGcolor color) override {
    nvgFillColor(_vg, color);
  }

  void fillPaint(NVGpaint paint) override {
    nvgFillPaint(_vg, paint);
  }

  void miterLimit(float limit) override {
    nvgMiterLimit(_vg, limit);
  }

  void strokeWidth(float size) override {
    nvgStrokeWidth(_vg, size);
  }

  void lineCap(int cap) override {
    nvgLineCap(_vg, cap);
  }

  void lineJoin(int join) override {
    nvgLineJoin(_vg, join);
  }

  void globalAlpha(float alpha) override {
    nvgGlobalAlpha(_vg, alpha);
  }

  void resetTransform() override {
    nvgResetTransform(_vg);
  }

  void transform(float a, float b, float c, float d, float e, float f) override {
    nvgTransform(_vg, a, b, c, d, e, f);
  }

  void translate(float x, float y) override {
    nvgTranslate(_vg, x, y);
  }

  void rotate(float angle) override {
    nvgRotate(_vg, angle);
  }

  void skewX(float angle) override {
    nvgSkewX(_vg, angle);
  }

  void skewY(float angle) override {
    nvgSkewY(_vg, angle);
  }

  void scale(float x, float y) override {
    nvgScale(_vg, x, y);
  }

  void currentTransform(float* xform) override {
    nvgCurrentTransform(_vg, xform);
  }

  int createImage(const char* filename, int imageFlags) override {
    return nvgCreateImage(_vg, filename, imageFlags);
  }

  int createImageMem(int imageFlags, unsigned char* data, int ndata) override {
    return nvgCreateImageMem(_vg, imageFlags, data, ndata);
  }

  int createImageRGBA(int w, int h, int imageFlags, const unsigned char* data) override {
    return nvgCreateImageRGBA(_vg, w, h, imageFlags, data);
  }

  void updateImage(int image, const unsigned char* data) override {
    nvgUpdateImage(_vg, image, data);
  }

  void imageSize(int image, int* w, int* h) override {
    nvgImageSize(_vg, image, w, h);
  }

  void deleteImage(int image) override {
    nvgDeleteImage(_vg, image);
  }

  NVGpaint linearGradient(float sx, float sy, float ex, float ey, NVGcolor icol, NVGcolor ocol) override {
    return nvgLinearGradient(_vg, sx, sy, ex, ey, icol, ocol);
  }

  NVGpaint boxGradient(float x, float y, float w, float h, float r, float f, NVGcolor icol, NVGcolor ocol) override {
    return nvgBoxGradient(_vg, x, y, w, h, r, f, icol, ocol);
  }

  NVGpaint radialGradient(float cx, float cy, float inr, float outr, NVGcolor icol, NVGcolor ocol) override {
    return nvgRadialGradient(_vg, cx, cy, inr, outr, icol, ocol);
  }

  NVGpaint imagePattern(float ox, float oy, float ex, float ey, float angle, int image, float alpha) override {
    return nvgImagePattern(_vg, ox, oy, ex, ey, angle, image, alpha);
  }

  void scissor(float x, float y, float w, float h) override {
    nvgScissor(_vg, x, y, w, h);
  }

  void intersectScissor(float x, float y, float w, float h) override {
    nvgIntersectScissor(_vg, x, y, w, h);
  }

  void resetScissor() override {
    nvgResetScissor(_vg);
  }

  void beginPath() override {
    nvgBeginPath(_vg);
  }

  void moveTo(float x, float y) override {
    nvgMoveTo(_vg, x, y);
  }

  void lineTo(float x, float y) override {
    nvgLineTo(_vg, x, y);
  }

  void bezierTo(float c1x, float c1y, float c2x, float c2y, float x, float y) override {
    nvgBezierTo(_vg, c1x, c1y, c2x, c2y, x, y);
  }

  void quadTo(float cx, float cy, float x, float y) override {
    nvgQuadTo(_vg, cx, cy, x, y);
  }

  void arcTo(float x1, float y1, float x2, float y2, float radius) override {
    nvgArcTo(_vg, x1, y1, x2, y2, radius);
  }

  void closePath() override {
    nvgClosePath(_vg);
  }

  void pathWinding(int dir) override {
    nvgPathWinding(_vg, dir);
  }

  void arc(float cx, float cy, float r, float a0, float a1, int dir) override {
    nvgArc(_vg, cx, cy, r, a0, a1, dir);
  }

  void rect(float x, float y, float w, float h) override {
    nvgRect(_vg, x, y, w, h);
  }

  void roundedRect(float x, float y, float w, float h, float r) override {
    nvgRoundedRect(_vg, x, y, w, h, r);
  }

  void roundedRectVarying(float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft) override {
    nvgRoundedRectVarying(_vg, x, y, w, h, radTopLeft, radTopRight, radBottomRight, radBottomLeft);
  }

  void ellipse(float cx, float cy, float rx, float ry) override {
    nvgEllipse(_vg, cx, cy, rx, ry);
  }

  void circle(float cx, float cy, float r) override {
    nvgCircle(_vg, cx, cy, r);
  }

  void fill() override {
    nvgFill(_vg);
  }

  void stroke() override {
    nvgStroke(_vg);
  }

  int createFont(const char* name, const char* filename) override {
    nvgCreateFont(_vg, name, filename);
  }

  int createFontMem(const char* name, unsigned char* data, int ndata, int freeData) override {
    nvgCreateFontMem(_vg, name, data, ndata, freeData);
  }

  int findFont(const char* name) override {
    nvgFindFont(_vg, name);
  }

  int addFallbackFontId(int baseFont, int fallbackFont) override {
    nvgAddFallbackFontId(_vg, baseFont, fallbackFont);
  }

  int addFallbackFont(const char* baseFont, const char* fallbackFont) override {
    nvgAddFallbackFont(_vg, baseFont, fallbackFont);
  }

  void fontSize(float size) override {
    nvgFontSize(_vg, size);
  }

  void fontBlur(float blur) override {
    nvgFontBlur(_vg, blur);
  }

  void textLetterSpacing(float spacing) override {
    nvgTextLetterSpacing(_vg, spacing);
  }

  void textLineHeight(float lineHeight) override {
    nvgTextLineHeight(_vg, lineHeight);
  }

  void textAlign(int align) override {
    nvgTextAlign(_vg, align);
  }

  void fontFaceId(int font) override {
    nvgFontFaceId(_vg, font);
  }

  void fontFace(const char* font) override {
    nvgFontFace(_vg, font);
  }

  float text(float x, float y, const char* string, const char* end) override {
    return nvgText(_vg, x, y, string, end);
  }

  void textBox(float x, float y, float breakRowWidth, const char* string, const char* end) override {
    nvgTextBox(_vg, x, y, breakRowWidth, string, end);
  }

  float textBounds(float x, float y, const char* string, const char* end, float* bounds) override {
    return nvgTextBounds(_vg, x, y, string, end, bounds);
  }

  void textBoxBounds(float x, float y, float breakRowWidth, const char* string, const char* end, float* bounds) override {
    nvgTextBoxBounds(_vg, x, y, breakRowWidth, string, end, bounds);
  }

  int textGlyphPositions(float x, float y, const char* string, const char* end, NVGglyphPosition* positions, int maxPositions) override {
    return nvgTextGlyphPositions(_vg, x, y, string, end, positions, maxPositions);
  }

  void textMetrics(float* ascender, float* descender, float* lineh) override {
    nvgTextMetrics(_vg, ascender, descender, lineh);
  }

  int textBreakLines(const char* string, const char* end, float breakRowWidth, NVGtextRow* rows, int maxRows) override {
    return nvgTextBreakLines(_vg, string, end, breakRowWidth, rows, maxRows);
  }

  void debugDumpPathCache() override {
    nvgDebugDumpPathCache(_vg);
  }
};

} // namespace a8::gfx
