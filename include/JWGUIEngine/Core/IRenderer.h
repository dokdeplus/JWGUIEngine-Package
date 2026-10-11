#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Export.h"
#include "CoreTypes.h"

namespace jwgui {

enum class ImageInterpolation {
    Nearest,
    Linear,
    Cubic
};

class UIPath;

enum class StrokeCap {
    Butt,    // ends at the end point
    Round,   // half circle past the end point
    Square   // half the width past the end point
};

// Stroke for lines, arcs and paths. Width in px.
struct StrokeStyle {
    float width = 1.0f;
    StrokeCap cap = StrokeCap::Butt;
    bool roundJoin = false;

    StrokeStyle() = default;
    StrokeStyle(float w, StrokeCap c = StrokeCap::Butt, bool round = false) : width(w), cap(c), roundJoin(round) {}
};

class JWGUI_API IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void SetInterpolationMode(ImageInterpolation mode);

    // Initialize renderer with a target platform window handle (e.g. HWND)
    virtual bool Initialize(void* windowHandle) = 0;
    
    // Cleanup resources
    virtual void Cleanup() = 0;

    // Call before any drawing operations
    virtual void BeginDraw() = 0;
    
    // Call after drawing operations to present to window
    virtual void EndDraw(void* systemContext = nullptr) = 0;

    // Optional dirty rect in layout units. Clear/EndDraw/cull use it when set.
    virtual void SetPresentBounds(float x, float y, float w, float h);
    virtual void ClearPresentBounds();
    virtual bool HasPresentBounds() const;
    virtual bool PresentBoundsIntersect(float x, float y, float w, float h) const;

    // Call when the target window resizes. Device pixels.
    virtual void OnResize(int width, int height) = 0;

    // Device pixels per layout unit (Windows: DPI / 96, browser:
    // devicePixelRatio). Drawing calls take layout units and land on the
    // device pixels, so text and shapes stay sharp at 150 %; only the surface
    // size (OnResize) is in device pixels. Set before BeginDraw.
    virtual void SetScale(float scale);
    virtual float GetScale() const;

    // GPU drawing. Ask before Initialize; the renderer falls back to the CPU
    // when the GPU cannot be used. IsAccelerated tells which one is in use:
    // on the GPU, drawing scaled images is cheap, so views may redraw every
    // frame of a zoom instead of caching.
    virtual void SetAccelerationPreferred(bool prefer);
    virtual bool IsAccelerated() const;

    // Clipping
    virtual void PushClipRect(float x, float y, float w, float h) = 0;
    virtual void PushClipPath(const UIPath& path) = 0;
    virtual void PopClip() = 0;

    // Transform
    virtual void PushTransform(float pivotX, float pivotY, float scaleX, float scaleY, float translateX, float translateY) = 0;
    virtual void PopTransform() = 0;

    // Opacity
    virtual void PushOpacity(float opacity, float boundsX, float boundsY, float boundsW, float boundsH) = 0;
    virtual void PopOpacity() = 0;

    // Basic drawing functions
    // color format is ARGB: 0xAARRGGBB
    virtual void Clear(uint32_t color) = 0;
    virtual void DrawRect(float x, float y, float width, float height, uint32_t color) = 0;
    // Draw an outline rectangle. strokeWidth is in px.
    virtual void DrawRectOutline(float x, float y, float width, float height,
                                 uint32_t color, float strokeWidth = 1.0f) = 0;
    virtual void DrawRectOutlineDashed(float x, float y, float width, float height,
                                       uint32_t color, float strokeWidth, const float* intervals, int count) = 0;
    virtual void DrawPoint(float x, float y, uint32_t color) = 0;
    virtual void DrawLine(float x1, float y1, float x2, float y2, uint32_t color) = 0;
    virtual void DrawCircle(float cx, float cy, float radius, uint32_t color) = 0;
    virtual void DrawCircleOutline(float cx, float cy, float radius, uint32_t color, float strokeWidth = 1.0f) = 0;
    virtual void DrawRoundRect(float x, float y, float width, float height, float radius, uint32_t color) = 0;
    
    // Gradient drawing functions
    virtual void DrawRectGradient(float x, float y, float width, float height, const uint32_t* colors, size_t colorCount, float angleDegrees) = 0;
    virtual void DrawRoundRectGradient(float x, float y, float width, float height, float radius, const uint32_t* colors, size_t colorCount, float angleDegrees) = 0;

    virtual void DrawRoundRectOutline(float x, float y, float width, float height, float radius, uint32_t color, float strokeWidth = 1.0f) = 0;

    virtual void DrawRRect(float x, float y, float width, float height, const CornerRadius& radius, uint32_t color) = 0;
    virtual void DrawRRectGradient(float x, float y, float width, float height, const CornerRadius& radius, const uint32_t* colors, size_t colorCount, float angleDegrees) = 0;
    virtual void DrawRRectOutline(float x, float y, float width, float height, const CornerRadius& radius, uint32_t color, float strokeWidth = 1.0f) = 0;
    virtual void DrawRRectShadow(float x, float y, float width, float height, const CornerRadius& radius,
                                 float offsetX, float offsetY, float blur, float spread, uint32_t color) = 0;
    virtual void DrawRhombus(float x, float y, float width, float height, uint32_t color) = 0;
    virtual void DrawRhombusOutline(float x, float y, float width, float height, uint32_t color, float strokeWidth = 1.0f) = 0;
    virtual void DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, uint32_t color) = 0;
    virtual void DrawPath(const UIPath& path, uint32_t color) = 0;
    virtual void DrawPathOutline(const UIPath& path, uint32_t color, float strokeWidth) = 0;

    // Dials, gauges and round faces. Angles in degrees: 0 = +x (3 o'clock),
    // positive = clockwise on screen. The defaults fall back to the plain
    // calls above, so a renderer without them still draws something.
    virtual void DrawLineStyled(float x1, float y1, float x2, float y2, uint32_t color, const StrokeStyle& stroke);
    virtual void DrawPathStroke(const UIPath& path, uint32_t color, const StrokeStyle& stroke);
    virtual void DrawArc(float cx, float cy, float radius, float startDeg, float sweepDeg,
                         uint32_t color, const StrokeStyle& stroke);
    // An arc whose color runs along it: colors[i] at stops[i] (0 = start,
    // 1 = end of the sweep); stops may be null for even spacing.
    virtual void DrawArcGradient(float cx, float cy, float radius, float startDeg, float sweepDeg,
                                 const uint32_t* colors, const float* stops, size_t count, const StrokeStyle& stroke);
    // Filled circle shaded from the center (stop 0) to the edge (stop 1).
    virtual void DrawCircleRadialGradient(float cx, float cy, float radius,
                                          const uint32_t* colors, const float* stops, size_t count);
    // Soft light around a circle (lit lamps); blur in px.
    virtual void DrawCircleGlow(float cx, float cy, float radius, uint32_t color, float blur);
    // Rotates what is drawn until PopTransform, about (pivotX, pivotY).
    virtual void PushRotate(float degrees, float pivotX, float pivotY);

    enum class TextAlignH { Left, Center, Right };
    enum class TextAlignV { Top, Middle, Bottom };

    // Text configuration
    virtual void SetFont(const char* fontFamily, float size) = 0;
    // The font families this renderer can draw with, sorted: what a font
    // picker offers. Empty when the renderer cannot tell.
    virtual std::vector<std::string> FontFamilies() const { return {}; }
    
    // outWidth and outHeight will receive the bounding box size of the text.
    virtual void MeasureText(const char* text, float& outWidth, float& outHeight) = 0;
    
    // Draw text with (x, y) representing the TOP-LEFT coordinate of the text.
    virtual void DrawTextStr(const char* text, float x, float y, uint32_t color) = 0;
    
    // Draw text inside a given rectangle with specified alignment.
    virtual void DrawTextInRect(const char* text, float x, float y, float w, float h, 
                                uint32_t color, TextAlignH hAlign, TextAlignV vAlign) = 0;
                                
    virtual void DrawTextRotated(const char* text, float x, float y, float angleDegrees, uint32_t color) {
        (void)text; (void)x; (void)y; (void)angleDegrees; (void)color;
    }

    // Image handling
    typedef void* ImageHandle;
    // Decoded when first drawn (on the UI thread).
    virtual ImageHandle LoadImageFromFile(const char* filePath) = 0;
    virtual ImageHandle LoadImageFromMemory(const void* data, size_t size) = 0;
    // maxEdge > 0 decodes now, downsampled so the longer edge is at most
    // maxEdge (thumbnails); kDecodeNow decodes now at the native size; 0 keeps
    // the native size and decodes when first drawn. The decoding forms may be
    // called from a worker thread (Application::RunAsync), so a large picture
    // does not hold up the UI: they use nothing of the renderer's state.
    static constexpr int kDecodeNow = -1;
    virtual ImageHandle LoadImageFromMemory(const void* data, size_t size, int maxEdge);
    virtual ImageHandle CreateResampledImage(ImageHandle source,
                                             int targetWidth,
                                             int targetHeight,
                                             ImageInterpolation interpolation);
    // An image over pixels the caller keeps: 0xAARRGGBB, premultiplied, rows
    // of `width` pixels (a Bitmap's). Nothing is copied, so the pixels must
    // stay where they are until FreeImage. After changing them make a new
    // image (it costs nothing): a drawn image may be remembered by what it
    // showed. Null where not supported.
    virtual ImageHandle WrapPixels(const void* pixels, int width, int height);
    virtual void GetImageSize(ImageHandle handle, float& outWidth, float& outHeight) = 0;
    virtual void FreeImage(ImageHandle handle) = 0;
    virtual void DrawImage(ImageHandle handle, float x, float y, float width, float height) = 0;
};
} // namespace jwgui
