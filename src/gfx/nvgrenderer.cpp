#include "a8/gfx/renderer.h"

#include "nanovg.h"

using namespace a8::gfx;

Renderer::~Renderer() {}

void NvgRenderer::beginFrame(float windowWidth, float windowHeight,
                             float devicePixelRatio) {
  nvgBeginFrame(_vg, windowWidth, windowHeight, devicePixelRatio);
}

void NvgRenderer::NvgRenderer::cancelFrame() {
  nvgCancelFrame(_vg);
}

void NvgRenderer::endFrame() {
  nvgEndFrame(_vg);
}

void NvgRenderer::globalCompositeOperation(int op) {
  nvgGlobalCompositeOperation(_vg, op);
}

void NvgRenderer::globalCompositeBlendFunc(int sfactor, int dfactor) {
  nvgGlobalCompositeBlendFunc(_vg, sfactor, dfactor);
}

void NvgRenderer::globalCompositeBlendFuncSeparate(int srcRGB,
                                                   int dstRGB,
                                                   int srcAlpha,
                                                   int dstAlpha) {
  nvgGlobalCompositeBlendFuncSeparate(_vg, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

void NvgRenderer::save() {
  nvgSave(_vg);
}

void NvgRenderer::restore() {
  nvgRestore(_vg);
}

void NvgRenderer::reset() {
  nvgReset(_vg);
}

void NvgRenderer::shapeAntiAlias(int enabled) {
  nvgShapeAntiAlias(_vg, enabled);
}

void NvgRenderer::strokeColor(NVGcolor color) {
  nvgStrokeColor(_vg, color);
}

void NvgRenderer::strokePaint(NVGpaint paint) {
  nvgStrokePaint(_vg, paint);
}

void NvgRenderer::fillColor(NVGcolor color) {
  nvgFillColor(_vg, color);
}

void NvgRenderer::fillPaint(NVGpaint paint) {
  nvgFillPaint(_vg, paint);
}

void NvgRenderer::miterLimit(float limit) {
  nvgMiterLimit(_vg, limit);
}

void NvgRenderer::strokeWidth(float size) {
  nvgStrokeWidth(_vg, size);
}

void NvgRenderer::lineCap(int cap) {
  nvgLineCap(_vg, cap);
}

void NvgRenderer::lineJoin(int join) {
  nvgLineJoin(_vg, join);
}

void NvgRenderer::globalAlpha(float alpha) {
  nvgGlobalAlpha(_vg, alpha);
}

void NvgRenderer::resetTransform() {
  nvgResetTransform(_vg);
}

void NvgRenderer::transform(float a, float b, float c, float d, float e,
                            float f) {
  nvgTransform(_vg, a, b, c, d, e, f);
}

void NvgRenderer::translate(float x, float y) {
  nvgTranslate(_vg, x, y);
}

void NvgRenderer::rotate(float angle) {
  nvgRotate(_vg, angle);
}

void NvgRenderer::skewX(float angle) {
  nvgSkewX(_vg, angle);
}

void NvgRenderer::skewY(float angle) {
  nvgSkewY(_vg, angle);
}

void NvgRenderer::scale(float x, float y) {
  nvgScale(_vg, x, y);
}

void NvgRenderer::currentTransform(float* xform) {
  nvgCurrentTransform(_vg, xform);
}

int NvgRenderer::createImage(const char* filename, int imageFlags) {
  return nvgCreateImage(_vg, filename, imageFlags);
}

int NvgRenderer::createImageMem(int imageFlags, unsigned char* data,
                                int ndata) {
  return nvgCreateImageMem(_vg, imageFlags, data, ndata);
}

int NvgRenderer::createImageRGBA(int w, int h, int imageFlags,
                                 const unsigned char* data) {
  return nvgCreateImageRGBA(_vg, w, h, imageFlags, data);
}

void NvgRenderer::updateImage(int image, const unsigned char* data) {
  nvgUpdateImage(_vg, image, data);
}

void NvgRenderer::imageSize(int image, int* w, int* h) {
  nvgImageSize(_vg, image, w, h);
}

void NvgRenderer::deleteImage(int image) {
  nvgDeleteImage(_vg, image);
}

NVGpaint NvgRenderer::linearGradient(float sx, float sy, float ex, float ey,
                                     NVGcolor icol, NVGcolor ocol) {
  return nvgLinearGradient(_vg, sx, sy, ex, ey, icol, ocol);
}

NVGpaint NvgRenderer::boxGradient(float x, float y, float w, float h, float r,
                                  float f, NVGcolor icol, NVGcolor ocol) {
  return nvgBoxGradient(_vg, x, y, w, h, r, f, icol, ocol);
}

NVGpaint NvgRenderer::radialGradient(float cx, float cy, float inr, float outr,
                                     NVGcolor icol, NVGcolor ocol) {
  return nvgRadialGradient(_vg, cx, cy, inr, outr, icol, ocol);
}

NVGpaint NvgRenderer::imagePattern(float ox, float oy, float ex, float ey,
                                   float angle, int image, float alpha) {
  return nvgImagePattern(_vg, ox, oy, ex, ey, angle, image, alpha);
}

void NvgRenderer::scissor(float x, float y, float w, float h) {
  nvgScissor(_vg, x, y, w, h);
}

void NvgRenderer::intersectScissor(float x, float y, float w, float h) {
  nvgIntersectScissor(_vg, x, y, w, h);
}

void NvgRenderer::resetScissor() {
  nvgResetScissor(_vg);
}

void NvgRenderer::beginPath() {
  nvgBeginPath(_vg);
}

void NvgRenderer::moveTo(float x, float y) {
  nvgMoveTo(_vg, x, y);
}

void NvgRenderer::lineTo(float x, float y) {
  nvgLineTo(_vg, x, y);
}

void NvgRenderer::bezierTo(float c1x, float c1y, float c2x, float c2y,
                           float x, float y) {
  nvgBezierTo(_vg, c1x, c1y, c2x, c2y, x, y);
}

void NvgRenderer::quadTo(float cx, float cy, float x, float y) {
  nvgQuadTo(_vg, cx, cy, x, y);
}

void NvgRenderer::arcTo(float x1, float y1, float x2, float y2, float radius) {
  nvgArcTo(_vg, x1, y1, x2, y2, radius);
}

void NvgRenderer::closePath() {
  nvgClosePath(_vg);
}

void NvgRenderer::pathWinding(int dir) {
  nvgPathWinding(_vg, dir);
}

void NvgRenderer::arc(float cx, float cy, float r, float a0, float a1,
                      int dir) {
  nvgArc(_vg, cx, cy, r, a0, a1, dir);
}

void NvgRenderer::rect(float x, float y, float w, float h) {
  nvgRect(_vg, x, y, w, h);
}

void NvgRenderer::roundedRect(float x, float y, float w, float h, float r) {
  nvgRoundedRect(_vg, x, y, w, h, r);
}

void NvgRenderer::roundedRectVarying(float x, float y, float w, float h,
                                     float radTopLeft, float radTopRight,
                                     float radBottomRight,
                                     float radBottomLeft) {
  nvgRoundedRectVarying(_vg, x, y, w, h, radTopLeft, radTopRight,
                        radBottomRight, radBottomLeft);
}

void NvgRenderer::ellipse(float cx, float cy, float rx, float ry) {
  nvgEllipse(_vg, cx, cy, rx, ry);
}

void NvgRenderer::circle(float cx, float cy, float r) {
  nvgCircle(_vg, cx, cy, r);
}

void NvgRenderer::fill() {
  nvgFill(_vg);
}

void NvgRenderer::stroke() {
  nvgStroke(_vg);
}

int NvgRenderer::createFont(const char* name, const char* filename) {
  return nvgCreateFont(_vg, name, filename);
}

int NvgRenderer::createFontMem(const char* name, unsigned char* data,
                               int ndata, int freeData) {
  return nvgCreateFontMem(_vg, name, data, ndata, freeData);
}

int NvgRenderer::findFont(const char* name) {
  return nvgFindFont(_vg, name);
}

int NvgRenderer::addFallbackFontId(int baseFont, int fallbackFont) {
  return nvgAddFallbackFontId(_vg, baseFont, fallbackFont);
}

int NvgRenderer::addFallbackFont(const char* baseFont,
                                 const char* fallbackFont) {
  return nvgAddFallbackFont(_vg, baseFont, fallbackFont);
}

void NvgRenderer::fontSize(float size) {
  nvgFontSize(_vg, size);
}

void NvgRenderer::fontBlur(float blur) {
  nvgFontBlur(_vg, blur);
}

void NvgRenderer::textLetterSpacing(float spacing) {
  nvgTextLetterSpacing(_vg, spacing);
}

void NvgRenderer::textLineHeight(float lineHeight) {
  nvgTextLineHeight(_vg, lineHeight);
}

void NvgRenderer::textAlign(int align) {
  nvgTextAlign(_vg, align);
}

void NvgRenderer::fontFaceId(int font) {
  nvgFontFaceId(_vg, font);
}

void NvgRenderer::fontFace(const char* font) {
  nvgFontFace(_vg, font);
}

float NvgRenderer::text(float x, float y,
                        const char* string, const char* end) {
  return nvgText(_vg, x, y, string, end);
}

void NvgRenderer::textBox(float x, float y, float breakRowWidth,
                          const char* string, const char* end) {
  nvgTextBox(_vg, x, y, breakRowWidth, string, end);
}

float NvgRenderer::textBounds(float x, float y, const char* string,
                              const char* end, float* bounds) {
  return nvgTextBounds(_vg, x, y, string, end, bounds);
}

void NvgRenderer::textBoxBounds(float x, float y, float breakRowWidth,
                                const char* string, const char* end,
                                float* bounds) {
  nvgTextBoxBounds(_vg, x, y, breakRowWidth, string, end, bounds);
}

int NvgRenderer::textGlyphPositions(float x, float y,
                                    const char* string, const char* end,
                                    NVGglyphPosition* positions,
                                    int maxPositions) {
  return nvgTextGlyphPositions(_vg, x, y, string, end, positions, maxPositions);
}

void NvgRenderer::textMetrics(float* ascender, float* descender, float* lineh) {
  nvgTextMetrics(_vg, ascender, descender, lineh);
}

int NvgRenderer::textBreakLines(const char* string, const char* end,
                                float breakRowWidth, NVGtextRow* rows,
                                int maxRows) {
  return nvgTextBreakLines(_vg, string, end, breakRowWidth, rows, maxRows);
}

void NvgRenderer::debugDumpPathCache() {
  nvgDebugDumpPathCache(_vg);
}
