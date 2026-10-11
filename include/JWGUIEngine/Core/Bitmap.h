#pragma once

#include "Export.h"
#include "IRenderer.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace jwgui {

// A whole-pixel rectangle (bitmap coordinates).
struct IntRect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    IntRect() = default;
    IntRect(int left, int top, int w, int h) : x(left), y(top), width(w), height(h) {}

    bool IsEmpty() const { return width <= 0 || height <= 0; }
    int Right() const { return x + width; }
    int Bottom() const { return y + height; }
    bool Contains(int px, int py) const { return px >= x && py >= y && px < Right() && py < Bottom(); }
    // The part both cover (empty when they do not meet).
    IntRect Intersect(const IntRect& other) const {
        const int left = x > other.x ? x : other.x;
        const int top = y > other.y ? y : other.y;
        const int right = Right() < other.Right() ? Right() : other.Right();
        const int bottom = Bottom() < other.Bottom() ? Bottom() : other.Bottom();
        return right > left && bottom > top ? IntRect(left, top, right - left, bottom - top) : IntRect();
    }
    // The smallest rectangle around both; an empty one does not count.
    IntRect Union(const IntRect& other) const {
        if (IsEmpty()) return other;
        if (other.IsEmpty()) return *this;
        const int left = x < other.x ? x : other.x;
        const int top = y < other.y ? y : other.y;
        const int right = Right() > other.Right() ? Right() : other.Right();
        const int bottom = Bottom() > other.Bottom() ? Bottom() : other.Bottom();
        return IntRect(left, top, right - left, bottom - top);
    }
};

// How one picture goes over another (Bitmap::Blend).
enum class BlendMode {
    Normal,      // over what is there
    Multiply,
    Screen,
    Overlay,
    Darken,
    Lighten,
    Add,
    Difference,
    Erase,       // takes away from what is there by the source's alpha
    Replace      // instead of what is there (alpha too)
};

enum class ImageFileFormat { Png, Jpeg };

// Pixels in memory, for programs that work on pictures themselves (a paint
// program's layers, a generated picture): read and write them, draw into
// them with an IRenderer (CreateBitmapRenderer), put one over another, read
// and write PNG / JPEG.
//
// One pixel is 32 bits, 0xAARRGGBB, with the color already multiplied by the
// alpha (premultiplied: half-clear red is 0x80800000); rows run top to
// bottom. GetColor / SetColor take and give plain (not premultiplied) ARGB.
// Not tied to a window; Decode, Encode and Blend may run on a worker thread
// (each bitmap from one thread at a time).
class JWGUI_API Bitmap {
public:
    Bitmap() = default;
    // All clear (transparent).
    Bitmap(int width, int height);

    bool IsEmpty() const { return m_width <= 0 || m_height <= 0; }
    int Width() const { return m_width; }
    int Height() const { return m_height; }
    IntRect Bounds() const { return IntRect(0, 0, m_width, m_height); }
    uint32_t* Pixels() { return m_pixels.data(); }
    const uint32_t* Pixels() const { return m_pixels.data(); }
    uint32_t* Row(int y) { return m_pixels.data() + static_cast<size_t>(y) * static_cast<size_t>(m_width); }
    const uint32_t* Row(int y) const { return m_pixels.data() + static_cast<size_t>(y) * static_cast<size_t>(m_width); }
    // Counts up with every change made through this class, so a view can
    // tell that its picture of the bitmap is old. Code that writes through
    // Pixels() / Row() or a bitmap renderer calls Touch.
    unsigned int Version() const { return m_version; }
    void Touch() { ++m_version; }

    // A new size; the pixels start clear.
    void Resize(int width, int height);
    // Every pixel (or those of `rect`) to one premultiplied value.
    void Clear(uint32_t premultiplied = 0);
    void Clear(const IntRect& rect, uint32_t premultiplied = 0);

    // Plain ARGB; 0 outside the bitmap.
    uint32_t GetColor(int x, int y) const;
    void SetColor(int x, int y, uint32_t argb);
    static uint32_t Premultiply(uint32_t argb);
    static uint32_t Unpremultiply(uint32_t premultiplied);

    // A copy of a part (clipped to the bitmap).
    Bitmap Copy(const IntRect& rect) const;
    // Puts `source` over this one with its top-left at (x, y): opacity 0..1
    // and a blend mode; `clip` limits what may change.
    void Blend(const Bitmap& source, int x, int y, float opacity = 1.0f, BlendMode mode = BlendMode::Normal,
               const IntRect* clip = nullptr);

    // PNG, JPEG, BMP, GIF (first frame), WebP; a photo's EXIF orientation is
    // applied. False (and the bitmap as it was) when the data is not a picture.
    bool Decode(const void* data, size_t size);
    bool Load(const std::string& path);
    // quality: JPEG, 1..100. JPEG has no alpha: clear areas come out black,
    // so put the picture over a background first.
    bool Encode(ImageFileFormat format, std::vector<uint8_t>& out, int quality = 90) const;
    bool Save(const std::string& path, ImageFileFormat format, int quality = 90) const;

private:
    int m_width = 0;
    int m_height = 0;
    unsigned int m_version = 0;
    std::vector<uint32_t> m_pixels;
};

// A renderer that draws into the bitmap instead of a window: the same calls
// as on screen (shapes, paths, text, images), in the bitmap's pixels (no UI
// scale). Call BeginDraw before drawing and EndDraw after. The bitmap must
// outlive the renderer and keep its size while it is used; call
// Bitmap::Touch after drawing. Null where the engine has no such renderer.
JWGUI_API std::shared_ptr<IRenderer> CreateBitmapRenderer(Bitmap& bitmap);

} // namespace jwgui
